/*
 *Refer to: rabbitmq-c-0.6.0 
 *after the configs, for step to use: 1.init;2.declare;3.send;4.exit
 */

#ifndef _RABBITMQ_C

/*
 *new connection and login for new channel.
 */
void rmq_init();

/*
 *declare exchange and queues. will new ones if no exist.
 *in rabbitmq, messages will go through like: exchange->topics->queues.
 */
void rmq_exchange_queues_declare();

/*
 *send with exchange/routingkey/priority.
 */
void rmq_send(const char* exchange, int priority, const char* routing_key, const void* sendbuf, int sendlen);

/*
 *close channel and connection.
 */
void rmq_exit();

#else
#define _RABBITMQ_C
#endif

