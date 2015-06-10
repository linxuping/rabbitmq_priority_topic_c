#include <stdio.h>
#include "rabbitmq_mgr.h"

int main()
{
	rmq_init();

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

