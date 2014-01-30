#include <pcx/Configuration.h>

#include "configurations/FileConfiguration.h"

namespace pcx
{
   std::unique_ptr<IConfiguration> createFileConfiguration(std::string filename)
   {
      return std::unique_ptr<IConfiguration>(new impl::FileConfiguration(filename));
   }
} // namespace pcx

