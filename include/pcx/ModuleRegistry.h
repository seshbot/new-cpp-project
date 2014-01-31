#ifndef PCX_MODULE_REGISTRY_H
#define PCX_MODULE_REGISTRY_H

#include <vector>
#include <pcx/Logging.h>
#include <pcx/Configuration.h>
#include <pcx/ServiceRegistry.h>

#include <pcx/impl/BaseDependencyContainer.h>

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

      virtual void startup(IConfiguration const & config, ServiceRegistry const & services) {}
      virtual void shutdown() {}
      virtual void restart(IConfiguration const & config, ServiceRegistry const & services) {}

      virtual void update(double timeSinceLast) {}
   };


   /**
    * @brief The ModuleRegistry class is where all modules and their dependencies are
    * registered. It coordinates modules by:
    *  - initialising them as appropriate
    *  - starting them up, shutting them down, restarting them
    *  - providing a mechanism to express module dependencies (startup order)
    */
   class ModuleRegistry : public BaseDependencyContainer<ModuleRegistry, std::string> {
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

      void addDependency(std::string dependent, std::string dependsOn);

   private:
      friend class BaseDependencyContainer;

      virtual void initialiseObjectDependencies(std::string objectId)
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
            initialiseObject(dependsOn);
         }
      }

      template <typename ObjectT>
      ObjectT* createObject(std::string objectId)
      {
         LOG(debug) << "Creating instance of module '" << objectId << "'";
         auto * module = new ObjectT();
         modules_.push_back(static_cast<Module*>(module));
         return module;
      }

      void addModuleName(std::string name)
      {
         if (moduleIds_.find(name) != moduleIds_.end())
            throw std::runtime_error(std::string("Cannot register module '") + name + "' - a module with that name is already registered");

         moduleIds_.insert(name);
      }

      std::vector<Module*> modules_;

      typedef std::unordered_set<std::string> ModuleIdSetT;
      ModuleIdSetT moduleIds_;
      typedef std::unordered_map<std::string, ModuleIdSetT> ModuleDependenciesMapT;
      ModuleDependenciesMapT moduleDependencies_;
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

} // namespace game

#endif // #ifndef PCX_MODULE_REGISTRY_H
