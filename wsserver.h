#ifndef WSSERVER_H
#define WSSERVER_H

#define CONFIG_PATH "./config.cfg"
#define HOSTNAME_BUF_SIZE 1024

/*
 * This struct is used to hold all information pertinent to dealing with connections.
 */
typedef struct WS_Connection {
	int socket;
} WS_Connection;

#endif
