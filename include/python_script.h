#ifndef _PYTHON_SCRIPT_H
#define _PYTHON_SCRIPT_H
/**
 * Python script encapsulation for noprth plugin
 *
 * Copyright (c) 2022 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch
 */

#include <logger.h>
#include <reading.h>
#include <pyruntime.h>

class PythonScript {
	public:
		PythonScript(const std::string& name);
		~PythonScript();
		bool			setScript(const std::string& file);
		bool			execute(Reading *reading, std::string& payload);
	private:

		std::string		m_script;
		Logger			*m_logger;
		PythonRuntime		*m_runtime;
		PyObject		*m_module;
};

#endif
