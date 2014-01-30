#ifndef PCX_MODULE_REGISTRY_H
#define PCX_MODULE_REGISTRY_H

#include <vector>
#include <pcx/Configuration.h>
#include <pcx/ServiceRegistry.h>

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
   class ModuleRegistry {
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
      template <typename ModuleT>
      struct Registration {
         Registration(std::string moduleName) {}
         Registration<ModuleT> & withDependency(std::string moduleName) { return *this; }
      };

      template <typename ModuleT>
      Registration<ModuleT> add(std::string name) {}

   private:
      std::vector<Module*> modules_;
   };

} // namespace game

#endif // #ifndef PCX_MODULE_REGISTRY_H
