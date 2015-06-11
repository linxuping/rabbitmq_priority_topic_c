#include <stdio.h>
#include <string.h>
#include "rabbitmq_log.h"
#include "rabbitmq_mgr.h"

int main()
{
	BOOL ret;

	ret = rmq_init();
	rmq_log_set_handler(_rmq_log_write);
	rmq_log_write("test", RMQ_ERROR);
	rmq_log_write("hello", RMQ_WARNING);
	rmq_log_write("world", 20);

	rmq_exchange_queues_declare();

	char *buf = "it is zb.yy, high priority=10";
	rmq_send("apple_types", 10, "genuine.jailbreak", (void*)buf, strlen(buf));
	rmq_send("apple_types", 9, "genuine.jailbreak", (void*)buf, strlen(buf));
	rmq_send("apple_types", 8, "genuine", (void*)buf, strlen(buf));
	rmq_send("apple_types", 7, "genuine", (void*)buf, strlen(buf));
	rmq_send("apple_types", 10, "genuine", (void*)buf, strlen(buf));

	rmq_exit();

	printf("++ send finish.\n");
	return 0;
}

