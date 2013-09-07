#include "plugin_discovery.h"
#include "ansi_terminal_defs.h"

#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Plugin_DiscoveryState manages the state of plugin discovery. Holds a list
// of handles returned by dlopen() on the plugin DSOs.
// An opaque pointer to this object can be held by the caller.
typedef struct Plugin_HandleList_t {
	void* handle;
	struct Plugin_HandleList_t* next;
} Plugin_HandleList;

typedef struct Plugin_DiscoveryState_t {
	Plugin_HandleList* handle_list;
} Plugin_DiscoveryState;

// This is the signature of a plugin initialization function that the plugins
// must export as init_<pluginname>. This function accepts a pointer to a plugin
// manager. Its return value means an error if < 0, success otherwise.
typedef int (*Plugin_InitFunc)(Plugin_Manager*);

// Given a filename, return the name of the plugin (the filename
// without .so extension) if it appears like a valid plugin path, or
// NULL.
static char* plugin_get_name(char* filename) {
	// Find where the file name starts, and where it ends before
	// the extension starts
	char* last_slash = strrchr(filename, '/');
	char* name_start = last_slash ? last_slash + 1 : filename;
	char* last_dot = strrchr(filename, '.');

	// We only care about file names that have a .so extension
	if (!last_dot || strcmp(last_dot, ".so")) {
		return NULL;
	}

	// Get just the name
	char* dest = malloc((last_dot - name_start) + 2);
	memset(dest, 0x00, ((last_dot - name_start) + 2));
	strncpy(dest, name_start, last_dot - name_start);

	return dest;
}


// Attempt to load the plugin specified by a name and a full path, by
// dynamically loading the DSO and calling its initialization function.
// If everything goes well, the plugin initialization function will be
// called and passed the plugin manager. In this case, the DSO handle
// (the result of dlopen) will be returned.
// If the loading fails for some reason or the DSO is not a valid plugin
// (e.g. has no init function, or the init function returned an error code),
// NULL is returned.
static void* plugin_load(char* name, char* fullpath, Plugin_Manager* pm) {
	// Make sure the path to dlopen has a slash, for it to consider it 
	// an actual filesystem path and not just a lookup name.
	char *slashedpath = malloc(strlen(fullpath) + 8);
	memset(slashedpath, 0x00, (strlen(fullpath) + 8));
	snprintf(slashedpath, (strlen(fullpath) + 8), "./%s", fullpath);

	// Attempt to open the plugin DSO, load it immediately and make it's symbols
	// globally available
	void* libhandle = dlopen(slashedpath, RTLD_NOW | RTLD_GLOBAL);
	free(slashedpath);
	if (!libhandle) {
		printf("Error loading DSO: %s\n", dlerror());
		return NULL;
	}

	// Attempt to find the init function and then call it
	char *initfunc_name = malloc(strlen(name) + 8);
	memset(initfunc_name, 0x00, (strlen(name) + 8));
	snprintf(initfunc_name, (strlen(name) + 8), "init_%s", name);

	// dlsym returns void*, but we obviously need to cast it to a function
	// pointer to be able to call it. Since void* and function pointers are
	// mutually inconvertible in the eyes of C99, and -pedantic complains about
	// a plain cast, we cast through a pointer-sized integer.
	Plugin_InitFunc initfunc = (Plugin_InitFunc) (intptr_t) dlsym(libhandle, initfunc_name);
	free(initfunc_name);

	if (!initfunc) {
		printf("Error loading init function: %s\n", dlerror());
		dlclose(libhandle);
		return NULL;
	}

	int rc = initfunc(pm);
	if (rc < 0) {
		printf("Error: Plugin init function returned %d\n", rc);
		dlclose(libhandle);
		return NULL;
	}

	printf(ANSI_BOLD ANSI_COLOR_GREEN "Loaded plugin from: \"%s\"\n" ANSI_RESET, fullpath);
	return libhandle;
}

void* plugin_discover(char* dirname, Plugin_Manager* pm) {
	DIR* dir = opendir(dirname);
	
	if (!dir) {
		//char* err = dstring_format("Unable to open path '%s'", dirname);
		perror("Couldn't open plugin directory");
		//free(err);
		return NULL;
	}

	Plugin_DiscoveryState* plugins_state = malloc(sizeof(*plugins_state));
	plugins_state->handle_list = NULL;

	// Look at all DSOs in the plugin directory and attempt to load them.
	struct dirent* direntry;
	while ((direntry = readdir(dir))) {
		char* name = plugin_get_name(direntry->d_name);
		if (!name)
			continue;
		char *fullpath = malloc(strlen(dirname) + strlen(direntry->d_name) + 8);
		memset(fullpath, 0x00, (strlen(dirname) + strlen(direntry->d_name) + 8));
		snprintf(fullpath, (strlen(dirname) + strlen(direntry->d_name) + 8), "%s/%s", dirname, direntry->d_name);

		// Load the plugin, get the DSO handle and add it to the list
		void* handle = plugin_load(name, fullpath, pm);
		if (handle) {
			Plugin_HandleList* handle_node = malloc(sizeof(*handle_node));
			handle_node->handle = handle;
			handle_node->next = plugins_state->handle_list;
			plugins_state->handle_list = handle_node;
		}

		free(name);
		free(fullpath);
	}

	closedir(dir);
	// Return a state if plugins were found.
	if (plugins_state->handle_list)
		return (void *) plugins_state;
	else {
		free(plugins_state);
		return NULL;
	}
}

void plugin_cleanup(void* vpds) {
	Plugin_DiscoveryState* pds = (Plugin_DiscoveryState*)vpds;
	Plugin_HandleList* node = pds->handle_list;
	
	while(node) {
		Plugin_HandleList* next = node->next;
		dlclose(node->handle);
		free(node);
		node = next;
	}
	
	free(pds);
}
