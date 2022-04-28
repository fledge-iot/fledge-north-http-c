/*      
 * FogLAMP python script runner
 *                             
 * Copyright (c) 2022 Dianomic Systems
 *                             
 * Released under the Apache 2.0 Licence
 *                                      
 * Author: Mark Riddoch
 */
#include <python_script.h>
#include <pyruntime.h>
#include <pythonreading.h>
#include <utils.h>
#include <string_utils.h>

#define FUNCNAME	"convert"	// Name of Python function to call

using namespace std;

/**
 * Constructor for the PythonScript class that is used to
 * convert the message payload
 *
 * @param name	The name of the south service
 */
PythonScript::PythonScript(const string& name)
{
	m_logger = Logger::getLogger();

	m_runtime = PythonRuntime::getPythonRuntime();

}

/**
 * Destructor for the Python script class
 */
PythonScript::~PythonScript()
{
}

/**
 * Called to set the script to run. Imports the Python module
 * and sets up the environment in order to facilitate the calling
 * of the Python function.
 *
 * @param text	The Python script to import
 */
bool PythonScript::setScript(const string& text)
{
	string script = text.substr(1, text.length()-2);	// Strip off surounding quotes
	StringReplaceAll(script, "\\n", "\n");
	StringReplaceAll(script, "\\\"", "\"");
	m_logger->info("Script to execute is '%s'", script.c_str());

	m_runtime->execute(script);

	return true;
}

/**
 * Execute the mapping function. The function is called with a
 * readings and should return the payload to send as a string
 *
 * @param reading	The Reading to convert for sending
 * @param payload	String to populate with the payload to send
 * @return bool		True if the payload conversion was a success
 */
bool PythonScript::execute(Reading *reading, std::string& payload)
{
	PyObject *pyReading = ((PythonReading *)reading)->toPython();
	PyObject *obj = m_runtime->call(FUNCNAME, "(O)", pyReading);
	if (obj)
	{
		if (PyUnicode_Check(obj))
		{
			const char *str = PyUnicode_AsUTF8(obj);
			payload = str;
			m_logger->debug("Convert reading to '%s'", str);
			return true;
		}
		else
		{
			m_logger->error("Python script returned incorrect type");
		}
	}
	else
	{
		m_logger->error("Python script failed to execute");
	}
	return false;
}

