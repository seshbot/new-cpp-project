#include <pcx/ModuleRegistry.h>


namespace pcx
{
   ModuleRegistry::~ModuleRegistry()
   {
   }

   void ModuleRegistry::startup(IConfiguration const & config, ServiceRegistry & services)
   {
      forEach([&](Module & m){ m.startup(config, services); });
   }

   void ModuleRegistry::shutdown()
   {
      forEach([&](Module & m){ m.shutdown(); });
   }

   void ModuleRegistry::restart(IConfiguration const & config, ServiceRegistry & services)
   {
      forEach([&](Module & m){ m.restart(config, services); });
   }

} // namespace pcx

