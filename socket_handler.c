#include "socket_handler.h"
#include "config_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

static pthread_t socket_listener_thread;
static uint8_t socket_listener_should_run;

int socket_handler_get_connection(int sock);
void *socket_handler_listening_thread(void *param);
void *socket_handler_connection_handler(void *connection);

int socket_handler_init(WS_Connection *connection) {
	socket_listener_should_run = 1;
    int error = pthread_create(&socket_listener_thread, NULL, socket_handler_listening_thread, connection);
    
    if(error) {
        socket_listener_should_run = 0;
        close(connection->socket);

        perror("Creating listening thread");
        
        return error | 0x8000;
    }
    
    return 0;
}

/*
 * This thread processes connections.
 */
void *socket_handler_listening_thread(void *param) {
    int t; // holds the connection we establish with clients
    WS_Connection *conn = (WS_Connection *) param;

    printf("Client listening thread active. Listening for clients on port %lli...\n"
           , config_get_number("WS_Port"));
    fflush(stdout);


    while(socket_listener_should_run == 1) {
        if((t = socket_handler_get_connection(conn->socket)) < 0) {
            if(errno == EINTR) { // The socket might sometimes get EINTR
                continue;
            }
            
            perror("Accept client connection"); // We got an unknown error accepting the connection
        } else {
            pthread_t thread; // we don't really care about this thread later, it'll die eventually
            pthread_create(&thread, NULL, socket_handler_connection_handler, (void *) &t);
        }
    }
    
    close(conn->socket);
    pthread_exit(NULL);
}

/*
 * Terminates the socket handler.
 */
void socket_handler_die() {
	socket_listener_should_run = 0;
}

/*
 * Waits for a connection to come in on the socket and accepts it, then
 * establishes a socket with the client.
 */
int socket_handler_get_connection(int sock) {
    int t;
    if((t = accept(sock, NULL, NULL)) < 0) {
        return -1;
    }
    
    return t;
}

/*
 * Thread spawned for each client that connects.
 */
void *socket_handler_connection_handler(void *connection) {
    if(connection == NULL) return NULL;
    
    int sock = *((int *) connection);

    pthread_exit(NULL);
}