#include "ws_protocol.h"
#include "ws_client.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

/*
 * Generates the accept header that the server returns.
 */
void ws_calculate_accept(WS_Client *client) {
	int bufSize = strlen(client->ws_key) + strlen(WS_ACCEPT_MAGIC) + 2;

	// Allocate memory for buffer, clear it
	char *accept_buf = malloc(bufSize);
	memset(accept_buf, 0x00, bufSize);

	// Copy the randomly generated value the client sent
	strcpy(accept_buf, client->ws_key);

	// Concatenate the magic to the end
	strcpy(accept_buf+strlen(client->ws_key), WS_ACCEPT_MAGIC);

	// Calculate SHA1, base64 it
	char *sha1_accept_buf = util_sha1(accept_buf, strlen(accept_buf));
	char *base64_accept_header = util_base64_encode(sha1_accept_buf, SHA_DIGEST_LENGTH);

	// Free memory
	free(accept_buf);
	free(sha1_accept_buf);

	client->ws_accept = base64_accept_header;
}
