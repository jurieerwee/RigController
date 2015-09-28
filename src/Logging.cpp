/*
 * Logging.cpp
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#include "Logging.h"
#include <iostream>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/to_log.hpp>


namespace logging = boost::log;
namespace src = boost::log::sources;
using namespace std;

void initLogger()
{
	logging::add_file_log(logging::keywords::file_name="./logfiles/standardlog_%Y%m%d_%H%M.log",logging::keywords::format = "[%TimeStamp%] <%Severity%>: %Message%", logging::keywords::auto_flush = true);
	logging::add_console_log(cout);
	logging::add_common_attributes();
}
