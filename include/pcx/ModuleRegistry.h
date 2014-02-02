#ifndef PCX_MODULE_REGISTRY_H
#define PCX_MODULE_REGISTRY_H

#include <vector>
#include <set>
#include <unordered_map>

#include <pcx/Logging.h>
#include <pcx/Configuration.h>
#include <pcx/ServiceRegistry.h>

#include <pcx/impl/BaseLazyFactory.h>

namespace pcx
{
   //
   /**
    * @brief A Module is the entry point for a related set of services and functionality.
    * This is the highest level of abstraction in an application, ideally the only thing
    * visible from the application mainline.
    */
   class Module {
   public:
      virtual ~Module() {}

      virtual void startup(IConfiguration const & config, ServiceRegistry & services) {}
      virtual void shutdown() {}
      virtual void restart(IConfiguration const & config, ServiceRegistry & services) {}

      virtual void update(double timeSinceLast) {}
   };


   /**
    * @brief The ModuleRegistry class is where all modules and their dependencies are
    * registered. It coordinates modules by:
    *  - initialising them as appropriate
    *  - starting them up, shutting them down, restarting them
    *  - providing a mechanism to express module dependencies (startup order)
    */
   class ModuleRegistry : public BaseLazyFactory<ModuleRegistry, std::string> {
   public:
      ~ModuleRegistry();

      void startup(IConfiguration const & config, ServiceRegistry & services);
      void shutdown();
      void restart(IConfiguration const & config, ServiceRegistry & services);

      void forEach(std::function<void(Module &)> func) {
         for (auto * c : modules_) func(*c);
      }

      // little helper class to allow fluid configuration of registered modules
      // e.g., reg.add<MyModule>("my").withDependency("x").withDependency("y");
      class Registration {
      public:
         Registration(ModuleRegistry & registry, std::string moduleName);
         Registration & withDependency(std::string dependsOn);

      private:
         ModuleRegistry & registry_;
         std::string moduleName_;
      };

      template <typename ModuleT>
      Registration add(std::string name);
      template <typename ModuleT>
      Registration add(ModuleT & module, std::string name);
      template <typename ModuleT>
      Registration add(std::unique_ptr<ModuleT> module, std::string name);

      template <typename ModuleT>
      ModuleT & find();

      void addDependency(std::string dependent, std::string dependsOn);

   private:
      typedef std::set<std::string> ModuleIdCollectionT;
      typedef std::unordered_map<std::string, ModuleIdCollectionT> ModuleDependenciesMapT;

      ModuleIdCollectionT moduleIds_;
      ModuleDependenciesMapT moduleDependencies_;
      std::vector<Module*> modules_;

      void addModuleName(std::string name);

      void initialiseObjectDependencies(std::string objectId, void * context);

      struct StartupParams {
         IConfiguration const & config;
         ServiceRegistry & services;
      };

      friend class BaseLazyFactory;

      template <typename ObjectT>
      ObjectT* createObjectCallback(std::string objectId);
      template <typename ObjectT>
      void initialiseObjectCallback(ObjectT & object, std::string objectId, void * context);
   };

   //
   // implementation
   //


   template <typename ModuleT>
   ModuleRegistry::Registration ModuleRegistry::add(std::string name)
   {
      LOG(debug) << "Registering module '" << name << "'";

      addModuleName(name);
      addObject<ModuleT>(name);
      return Registration(*this, name);
   }

   template <typename ModuleT>
   ModuleRegistry::Registration ModuleRegistry::add(ModuleT & module, std::string name)
   {
      LOG(debug) << "Registering module '" << name << "'";

      addModuleName(name);
      addObject<ModuleT>(module, name);
      return Registration(*this, name);
   }

   template <typename ModuleT>
   ModuleRegistry::Registration ModuleRegistry::add(std::unique_ptr<ModuleT> module, std::string name)
   {
      LOG(debug) << "Registering module '" << name << "'";

      addModuleName(name);
      addObject<ModuleT>(std::move(module), name);
      return Registration(*this, name);
   }

   template <typename ModuleT>
   ModuleT & ModuleRegistry::find()
   {
      return findObject<ModuleT>();
   }

   template <typename ObjectT>
   ObjectT* ModuleRegistry::createObjectCallback(std::string objectId)
   {
      LOG(debug) << "Creating instance of module '" << objectId << "'";
      auto * module = new ObjectT();
      return module;
   }

   template <typename ObjectT>
   void ModuleRegistry::initialiseObjectCallback(ObjectT & object, std::string objectId, void * context)
   {
      LOG(debug) << "Starting up module '" << objectId << "'";

      initialiseObjectDependencies(objectId, context);

      StartupParams & startupParams = *reinterpret_cast<StartupParams*>(context);
      object.startup(startupParams.config, startupParams.services);

      modules_.push_back(&object);
   }

} // namespace game

#endif // #ifndef PCX_MODULE_REGISTRY_H
