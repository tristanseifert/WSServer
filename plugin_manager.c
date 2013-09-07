#include "plugin_manager.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 * Creates a new instance of the plugin manager.
 */
Plugin_Manager* plugin_manager_new() {
	Plugin_Manager* pm = malloc(sizeof(*pm));
	pm->plugin_endpoint_map = malloc(sizeof(Plugin_Endpoint_Registry));

	// plugin_endpoint_map_last points to the last entry in the linked list
	pm->plugin_endpoint_map_last = pm->plugin_endpoint_map;
	return pm;
}

/*
 * Frees memory allocated to the plugin manager.
 */
void plugin_manager_free(Plugin_Manager* pm) {
	Plugin_Endpoint_Registry* entry = (Plugin_Endpoint_Registry*) pm->plugin_endpoint_map;

	while(entry) {
		free(entry);
		free(entry->endpoint);

		entry = entry->next;
	}

	free(pm);
}

/*
 * Registers a plugin for a certain endpoint.
 */
void plugin_register_handler(char* endpointPath, Plugin_EndpointCallback function, Plugin_Manager* pm) {
	Plugin_Endpoint_Registry *lastEntry = pm->plugin_endpoint_map_last;

	// Allocate memory for the next struct
	Plugin_Endpoint_Registry *newEntry = malloc(sizeof(*newEntry));
	memset(newEntry, 0x00, sizeof(*newEntry));

	newEntry->endpoint = endpointPath;
	newEntry->callback = function;

	// Update pointers
	lastEntry->next = newEntry;
	pm->plugin_endpoint_map_last = newEntry;

	printf("Registered plugin for endpoint \"%s\" with callback %X\n", endpointPath, function);
}