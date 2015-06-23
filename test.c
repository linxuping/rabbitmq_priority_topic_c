#ifndef WIN32
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "rabbitmq_log.h"
#include "rabbitmq_mgr.h" 
void test_send();
void test_get();

int main()
{
	test_send();
	//test_get();

	printf("++ send finish.\n");
	return 0;
}

//---- ---- def ---- ----
void test_send()
{ 
	BOOL ret;
	char buf[255] = "it is zb.yy, high priority=10";

	printf("init...\n");
	ret = rmq_init();
	//rmq_log_set_handler(_rmq_log_write);
	rmq_log_write("error_info", RMQ_ERROR);
	rmq_log_write("warning_info", RMQ_WARNING);
	//rmq_log_write("world", 20);

	printf("declare...\n");
	rmq_exchange_queues_declare();
	
	do{
		printf("sending...\n");
		printf("ret:%d\n", rmq_send("apple_types", 10, "genuine.jailbreak", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("apple_types", 9, "genuine.jailbreak", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("apple_types", 8, "genuine", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("apple_types", 7, "genuine", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("apple_types", 10, "genuine", (void*)buf, strlen(buf)) );
	}while(1);

	rmq_exit();
}

void test_get()
{
	BOOL ret;
	int i;
	char buf[1024];

	ret = rmq_init();
	for(i=0; i<2; ++i){
		rmq_get("genuine", buf,1024);
		printf("get from genuine: %s\n",buf);
		rmq_get("jailbreak", buf,1024);
		printf("get from jailbreak: %s\n",buf);
		sleep(0.2);
	}

	rmq_exit();
}

#endif

