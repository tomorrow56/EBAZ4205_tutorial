/*
 * xvcd_ft2232d.c - Xilinx Virtual Cable Daemon for FT2232D  [Linux]
 *
 * Based on xvcd by tmbinc (https://github.com/tmbinc/xvcd)
 * FT2232D MPSSE JTAG implementation using libftdi
 *
 * Build:  make
 * Usage:  xvcd_ft2232d [-v] [-p port] [-V vid] [-P pid] [-s serial] [-f freq_hz]
 *
 * All MPSSE/XVC logic lives in mpsse_core.h.
 * This file provides only the Linux socket layer and main().
 */

#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

/* -------------------------------------------------------------------------
 * Linux socket abstraction  (consumed by mpsse_core.h)
 * ---------------------------------------------------------------------- */

typedef int sock_t;
#define SOCK_INVALID  (-1)
#define sock_close(s) close(s)

static int sock_read(sock_t s, void *buf, int len)
{
  return (int)read(s, buf, (size_t)len);
}

static int sock_write(sock_t s, const void *buf, int len)
{
  return (int)write(s, buf, (size_t)len);
}

static void msleep(unsigned int ms)
{
  struct timespec ts;
  ts.tv_sec  = (time_t)(ms / 1000U);
  ts.tv_nsec = (long)((ms % 1000U) * 1000000L);
  nanosleep(&ts, NULL);
}

/* Pull in shared MPSSE/XVC core */
#include "mpsse_core.h"

/* -------------------------------------------------------------------------
 * Signal handler
 * ---------------------------------------------------------------------- */

static void sigint_handler(int sig)
{
  (void)sig;
  g_running = 0;
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

  signal(SIGINT,  sigint_handler);
  signal(SIGTERM, sigint_handler);
  signal(SIGPIPE, SIG_IGN);

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
      return 1;
    }
  }

  if (ftdi_open_device(vid, pid, serial, freq_hz) < 0) {
    return 1;
  }

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == SOCK_INVALID) {
    perror("socket");
    ftdi_close_device();
    return 1;
  }

  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port        = htons((uint16_t)port);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    sock_close(server_fd);
    ftdi_close_device();
    return 1;
  }

  if (listen(server_fd, 1) < 0) {
    perror("listen");
    sock_close(server_fd);
    ftdi_close_device();
    return 1;
  }

  printf("Listening on port %d\n", port);
  printf("Connect Vivado/iMPACT to: TCP:localhost:%d\n", port);

  while (g_running) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    sock_t client_fd;
    int nodelay = 1;

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == SOCK_INVALID) {
      if (errno == EINTR) {
        continue;
      }
      perror("accept");
      break;
    }

    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    if (g_verbose) {
      char ipstr[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client_addr.sin_addr, ipstr, sizeof(ipstr));
      printf("Client connected: %s:%d\n", ipstr, ntohs(client_addr.sin_port));
    }

    handle_client(client_fd);
    sock_close(client_fd);

    if (g_verbose) {
      printf("Client disconnected\n");
    }
  }

  sock_close(server_fd);
  ftdi_close_device();
  printf("xvcd_ft2232d exited cleanly\n");
  return 0;
}
