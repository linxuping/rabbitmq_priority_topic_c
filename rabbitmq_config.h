//internal-configs
#define RMQ_ITEMS     8
#define RMQ_ITEM_SIZE 24 
#define FRAME_MAX     131072
#define PRIORITY_MAX  10


//external-configs
const char *rmq_hostname = "127.0.0.1";
const int   rmq_port     = 5672;
const char *rmq_username = "teiron";
const char *rmq_passwd   = "teiron";

const char rmq_exchange_queues[][RMQ_ITEMS][RMQ_ITEM_SIZE] = { 
	//{exchange, qname1, qname2 ...}
	{"apple_types", "genuine", "jailbreak"},
};

const char rmq_topics[][RMQ_ITEMS][RMQ_ITEM_SIZE] = { 
	//{qname, topic1, topic2 ...}
	{"genuine", "genuine.#"},
	{"jailbreak", "#.jailbreak"},
};

