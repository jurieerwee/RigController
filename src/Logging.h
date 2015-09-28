/*
 * Logging.h
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#ifndef SRC_LOGGING_H_
#define SRC_LOGGING_H_

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

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::severity_logger_mt<>)

void initLogger();

#endif /* SRC_LOGGING_H_ */
