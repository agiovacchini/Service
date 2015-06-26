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


#define RC_OK                               00000
#define RC_ERR                              00001

#define ITEM                                10001
#define DEPT                                10002
#define TOTAL                               10003
#define LOY_CARD                            10004
#define CARD_PREFIX                         10005

#define CART_STATE_READY_FOR_ITEM           20000
#define CART_STATE_TOTAL                    20001
#define CART_STATE_ASKING                   20002
#define CART_STATE_PAYING                   20003
#define CART_STATE_CLOSED                   20020
#define CART_STATE_TMPFILE_LOADING          20030
#define CART_STATE_STATE_NOT_INITIALIZED    20040
#define CART_STATE_ERROR                    20050
#define CART_STATE_VOIDED                   20097
#define CART_STATE_NOT_READY                20098
#define CART_NOT_FOUND                      20099


#define GEN_CART_NEW                        30001
#define GEN_CART_LOAD                       30002

#define BCODE_EAN13                         40000
#define BCODE_EAN13_PRICE_REQ               40001
#define BCODE_UPC                           40002
#define BCODE_EAN8                          40003
#define BCODE_CODE39                        40004
#define BCODE_CODE128                       40005

#define BCODE_ITEM_NOT_FOUND                41003
#define BCODE_LOYCARD                       41080
#define BCODE_NOT_RECOGNIZED                41081
#define RC_LOY_CARD_NOT_PRESENT             41090
#define RC_LOY_CARD_ALREADY_PRESENT         41091
#define RC_LOY_CARD_IN_ANOTHER_TRANSACTION  41092
#define RC_LOY_MAX_CARD_NUMBER              41099

#define WEBI_SESSION_INIT                   50001
#define WEBI_SESSION_END                    50002
#define WEBI_SESSION_VOID                   50003
#define WEBI_CUSTOMER_ADD                   50010
#define WEBI_CUSTOMER_VOID                  50011
#define WEBI_ITEM_ADD                       50020
#define WEBI_ITEM_VOID                      50021
#define WEBI_GET_TOTALS                     50030
#define WEBI_GET_CARTS_LIST                 50039
#define WEBI_GET_ALL_CART                   50040
#define WEBI_GET_ALL_CART_WITH_BARCODES     50041
#define WEBI_GET_STORE_INFO                 50079
#define WEBI_ACTION_NOT_RECOGNIZED          50080

struct Totals {
    long itemsNumber ;
    long totalAmount ;
};

struct CartRow {
    int type ;
    long quantity ;
};

struct ItemCodePrice {
    uint64_t code ;
    uint32_t price ;
    uint64_t barcode ;
    unsigned int type ;
};

typedef Totals Totals ;


struct CardSessionRow {
    uint64_t loyCode ;
    uint64_t sessionId ;
};


namespace lt = boost::log::trivial;
namespace qi = boost::spirit::qi;

template <typename Iterator>
struct keys_and_values
: qi::grammar<Iterator, std::map<std::string, std::string>()>
{
    keys_and_values()
    : keys_and_values::base_type(query)
    {
        query =  pair >> *((qi::lit(';') | '&' ) >> pair);
        pair  =  key >> -('=' >> value);
        key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
        value = +qi::char_("a-zA-Z_0-9");
    }
    qi::rule<Iterator, std::map<std::string, std::string>()> query;
    qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
    qi::rule<Iterator, std::string()> key, value;
};

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::severity_logger_mt< >)

bool fileMove(std::string fileOri, std::string fileDest) ;
bool fileDelete(std::string pFileName) ;

#endif
