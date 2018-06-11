//
//  BaseTypes.h
//  PromoCalculator
//
//  Created by Andrea Giovacchini on 05/01/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//

#ifndef PromoCalculator_BaseTypes_h
#define PromoCalculator_BaseTypes_h


#undef __STRICT_ANSI__ 

#if !defined _WIN32
	#define BOOST_ALL_DYN_LINK
	#define BOOST_LOG_DYN_LINK
#endif

//https://groups.google.com/forum/#!topic/cpp-netlib/gZ280gQbqcM
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/channel_feature.hpp>
#include <boost/log/sources/channel_logger.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/exception_handler.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <sstream>


namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

#if defined _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <winbase.h>
	#include <aclapi.h>
	#include <sddl.h>
#endif 

namespace lt = boost::log::trivial;
namespace qi = boost::spirit::qi;
template <typename Iterator>
struct keys_and_values
	: qi::grammar<Iterator, std::map<std::string, std::string>()>
{
	keys_and_values()
		: keys_and_values::base_type(query)
	{
		query = pair >> *((qi::lit(';') | '&') >> pair);
		pair = key >> -('=' >> value);
		key = qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
		value = +qi::char_("a-zA-Z_0-9");
	}
	qi::rule<Iterator, std::map<std::string, std::string>()> query;
	qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
	qi::rule<Iterator, std::string()> key, value;
};
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::severity_logger_mt< >)
bool fileMove(std::string fileOri, std::string fileDest);
bool fileDelete(std::string pFileName);

#define RC_OK                               00000
#define RC_ERR                              00001

#endif