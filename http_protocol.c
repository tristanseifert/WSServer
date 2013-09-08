#include "http_protocol.h"
#include "ws_protocol.h"
#include "ws_client.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <openssl/sha.h>

/*
 * Takes in the standard HTTP headers, then extracts the relevant information
 * from them.
 */
void http_parse_headers(char* headers, WS_Client* client) {
	char* headers_copy = malloc(strlen(headers) + 2);
	strcpy(headers_copy, headers);

	// Store headers in a temporary struct
	http_header* temp_headers = malloc(sizeof(*temp_headers));
	memset(temp_headers, 0x00, sizeof(*temp_headers));
	http_header* current_header = temp_headers;

	// Find all headers
	char* header = strtok(headers_copy, "\r\n");
	while(header != NULL) {
		// Allocate memory for one header
		http_header* new_header = malloc(sizeof(*current_header));
		memset(new_header, 0x00, sizeof(*current_header));

		new_header->value = header;

		// Do linked-list shenanigans
		current_header->next = new_header;
		current_header = new_header;

		header = strtok(NULL, "\r\n");
	}

	// Loop through all headers and split them
	current_header = temp_headers->next;

	// Process the first header specially as it's the method with the endpoint in it
	char *endpoint = strtok(current_header->value, " ");
	uint8_t i = 0;
	while(endpoint != NULL) {
		// Check if the endpoint starts with a slash like it should (we don't work as a proxy!)
		if(*endpoint == '/') {
			current_header->value = endpoint;
			current_header->key = "endpoint";
			break;
		}

		// Continue on to find the endpoint
		endpoint = strtok(NULL, " ");

		// Make sure we cannot fall into an infinite loop if we don't find the endpoint in 8 iterations.
		i++;
		if(i > 8) {
			return;
		}
	}

	// Process the regular headers
	current_header = current_header->next;
	char *header_token;

	while(current_header != NULL) {
		int position_of_colon = strcspn(current_header->value, ":");
		char *value_colon = current_header->value + position_of_colon;
		*value_colon = 0x00;

		current_header->key = current_header->value;
		// Trim preceding whitespace (whitespace is OPTIONAL per RFC)
		util_string_trim(value_colon + 1);
		current_header->value = value_colon + 1;

		current_header = current_header->next;
	}

	// Set up the iterator once more, point headers to struct.
	client->headers = temp_headers->next;
	current_header = temp_headers->next;

	// Now, loop through all the headers again and find the ones we care about.
	while(current_header != NULL) {
		if(!strcmp(current_header->key, "Sec-WebSocket-Key")) {
			client->ws_key = current_header->value;
		} else if(!strcmp(current_header->key, "Sec-WebSocket-Protocol")) {
			client->ws_protocol = current_header->value;
		} else if(!strcmp(current_header->key, "Sec-WebSocket-Version")) {
			client->ws_version = strtoll(current_header->value, (char **) NULL, 10);
		}

		current_header = current_header->next;
	}
}