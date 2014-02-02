#include <pcx/ServiceRegistry.h>
#include <pcx/Logging.h>


namespace pcx
{
   //
   // ServiceRegistration
   //
   //

   ServiceRegistration::ServiceRegistration(ServiceRegistry& serviceRegistry, std::type_index const & typeIndex)
      : serviceRegistry_(serviceRegistry)
      , typeIndex_(typeIndex)
   {
   }

   ServiceRegistration ServiceRegistration::dependsOn(std::type_index const & typeIndex)
   {
      serviceRegistry_.addDependency(typeIndex_, typeIndex);
      return *this;
   }


   //
   // ServiceRegistry
   //
   //

   ServiceRegistry::~ServiceRegistry()
   {
   }

   void ServiceRegistry::addDependency(std::type_index const & dependent, std::type_index const & dependsOn)
   {
      serviceDependencies_[dependent].insert(dependsOn);
   }

   void ServiceRegistry::initialiseObjectDependencies(std::type_index objectId)
   {
      auto & dependencies = serviceDependencies_[objectId];

      if (dependencies.size() > 0)
      {
         LOG(debug)
            << "Initialising " << dependencies.size()
            << " dependencies for service '" << demangle_name(objectId.name()) << "'";
      }

      for (auto dependsOn : dependencies)
      {
         initialiseObject(dependsOn, nullptr);
      }
   }
} // namespace pcx

