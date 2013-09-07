#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "ws_client.h"

/*
 * All callbacks must follow this function signature.
 * Arg1: A pointer to a client structure for the client that originated the message.
 * Arg2: Data sent by client.
 * Arg3: Length of data sent by client.
 */
typedef int (*Plugin_EndpointCallback)(WS_Client*, char*, unsigned int);

/*
 * A simple linked list to store endpoints plugins have callbacks for.
 */
typedef struct Plugin_Endpoint_Registry {
	void* next;
	char* endpoint;
	Plugin_EndpointCallback callback;
} Plugin_Endpoint_Registry;

/*
 * This is the definition of the struct that holds all plugin-related information,
 * such as what paths plugins are registered for, and so forth.
 */
typedef struct Plugin_Manager {
	int num_plugins_loaded;
	Plugin_Endpoint_Registry* plugin_endpoint_map;
	Plugin_Endpoint_Registry* plugin_endpoint_map_last; // for optimisation
} Plugin_Manager;

Plugin_Manager* plugin_manager_new();
void plugin_manager_free(Plugin_Manager* pm);

/* 
 * These functions are called from plugins to register a callback function that
 * can handle a specific endpoint.
 */
extern void plugin_register_handler(char* endpointPath, Plugin_EndpointCallback function, Plugin_Manager* pm);

#endif