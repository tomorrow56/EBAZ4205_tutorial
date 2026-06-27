/*
 * lcd_ext_ps: SPI LCD test application
 *   ps7_uart    115200
 */

#include <stdio.h>

#include "xparameters.h"
#include "xgpiops.h"
#include "xspips.h"  /* PS SPIドライバ */
#include "xstatus.h"

#include <xil_printf.h>
#include "sleep.h"
#include "lv_conf.h"
#include "lvgl.h"      /* LVGL メインヘッダ */

#define WHITE	0xFFFF
#define BLACK	0x0000
#define BLUE	0x001F
#define BRED	0XF81F
#define GRED	0XFFE0
#define GBLUE	0X07FF
#define RED		0xF800
#define MAGENTA	0xF81F
#define GREEN	0x07E0
#define CYAN	0x7FFF
#define YELLOW	0xFFE0
#define BROWN	0XBC40
#define BRRED	0XFC07
#define GRAY	0X8430

/* GPIOピン定義 (EMIO) */
/* SCL, SDAはPS SPIが制御 */
#define EMIO_LCD_CS		54
#define EMIO_LCD_CD		55
#define EMIO_LCD_RES	56

#define GPIO_DEVICE_ID	XPAR_XGPIOPS_0_DEVICE_ID
#define SPI_DEVICE_ID   XPAR_XSPIPS_0_DEVICE_ID

XGpioPs Gpio;  /* Diver instance for GPIO */
XSpiPs  Spi;   /* Diver instance for SPI */

/* GPIO出力操作マクロ */
#define LCD_CS_HIGH XGpioPs_WritePin(&Gpio, EMIO_LCD_CS, 1)
#define LCD_CS_LOW  XGpioPs_WritePin(&Gpio, EMIO_LCD_CS, 0)

#define LCD_CD_HIGH XGpioPs_WritePin(&Gpio, EMIO_LCD_CD, 1)
#define LCD_CD_LOW  XGpioPs_WritePin(&Gpio, EMIO_LCD_CD, 0)

#define LCD_RES_HIGH XGpioPs_WritePin(&Gpio, EMIO_LCD_RES, 1)
#define LCD_RES_LOW  XGpioPs_WritePin(&Gpio, EMIO_LCD_RES, 0)

/* LCD Panel Pixels */
#define LCD_SIZE_X 240
#define LCD_SIZE_Y 240

static lv_obj_t * demo_bar;
static lv_obj_t * demo_val_label;
static lv_obj_t * demo_box;
static int32_t   demo_val = 0;
static int32_t   demo_dir = 1;

static uint8_t lv_buf1[LCD_SIZE_X * 10 * sizeof(lv_color_t)];

static void lv_demo_timer_cb(lv_timer_t * timer);
static void lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/* 遅延関数 */
static void delayMilliseconds(u32 milliseconds){
	usleep(milliseconds * 1000U);
}

/* ハードウェアSPI初期化 */
int Lcd_Spi_Init(void){
	XSpiPs_Config *SpiConfig;
	int Status;

	SpiConfig = XSpiPs_LookupConfig(SPI_DEVICE_ID);
	Status = XSpiPs_CfgInitialize(&Spi, SpiConfig, SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) return XST_FAILURE;

	// マスターモード、手動CS制御、クロック位相/極性設定(Mode 3)
	XSpiPs_SetOptions(&Spi, XSPIPS_MASTER_OPTION |
	                       XSPIPS_CLK_ACTIVE_LOW_OPTION | // CPOL=1
	                       XSPIPS_CLK_PHASE_1_OPTION |    // CPHA=1
	                       XSPIPS_FORCE_SSELECT_OPTION);  // 念のため追加

	// クロック分周比の設定 (CPU_1x / 4)
	// LCDの最大周波数に合わせて調整
	XSpiPs_SetClkPrescaler(&Spi, XSPIPS_CLK_PRESCALE_4);

	// SPI有効化
	XSpiPs_Enable(&Spi);

	return XST_SUCCESS;
}

/* SPI送信関数 */
static void spi_send_hw(u8 dat){
	u8 buffer = dat;
	// CSはGPIO制御のため、ここではデータ転送のみ
	XSpiPs_PolledTransfer(&Spi, &buffer, NULL, 1);
}

/* GPIO初期化 (CD, RES, CS) */
void Lcd_Gpio_Init(void){
	XGpioPs_Config *ConfigPtr;
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr);

	XGpioPs_SetDirectionPin(&Gpio, EMIO_LCD_CS, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, EMIO_LCD_CS, 1);
	XGpioPs_WritePin(&Gpio, EMIO_LCD_CS, 1);

	XGpioPs_SetDirectionPin(&Gpio, EMIO_LCD_CD, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, EMIO_LCD_CD, 1);
	XGpioPs_WritePin(&Gpio, EMIO_LCD_CD, 1);

	XGpioPs_SetDirectionPin(&Gpio, EMIO_LCD_RES, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, EMIO_LCD_RES, 1);
	XGpioPs_WritePin(&Gpio, EMIO_LCD_RES, 1);
}

/* Hardware Reset */
static void lcdHardwareReset(void){
	LCD_RES_HIGH;
	delayMilliseconds(1U);
	LCD_RES_LOW;
	delayMilliseconds(1U);
	LCD_RES_HIGH;
	delayMilliseconds(120U);
}

/* write command */
void LCD_WR_REG(u8 dat){
	LCD_CD_LOW;
	spi_send_hw(dat);
}

/* Write data(8bit) */
void LCD_WR_DATA8(u8 dat){
	LCD_CD_HIGH;
	spi_send_hw(dat);
}

/* Write data(16bit) */
void LCD_WR_DATA(u16 dat){
	u8 buffer[2];
	buffer[0] = (u8)(dat >> 8);
	buffer[1] = (u8)(dat & 0xFF);

	LCD_CD_HIGH;
	// 16ビット(2バイト)を送信
	XSpiPs_PolledTransfer(&Spi, buffer, NULL, 2);
}


void Lcd_Init(void){
	lcdHardwareReset();
	LCD_CS_LOW;

	LCD_WR_REG(0x36);   // Memory Data Access Control
	LCD_WR_DATA8(0x00);
	LCD_WR_REG(0x3A);   // Interface Pixel Format
	LCD_WR_DATA8(0x55); //  65K of RGB interface, 16bit/pixel
	LCD_WR_REG(0xB2);   // Porch Setting
	LCD_WR_DATA8(0x0C); //  BPA[6:0]: Back porch setting in normal mode
	LCD_WR_DATA8(0x0C); //  FPA[6:0]: Front porch setting in normal mode
	LCD_WR_DATA8(0x00); //  PSEN: Enable separate porch control
	LCD_WR_DATA8(0x33); //  BPB[3:0],FPB[3:0]: Back and Front porch setting in idle mode
	LCD_WR_DATA8(0x33); //  BPC[3:0],FPC[3:0]: Back and Front porch setting in partial mode
	LCD_WR_REG(0xB7);   // Gate Control
	LCD_WR_DATA8(0x35); //  0,VGHS[2:0],0,VGLS[2:0]
	LCD_WR_REG(0xBB);   // VCOM Setting
	LCD_WR_DATA8(0x19); //  VCOMS[5:0]:0.725V
	LCD_WR_REG(0xC0);   // LCM Control
	LCD_WR_DATA8(0x2C);
	LCD_WR_REG(0xC2);   // VDV and VRH Command Enable
	LCD_WR_DATA8(0x01); // CMDEN: VDV and VRH command write enable
	LCD_WR_DATA8(0xFF); // CMDEN: VDV and VRH command write enable
	LCD_WR_REG(0xC3);   // VRH Set
	LCD_WR_DATA8(0x12); //  4.45+( vcom+vcom offset+vdv)
	LCD_WR_REG(0xC4);   // VDV Set
	LCD_WR_DATA8(0x20); //  default
	LCD_WR_REG(0xC6);   // Frame Rate Control in Normal Mode
	LCD_WR_DATA8(0x0F); //  60Hz
	LCD_WR_REG(0xD0);   // Power Control 1
	LCD_WR_DATA8(0xA4); //  default
	LCD_WR_DATA8(0xA1); //  default
	LCD_WR_REG(0xE0);   // Positive Voltage Gamma Control
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);
	LCD_WR_REG(0xE1);   // Negative Voltage Gamma Control
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);
	LCD_WR_REG(0x21);  // Display Inversion On
	LCD_WR_REG(0x11);  // Sleep Out

	delayMilliseconds(120U);
	LCD_WR_REG(0x29);  // Display ON
	delayMilliseconds(20U);

	LCD_CS_HIGH;
}

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2){
	LCD_CS_LOW;

	LCD_WR_REG(0x2a);    // Column Address Set
	LCD_WR_DATA8(x1>>8); //  XStart[15:8]
	LCD_WR_DATA8(x1);    //  XStart[7:0]
	LCD_WR_DATA8(x2>>8); //  XEnd[15:8]
	LCD_WR_DATA8(x2);    //  XEnd[7:0]
	LCD_WR_REG(0x2b);    // Row Address Set
	LCD_WR_DATA8(y1>>8); //  YStart[15:8]
	LCD_WR_DATA8(y1);    //  YStart[7:0]
	LCD_WR_DATA8(y2>>8); //  YEnd[15:8]
	LCD_WR_DATA8(y2);    //  YEnd[7:0]
	LCD_WR_REG(0x2C);    // Memory Write

	LCD_CS_HIGH;
}

static void lv_demo_setup(void);
static void lv_init_display(void);

/* --- メイン関数 --- */
int main(void){
	print("EBAZ4205 LCD Test with PS SPI\n\r");

	Lcd_Gpio_Init();

	if(Lcd_Spi_Init() != XST_SUCCESS){
		print("SPI Initialization Failed!\n\r");
		return XST_FAILURE;
	}

	Lcd_Init();

	/* LVGL ライブラリの初期化 */
	lv_init();
	lv_init_display();
	lv_demo_setup();

	printf("LVGL initialized successfully!\n\r");

	while(1) {
	    lv_tick_inc(5);
	    lv_timer_handler();
	    usleep(5000);
	}

	return XST_SUCCESS;
}

static void lv_init_display(void){
	lv_display_t * disp = lv_display_create(LCD_SIZE_X, LCD_SIZE_Y);
	lv_display_set_flush_cb(disp, lvgl_flush_cb);
	lv_display_set_buffers(disp, lv_buf1, NULL, sizeof(lv_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);
}

static void lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    const int width  = area->x2 - area->x1 + 1;
    uint8_t * row    = px_map;
    for(int yy = area->y1; yy <= area->y2; ++yy) {
        Address_set(area->x1, yy, area->x2, yy);
        LCD_CS_LOW;
        LCD_CD_HIGH;
        XSpiPs_PolledTransfer(&Spi, row, NULL, width * 2);
        LCD_CS_HIGH;
        row += width * 2;
    }
    lv_display_flush_ready(disp);
}

static void lv_demo_setup(void){
	/* ---- 背景 ---- */
	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0x10, 0x18, 0x40), 0);
	lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

	/* ---- タイトル ---- */
	lv_obj_t * title = lv_label_create(lv_scr_act());
	lv_label_set_text(title, "LVGL v9 Demo");
	lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
	lv_obj_set_style_text_color(title, lv_color_make(0xff, 0x60, 0x60), 0);
	lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

	/* ---- 中央カラーボックス ---- */
	demo_box = lv_obj_create(lv_scr_act());
	lv_obj_set_size(demo_box, 130, 130);
	lv_obj_center(demo_box);
	lv_obj_set_style_bg_color(demo_box, lv_color_make(0x00, 0xaa, 0xff), 0);
	lv_obj_set_style_bg_opa(demo_box, LV_OPA_COVER, 0);
	lv_obj_set_style_border_width(demo_box, 0, 0);
	lv_obj_set_style_radius(demo_box, 16, 0);
	lv_obj_remove_flag(demo_box, LV_OBJ_FLAG_SCROLLABLE);

	/* ---- 数値ラベル ---- */
	demo_val_label = lv_label_create(lv_scr_act());
	lv_label_set_text(demo_val_label, "0");
	lv_obj_set_style_text_font(demo_val_label, &lv_font_montserrat_28, 0);
	lv_obj_set_style_text_color(demo_val_label, lv_color_make(0xff, 0xff, 0xff), 0);
	lv_obj_align(demo_val_label, LV_ALIGN_CENTER, 0, 0);

	/* ---- バー (下部) ---- */
	demo_bar = lv_bar_create(lv_scr_act());
	lv_obj_set_size(demo_bar, 200, 14);
	lv_obj_align(demo_bar, LV_ALIGN_BOTTOM_MID, 0, -20);
	lv_bar_set_range(demo_bar, 0, 100);
	lv_bar_set_value(demo_bar, 0, LV_ANIM_OFF);
	lv_obj_set_style_bg_color(demo_bar, lv_color_make(0x30, 0x30, 0x60), LV_PART_MAIN);
	lv_obj_set_style_bg_color(demo_bar, lv_color_make(0x00, 0xff, 0x80), LV_PART_INDICATOR);

	/* ---- タイマー: 50ms ---- */
	lv_timer_create(lv_demo_timer_cb, 50, NULL);
}

static void lv_demo_timer_cb(lv_timer_t * timer){
	LV_UNUSED(timer);

	/* 0→100→0 往復 */
	demo_val += demo_dir;
	if(demo_val >= 100) { demo_val = 100; demo_dir = -1; }
	if(demo_val <= 0)   { demo_val = 0;   demo_dir =  1; }

	/* バー更新 */
	lv_bar_set_value(demo_bar, demo_val, LV_ANIM_OFF);

	/* ボックス色: 青→緑→赤 */
	uint8_t r = (uint8_t)(demo_val * 2 > 255 ? 255 : demo_val * 2);
	uint8_t g = (uint8_t)((100 - demo_val) * 2 > 255 ? 255 : (100 - demo_val) * 2);
	uint8_t b = (uint8_t)(200 - demo_val);
	lv_obj_set_style_bg_color(demo_box, lv_color_make(r, g, b), 0);

	/* 数値ラベル */
	char buf[8];
	snprintf(buf, sizeof(buf), "%ld", (long)demo_val);
	lv_label_set_text(demo_val_label, buf);
	lv_obj_align(demo_val_label, LV_ALIGN_CENTER, 0, 0);
}
