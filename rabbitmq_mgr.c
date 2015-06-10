/* vim:set list */
/*
 * ***** BEGIN DESC BLOCK *****
 * Rabbitmq-c Version: 0.6.0
 * Author: linxp 
 * Date: 2015.06.10 
 * ***** END DESC BLOCK *****
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include "utils.h"
#include "rabbitmq_config.h"
#include "rabbitmq_mgr.h"

amqp_connection_state_t g_conn;

void rmq_init()
{
	g_conn = amqp_new_connection();

	amqp_socket_t *socket = amqp_tcp_socket_new(g_conn);
	if (!socket) 
		die("creating TCP socket"); //log.err

	int status = amqp_socket_open(socket, rmq_hostname, rmq_port);
	if (status) 
		die("opening TCP socket"); //log.err

	die_on_amqp_error(amqp_login(g_conn, "/", 0, FRAME_MAX, 0, AMQP_SASL_METHOD_PLAIN, rmq_username, rmq_passwd), "Logging in"); //log.err
	amqp_channel_open(g_conn, 1);
	die_on_amqp_error(amqp_get_rpc_reply(g_conn), "Opening channel"); //log.err
}

void rmq_exchange_queues_declare()
{
	int i,j,m,n, count = 0;
	amqp_bytes_t queuename;
	char *exchange;

	amqp_table_entry_t inner_entries[1];
	amqp_table_t inner_table;
	inner_entries[0].key = amqp_cstring_bytes("x-max-priority");
	inner_entries[0].value.kind = AMQP_FIELD_KIND_I32;
	inner_entries[0].value.value.i32 = PRIORITY_MAX;
	inner_table.num_entries = 1;
	inner_table.entries = inner_entries;

	int conf_item_count = sizeof(rmq_exchange_queues)/(RMQ_ITEMS*RMQ_ITEM_SIZE);
	int topic_item_count = sizeof(rmq_topics)/(RMQ_ITEMS*RMQ_ITEM_SIZE);
	for (i=0; i<conf_item_count; ++i){
		exchange = (char*)rmq_exchange_queues[i][0];
		if (strlen(exchange) <= 0)
			break;

		amqp_exchange_declare(g_conn, 1, amqp_cstring_bytes(exchange), amqp_cstring_bytes("topic"),
				0, 0, 0, 0, amqp_empty_table);
		die_on_amqp_error(amqp_get_rpc_reply(g_conn), "Declaring exchange"); //log.err

		for (j=1; j<RMQ_ITEMS; ++j){
			if (strlen(rmq_exchange_queues[i][j]) <= 0) 
				break;

			queuename = amqp_cstring_bytes(rmq_exchange_queues[i][j]);
			amqp_queue_declare_ok_t *r = amqp_queue_declare(g_conn, 1, queuename, 0, 1, 0, 0, inner_table);
			die_on_amqp_error(amqp_get_rpc_reply(g_conn), "Declaring queue"); //log.err
			queuename = amqp_bytes_malloc_dup(r->queue);
			if (queuename.bytes == NULL) {
				fprintf(stderr, "Out of memory while copying queue name"); //log.err
				exit(0);
			}
			//bind topics
			for (n=0; n<topic_item_count; ++n){
				if (strcmp(rmq_topics[n][0],rmq_exchange_queues[i][j]) != 0) 
					continue;
				if (strlen(rmq_topics[n][0]) == 0) 
					break;

				for (m=1; m<RMQ_ITEMS; ++m){
					if (strlen(rmq_topics[n][m]) == 0) 
						break;
					amqp_queue_bind(g_conn, 1, queuename, amqp_cstring_bytes(exchange), 
													amqp_cstring_bytes(rmq_topics[n][m]),amqp_empty_table);
					//printf("bind  %s - %s \n",rmq_exchange_queues[i][j],rmq_topics[n][m] ); log.info
				}	
			}
		}
	}
}

amqp_bytes_t amqp_mystring_bytes(const void *buf, int len)
{
  amqp_bytes_t result;
  result.len = len;
  result.bytes = (void*)buf;
  return result;
}

void rmq_send(const char* exchange, int priority, const char* routing_key, const void* sendbuf, int sendlen)
{
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_PRIORITY_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.delivery_mode = 2; /*persistent delivery mode*/
	props.priority = priority; 
	die_on_error(amqp_basic_publish(g_conn,
				1,
				amqp_cstring_bytes(exchange),
				amqp_cstring_bytes(routing_key),
				0,
				0,
				&props,
				amqp_mystring_bytes(sendbuf, sendlen)),
			"Publishing"); //log.err
}

void rmq_exit()
{
	die_on_amqp_error(amqp_channel_close(g_conn, 1, AMQP_REPLY_SUCCESS), "Closing channel"); //log.err
	die_on_amqp_error(amqp_connection_close(g_conn, AMQP_REPLY_SUCCESS), "Closing connection"); //log.err
	die_on_error(amqp_destroy_connection(g_conn), "Ending connection"); //log.err
}


