/*
 *doc related: /resource_gather/rabbitmq/doc/数据协议.xlsx (汇总的详细的说明文档)
 */
//internal.configs
#define RMQ_ITEMS     8    //rmq_exchange_queues配置项{exchange,qname1,...}对应的数组最大size
#define RMQ_ITEM_SIZE 24   //rmq_exchange_queues配置项{exchange,qname1,...}对应的item size
#define FRAME_MAX     131072 
#define QUEUE_ITEM_BODY_SIZE  4096 //队列条目的body最大size
#define FRAME_WAIT_TIMEOUT    12   //获取队列条目超时时间
#define PRIORITY_MAX  10   //定义优先级范围(0到PRIORITY_MAX)，数字越大优先级越高.
#define RECONNECT_TIME  3*3   //按照keepalived.conf上TCP_CHECK的配置，connect_timeout*nb_get_retry为重连时间

#ifdef WIN32
const char *log_path   = "log\\rabbitmq.log"; 
#else
const char *log_path   = "log/rabbitmq.log"; //独立的rabbitmq-c接口日志，可以通过rmq_log_set_handler使用自定义日志接口.
#endif
//end.


//external.configs
const char *rmq_hostname = "192.168.12.200";
const int   rmq_port     = 5672;
const char *rmq_username = "teiron";
const char *rmq_passwd   = "teiron";
const int  rmq_heartbeat = 60;//Detecting Dead TCP Connections

//按照rabbitmq协议，当发送到队列，先根据约定的exchange进行路由，再根据topic的匹配规则路由到最终的queue,
//topic相当于queue感兴趣的专题属性，进行绑定后一旦属性匹配上，就能进入当前队列.
const char rmq_exchange_queues[][RMQ_ITEMS][RMQ_ITEM_SIZE] = { 
	//{exchange, qname1, qname2 ...}
	{"app_collect", "ap_check", "ge_check", "jb_check"},
};
const char rmq_topics[][RMQ_ITEMS][RMQ_ITEM_SIZE] = { 
	//{qname, topic1, topic2 ...}
	{"ap_check", "ap_check"},
	{"ge_check", "ge_check.#"},
	{"jb_check", "#.jb_check"},
};
//end.



//test configs.
/*lxp.test for test.c
const char rmq_exchange_queues[][RMQ_ITEMS][RMQ_ITEM_SIZE] = { 
	//{exchange, qname1, qname2 ...}
	{"apple_types", "linxpq1", "linxpq2"},
};

const char rmq_topics[][RMQ_ITEMS][RMQ_ITEM_SIZE] = { 
	//{qname, topic1, topic2 ...}
	{"linxpq1", "genuine.#"},
	{"linxpq2", "#.jailbreak"},
};
*/
//end.

