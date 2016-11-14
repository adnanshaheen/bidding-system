
#pragma once

/* Include config.h, which has all the available headers */
#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <string>
#include <list>
#include <map>

#define INVALID_SOCKET -1								/* Invalid socket handle */
#define MAX_MESSAGE_SIZE 200							/* Maximum message size between manager and bidders */
#define MAX_MESSAGE_SIZE_2 MAX_MESSAGE_SIZE * 2			/* Maximum2 message size between manager and bidders */

const int DEFAULT_MANAGER_PORT = 5000;					/* Default manager port */
const int DEFAULT_BIDDERS = 3;							/* Default bidders */
const int MAX_BIDDERS = 20;								/* Maximum bidders */

/* error codes */
enum _err_codes {
	ERR_MANAGER_DONE = 11,
	ERR_RESTART_BIDS = 10,
	ERR_SUCCESS = 0,
	ERR_SOCKET_OPEN = -1,
	ERR_SOCKET6_OPEN = -2,
	ERR_SOCKET_SETOPT = -3,
	ERR_SOCKET6_SETOPT = -4,
	ERR_SOCKET_BIND = -5,
	ERR_SOCKET6_BIND = -6,
	ERR_SOCKET_LISTEN = -7,
	ERR_SOCKET6_LISTEN = -8,
	ERR_TIMEOUT = -9,
	ERR_SOCKET_SEL = -10,
	ERR_SOCKET_SEND = -11,
	ERR_SOCKET_RECV = -12,
	ERR_SOCKET_ACCEPT = -13,
	ERR_HOSTNAME = -14,
	ERR_SOCKET_CONNECT = -15,
	ERR_SHUTDOWN = -16,
	ERR_KILLED = -17,
	ERR_INVALID_PORT = -18,
	ERR_FORK_FAILED = -19,
	ERR_GET_SOCK_NAME = -20,
	ERR_KEEP_WAITING = -21
};

/* macros for checking and testing a value */
#define test_and_out(a) if (a) goto out;
#define test_and_exit(a) if (a < 0) goto err_exit;
