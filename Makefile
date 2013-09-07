# Separate flags for building objects for plugins and for the main
# server executable.
#
CC=gcc
CCFLAGS=-g -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-format -Wno-unused-variable -std=gnu99 -L/usr/lib -pthread
CCOPT_SO=$(CCFLAGS) -fpic
CCOPT_EXE=$(CCFLAGS)

PLUGIN_DIR = plugins

all: wsserver $(PLUGIN_DIR)/echo.so

#
# The main server
#
WSSERVER_SRC=$(wildcard *.c)
WSSERVER_H=$(wildcard *.h)

%.o: %.c $(WSSERVER_H)
	$(CC) -c $< $(CCOPT_EXE)

#
# Link with --export-dynamic, so that public symbols will be exposed
# to plugins.
#
wsserver: wsserver.o plugin_manager.o plugin_discovery.o config_parser.o util.o socket_handler.o
	$(CC) -o $@ $^ -I. -ldl -Wl #,--export-dynamic

#
# Plugins
#
$(PLUGIN_DIR)/echo.o: $(PLUGIN_DIR)/echo.c $(WSSERVER_H)
	$(CC) -c $< -o $@ $(CCOPT_SO) -I.

#
# The -undefned suppress option is so LD shuts up about not being able to find
# the symbols that are defined in the application itself.
# The -flat_namespace option is namely to appease OS X's dynamic linker.
#
$(PLUGIN_DIR)/echo.so: $(PLUGIN_DIR)/echo.o
	$(CC) -o $@ $^ -shared -flat_namespace -undefined suppress 

clean:
	rm -rf core *.o *.a wsserver plugins/*.o plugins/*.so 

