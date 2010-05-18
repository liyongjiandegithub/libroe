#include "http.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <request/parser.h>
#include <util.h>

struct http_context_t {
	http_parser parser;
	http_response response;
};

struct http_context_t *http_context_new(int fd)
{
	struct http_context_t *context = malloc(sizeof(struct http_context_t));
	http_parser_init(&context->parser);
	http_response_init(&context->response, fd);
	return context;
}

void http_context_free(void *data)
{
	struct http_context_t *context = data;
	http_parser_free(&context->parser);
	http_response_end(&context->response);
	free(data);
}

////////////////////////////////////////////////////////////////////////////////

void http_on_open(struct fd_context_t *context)
{
	printf("--> opening\n");
	context->data = http_context_new(context->fd);
	context->context_free = http_context_free;
}

int http_on_message(struct fd_context_t *context, const char *msg, size_t nbytes)
{
	struct http_context_t *http_context = context->data;
	http_parser *parser = &http_context->parser;
	http_response *response = &http_context->response;

	printf("here - bytes: %d/%d\n%s---\n", nbytes, strlen(msg), msg);
	http_parser_read(parser, msg, nbytes);
	printf("--> reading done\n");

	const http_request *request;
	int result = 0;

	if ((request = http_parser_done(parser))) {
		printf("%d: got request\n", context->fd);
		struct http_events_t *events = context->shared;
		if (events) {
			switch (request->method) {
				case HTTP_GET:
					if (events->GET)
						result = events->GET(request, response);
					break;
				case HTTP_POST:
					if (events->POST)
						result = events->POST(request, response);
					break;
				case HTTP_PUT:
					if (events->PUT)
						result = events->PUT(request, response);
					break;
			}
		}
		return result;
	}
	return 1;
}

void http_on_close(struct fd_context_t *context)
{
}

static struct fd_cbs_t http_callbacks = {
	.onopen		= http_on_open,
	.onmessage	= http_on_message,
	.onclose	= http_on_close,
};

void http_start(struct service_t *http, poll_mgmt_t *mgmt, int port, struct http_events_t *events)
{
	http->type = SERVICE_HTTP;
	http->fd = poll_mgmt_listen(mgmt, port, &http_callbacks, events);//, &http_callbacks, http);
	http->mgmt = mgmt;
	http->events = events;
}
