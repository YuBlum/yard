#ifndef __CORE_H__
#define __CORE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if DEV
#  include <stdio.h>
#  define log_info(msg) fprintf(stderr, "\x1b[1;32minfo: " msg "\x1b[0m\n")
#  define log_warn(msg) fprintf(stderr, "\x1b[1;33mwarn: " msg "\x1b[0m\n")
#  define log_error(msg) fprintf(stderr, "\x1b[1;31merror: " msg "\x1b[0m\n")
#  define log_infof(fmt, ...) fprintf(stderr, "\x1b[1;32minfo: " fmt "\x1b[0m\n", ## __VA_ARGS__)
#  define log_warnf(fmt, ...) fprintf(stderr, "\x1b[1;33mwarn: " fmt "\x1b[0m\n", ## __VA_ARGS__)
#  define log_errorf(fmt, ...) fprintf(stderr, "\x1b[1;31merror: " fmt "\x1b[0m\n", ## __VA_ARGS__)
#else
#  define log_info(msg)
#  define log_warn(msg)
#  define log_error(msg)
#  define log_infof(fmt, ...)
#  define log_warnf(fmt, ...)
#  define log_errorf(fmt, ...)
#endif

#endif/*__CORE_H__*/
