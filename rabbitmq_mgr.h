/*
 *Refer to: rabbitmq-c-0.6.0 
 *after the configs, for step to use: 1.init;2.declare;3.send;4.exit
 */
#include "rabbitmq_common.h"

#ifndef _RABBITMQ_C
#define _RABBITMQ_C
/*
 *new connection and login for new channel.
 */
BOOL rmq_init();

/*
 *declare exchange and queues. will new ones if no exist.
 *in rabbitmq, messages will go through like: exchange->topics->queues.
 */
BOOL rmq_exchange_queues_declare();

/*
 *send with exchange/routingkey/priority.
 *return doc: for details you can see 'enum amqp_status_enum_' in /usr/local/include/amqp.h 
 *  AMQP_STATUS_CONNECTION_CLOSED = -0x0007, The connection to the broker has been closed.
 *  AMQP_STATUS_SOCKET_ERROR = -0x0009, A socket error occurred.
 *  AMQP_STATUS_TIMEOUT = -0x000D, Operation timed out 
 *  AMQP_STATUS_SOCKET_CLOSED = -0x0011, Underlying socket is closed. 
 *  AMQP_STATUS_TCP_ERROR\AMQP_STATUS_TCP_SOCKETLIB_INIT_ERROR and so on. 
 */
int rmq_send(const char* exchange, int priority, const char* routing_key, const void* sendbuf, int sendlen);

#define AMQP_GET_TIMEOUT -1                //maybe empty queue.
#define AMQP_GET_ERROR_HEADER -2           //header error
#define AMQP_GET_ERROR_FRAME_TYPE -3       //frame type error.
#define AMQP_GET_ERROR_BODYSIZE_EXCCEED -4 //frame type error.
/*
 *get one item from queue with name qname.
 *return:
 * AMQP_GET_* above 
 * AMQP_RESPONSE_NONE = 0,         //< the library got an EOF from the socket 
 * AMQP_RESPONSE_NORMAL,           //< response normal, the RPC completed successfully 
 * AMQP_RESPONSE_LIBRARY_EXCEPTION,//< library error, an error occurred in the library, examine the library_error 
 * AMQP_RESPONSE_SERVER_EXCEPTION  //< server exception, the broker returned an error, check replay 
 */
int rmq_get(const char *qname, void *buf, int &buflen, int max_buflen, int &priority);

/*
 *get queue item size.
 */
uint32_t rmq_get_count_nolock(const char* qname);
uint32_t rmq_get_count(const char* qname);

/*
 *close channel and connection.
 */
void rmq_exit();

#endif

