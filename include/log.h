
#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#define ERR_PREFIX   "ERROR"
#define PERR_PREFIX  ERR_PREFIX "(%d): "
#define NERR_PREFIX  ERR_PREFIX ": "

/**
 * perr_printf
 *
 * Print an error message, with errno
 */
__attribute__((format(printf, 1, 2)))
static void perr_printf(const char *format, ...)
{
	va_list args;
	int eo = errno;

	fprintf(stdout, PERR_PREFIX, eo);
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
	fprintf(stdout, ": %s\n", strerror(eo));
	fflush(stdout);
	fflush(stderr);
}

/**
 * err_printf
 *
 * Print an error message.
 */
__attribute__((format(printf, 1, 2)))
static void err_printf(const char *format, ...)
{
	va_list args;

	fprintf(stdout, NERR_PREFIX);
	va_start(args, format);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	va_end(args);
	fflush(stdout);
	fflush(stderr);
}

/**
 * log_message
 *
 * Print a message.
 */
__attribute__((format(printf, 1, 2)))
static void log_message(const char *format, ...)
{
	va_list args;
	int eo = errno;

	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
	fprintf(stdout, "\n");
	fflush(stdout);
	fflush(stderr);
}

/**
 * debug_log
 *
 * Print an debug message.
 */
__attribute__((format(printf, 1, 2)))
static void debug_log(const char *format, ...)
{
#ifdef DEBUG
	va_list args;
	int eo = errno;

	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
	fprintf(stdout, "\n");
	fflush(stdout);
	fflush(stderr);
#endif /* DEBUG */
}
