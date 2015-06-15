//internal.configs
#define RMQ_ITEMS     8
#define RMQ_ITEM_SIZE 24 
#define FRAME_MAX     131072
#define PRIORITY_MAX  10
const char *log_path   = "log/rabbitmq.log";


//external.configs
const char *rmq_hostname = "192.168.102.50";
const int   rmq_port     = 5672;
const char *rmq_username = "teiron";
const char *rmq_passwd   = "teiron";
const int  rmq_heartbeat = 60;//Detecting Dead TCP Connections

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

