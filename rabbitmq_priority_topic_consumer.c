/* vim:set list */
/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: 1.0
 * Author: linxp  2015.02.05
 * ***** END LICENSE BLOCK ***** */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <assert.h>
#include "utils.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

const char *amqp_server_exception_string(amqp_rpc_reply_t r);
const char *amqp_rpc_reply_string(amqp_rpc_reply_t r);
void die_rpc(amqp_rpc_reply_t r, const char *fmt, ...);
void die_errno(int err, const char *fmt, ...);
void die_amqp_error(int err, const char *fmt, ...);
void write_all(int fd, amqp_bytes_t data);
void copy_body(amqp_connection_state_t conn, int fd);

int main(/*int argc, char const *const *argv*/)
{
	char const *hostname;
	int port, status;
	char const *exchange;
	char const *routingkey;
	char const *messagebody;
	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;
	amqp_rpc_reply_t t;
	int i = 0;

	hostname = "192.168.29.131";
	port = 5672;
	exchange = "topic_logs";
	routingkey = "linxpq1";
	messagebody = "hello,world";

	conn = amqp_new_connection();
	socket = amqp_tcp_socket_new(conn); if (!socket) {
		die("creating TCP socket");
	}

	status = amqp_socket_open(socket, hostname, port);
	if (status) {
		die("opening TCP socket");
	}

	die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "test_user", "test_user"), "Logging in");
	amqp_channel_open(conn, 1);
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");
	printf("body: %s \n",messagebody);

	while (1){
		t = amqp_basic_get(conn, 1, amqp_cstring_bytes("linxpq1"), 1);
		//die_rpc(t, "basic.get");
		copy_body(conn, 1);	
		sleep(1);
	}

	die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
	die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
	die_on_error(amqp_destroy_connection(conn), "Ending connection");
	return 0;
}


//------------- functions definition -------------
const char *amqp_server_exception_string(amqp_rpc_reply_t r)
{
	int res;
	static char s[512];

	switch (r.reply.id) {
		case AMQP_CONNECTION_CLOSE_METHOD: {
							amqp_connection_close_t *m
								 = (amqp_connection_close_t *)r.reply.decoded;
							res = snprintf(s, sizeof(s), "server connection error %d, message: %.*s",
									m->reply_code,
									(int)m->reply_text.len,
									(char *)m->reply_text.bytes);
							break;
						}

		case AMQP_CHANNEL_CLOSE_METHOD: {
							amqp_channel_close_t *m
								= (amqp_channel_close_t *)r.reply.decoded;
							res = snprintf(s, sizeof(s), "server channel error %d, message: %.*s",
									m->reply_code,
									(int)m->reply_text.len,
									(char *)m->reply_text.bytes);
							break;
						}

		default:
						res = snprintf(s, sizeof(s), "unknown server error, method id 0x%08X",
								r.reply.id);
						break;
	}

	return res >= 0 ? s : NULL;
}

const char *amqp_rpc_reply_string(amqp_rpc_reply_t r)
{
	switch (r.reply_type) {
		case AMQP_RESPONSE_NORMAL:
			return "normal response";

		case AMQP_RESPONSE_NONE:
			return "missing RPC reply type";

		case AMQP_RESPONSE_LIBRARY_EXCEPTION:
			return amqp_error_string2(r.library_error);

		case AMQP_RESPONSE_SERVER_EXCEPTION:
			return amqp_server_exception_string(r);

		default:
			abort();
	}
}

void die_rpc(amqp_rpc_reply_t r, const char *fmt, ...)
{
	va_list ap;

	if (r.reply_type == AMQP_RESPONSE_NORMAL) {
		return;
	}

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, ": %s\n", amqp_rpc_reply_string(r));
	exit(1);
} 
void die_errno(int err, const char *fmt, ...)
{
	va_list ap;

	if (err == 0) {
		return;
	}

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, ": %s\n", strerror(err));
	exit(1);
}

void die_amqp_error(int err, const char *fmt, ...)
{
	va_list ap;

	if (err >= 0) {
		return;
	}

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, ": %s\n", amqp_error_string2(err));
	exit(1);
}


void write_all(int fd, amqp_bytes_t data)
{ 
	while (data.len > 0) {
		ssize_t res = write(fd, data.bytes, data.len);
		if (res < 0) {
			die_errno(errno, "write");
		}

		data.len -= res;  
		data.bytes = (char *)data.bytes + res;
		printf("buf:%s \n",(char*)data.bytes);
	}
}   

void copy_body(amqp_connection_state_t conn, int fd)
{
	size_t body_remaining;
	amqp_frame_t frame;

	int res = amqp_simple_wait_frame(conn, &frame);
	die_amqp_error(res, "waiting for header frame");
	if (frame.frame_type != AMQP_FRAME_HEADER) {
		die("expected header, got frame type 0x%X",
				frame.frame_type);
	}

	body_remaining = frame.payload.properties.body_size;
	while (body_remaining) {
		res = amqp_simple_wait_frame(conn, &frame);
		die_amqp_error(res, "waiting for body frame");
		if (frame.frame_type != AMQP_FRAME_BODY) {
			die("expected body, got frame type 0x%X", frame.frame_type);
		}

		write_all(fd, frame.payload.body_fragment);
		body_remaining -= frame.payload.body_fragment.len;
	}
}


