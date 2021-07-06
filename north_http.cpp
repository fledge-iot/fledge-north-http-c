/*
 * Fledge HTTP north plugin.
 *
 * Copyright (c) 2020 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */

#include <http_north.h>
#include <iostream>

using namespace std;

/**
 * Constructor for the north HTTP plugin class.
 * Creates the two streams and sets the failed over state
 *
 * @param config	The plugin configuration
 */
HttpNorth::HttpNorth(ConfigCategory *config) : m_failedOver(false)
{
	string url = config->getValue("URL");
	m_primary = new HttpStream(config, url);
	url = config->getValue("URL2");
	if (url.length() > 0)
		m_secondary = new HttpStream(config, url);
	else
		m_secondary = NULL;
}

/**
 * Destructor fo rthe HTTP north class
 */
HttpNorth::~HttpNorth()
{
	if (m_primary)
		delete m_primary;
	if (m_secondary)
		delete m_secondary;
}

/**
 * Send the set of readings to the north system.
 *
 * Uses the priamry or secondary connection, if defined to send
 * the readings as JSON to the north FogLAMP.
 *
 * The member varaible m_failedOver is used to determine if the
 * primary or secoindary connection should be used.
 *
 * @param readings	The readings to send
 */
uint32_t HttpNorth::send(const vector<Reading *> readings)
{
	ostringstream jsonData;
	jsonData << "[";

	// Fetch Reading* data
	for (vector<Reading *>::const_iterator elem = readings.begin(); elem != readings.end(); ++elem)
	{
		string value;
		getReadingString(value, **elem);
		jsonData << value << (elem < (readings.end() -1 ) ? ", " : "");
	}

	jsonData << "]";
	string data = jsonData.str();

	lock_guard<mutex> guard(m_mutex);
	if (m_failedOver)
	{
		if (m_secondary && m_secondary->send(data))
		{
			return readings.size();
		}
		if (m_primary && m_primary->send(data))
		{
			m_failedOver = false;
			return readings.size();
		}
	}
	else
	{
		if (m_primary && m_primary->send(data))
		{
			return readings.size();
		}
		if (m_secondary && m_secondary->send(data))
		{
			m_failedOver = true;
			return readings.size();
		}
	}
	return 0;

}

/**
 * Format a reading to send
 *
 * @param value	The JSON formatted reading
 * @param reading	The reading to format
 */
void HttpNorth::getReadingString(string& value, const Reading& reading)
{
	// Convert reading data into JSON string
	value.append("{\"timestamp\" : \"" + reading.getAssetDateUserTime(Reading::FMT_STANDARD) + "Z" + "\"");
	value.append(",\"asset\" : \"" + reading.getAssetName() + "\"");
	value.append(",\"readings\" : {");

	// Get reading data
	const vector<Datapoint*> data = reading.getReadingData();

	bool first = true;
	for (vector<Datapoint*>::const_iterator it = data.begin(); it != data.end(); ++it)
	{
		if (first)
			first = false;
		else
			value.append(",");
		if ((*it)->getData().getType() == DatapointValue::T_STRING)
		{
			value.append("\"" + (*it)->getName() + "\": " + (*it)->getData().toStringValue());
		}
		else
		{
			value.append("\"" + (*it)->getName() + "\": " + (*it)->getData().toString());
		}
	}

	value.append("}}");
}

/**
 * Constructor for the HttpStream class
 *
 * @param config	The plugin configuration
 * @param url		The url to connect to
 */

HttpNorth::HttpStream::HttpStream(ConfigCategory *config, string& url) : m_sender(NULL)
{
	/**
	 * Handle the HTTP(S) parameters here
	 */
	unsigned int retrySleepTime = atoi(config->getValue("retrySleepTime").c_str());
	unsigned int maxRetry = atoi(config->getValue("maxRetry").c_str());
	unsigned int timeout = atoi(config->getValue("HttpTimeout").c_str());
	/**
	 * Extract host, port, path from URL
	 */
	size_t findProtocol = url.find_first_of(":");
	string protocol = url.substr(0,findProtocol);

	string tmpUrl = url.substr(findProtocol + 3);
	size_t findPort = tmpUrl.find_first_of(":");
	string hostName = tmpUrl.substr(0, findPort);

	size_t findPath = tmpUrl.find_first_of("/");
	string port = tmpUrl.substr(findPort + 1 , findPath - findPort -1);
	m_path = tmpUrl.substr(findPath);

	/**
	 * Allocate the HTTP(S) handler for "Hostname : port",
	 * connect_timeout and request_timeout.
	 */
	string hostAndPort(hostName + ":" + port);	

	if (protocol == string("http"))
		m_sender = new SimpleHttp(hostAndPort,
							timeout,
							timeout,
							retrySleepTime,
							maxRetry);
	else if (protocol == string("https"))
		m_sender = new SimpleHttps(hostAndPort,
							 timeout,
							 timeout,
							 retrySleepTime,
							 maxRetry);

	m_header.push_back(pair<string, string>("Content-Type", "application/json"));
}

/**
 * Desstructor fo the HttpStream
 */
HttpNorth::HttpStream::~HttpStream()
{
	if (m_sender)
		delete m_sender;
}

/**
 * Send the given payload on this HttpStream
 *
 * @param data	The JSON payload
 * @return bool	The status of the send operation.
 */
bool HttpNorth::HttpStream::send(const string& data)
{
	try
	{
		int res = m_sender->sendRequest("POST", m_path, m_header, data);
		if (res != 200 && res != 204 && res != 201)
		{
			Logger::getLogger()->error("http-north C plugin: Sending JSON readings HTTP(S) error: %d", res);
			return false;
		}

		Logger::getLogger()->info("http-north C plugin: Successfully sent readings");

		// Return number of sent readings to the caller
		return true;
	}
	catch (const std::exception& e)
	{
		Logger::getLogger()->error("http-north C plugin: Sending JSON data exception: %s", e.what());
		return false;
	}
}
