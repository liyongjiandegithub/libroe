#ifndef SMALLHTTP_RESPONSE
#define SMALLHTTP_RESPONSE

#include <stddef.h>

enum {
	TRANSFER_ENCODING_NONE = 0,
	TRANSFER_ENCODING_CHUNKED
};

typedef struct {
	int encoding;
	int sending;
	int fd;
} http_response;

void http_response_init(http_response *, int fd);
void http_response_begin(http_response *, int encoding, int code, const char *msg, const char *mime, int content_length);
void http_response_write(http_response *, const char *data, size_t nbytes);
void http_response_end(http_response *);

#endif
