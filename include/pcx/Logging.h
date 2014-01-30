#ifndef PCX_LOGGING_H
#define PCX_LOGGING_H

#include <boost/filesystem.hpp>    // to stop link errors


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#pragma clang diagnostic pop

#define LOG(lvl) BOOST_LOG_TRIVIAL(lvl)


#endif // PCX_LOGGING
