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

/**********
 *  Frame Buffer: 2次元配列として定義 [Y, X] 2バイト分)
 *  ST7789は1ピクセル2バイトなのでu16型
 **********/
static u16 frame_buffer[LCD_SIZE_Y][LCD_SIZE_X];

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

/* Color Bar */
void LCD_ColorBar_2D(void) {
    u16 colors[8] = {
    	BLACK,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE
    };

    int x, y, c;
    int bar_width = LCD_SIZE_X / 8;

    /* 2次元配列への書き込み[Y, X] */
    for (y = 0; y < LCD_SIZE_Y; y++) {
        for (c = 0; c < 8; c++) {
            for (x = 0; x < bar_width; x++) {
                u16 color = colors[c];
                // エンディアン変換(上位・下位バイトの入れ替え), ST7789はビッグエンディアン
                frame_buffer[y][c * bar_width + x] = (color << 8) | (color >> 8);
            }
        }
    }

    /* 描画範囲設定 */
	Address_set(0,0,LCD_SIZE_Y-1,LCD_SIZE_X-1);

    LCD_CD_HIGH; // Write Data

    /* バルク転送 */
    LCD_CS_LOW;
    XSpiPs_PolledTransfer(&Spi, (u8*)frame_buffer, NULL, LCD_SIZE_Y * LCD_SIZE_X * 2);
    LCD_CS_HIGH;
}

/* Fill Screen */
void LCD_FillScreen(unsigned char clr){
    u16 colors[8] = {
    	BLACK,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE
    };

    unsigned int x,y;

    /* 描画範囲設定 */
	Address_set(0,0,LCD_SIZE_Y-1,LCD_SIZE_X-1);

    /* 2次元配列への書き込み[Y, X] */
    for (y = 0; y < LCD_SIZE_Y; y++) {
        for (x = 0; x < LCD_SIZE_X; x++) {
             // エンディアン変換(上位・下位バイトの入れ替え), ST7789はビッグエンディアン
            frame_buffer[y][x] = (colors[clr] << 8) | (colors[clr] >> 8);
        }
    }

    LCD_CD_HIGH; // Write Data

	/* バルク転送 */
    LCD_CS_LOW;
    XSpiPs_PolledTransfer(&Spi, (u8*)frame_buffer, NULL, LCD_SIZE_Y * LCD_SIZE_X * 2);
    LCD_CS_HIGH;
}

/* --- メイン関数 --- */
int main(void){
	unsigned int clr = 0;

	print("EBAZ4205 LCD Test with PS SPI\n\r");

	Lcd_Gpio_Init();

	if(Lcd_Spi_Init() != XST_SUCCESS){
		print("SPI Initialization Failed!\n\r");
		return XST_FAILURE;
	}

	Lcd_Init();

	while(1){
		LCD_FillScreen(0);
		print("COLOR BAR\n\r");
		delayMilliseconds(500U);
		LCD_ColorBar_2D();
		delayMilliseconds(1000U);
		for(clr=0;clr<8;clr++){
			xil_printf("Fill Screen Color=%d\n\r", clr);
			LCD_FillScreen(clr);
			delayMilliseconds(500U);
		}
	}
	return XST_SUCCESS;
}
