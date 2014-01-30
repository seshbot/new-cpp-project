#include <pcx/Utils.h>

#include <boost/assert.hpp>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace pcx
{

   std::string demangle_name(std::string name) {
       #if defined(_MSC_VER)
           return name;
       #else
           std::string ret;
           int status = 0;
           char* demang = abi::__cxa_demangle(name.c_str(), NULL, 0, &status);
           BOOST_ASSERT(!status);

           try
           {
               ret = demang;
           } catch (...) {
               free(demang);
               throw;
           }

           free(demang);

           return ret;
       #endif
   }

} // namespace pcx

