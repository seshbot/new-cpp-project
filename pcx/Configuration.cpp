#include <pcx/Configuration.h>

#include "impl/FileConfiguration.h"

namespace pcx
{
   std::unique_ptr<IConfiguration> createEmptyConfiguration()
   {
      struct EmptyConfiguration : public IConfiguration
      {
         virtual ISection & section(std::string const &)             { throw std::runtime_error("cannot get named sections from empty configuration"); }
         virtual ISection const & section(std::string const &) const { throw std::runtime_error("cannot get named sections from empty configuration"); }
         virtual std::vector<std::string> sectionNames() const       { return {}; }
         virtual bool sectionExists(std::string const & name) const  { return false; }

         virtual IOption & option(std::string const & name)          { throw std::runtime_error("cannot get named options from empty configuration"); }
         virtual IOption const & option(std::string const & name) const { throw std::runtime_error("cannot get named options from empty configuration"); }
         virtual std::vector<std::string> optionNames() const        { return {}; }
         virtual bool optionExists(std::string const & name) const   { return false; }
      };

      return std::unique_ptr<IConfiguration>(new EmptyConfiguration);
   }

   std::unique_ptr<IConfiguration> createFileConfiguration(std::string filename)
   {
      return std::unique_ptr<IConfiguration>(new impl::FileConfiguration(filename));
   }

} // namespace pcx

