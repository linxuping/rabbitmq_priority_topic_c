#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "rabbitmq_log.h"

static char rmq_log_level_desc[3][8] = {"I","W","E"};
static FILE* g_logger = NULL;
static rmq_log_handler g_log_handler = NULL;

BOOL rmq_log_init(const char* log_path)
{
	if (g_logger != NULL)
		return TRUE;
	g_logger = fopen(log_path, "at+");
	if (g_logger == NULL){
		printf("open rabbitmq.log error. ");
		return FALSE;
	}
	return TRUE;
}

void rmq_log_set_handler(const rmq_log_handler _handler){
	g_log_handler = _handler;
}

void _rmq_log_write(const char* str, rmq_log_level level)
{
	char desc[16];
	if (level>=0 && level<=3)
		strcpy(desc, rmq_log_level_desc[level]);
	else
		strcpy(desc, rmq_log_level_desc[0]);

	time_t nowtime = time(NULL);  
	struct tm *now = localtime(&nowtime);  
	char buf[1024];
	memset(buf, 0, 1024);
	sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d] [%s.]%s\n",now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, desc, str);
	fwrite(buf, strlen(buf)+1, 1, g_logger);
	fflush(g_logger);
}

void rmq_log_write(const char* str, rmq_log_level level)
{
	if (g_log_handler == NULL)	
		_rmq_log_write(str, level);
	else
		g_log_handler(str, level);
}

void rmq_log_write_errno(const char* log){
	char buf[1024];
	memset(buf, 0, 1024);
	sprintf(buf, "%s. strerror:%s", log, strerror(errno));
	rmq_log_write(buf, RMQ_ERROR);
}

void rmq_log_exit()
{
	fclose(g_logger);
}

