#ifndef PCX_UTILS_H
#define PCX_UTILS_H

#include <string>
#include <typeindex>

namespace pcx
{
   /// Returns user-friendly name
   std::string demangle_name(std::string name);

} // namespace pcx

namespace std
{
   std::ostream & operator<<(std::ostream & stream, std::type_index const & t);
}

#endif // #ifndef PCX_UTILS_H
