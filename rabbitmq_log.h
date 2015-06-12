#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "rabbitmq_common.h"

#ifndef _RABBITMQ_LOG
#define _RABBITMQ_LOG

#define RMQ_LOG_MAXSIZE 4096
typedef enum{
	RMQ_INFO = 0,
	RMQ_WARNING,
	RMQ_ERROR
} rmq_log_level;
typedef void (*rmq_log_handler)(const char*, rmq_log_level level);

BOOL rmq_log_init(const char* log_path);

void rmq_log_set_handler(const rmq_log_handler _handler);

void rmq_log_write(const char* str, rmq_log_level level);

void _rmq_log_write(const char* str, rmq_log_level level);

void rmq_log_write_errno(const char* log);

void rmq_log_exit();

#endif

