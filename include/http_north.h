#ifndef	_HTTP_NORTH_H
#define _HTTP_NORTH_H

#include <string>
#include "simple_https.h"
#include "simple_http.h"
#include "reading.h"
#include "config_category.h"
#include <vector>
#include <mutex>
#include <python_script.h>

/**
 * THe HTTP North C++ class
 *
 * The class support two stream, a primary and secondary. It will switch between
 * them if a connection fails to the current stream.
 */
class HttpNorth {
	public:
		HttpNorth(ConfigCategory *config);
		~HttpNorth();

		uint32_t		send(const std::vector<Reading *> readings);
	private:
		class HttpStream {

			public:
				HttpStream(ConfigCategory *config, std::string& url);
				~HttpStream();
				void		addHeader(const std::string& name, const std::string& value);
				void		setProxy(const std::string& proxy);
				bool		send(const std::string& data);
				void		setAuth(const std::string& username,
							const std::string& password);
			private:
				std::vector<std::pair<std::string, std::string>>
						m_header;
				HttpSender	*m_sender;
				std::string	m_path;
		};
		void		getReadingString(std::string& value, const Reading& reading);
		bool		sendData(const std::string& data);
		HttpStream	*m_primary;
		HttpStream	*m_secondary;
		bool		m_failedOver;
		std::mutex	m_mutex;
		std::string	m_script;
		std::string	m_content;
		PythonScript	*m_python;
		std::string	m_username;
		std::string	m_password;
};
#endif
