/**
 * @file main.c
 * @brief HDMI AXI Pattern Generator Example Application
 *
 * This example demonstrates how to use the HDMI AXI Pattern Generator
 * driver on Xilinx Zynq platforms using Vitis.
 *
 * Supports 3 timing modes (VGA/480p/720p) and 10 test patterns.
 * Timing changes trigger MMCM DRP reconfiguration.
 *
 * Build Instructions:
 *   1. Create a Vitis application project
 *   2. Add hdmi_axi.h and hdmi_axi.c to the project
 *   3. Add this main.c file
 *   4. Build and download to the board
 */

#include <stdio.h>
#include <stdlib.h>
#include "hdmi_axi.h"
#include "sleep.h"

/* Base address of the HDMI AXI IP from Vivado design */
#define HDMI_AXI_BASE_ADDR  0x40000000

/* Display duration per pattern in seconds */
#define PATTERN_DISPLAY_SEC  8

/**
 * @brief Main function
 */
int main(void)
{
    Hdmiaxi hdmi;
    int32_t ret;

    printf("\n");
    printf("========================================\n");
    printf("  HDMI AXI Pattern Generator Demo\n");
    printf("  3 Timings x 10 Patterns\n");
    printf("========================================\n");
    printf("\n");

    /* Initialize the driver */
    HdmiaxiInit(&hdmi, HDMI_AXI_BASE_ADDR);
    printf("Driver initialized at base address: 0x%08X\n", HDMI_AXI_BASE_ADDR);

    /* Reset to default values (VGA + Color Bars) */
    HdmiaxiReset(&hdmi);

    /* Wait for initial MMCM lock */
    printf("Waiting for initial MMCM lock...\n");
    ret = HdmiaxiWaitReconfig(&hdmi, 1000);
    if (ret != 0) {
        printf("ERROR: MMCM not locked after 1 second\n");
        return -1;
    }
    printf("MMCM locked!\n");
    HdmiaxiPrintInfo(&hdmi);

    /* Cycle through all timing modes */
    HdmiaxiTimingMode timings[] = {
        HDMI_AXI_TIMING_VGA,
        HDMI_AXI_TIMING_480P,
        HDMI_AXI_TIMING_720P
    };

    while(1){
        for (int t = 0; t < HDMI_AXI_TIMING_NUM; t++) {
            printf("\n=== %s ===\n", HdmiaxiGetTimingName(timings[t]));

            /* Cycle through all patterns for this timing */
            for (int p = 0; p < HDMI_AXI_PATTERN_NUM; p++) {
                printf("  [%d] %s...\n", p, HdmiaxiGetPatternName((HdmiaxiPatternMode)p));

                ret = HdmiaxiSetControl(&hdmi, timings[t], (HdmiaxiPatternMode)p);
                if (ret != 0) {
                    printf("    ERROR: SetControl failed\n");
                    continue;
                }

                /* Wait for reconfigure if timing changed */
                ret = HdmiaxiWaitReconfig(&hdmi, 500);
                if (ret != 0) {
                    printf("    WARNING: Reconfigure timeout\n");
                }

                sleep(PATTERN_DISPLAY_SEC);
            }
        }

        /* Final status */
        printf("\n=== Final Status ===\n");
        HdmiaxiPrintInfo(&hdmi);
    }

    printf("\nDemo completed successfully!\n");
    printf("\n");

    return 0;
}
