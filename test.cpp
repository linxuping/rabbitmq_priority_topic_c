#include <stdio.h>
#include <string.h>
#include "rabbitmq_log.h"
#include "rabbitmq_mgr.h" 

#ifdef WIN32
#include <windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#endif

void test_send();
void test_get();

int main(int argc, char *argv[])
{
	if (2 != argc){
		printf("input like: ./a.out send(or recv) \n");
		return 0;
	}
	if (strcmp(argv[1],"send") == 0)
		test_send();
	else
		test_get();

	printf("++ q test finish.\n");
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
		printf("ret:%d\n", rmq_send("testexchange", 10, "testq1.testq2", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("testexchange", 9, "testq1.testq2", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("testexchange", 5, "testq1", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("testexchange", 0, "testq1", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("testexchange", -2, "testq1", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("testexchange", 100, "testq1", (void*)buf, strlen(buf)) );
		printf("ret:%d\n", rmq_send("testexchange", 10000, "testq1", (void*)buf, strlen(buf)) );
		sleep(1);
	}while(1);

	rmq_exit();
}

void test_get()
{
	BOOL ret;
	int i,ret2,priority,len;
	char buf[1024];

	ret = rmq_init();
	for(i=0; i>=0; ++i){
		ret2 = rmq_get("testq1", buf,len,1024, priority);
		printf("get from testq1: %d:%s pri:%d bodysize:%d \n",ret2,buf,priority,len);
		//ret2 = rmq_get("testq1", buf,len,1024, priority);
		//printf("get from testq2: %d:%s pri:%d bodysize:%d \n",ret2,buf,priority,len);
		//sleep(1);
	}

	rmq_exit();
}

/*priority test: 
get from testq1: 1:it is zb.yy, high priority=10 pri:10 bodysize:29  
get from testq1: 1:it is zb.yy, high priority=10 pri:254 bodysize:29    -2 -> 254 
get from testq1: 1:it is zb.yy, high priority=10 pri:100 bodysize:29    100 
get from testq1: 1:it is zb.yy, high priority=10 pri:16 bodysize:29     10000 -> (0001 0000) -> 16
get from testq1: 1:it is zb.yy, high priority=10 pri:10 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:254 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:100 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:16 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:9 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:9 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:5 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:5 bodysize:29 
get from testq1: 1:it is zb.yy, high priority=10 pri:0 bodysize:29      0 
get from testq1: 1:it is zb.yy, high priority=10 pri:0 bodysize:29
*/

