/*
 * Fledge HTTP north plugin.
 *
 * Copyright (c) 2020 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */
#include <string>
#include <logger.h>
#include <http_north.h>
#include <plugin_api.h>
#include "version.h"

using namespace std;

#define PLUGIN_NAME "httpc"

/**
 * Plugin specific default configuration
 */
static const char *default_config = QUOTE({
		"plugin": {
			"description": "HTTP North C Plugin, support primary and secondary destinations with failover",
			"type": "string",
			"default": PLUGIN_NAME,
		       	"readonly": "true"
			},
		"URL": {
			"description": "The URL of the HTTP Connector to send data to",
			"type": "string",
			"default": "http://localhost:6683/sensor-reading",
			"order": "1",
			"displayName": "URL"
			},
		"URL2": {
			"description": "The URL of the HTTP Connector to send data to if the primary is unavailable, if empty there is no secondary",
			"type": "string",
			"default": "",
			"order": "2",
			"displayName": "Secondary URL"
			},
		"proxy": {
			"description": "The name or address and port of a proxy server to use",
			"type": "string",
			"default": "",
			"order": "3",
			"displayName": "Proxy"
			},
		"source": {
			"description": "Defines the source of the data to be sent on the stream, this may be one of either readings, statistics or audit.",
			"type": "enumeration",
			"default": "readings",
			"options": [ "readings", "statistics"],
			"order": "4",
			"displayName": "Source"
			},
		"headers": {
			"description": "An optional set of header fields expressed as a JSON document",
			"type": "JSON",
			"default": "{}",
			"order": "5",
			"displayName": "Headers"
			},
		"retrySleepTime": {
        		"description": "Seconds between each retry for the communication, NOTE : the time is doubled at each attempt.",
			"type": "integer",
			"default": "1",
			"order": "6",
			"displayName" : "Sleep Time Retry"
			},
		"maxRetry": {
			"description": "Max number of retries for the communication",
			"type": "integer",
			"default": "3",
			"order": "7",
			"displayName" : "Maximum Retry"
			},
		"HttpTimeout": {
			"description": "Timeout in seconds for the HTTP operations with the HTTP Connector Relay",
			"type": "integer",
			"default": "10",
			"order": "8",
			"displayName": "Http Timeout (in seconds)"
			},
		"verifySSL": {
        		"description": "Verify SSL certificate",
			"type": "boolean",
			"default": "False",
			"order": "9",
			"displayName": "Verify SSL"
			},
		"applyFilter": {
        		"description": "Whether to apply filter before processing the data",
			"type": "boolean",
			"default": "False",
			"order": "10",
			"displayName": "Apply Filter"
			},
		"filterRule": {
			"description": "JQ formatted filter to apply (applicable if applyFilter is True)",
			"type": "string",
			"default": ".[]",
			"order": "11",
			"displayName": "Filter Rule"
			}
	});



/**
 * The HTTP north plugin interface
 */
extern "C" {

/**
 * The C API plugin information structure
 */
static PLUGIN_INFORMATION info = {
	PLUGIN_NAME,			// Name
	VERSION,			// Version
	0,				// Flags
	PLUGIN_TYPE_NORTH,		// Type
	"1.0.0",			// Interface version
	default_config			// Configuration
};


/**
 * Return the information about this plugin
 */
PLUGIN_INFORMATION *plugin_info()
{
	return &info;
}

/**
 * Initialise the plugin with configuration.
 *
 * This function is called to get the plugin handle.
 */
PLUGIN_HANDLE plugin_init(ConfigCategory* config)
{
	return new HttpNorth(config);

}

/**
 * Send Readings data to historian server
 */
uint32_t plugin_send(const PLUGIN_HANDLE handle,
		     const vector<Reading *> readings)
{
HttpNorth *north = (HttpNorth *)handle;
	return north->send(readings);
}

/**
 * Shutdown the plugin
 *
 * Delete allocated data
 *
 * @param handle    The plugin handle
 */
void plugin_shutdown(PLUGIN_HANDLE handle)
{
HttpNorth *north = (HttpNorth *)handle;
	delete north;
}

// End of extern "C"
};

