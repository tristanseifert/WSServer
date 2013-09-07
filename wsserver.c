#include "wsserver.h"
#include "plugin_discovery.h"
#include "plugin_manager.h"
#include "ansi_terminal_defs.h"

#include "config_parser.h"
#include "socket_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>

void intHandler(int sig);

static WS_Connection *connection = NULL;

int main(int argc, const char* argv[]) {
	// Read in configuration
	config_parse(CONFIG_PATH);

	Plugin_Manager *pm = plugin_manager_new();

	printf(ANSI_BOLD "Discovering plugins...\n" ANSI_RESET);
    void* pdstate = plugin_discover(config_get_value("PluginDirectory"), pm);

    // Allocate memory for connection
    connection = malloc(sizeof(*connection));
    memset(connection, 0x00, sizeof(*connection));

    // Try to open a listening socket
    char ourname[HOSTNAME_BUF_SIZE+1];
    struct sockaddr_in sa;
    struct hostent *hp;
    
    memset(&sa, 0, sizeof(struct sockaddr_in));
    gethostname(ourname, HOSTNAME_BUF_SIZE);
    
    hp = gethostbyname(ourname);
    if(hp == NULL) { // we apparently don't exist - what is this, the matrix?
        perror("Finding hostname");
        return -1;
    }
    
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons(config_get_number("WS_Port"));
    
    // create socket
    if((connection->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Creating listening socket");
        return 255;
    }
    
    // bind socket
    if(bind(connection->socket, (const struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
        close(connection->socket);
        perror("Binding listening socket");
        return 512;
    }
    
    listen(connection->socket, (int) config_get_number("WS_ListenBacklog"));
    
    printf(ANSI_BOLD ANSI_COLOR_GREEN "\nSocket connected!\n\n" ANSI_RESET);

	// Trap Ctrl+C so we can clean up the connection (and flush queue)
	signal(SIGINT, intHandler);
    
    socket_handler_init(connection);
    printf("Server is ready. Waiting for connections.\n");
    
    // Go into an infinite loop here
    while(1) {
        sleep(5);
    }
}

void intHandler(int sig) {
    // Ignore the signal so it can't happen twice
    signal(sig, SIG_IGN);
    printf(ANSI_BOLD ANSI_COLOR_RED "\nReceived Ctrl^C!\n" ANSI_RESET);
    
    // Kill socket handler thread
	socket_handler_die();

    exit(0);
}