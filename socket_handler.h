#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include "wsserver.h"

// Initialises socket handler and starts thread
int socket_handler_init(WS_Connection *connection);
void socket_handler_die();

#endif