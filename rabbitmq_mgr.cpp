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
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>
#include "utils.h"
#include "rabbitmq_config.h"
#include "rabbitmq_mgr.h"
#include "rabbitmq_log.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#else
#include <windows.h>
#define sleep Sleep
#endif

#define RECONNECT_RETRY_TIMES  8
struct timeval g_frame_wait_timeout;
amqp_connection_state_t g_conn;

BOOL rmq_init()
{
	amqp_socket_t *socket;
	int status;
	char _buf[RMQ_LOG_MAXSIZE]; //big enough.

	if (!rmq_log_init(log_path)){
		printf("[ERROR] init logger failed. please check it.");
		exit(0);
	}
	g_conn = amqp_new_connection();

	socket = amqp_tcp_socket_new(g_conn);
	if (!socket){ 
		amqp_channel_close(g_conn, 1, AMQP_REPLY_SUCCESS); 
		rmq_log_write_errno("creating TCP socket");//die("creating TCP socket"); //log.err
		return FALSE;
	}

	status = amqp_socket_open(socket, rmq_hostname, rmq_port);
	if (status){
		amqp_channel_close(g_conn, 1, AMQP_REPLY_SUCCESS); 
		rmq_log_write_errno("opening TCP socket");
		return FALSE;
	}

	if (check_amqp_error(amqp_login(g_conn, "/", 0, FRAME_MAX, rmq_heartbeat, AMQP_SASL_METHOD_PLAIN, rmq_username, rmq_passwd), 
				"Logging in", _buf, RMQ_LOG_MAXSIZE)){
		rmq_log_write(_buf, RMQ_ERROR);
		return FALSE;
	}
	amqp_channel_open(g_conn, 1);
	if (check_amqp_error(amqp_get_rpc_reply(g_conn), "Opening channel", _buf, RMQ_LOG_MAXSIZE)){
		rmq_log_write(_buf, RMQ_ERROR);
		return FALSE;
	}

	g_frame_wait_timeout.tv_sec = FRAME_WAIT_TIMEOUT;
	g_frame_wait_timeout.tv_usec = 0;

	rmq_log_write("New channel and Login successfully.", RMQ_INFO);
	return TRUE;
}

BOOL rmq_exchange_queues_declare()
{
	int i,j,m,n, count = 0;
	amqp_bytes_t queuename;
	char *exchange;
	int conf_item_count, topic_item_count;
	amqp_queue_declare_ok_t *r;
	char _buf[RMQ_LOG_MAXSIZE]; //big enough.

	amqp_table_entry_t inner_entries[1];
	amqp_table_t inner_table;
	inner_entries[0].key = amqp_cstring_bytes("x-max-priority");
	inner_entries[0].value.kind = AMQP_FIELD_KIND_I32;
	inner_entries[0].value.value.i32 = PRIORITY_MAX;
	inner_table.num_entries = 1;
	inner_table.entries = inner_entries;

	conf_item_count = sizeof(rmq_exchange_queues)/(RMQ_ITEMS*RMQ_ITEM_SIZE);
	topic_item_count = sizeof(rmq_topics)/(RMQ_ITEMS*RMQ_ITEM_SIZE);
	for (i=0; i<conf_item_count; ++i){
		exchange = (char*)rmq_exchange_queues[i][0];
		if (strlen(exchange) <= 0)
			break;

		amqp_exchange_declare(g_conn, 1, amqp_cstring_bytes(exchange), amqp_cstring_bytes("topic"),
				0, 1, 0, 0, amqp_empty_table);
		if (check_amqp_error(amqp_get_rpc_reply(g_conn), "Declaring exchange", _buf, RMQ_LOG_MAXSIZE)){
				rmq_log_write(_buf, RMQ_ERROR);
				return FALSE;
		}
		sprintf(_buf, "bind exchange successfully. exchange:%s", rmq_exchange_queues[i][0]);
		rmq_log_write(_buf, RMQ_INFO);

		for (j=1; j<RMQ_ITEMS; ++j){
			if (strlen(rmq_exchange_queues[i][j]) <= 0) 
				break;

			queuename = amqp_cstring_bytes(rmq_exchange_queues[i][j]);
			r = amqp_queue_declare(g_conn, 1, queuename, 0, 1, 0, 0, inner_table);
			if (check_amqp_error(amqp_get_rpc_reply(g_conn), "Declaring queue", _buf, RMQ_LOG_MAXSIZE)){
					rmq_log_write(_buf, RMQ_ERROR);
					return FALSE;
			}

			queuename = amqp_bytes_malloc_dup(r->queue);
			if (queuename.bytes == NULL) {
				rmq_log_write("Out of memory while copying queue name", RMQ_ERROR);
				return FALSE;
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
					sprintf(_buf, "bind queue successfully. queue:%s topic:%s", rmq_exchange_queues[i][j],rmq_topics[n][m]);
					rmq_log_write(_buf, RMQ_INFO);
				}	
			}
		}
	}
	return TRUE;
}

amqp_bytes_t amqp_mystring_bytes(const void *buf, int len)
{
  amqp_bytes_t result;
  result.len = len;
  result.bytes = (void*)buf;
  return result;
}

int rmq_send(const char* exchange, int priority, const char* routing_key, const void* sendbuf, int sendlen)
{
	char _buf[RMQ_LOG_MAXSIZE],tmpbuf[128]; //big enough.
	int status;
	int i;
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_PRIORITY_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.delivery_mode = 2; /*persistent delivery mode*/
	props.priority = priority; 
	status = amqp_basic_publish(g_conn,
					1,
					amqp_cstring_bytes(exchange),
					amqp_cstring_bytes(routing_key),
					0,0,&props,
					amqp_mystring_bytes(sendbuf, sendlen));
	for (i=0; i<RECONNECT_RETRY_TIMES; ++i){
		sprintf(tmpbuf, "rmq_send failed and try to re-connect.%d", i);
		if (check_error(status, tmpbuf, _buf, RMQ_LOG_MAXSIZE)){
			rmq_log_write(_buf, RMQ_ERROR);
			if (AMQP_STATUS_CONNECTION_CLOSED==status || 
						AMQP_STATUS_SOCKET_ERROR==status || 
						AMQP_STATUS_TIMEOUT==status || 
						AMQP_STATUS_SOCKET_CLOSED==status ||
						AMQP_STATUS_TCP_ERROR==status || 
						AMQP_STATUS_TCP_SOCKETLIB_INIT_ERROR==status){
				sleep(RECONNECT_TIME);
				rmq_exit();
				rmq_init();
				status = amqp_basic_publish(g_conn,
								1,
								amqp_cstring_bytes(exchange),
								amqp_cstring_bytes(routing_key),
								0,0,&props,
								amqp_mystring_bytes(sendbuf, sendlen));
			}
		}
		else
			break;
	}
	if (check_error(status,"Publishing", _buf, RMQ_LOG_MAXSIZE))
		rmq_log_write(_buf, RMQ_ERROR);
	return status;
}

int fetch_body(amqp_connection_state_t conn, void *buf, amqp_basic_properties_t** props, int &bodysize, int max_bufsize)
{
	char _buf[RMQ_LOG_MAXSIZE]; //big enough.
	size_t body_remaining;
	amqp_frame_t frame;
	bodysize = 0;

	int res = amqp_simple_wait_frame_noblock(conn, &frame, &g_frame_wait_timeout);
	//die_amqp_error(res, "waiting for header frame");
	if (check_error(res, "waiting for header frame", _buf, RMQ_LOG_MAXSIZE)){
		rmq_log_write(_buf, RMQ_ERROR);
		return AMQP_GET_TIMEOUT;
	}

	if (frame.frame_type != AMQP_FRAME_HEADER) {
		//die("expected header, got frame type 0x%X",frame.frame_type);
		sprintf(_buf, "expected header, got frame type 0x%X",frame.frame_type); rmq_log_write(_buf, RMQ_ERROR);
		return AMQP_GET_ERROR_HEADER;
	}

	if (frame.payload.properties.class_id == 60){
		*props = (amqp_basic_properties_t *)frame.payload.properties.decoded;
	}
	else{
		sprintf(_buf, "error to get properties for priority. class_id:%d",frame.payload.properties.class_id);
		rmq_log_write(_buf, RMQ_WARNING);
	}

	body_remaining = frame.payload.properties.body_size;
	if (body_remaining > max_bufsize){
			//die("current size:%d, exceed body defined size %d", frame.frame_type, QUEUE_ITEM_BODY_SIZE);
			sprintf(_buf, "current size:%d, exceed body defined size %d", frame.frame_type, max_bufsize);
			rmq_log_write(_buf, RMQ_ERROR);
			return AMQP_GET_ERROR_BODYSIZE_EXCCEED;
	}
	while (body_remaining) {
		res = amqp_simple_wait_frame(conn, &frame);
		//die_amqp_error(res, "waiting for body frame");
		if (check_error(res, "waiting for body frame", _buf, RMQ_LOG_MAXSIZE)) 
			rmq_log_write(_buf, RMQ_ERROR);
		if (frame.frame_type != AMQP_FRAME_BODY) {
			//die("expected body, got frame type 0x%X", frame.frame_type);
			sprintf(_buf, "expected body, got frame type 0x%X", frame.frame_type);
			rmq_log_write(_buf, RMQ_ERROR);
			return AMQP_GET_ERROR_FRAME_TYPE;
		}

		//memcpy(buf+strlen(buf), frame.payload.body_fragment.bytes, frame.payload.body_fragment.len);
		memcpy((char*)buf+bodysize, frame.payload.body_fragment.bytes, frame.payload.body_fragment.len);
		bodysize += frame.payload.body_fragment.len;
		//buf[frame.payload.body_fragment.len] = '\0';
		body_remaining -= frame.payload.body_fragment.len;
	}
	return AMQP_RESPONSE_NORMAL;
}

int rmq_get(const char *qname, void *buf, int &bodysize, int max_bufsize, int &priority)
{
	char _buf[RMQ_LOG_MAXSIZE],tmpbuf[128]; //big enough.
	int ret,i;
	amqp_rpc_reply_t t;
	amqp_basic_properties_t *props;

	priority = PRIORITY_MAX/2;
	t = amqp_basic_get(g_conn, 1, amqp_cstring_bytes(qname), 1);
	//die_rpc(t, "basic.get");
	for (i=0; i<=RECONNECT_RETRY_TIMES; ++i){
		if (i == RECONNECT_RETRY_TIMES)
			return t.reply_type;
		sprintf(tmpbuf, "basic.get.%d", i);
		if (check_amqp_error(t, tmpbuf, _buf, RMQ_LOG_MAXSIZE)){
			rmq_log_write(_buf, RMQ_ERROR);
			sleep(RECONNECT_TIME);
			rmq_exit();
			rmq_init();
			t = amqp_basic_get(g_conn, 1, amqp_cstring_bytes(qname), 1);
			continue;
		}
		else
			break;
	}

	memset(buf, 0, max_bufsize);
	props = NULL;
	ret = fetch_body(g_conn, buf, &props, bodysize, max_bufsize);
	if (NULL != props) priority = props->priority;
	return ret;
}

void rmq_exit()
{
	char _buf[RMQ_LOG_MAXSIZE]; //big enough.
	if (check_amqp_error(amqp_channel_close(g_conn, 1, AMQP_REPLY_SUCCESS), "Closing channel", _buf, RMQ_LOG_MAXSIZE))
		rmq_log_write(_buf, RMQ_ERROR);
	if (check_amqp_error(amqp_connection_close(g_conn, AMQP_REPLY_SUCCESS), "Closing connection", _buf, RMQ_LOG_MAXSIZE))
		rmq_log_write(_buf, RMQ_ERROR);
	if (check_error(amqp_destroy_connection(g_conn), "Ending connection", _buf, RMQ_LOG_MAXSIZE)) 
		rmq_log_write(_buf, RMQ_ERROR);
	rmq_log_write("close the channel successfully.", RMQ_INFO);
}

uint32_t rmq_get_count(const char* qname)
{
	amqp_table_entry_t inner_entries[1];
	amqp_table_t inner_table;
	inner_entries[0].key = amqp_cstring_bytes("x-max-priority");
	inner_entries[0].value.kind = AMQP_FIELD_KIND_I32;
	inner_entries[0].value.value.i32 = PRIORITY_MAX;
	inner_table.num_entries = 1;
	inner_table.entries = inner_entries;
	amqp_bytes_t bqueuename = amqp_cstring_bytes(qname);

	amqp_queue_declare_ok_t* r = amqp_queue_declare(g_conn, 1, bqueuename, 0, 1, 0, 0, inner_table);
	return r->message_count;
}

