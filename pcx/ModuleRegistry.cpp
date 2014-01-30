#include <pcx/ModuleRegistry.h>


namespace pcx
{
   ModuleRegistry::~ModuleRegistry()
   {
   }

   void ModuleRegistry::startup(Configuration const & config, ServiceRegistry & services) {
      forEach([&](Module & m){ m.startup(config, services); });
   }

   void ModuleRegistry::shutdown()
   {
      forEach([&](Module & m){ m.shutdown(); });
   }

   void ModuleRegistry::restart(Configuration const & config, ServiceRegistry & services)
   {
      forEach([&](Module & m){ m.restart(config, services); });
   }

} // namespace pcx

