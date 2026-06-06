/**
 * @file hdmi_axi.c
 * @brief HDMI AXI Pattern Generator Driver Implementation
 */

#include "hdmi_axi.h"
#include <stdio.h>
#include <stdlib.h>
#include "sleep.h"

/* Timing mode name strings */
static const char* timing_names[] = {
    "VGA 640x480@60Hz",
    "480p 720x480@60Hz",
    "720p 1280x720@60Hz"
};

/* Pattern mode name strings */
static const char* pattern_names[] = {
    "Color Bars",
    "Checkerboard",
    "H-Gradation",
    "Crosshatch",
    "Border",
    "RGB Field",
    "V-Gradation",
    "Walking Bar",
    "SMPTE Bars",
    "Zone Plate"
};

void HdmiaxiInit(Hdmiaxi *inst, uint32_t base_addr)
{
    if (inst != NULL) {
        inst->base_addr = base_addr;
    }
}

int32_t HdmiaxiSetTiming(Hdmiaxi *inst, HdmiaxiTimingMode timing_mode)
{
    if (inst == NULL) {
        return -1;
    }

    if (timing_mode >= HDMI_AXI_TIMING_NUM) {
        return -1;
    }

    /* Read current control register */
    uint32_t ctrl = Xil_In32(inst->base_addr + HDMI_AXI_REG_CTRL_OFFSET);

    /* Clear timing_sel bits and set new value */
    ctrl = (ctrl & ~HDMI_AXI_CTRL_TIMING_SEL_MASK) |
           ((timing_mode << HDMI_AXI_CTRL_TIMING_SEL_SHIFT) & HDMI_AXI_CTRL_TIMING_SEL_MASK);

    /* Write back */
    Xil_Out32(inst->base_addr + HDMI_AXI_REG_CTRL_OFFSET, ctrl);

    return 0;
}

int32_t HdmiaxiSetPattern(Hdmiaxi *inst, HdmiaxiPatternMode pattern_mode)
{
    if (inst == NULL) {
        return -1;
    }

    if (pattern_mode >= HDMI_AXI_PATTERN_NUM) {
        return -1;
    }

    /* Read current control register */
    uint32_t ctrl = Xil_In32(inst->base_addr + HDMI_AXI_REG_CTRL_OFFSET);

    /* Clear pattern_sel bits and set new value */
    ctrl = (ctrl & ~HDMI_AXI_CTRL_PATTERN_SEL_MASK) |
           ((pattern_mode << HDMI_AXI_CTRL_PATTERN_SEL_SHIFT) &
            HDMI_AXI_CTRL_PATTERN_SEL_MASK);

    /* Write back */
    Xil_Out32(inst->base_addr + HDMI_AXI_REG_CTRL_OFFSET, ctrl);

    return 0;
}

int32_t HdmiaxiSetControl(Hdmiaxi *inst, HdmiaxiTimingMode timing_mode,
                          HdmiaxiPatternMode pattern_mode)
{
    if (inst == NULL) {
        return -1;
    }

    if (timing_mode >= HDMI_AXI_TIMING_NUM) {
        return -1;
    }

    if (pattern_mode >= HDMI_AXI_PATTERN_NUM) {
        return -1;
    }

    /* Construct control register value */
    uint32_t ctrl = ((timing_mode << HDMI_AXI_CTRL_TIMING_SEL_SHIFT) &
                     HDMI_AXI_CTRL_TIMING_SEL_MASK) |
                    ((pattern_mode << HDMI_AXI_CTRL_PATTERN_SEL_SHIFT) &
                     HDMI_AXI_CTRL_PATTERN_SEL_MASK);

    /* Write control register */
    Xil_Out32(inst->base_addr + HDMI_AXI_REG_CTRL_OFFSET, ctrl);

    return 0;
}

int32_t HdmiaxiGetControl(Hdmiaxi *inst, HdmiaxiTimingMode *timing_mode,
                          HdmiaxiPatternMode *pattern_mode)
{
    if (inst == NULL || timing_mode == NULL || pattern_mode == NULL) {
        return -1;
    }

    uint32_t ctrl = Xil_In32(inst->base_addr + HDMI_AXI_REG_CTRL_OFFSET);

    *timing_mode = (HdmiaxiTimingMode)((ctrl & HDMI_AXI_CTRL_TIMING_SEL_MASK) >> HDMI_AXI_CTRL_TIMING_SEL_SHIFT);
    *pattern_mode = (HdmiaxiPatternMode)((ctrl & HDMI_AXI_CTRL_PATTERN_SEL_MASK) >> HDMI_AXI_CTRL_PATTERN_SEL_SHIFT);

    return 0;
}

uint32_t HdmiaxiGetStatus(Hdmiaxi *inst)
{
    if (inst == NULL) {
        return 0;
    }

    return Xil_In32(inst->base_addr + HDMI_AXI_REG_STATUS_OFFSET);
}

uint32_t HdmiaxiIsLocked(Hdmiaxi *inst)
{
    if (inst == NULL) {
        return 0;
    }

    uint32_t status = Xil_In32(inst->base_addr + HDMI_AXI_REG_STATUS_OFFSET);
    return (status & HDMI_AXI_STATUS_LOCKED_MASK) ? 1 : 0;
}

void HdmiaxiReset(Hdmiaxi *inst)
{
    if (inst != NULL) {
        /* Reset to VGA timing and color bars pattern */
        Xil_Out32(inst->base_addr + HDMI_AXI_REG_CTRL_OFFSET, 0);
    }
}

const char* HdmiaxiGetTimingName(HdmiaxiTimingMode timing_mode)
{
    if (timing_mode < HDMI_AXI_TIMING_NUM) {
        return timing_names[timing_mode];
    }
    return "Unknown";
}

const char* HdmiaxiGetPatternName(HdmiaxiPatternMode pattern_mode)
{
    if (pattern_mode < HDMI_AXI_PATTERN_NUM) {
        return pattern_names[pattern_mode];
    }
    return "Unknown";
}

uint32_t HdmiaxiIsReconfigBusy(Hdmiaxi *inst)
{
    if (inst == NULL) {
        return 0;
    }

    uint32_t status = Xil_In32(inst->base_addr + HDMI_AXI_REG_STATUS_OFFSET);
    return (status & HDMI_AXI_STATUS_RECONFIG_BUSY_MASK) ? 1 : 0;
}

int32_t HdmiaxiWaitReconfig(Hdmiaxi *inst, uint32_t timeout_ms)
{
    if (inst == NULL) {
        return -1;
    }

    uint32_t elapsed = 0;
    /* Wait for reconfig to finish */
    while (HdmiaxiIsReconfigBusy(inst)) {
        usleep(1000);
        elapsed++;
        if (elapsed >= timeout_ms) {
            return -1;
        }
    }
    /* Wait for MMCM lock */
    while (!HdmiaxiIsLocked(inst)) {
        usleep(1000);
        elapsed++;
        if (elapsed >= timeout_ms) {
            return -1;
        }
    }
    return 0;
}

void HdmiaxiPrintInfo(Hdmiaxi *inst)
{
    if (inst == NULL) {
        printf("Error: NULL instance\n");
        return;
    }

    HdmiaxiTimingMode timing;
    HdmiaxiPatternMode pattern;
    uint32_t locked;

    HdmiaxiGetControl(inst, &timing, &pattern);
    locked = HdmiaxiIsLocked(inst);

    uint32_t busy = HdmiaxiIsReconfigBusy(inst);

    printf("HDMI AXI Pattern Generator Status:\n");
    printf("  Base Address: 0x%08X\n", inst->base_addr);
    printf("  Timing: %s\n", HdmiaxiGetTimingName(timing));
    printf("  Pattern: %s\n", HdmiaxiGetPatternName(pattern));
    printf("  MMCM Locked: %s\n", locked ? "Yes" : "No");
    printf("  Reconfig Busy: %s\n", busy ? "Yes" : "No");
}
