/**
 * @file hdmi_axi.h
 * @brief HDMI AXI Pattern Generator Driver Header
 *
 * This driver provides an interface to control the HDMI pattern generator
 * IP through AXI4-Lite interface on Xilinx Zynq platforms.
 *
 * Register Map:
 *   0x00: Control Register (R/W)
 *         [1:0] - timing_sel (0=VGA 640x480, 1=480p 720x480, 2=720p 1280x720)
 *         [5:2] - pattern_sel (0..9: see HdmiaxiPatternMode)
 *   0x04: Status Register (R/O)
 *         [0]   - locked (MMCM lock status)
 *         [1]   - reconfig_busy (1 while MMCM reconfiguration in progress)
 *
 * Hardware:
 *   Base Address: 0x43C00000
 */

#ifndef HDMI_AXI_H_
#define HDMI_AXI_H_

#include <stdint.h>
#include "xil_types.h"
#include "xil_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Register offsets */
#define HDMI_AXI_REG_CTRL_OFFSET    0x00
#define HDMI_AXI_REG_STATUS_OFFSET  0x04

/* Control register bit definitions */
#define HDMI_AXI_CTRL_TIMING_SEL_MASK    0x03
#define HDMI_AXI_CTRL_TIMING_SEL_SHIFT   0
#define HDMI_AXI_CTRL_PATTERN_SEL_MASK   0x3C
#define HDMI_AXI_CTRL_PATTERN_SEL_SHIFT  2

/* Status register bit definitions */
#define HDMI_AXI_STATUS_LOCKED_MASK         0x01
#define HDMI_AXI_STATUS_RECONFIG_BUSY_MASK  0x02

/* Timing presets */
typedef enum {
    HDMI_AXI_TIMING_VGA  = 0,  /**< VGA 640x480@60Hz */
    HDMI_AXI_TIMING_480P = 1,  /**< 480p 720x480@60Hz */
    HDMI_AXI_TIMING_720P = 2,  /**< 720p 1280x720@60Hz */
    HDMI_AXI_TIMING_NUM  = 3   /**< Number of timing modes */
} HdmiaxiTimingMode;

/* Pattern modes */
typedef enum {
    HDMI_AXI_PATTERN_COLOR_BARS  = 0,  /**< 8 vertical color bars */
    HDMI_AXI_PATTERN_CHECKERBOARD = 1, /**< 32x32 checkerboard */
    HDMI_AXI_PATTERN_HGRADATION  = 2,  /**< Horizontal gradation */
    HDMI_AXI_PATTERN_CROSSHATCH  = 3,  /**< 64px grid crosshatch */
    HDMI_AXI_PATTERN_BORDER      = 4,  /**< 1px white border */
    HDMI_AXI_PATTERN_RGB_FIELD   = 5,  /**< R/G/B/W full field cycling */
    HDMI_AXI_PATTERN_VGRADATION  = 6,  /**< Vertical gradation */
    HDMI_AXI_PATTERN_WALKING_BAR = 7,  /**< Animated walking bar */
    HDMI_AXI_PATTERN_SMPTE       = 8,  /**< SMPTE color bars + PLUGE */
    HDMI_AXI_PATTERN_ZONE_PLATE  = 9,  /**< Concentric circle zone plate */
    HDMI_AXI_PATTERN_NUM         = 10  /**< Number of pattern modes */
} HdmiaxiPatternMode;

/**
 * @brief HDMI AXI driver instance structure
 */
typedef struct {
    uint32_t base_addr;  /**< Base address of the IP */
} Hdmiaxi;

/**
 * @brief Initialize the HDMI AXI driver
 *
 * @param inst Pointer to driver instance
 * @param base_addr Base address of the HDMI AXI IP
 */
void HdmiaxiInit(Hdmiaxi *inst, uint32_t base_addr);

/**
 * @brief Set the video timing preset
 *
 * @param inst Pointer to driver instance
 * @param timing_mode Timing mode (VGA, 480p, or 720p)
 * @return 0 on success, -1 on invalid parameter
 */
int32_t HdmiaxiSetTiming(Hdmiaxi *inst, HdmiaxiTimingMode timing_mode);

/**
 * @brief Set the test pattern
 *
 * @param inst Pointer to driver instance
 * @param pattern_mode Pattern mode (0..9)
 * @return 0 on success, -1 on invalid parameter
 */
int32_t HdmiaxiSetPattern(Hdmiaxi *inst, HdmiaxiPatternMode pattern_mode);

/**
 * @brief Set both timing and pattern in one operation
 *
 * @param inst Pointer to driver instance
 * @param timing_mode Timing mode (VGA, 480p, or 720p)
 * @param pattern_mode Pattern mode (0..9)
 * @return 0 on success, -1 on invalid parameter
 */
int32_t HdmiaxiSetControl(Hdmiaxi *inst, HdmiaxiTimingMode timing_mode,
                          HdmiaxiPatternMode pattern_mode);

/**
 * @brief Read the control register
 *
 * @param inst Pointer to driver instance
 * @param timing_mode Pointer to store timing mode
 * @param pattern_mode Pointer to store pattern mode
 * @return 0 on success
 */
int32_t HdmiaxiGetControl(Hdmiaxi *inst, HdmiaxiTimingMode *timing_mode,
                          HdmiaxiPatternMode *pattern_mode);

/**
 * @brief Read the status register
 *
 * @param inst Pointer to driver instance
 * @return Status register value
 */
uint32_t HdmiaxiGetStatus(Hdmiaxi *inst);

/**
 * @brief Check if MMCM is locked
 *
 * @param inst Pointer to driver instance
 * @return 1 if locked, 0 if not locked
 */
uint32_t HdmiaxiIsLocked(Hdmiaxi *inst);

/**
 * @brief Check if MMCM reconfiguration is in progress
 *
 * @param inst Pointer to driver instance
 * @return 1 if busy, 0 if idle
 */
uint32_t HdmiaxiIsReconfigBusy(Hdmiaxi *inst);

/**
 * @brief Wait for MMCM reconfiguration to complete and lock
 *
 * @param inst Pointer to driver instance
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, -1 on timeout
 */
int32_t HdmiaxiWaitReconfig(Hdmiaxi *inst, uint32_t timeout_ms);

/**
 * @brief Reset the IP to default values
 *
 * Resets to VGA timing and color bars pattern.
 *
 * @param inst Pointer to driver instance
 */
void HdmiaxiReset(Hdmiaxi *inst);

/**
 * @brief Get timing mode name string
 *
 * @param timing_mode Timing mode
 * @return String representation of timing mode
 */
const char* HdmiaxiGetTimingName(HdmiaxiTimingMode timing_mode);

/**
 * @brief Get pattern mode name string
 *
 * @param pattern_mode Pattern mode
 * @return String representation of pattern mode
 */
const char* HdmiaxiGetPatternName(HdmiaxiPatternMode pattern_mode);

/**
 * @brief Print current configuration and status
 *
 * @param inst Pointer to driver instance
 */
void HdmiaxiPrintInfo(Hdmiaxi *inst);

#ifdef __cplusplus
}
#endif

#endif /* HDMI_AXI_H_ */
