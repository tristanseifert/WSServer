#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#define WS_ACCEPT_MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

typedef struct WS_Client {
	int read_socket;
	char* endpoint;
	char *ws_key;
	char *ws_accept;
	char *ws_protocol;
	int ws_version;

	void *headers;
} WS_Client;

#endif
