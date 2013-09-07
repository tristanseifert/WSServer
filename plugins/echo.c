#include "plugin_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int echo_callback(WS_Client* client, char* data, unsigned int length);

/*
 * This function is called by the plugin loader when the plugin is loaded.
 * Setup of memory for the plugin and allocation of memory can happen here,
 * as well as registration for a certain endpoint.
 */
int init_echo(Plugin_Manager* pm) {
	plugin_register_handler("/echo", echo_callback, pm);
	return 1;
}

/*
 * This function simply echoes the data the client sent.
 */
static int echo_callback(WS_Client* client, char* data, unsigned int length) {
	printf("Got %i bytes of data: %s\n", length, data);

 	return 0;
}