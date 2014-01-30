#ifndef PCX_FILE_CONFIGURATION_H
#define PCX_FILE_CONFIGURATION_H

#include <pcx/Configuration.h>
#include <boost/property_tree/ptree.hpp>

namespace pcx
{
   namespace impl
   {
      class FileConfiguration : public pcx::IConfiguration
      {
      public:
         FileConfiguration(std::string const & filename);
         ~FileConfiguration();

         virtual ISection & section(std::string const & name);
         virtual ISection const & section(std::string const & name) const;
         virtual std::vector<std::string> sectionNames() const;
         virtual bool sectionExists(std::string const & name) const;

         virtual IOption & option(std::string const & name);
         virtual IOption const & option(std::string const & name) const;
         virtual std::vector<std::string> optionNames() const;
         virtual bool optionExists(std::string const & name) const;

      private:
         boost::property_tree::ptree ptree_;
         ISection * root_;
      };
   } // namespace impl
} // namespace pcx

#endif // #ifndef PCX_FILE_CONFIGURATION_H
