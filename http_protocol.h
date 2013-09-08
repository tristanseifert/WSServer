#ifndef HTTP_PROTOCOL_H
#define HTTP_PROTOCOL_H

#include "ws_client.h"

typedef struct http_header {
	char* value;
	char* key;
	void* next;
} http_header;

void http_parse_headers(char* headers, WS_Client* client);

#endif