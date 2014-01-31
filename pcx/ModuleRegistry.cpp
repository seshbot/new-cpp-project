#include <pcx/ModuleRegistry.h>


namespace pcx
{
   //
   // registration
   //

   ModuleRegistry::Registration::Registration(ModuleRegistry & registry, std::string moduleName)
   : registry_(registry)
   , moduleName_(moduleName)
   {
   }

   ModuleRegistry::Registration & ModuleRegistry::Registration::withDependency(std::string dependsOn)
   {
      registry_.addDependency(moduleName_, dependsOn);
      return *this;
   }

   //
   // module registry
   //

   ModuleRegistry::~ModuleRegistry()
   {
   }

   void ModuleRegistry::startup(IConfiguration const & config, ServiceRegistry & services)
   {
      for (auto moduleName : moduleIds_)
      {
         initialiseObject(moduleName);
      }

      forEach([&](Module & m)
      {
         m.startup(config, services);
      });
   }

   void ModuleRegistry::shutdown()
   {
      forEach([&](Module & m){ m.shutdown(); });
   }

   void ModuleRegistry::restart(IConfiguration const & config, ServiceRegistry & services)
   {
      forEach([&](Module & m){ m.restart(config, services); });
   }

   void ModuleRegistry::addDependency(std::string dependent, std::string dependsOn)
   {
      LOG(debug) << "Registering module dependency '" << dependent << "' -> '" << dependsOn << "'";
      moduleDependencies_[dependent].insert(dependsOn);
   }

} // namespace pcx

