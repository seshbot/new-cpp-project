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
      StartupParams params { config, services };

      for (auto moduleName : moduleIds_)
      {
         // triggers createObject(), which ensures dependencies are created first
         initialiseObject(moduleName, &params);
      }
   }

   void ModuleRegistry::shutdown()
   {
      // shutdown in reverse order of initialisation
      for (auto it = modules_.rbegin(); it != modules_.rend(); ++it)
      {
         (*it)->shutdown();
      }
   }

   void ModuleRegistry::restart(IConfiguration const & config, ServiceRegistry & services)
   {
      for (auto* m : modules_)
      {
         m->restart(config, services);
      }
   }

   void ModuleRegistry::addDependency(std::string dependent, std::string dependsOn)
   {
      LOG(debug) << "Registering module dependency '" << dependent << "' -> '" << dependsOn << "'";
      moduleDependencies_[dependent].insert(dependsOn);
   }

   void ModuleRegistry::addModuleName(std::string name)
   {
      if (moduleIds_.find(name) != moduleIds_.end())
         throw std::runtime_error(std::string("Cannot register module '") + name + "' - a module with that name is already registered");

      moduleIds_.insert(name);
   }

   void ModuleRegistry::initialiseObjectDependencies(std::string objectId, void * context)
   {
      auto & dependencies = moduleDependencies_[objectId];

      if (dependencies.size() > 0)
      {
         LOG(debug)
            << "Initialising " << dependencies.size()
            << " dependencies for module '" << objectId << "'";
      }

      for (auto dependsOn : dependencies)
      {
         initialiseObject(dependsOn, context);
      }
   }

} // namespace pcx

