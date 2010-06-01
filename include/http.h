#ifndef SMALLHTTP_HTTP
#define SMALLHTTP_HTTP

#include <services.h>
#include <response.h>
#include <parser.h>
#include <conn.h>

typedef struct {
	conn_t base;
} http_t;

////////////////////////////////////////////////////////////////////////////////

/* TODO move to a private header */
void http_on_open(conn_t *conn);
int http_on_message(conn_t *conn, const char *msg, size_t nbytes);
void http_on_close(conn_t *conn);

static const fd_cbs_t http_callbacks = {
	.conn_size  = sizeof(http_t),
	/* .data    = <-- FIXME
	 * .oninit  =
	 */
	.onopen		= http_on_open,
	.onmessage	= http_on_message,
	.onclose	= http_on_close,
};

////////////////////////////////////////////////////////////////////////////////

typedef struct {
	int method;
	char *path;//, *query, *frag;
	char *version;
} request_data;

typedef struct http_conn {
	request_data request;
	http_response response;

	int keep_alive;

	void (*onheader)(struct http_conn *conn, const char *header, const char *field);
	void (*makeresponse)(struct http_conn *conn);
} http_conn;

struct http_iface {
	int port;
	void (*onrequest)(http_conn *conn);
};

#endif