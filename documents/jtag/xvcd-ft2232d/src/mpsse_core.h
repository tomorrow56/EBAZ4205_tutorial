/*
 * mpsse_core.h - Shared MPSSE/XVC core for xvcd_ft2232d
 *
 * Included by both the Linux build (xvcd_ft2232d.c) and the
 * Windows build (xvcd_ft2232d_win.c).  Each platform-specific
 * source file must define the socket abstraction layer BEFORE
 * including this header.
 *
 * Required definitions from the includer:
 *   sock_t        - socket descriptor type
 *   SOCK_INVALID  - invalid socket sentinel
 *   sock_close(s) - close a socket
 *   sock_read(s, buf, len)  -> int
 *   sock_write(s, buf, len) -> int
 *   msleep(ms)    - sleep for ms milliseconds
 */

#ifndef MPSSE_CORE_H
#define MPSSE_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <ftdi.h>

/* -------------------------------------------------------------------------
 * Compile-time constants
 * ---------------------------------------------------------------------- */

#define DEFAULT_VID        0x0403
#define DEFAULT_PID        0x6010
#define DEFAULT_PORT       2542
#define DEFAULT_FREQ_HZ    6000000U

#define MAX_BITS           (32768U)
#define MAX_BYTES          ((MAX_BITS + 7U) / 8U)
#define MPSSE_CMD_BUF_SIZE (MAX_BYTES * 4U + 64U)

#define XVC_INFO_STRING    "xvcServer_v1.0:"

/* -------------------------------------------------------------------------
 * MPSSE command bytes
 * ---------------------------------------------------------------------- */

#define MPSSE_WRITE_NEG    0x01
#define MPSSE_BITMODE      0x02
#define MPSSE_READ_NEG     0x04
#define MPSSE_LSB          0x08
#define MPSSE_DO_WRITE     0x10
#define MPSSE_DO_READ      0x20
#define MPSSE_WRITE_TMS    0x40

#define SET_BITS_LOW       0x80
#define GET_BITS_LOW       0x81
#define SET_BITS_HIGH      0x82
#define TCK_DIVISOR        0x86
#define SEND_IMMEDIATE     0x87
#define DISABLE_CLK_DIV5   0x8A
#define LOOPBACK_END       0x85

/* ADBUS pin mapping: TCK=bit0, TDI=bit1, TDO=bit2, TMS=bit3 */
#define JTAG_INIT_GPIO_VAL 0x08   /* TMS=1 */
#define JTAG_GPIO_DIR      0x0B   /* TCK, TDI, TMS as outputs */

/* -------------------------------------------------------------------------
 * Global state
 * ---------------------------------------------------------------------- */

static struct ftdi_context g_ftdi;
static int g_verbose = 0;
static int g_running = 1;

/* -------------------------------------------------------------------------
 * FTDI helpers
 * ---------------------------------------------------------------------- */

static int ftdi_read_all(unsigned char *buf, int len)
{
  int got = 0;
  while (got < len) {
    int r = ftdi_read_data(&g_ftdi, buf + got, len - got);
    if (r < 0) {
      fprintf(stderr, "ftdi_read_data error: %s\n",
              ftdi_get_error_string(&g_ftdi));
      return -1;
    }
    got += r;
  }
  return 0;
}

static int mpsse_init(unsigned int freq_hz)
{
  unsigned char dummy;
  uint16_t divisor;

  if (freq_hz == 0U) {
    freq_hz = DEFAULT_FREQ_HZ;
  }
  {
    unsigned int div_val = 12000000U / (2U * freq_hz);
    if (div_val == 0U) {
      div_val = 1U;
    }
    divisor = (uint16_t)(div_val - 1U);
  }

  ftdi_tcioflush(&g_ftdi);

  if (ftdi_set_bitmode(&g_ftdi, 0x00, BITMODE_RESET) < 0) {
    fprintf(stderr, "ftdi_set_bitmode(RESET): %s\n",
            ftdi_get_error_string(&g_ftdi));
    return -1;
  }
  if (ftdi_set_bitmode(&g_ftdi, JTAG_GPIO_DIR, BITMODE_MPSSE) < 0) {
    fprintf(stderr, "ftdi_set_bitmode(MPSSE): %s\n",
            ftdi_get_error_string(&g_ftdi));
    return -1;
  }

  msleep(50U);
  while (ftdi_read_data(&g_ftdi, &dummy, 1) > 0) {
    /* discard stale bytes */
  }

  /* MPSSE synchronization: send bogus 0xAA, expect echo 0xFA 0xAA */
  {
    unsigned char sync_cmd = 0xAA;
    unsigned char sync_resp[2];

    if (ftdi_write_data(&g_ftdi, &sync_cmd, 1) != 1) {
      fprintf(stderr, "MPSSE sync write failed\n");
      return -1;
    }
    msleep(20U);
    if (ftdi_read_all(sync_resp, 2) != 0) {
      fprintf(stderr, "MPSSE sync read failed\n");
      return -1;
    }
    if (sync_resp[0] != 0xFA || sync_resp[1] != 0xAA) {
      fprintf(stderr, "MPSSE sync failed: got 0x%02x 0x%02x\n",
              sync_resp[0], sync_resp[1]);
      return -1;
    }
    if (g_verbose) {
      printf("MPSSE sync OK\n");
    }
  }

  {
    unsigned char init_buf[] = {
      SET_BITS_LOW, JTAG_INIT_GPIO_VAL, JTAG_GPIO_DIR,
      TCK_DIVISOR,  (unsigned char)(divisor & 0xFFU),
                    (unsigned char)((divisor >> 8) & 0xFFU),
      LOOPBACK_END,
      SEND_IMMEDIATE
    };
    if (ftdi_write_data(&g_ftdi, init_buf, (int)sizeof(init_buf))
        != (int)sizeof(init_buf)) {
      fprintf(stderr, "MPSSE init write failed: %s\n",
              ftdi_get_error_string(&g_ftdi));
      return -1;
    }
  }

  if (g_verbose) {
    unsigned int actual = 12000000U / (2U * (unsigned int)(divisor + 1U));
    printf("MPSSE initialized: TCK ~%u Hz (divisor=%u)\n", actual, divisor);
  }

  return 0;
}

/* -------------------------------------------------------------------------
 * Socket I/O helpers (use platform-provided sock_read / sock_write)
 * ---------------------------------------------------------------------- */

static int sread(sock_t fd, void *buf, int len)
{
  unsigned char *p = (unsigned char *)buf;
  while (len > 0) {
    int r = sock_read(fd, p, len);
    if (r <= 0) {
      return r;
    }
    p   += r;
    len -= r;
  }
  return 1;
}

static int swrite(sock_t fd, const void *buf, int len)
{
  const unsigned char *p = (const unsigned char *)buf;
  while (len > 0) {
    int r = sock_write(fd, p, len);
    if (r <= 0) {
      return r;
    }
    p   += r;
    len -= r;
  }
  return 1;
}

/* -------------------------------------------------------------------------
 * XVC command handlers
 * ---------------------------------------------------------------------- */

static int handle_getinfo(sock_t fd)
{
  char reply[64];
  int n = snprintf(reply, sizeof(reply), "%s%u\n", XVC_INFO_STRING, MAX_BITS);
  if (swrite(fd, reply, n) != 1) {
    fprintf(stderr, "getinfo: write failed\n");
    return -1;
  }
  return 0;
}

static int handle_settck(sock_t fd)
{
  uint32_t period_ns;
  uint32_t freq_hz;
  uint16_t divisor;
  uint32_t actual_period_ns;

  if (sread(fd, &period_ns, 4) != 1) {
    fprintf(stderr, "settck: read period failed\n");
    return -1;
  }

  freq_hz = (period_ns == 0U) ? DEFAULT_FREQ_HZ : (1000000000U / period_ns);
  if (freq_hz > DEFAULT_FREQ_HZ) {
    freq_hz = DEFAULT_FREQ_HZ;
  }
  if (freq_hz < 1U) {
    freq_hz = 1U;
  }

  {
    unsigned int div_val = 12000000U / (2U * freq_hz);
    if (div_val == 0U) {
      div_val = 1U;
    }
    divisor = (uint16_t)(div_val - 1U);
  }

  {
    unsigned char tck_buf[] = {
      TCK_DIVISOR,
      (unsigned char)(divisor & 0xFFU),
      (unsigned char)((divisor >> 8) & 0xFFU),
      SEND_IMMEDIATE
    };
    if (ftdi_write_data(&g_ftdi, tck_buf, (int)sizeof(tck_buf))
        != (int)sizeof(tck_buf)) {
      fprintf(stderr, "settck: MPSSE write failed\n");
      return -1;
    }
  }

  actual_period_ns =
    1000000000U / (12000000U / (2U * (unsigned int)(divisor + 1U)));

  if (g_verbose) {
    printf("settck: requested %u ns, actual %u ns (divisor=%u)\n",
           period_ns, actual_period_ns, divisor);
  }

  if (swrite(fd, &actual_period_ns, 4) != 1) {
    fprintf(stderr, "settck: write failed\n");
    return -1;
  }
  return 0;
}

static int build_shift_cmd(
  uint32_t len,
  const unsigned char *tms_buf,
  const unsigned char *tdi_buf,
  unsigned char *cmd_buf,
  int *cmd_len)
{
  uint32_t i;
  unsigned char *p = cmd_buf;
  int tdo_bytes = 0;

  for (i = 0U; i < len; i++) {
    int tms = (int)((tms_buf[i / 8U] >> (i & 7U)) & 1U);
    int tdi = (int)((tdi_buf[i / 8U] >> (i & 7U)) & 1U);

    *p++ = (unsigned char)(MPSSE_WRITE_TMS | MPSSE_DO_READ |
                           MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG);
    *p++ = 0x00;
    *p++ = (unsigned char)((tdi << 7) | (tms & 0x01));
    tdo_bytes++;
  }

  *p++ = SEND_IMMEDIATE;
  *cmd_len = (int)(p - cmd_buf);

  return tdo_bytes;
}

static int handle_shift(sock_t fd)
{
  static unsigned char s_tms_buf[MAX_BYTES];
  static unsigned char s_tdi_buf[MAX_BYTES];
  static unsigned char s_tdo_buf[MAX_BYTES];
  static unsigned char s_cmd_buf[MPSSE_CMD_BUF_SIZE];
  static unsigned char s_raw_buf[MAX_BITS];

  uint32_t len;
  uint32_t nr_bytes;
  int cmd_len = 0;
  int tdo_bytes;
  uint32_t i;

  if (sread(fd, &len, 4) != 1) {
    fprintf(stderr, "shift: read len failed\n");
    return -1;
  }
  if (len == 0U || len > MAX_BITS) {
    fprintf(stderr, "shift: invalid len=%u\n", len);
    return -1;
  }

  nr_bytes = (len + 7U) / 8U;

  if (sread(fd, s_tms_buf, (int)nr_bytes) != 1) {
    fprintf(stderr, "shift: read TMS failed\n");
    return -1;
  }
  if (sread(fd, s_tdi_buf, (int)nr_bytes) != 1) {
    fprintf(stderr, "shift: read TDI failed\n");
    return -1;
  }

  if (g_verbose) {
    uint32_t j;
    printf("shift: %u bits\n  TMS:", len);
    for (j = 0U; j < nr_bytes; j++) { printf(" %02x", s_tms_buf[j]); }
    printf("\n  TDI:");
    for (j = 0U; j < nr_bytes; j++) { printf(" %02x", s_tdi_buf[j]); }
    printf("\n");
  }

  tdo_bytes = build_shift_cmd(len, s_tms_buf, s_tdi_buf, s_cmd_buf, &cmd_len);

  if (ftdi_write_data(&g_ftdi, s_cmd_buf, cmd_len) != cmd_len) {
    fprintf(stderr, "shift: MPSSE write failed: %s\n",
            ftdi_get_error_string(&g_ftdi));
    return -1;
  }

  if (ftdi_read_all(s_raw_buf, tdo_bytes) != 0) {
    fprintf(stderr, "shift: MPSSE read failed\n");
    return -1;
  }

  memset(s_tdo_buf, 0, nr_bytes);
  for (i = 0U; i < len; i++) {
    if (s_raw_buf[i] & 0x80U) {
      s_tdo_buf[i / 8U] |= (unsigned char)(1U << (i & 7U));
    }
  }

  if (g_verbose) {
    uint32_t j;
    printf("  TDO:");
    for (j = 0U; j < nr_bytes; j++) { printf(" %02x", s_tdo_buf[j]); }
    printf("\n");
  }

  if (swrite(fd, s_tdo_buf, (int)nr_bytes) != 1) {
    fprintf(stderr, "shift: write TDO failed\n");
    return -1;
  }

  return 0;
}

/* -------------------------------------------------------------------------
 * Client connection dispatcher
 * ---------------------------------------------------------------------- */

static int handle_client(sock_t fd)
{
  char cmd[16];

  for (;;) {
    int r = sock_read(fd, cmd, (int)(sizeof(cmd) - 1U));
    if (r <= 0) {
      if (r == 0 && g_verbose) {
        printf("client disconnected\n");
      }
      return 0;
    }
    cmd[r] = '\0';

    if (strncmp(cmd, "getinfo:", 8) == 0) {
      if (g_verbose) { printf("cmd: getinfo\n"); }
      if (handle_getinfo(fd) < 0) { return -1; }
    } else if (strncmp(cmd, "settck:", 7) == 0) {
      if (g_verbose) { printf("cmd: settck\n"); }
      if (handle_settck(fd) < 0) { return -1; }
    } else if (strncmp(cmd, "shift:", 6) == 0) {
      if (g_verbose) { printf("cmd: shift\n"); }
      if (handle_shift(fd) < 0) { return -1; }
    } else {
      fprintf(stderr, "unknown cmd: '%s'\n", cmd);
      return -1;
    }
  }
}

/* -------------------------------------------------------------------------
 * Common FTDI open / close
 * ---------------------------------------------------------------------- */

static int ftdi_open_device(int vid, int pid, const char *serial,
                            unsigned int freq_hz)
{
  if (ftdi_init(&g_ftdi) < 0) {
    fprintf(stderr, "ftdi_init failed\n");
    return -1;
  }

  if (ftdi_set_interface(&g_ftdi, INTERFACE_A) < 0) {
    fprintf(stderr, "ftdi_set_interface: %s\n",
            ftdi_get_error_string(&g_ftdi));
    ftdi_deinit(&g_ftdi);
    return -1;
  }

  if (serial != NULL) {
    if (ftdi_usb_open_desc(&g_ftdi, vid, pid, NULL, serial) < 0) {
      fprintf(stderr, "ftdi_usb_open_desc (serial=%s): %s\n",
              serial, ftdi_get_error_string(&g_ftdi));
      ftdi_deinit(&g_ftdi);
      return -1;
    }
  } else {
    if (ftdi_usb_open(&g_ftdi, vid, pid) < 0) {
      fprintf(stderr, "ftdi_usb_open (vid=0x%04x pid=0x%04x): %s\n",
              vid, pid, ftdi_get_error_string(&g_ftdi));
      ftdi_deinit(&g_ftdi);
      return -1;
    }
  }

  printf("Opened FT2232D (VID=0x%04x PID=0x%04x)\n", vid, pid);

  ftdi_usb_reset(&g_ftdi);
  ftdi_set_latency_timer(&g_ftdi, 1);
  ftdi_set_event_char(&g_ftdi, 0, 0);
  ftdi_set_error_char(&g_ftdi, 0, 0);

  if (mpsse_init(freq_hz) < 0) {
    fprintf(stderr, "MPSSE initialization failed\n");
    ftdi_usb_close(&g_ftdi);
    ftdi_deinit(&g_ftdi);
    return -1;
  }

  return 0;
}

static void ftdi_close_device(void)
{
  ftdi_usb_reset(&g_ftdi);
  ftdi_usb_close(&g_ftdi);
  ftdi_deinit(&g_ftdi);
}

#endif /* MPSSE_CORE_H */
