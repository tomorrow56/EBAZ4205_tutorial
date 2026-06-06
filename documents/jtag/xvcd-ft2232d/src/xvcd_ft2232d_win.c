/*
 * xvcd_ft2232d_win.c - Xilinx Virtual Cable Daemon for FT2232D  [Windows]
 *
 * Based on xvcd by tmbinc (https://github.com/tmbinc/xvcd)
 * FT2232D MPSSE JTAG implementation using libftdi
 *
 * Build (MinGW-w64):
 *   gcc -Wall -Wextra -Os -o xvcd_ft2232d.exe xvcd_ft2232d_win.c \
 *       -I<libftdi-include> -L<libftdi-lib> -lftdi1 -lusb-1.0 -lws2_32
 *
 * Build (MSVC):
 *   cl /W3 /O2 xvcd_ft2232d_win.c /I<libftdi-include> \
 *      /link /LIBPATH:<libftdi-lib> ftdi1.lib libusb-1.0.lib ws2_32.lib
 *
 * Usage:
 *   xvcd_ft2232d.exe [-v] [-p port] [-V vid] [-P pid] [-s serial] [-f freq_hz]
 *
 * All MPSSE/XVC logic lives in mpsse_core.h.
 * This file provides only the Windows socket layer, getopt, and main().
 */

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Windows socket abstraction  (consumed by mpsse_core.h)
 * ---------------------------------------------------------------------- */

typedef SOCKET sock_t;
#define SOCK_INVALID  INVALID_SOCKET
#define sock_close(s) closesocket(s)

static int sock_read(sock_t s, void *buf, int len)
{
  return recv(s, (char *)buf, len, 0);
}

static int sock_write(sock_t s, const void *buf, int len)
{
  return send(s, (const char *)buf, len, 0);
}

static void msleep(unsigned int ms)
{
  Sleep((DWORD)ms);
}

/* -------------------------------------------------------------------------
 * Minimal getopt for Windows
 * ---------------------------------------------------------------------- */

static int   opterr = 1;
static int   optind = 1;
static int   optopt = 0;
static char *optarg = NULL;

static int getopt(int argc, char *const argv[], const char *optstring)
{
  static int sp = 1;
  int c;
  const char *cp;

  if (sp == 1) {
    if (optind >= argc ||
        argv[optind][0] != '-' ||
        argv[optind][1] == '\0') {
      return -1;
    }
    if (strcmp(argv[optind], "--") == 0) {
      optind++;
      return -1;
    }
  }

  optopt = c = (unsigned char)argv[optind][sp];
  cp = strchr(optstring, c);

  if (cp == NULL || c == ':') {
    if (opterr) {
      fprintf(stderr, "Unknown option: -%c\n", c);
    }
    if (argv[optind][++sp] == '\0') {
      optind++;
      sp = 1;
    }
    return '?';
  }

  if (cp[1] == ':') {
    if (argv[optind][sp + 1] != '\0') {
      optarg = &argv[optind++][sp + 1];
    } else if (++optind >= argc) {
      if (opterr) {
        fprintf(stderr, "Option -%c requires an argument\n", c);
      }
      sp = 1;
      return '?';
    } else {
      optarg = argv[optind++];
    }
    sp = 1;
  } else {
    if (argv[optind][++sp] == '\0') {
      optind++;
      sp = 1;
    }
    optarg = NULL;
  }

  return c;
}

/* Pull in shared MPSSE/XVC core */
#include "mpsse_core.h"

/* -------------------------------------------------------------------------
 * Signal / console-break handler
 * ---------------------------------------------------------------------- */

static BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
  if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_BREAK_EVENT) {
    g_running = 0;
    return TRUE;
  }
  return FALSE;
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */

static void print_usage(const char *prog)
{
  fprintf(stderr,
          "Usage: %s [options]\n"
          "  -v          Verbose output\n"
          "  -p port     TCP port (default: %d)\n"
          "  -V vid      USB vendor ID  (default: 0x%04x)\n"
          "  -P pid      USB product ID (default: 0x%04x)\n"
          "  -s serial   USB serial number string (optional)\n"
          "  -f freq     TCK frequency in Hz (default: %u, max: 6000000)\n",
          prog, DEFAULT_PORT, DEFAULT_VID, DEFAULT_PID, DEFAULT_FREQ_HZ);
}

int main(int argc, char **argv)
{
  int opt;
  int port             = DEFAULT_PORT;
  int vid              = DEFAULT_VID;
  int pid              = DEFAULT_PID;
  unsigned int freq_hz = DEFAULT_FREQ_HZ;
  const char *serial   = NULL;
  sock_t server_fd;
  int reuse            = 1;
  struct sockaddr_in addr;
  WSADATA wsa_data;

  /* Initialize Winsock */
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
    fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
    return 1;
  }

  SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

  while ((opt = getopt(argc, argv, "vp:V:P:s:f:")) != -1) {
    switch (opt) {
    case 'v': g_verbose = 1; break;
    case 'p': port    = atoi(optarg); break;
    case 'V': vid     = (int)strtol(optarg, NULL, 0); break;
    case 'P': pid     = (int)strtol(optarg, NULL, 0); break;
    case 's': serial  = optarg; break;
    case 'f': freq_hz = (unsigned int)strtoul(optarg, NULL, 0); break;
    default:
      print_usage(argv[0]);
      WSACleanup();
      return 1;
    }
  }

  if (ftdi_open_device(vid, pid, serial, freq_hz) < 0) {
    WSACleanup();
    return 1;
  }

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == SOCK_INVALID) {
    fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
    ftdi_close_device();
    WSACleanup();
    return 1;
  }

  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
             (const char *)&reuse, sizeof(reuse));

  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port        = htons((u_short)port);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
    fprintf(stderr, "bind failed: %d\n", WSAGetLastError());
    sock_close(server_fd);
    ftdi_close_device();
    WSACleanup();
    return 1;
  }

  if (listen(server_fd, 1) == SOCKET_ERROR) {
    fprintf(stderr, "listen failed: %d\n", WSAGetLastError());
    sock_close(server_fd);
    ftdi_close_device();
    WSACleanup();
    return 1;
  }

  printf("Listening on port %d\n", port);
  printf("Connect Vivado/iMPACT to: TCP:localhost:%d\n", port);

  while (g_running) {
    struct sockaddr_in client_addr;
    int client_len = (int)sizeof(client_addr);
    sock_t client_fd;
    int nodelay = 1;

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == SOCK_INVALID) {
      int err = WSAGetLastError();
      if (err == WSAEINTR) {
        continue;
      }
      if (g_running) {
        fprintf(stderr, "accept failed: %d\n", err);
      }
      break;
    }

    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY,
               (const char *)&nodelay, sizeof(nodelay));

    if (g_verbose) {
      char ipstr[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client_addr.sin_addr, ipstr, sizeof(ipstr));
      printf("Client connected: %s:%d\n",
             ipstr, ntohs(client_addr.sin_port));
    }

    handle_client(client_fd);
    sock_close(client_fd);

    if (g_verbose) {
      printf("Client disconnected\n");
    }
  }

  sock_close(server_fd);
  ftdi_close_device();
  WSACleanup();
  printf("xvcd_ft2232d exited cleanly\n");
  return 0;
}
