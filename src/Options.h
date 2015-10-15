/*
 * Options.h
 *
 *  Created on: 15 Oct 2015
 *      Author: Jurie
 */

#ifndef SRC_OPTIONS_H_
#define SRC_OPTIONS_H_

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

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
namespace po = boost::program_options;

class Options {
public:
	Options();
	virtual ~Options();
	bool initOptions();
	po::variables_map& getVM();


private:
	po::variables_map vm;
	src::severity_logger_mt<>& lg;
};

#endif /* SRC_OPTIONS_H_ */
