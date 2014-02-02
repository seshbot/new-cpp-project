#ifndef PCX_SERVICE_REGISTRY_H
#define PCX_SERVICE_REGISTRY_H

#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>

#include <pcx/Logging.h>
#include <pcx/impl/BaseLazyFactory.h>

#include "Utils.h"

namespace pcx
{
   class ServiceRegistry;
   class ServiceRegistration
   {
   public:
      ServiceRegistration(ServiceRegistry& serviceRegistry, std::type_index const & typeInfo);

      template <typename TService>
      ServiceRegistration dependsOn();

   private:
      ServiceRegistration dependsOn(std::type_index const & typeInfo);

      ServiceRegistry& serviceRegistry_;
      std::type_index typeIndex_;
   };

   /**
    * @brief The ServiceRegistry class initialises services on-demand, incorporating
    * inter-service dependencies.
    * Non-constructed services should have constructors accepting a ServiceRegistry& parameter
    */
   class ServiceRegistry : public BaseLazyFactory<ServiceRegistry, std::type_index>
   {
   public:
      ~ServiceRegistry();

      void addDependency(std::type_index const & dependent, std::type_index const & dependsOn);

      template <typename ServiceT>
      ServiceRegistration add();
      template <typename ServiceT>
      ServiceRegistration add(ServiceT & service);
      template <typename ServiceT>
      ServiceRegistration add(std::unique_ptr<ServiceT> service);

      template <typename ServiceT>
      ServiceT & find();

      template <typename TService>
      bool exists() const;

   private:
      typedef std::unordered_set<std::type_index> IdCollectionT;
      typedef std::unordered_map<std::type_index, IdCollectionT> DependenciesMapT;

      DependenciesMapT serviceDependencies_;

      void initialiseObjectDependencies(std::type_index objectId);

      friend class BaseLazyFactory;

      template <typename ObjectT>
      ObjectT* createObjectCallback(std::type_index);
      template <typename ObjectT>
      void initialiseObjectCallback(ObjectT &, std::type_index id, void *);
   };

   //
   // ServiceRegistration Implementation
   //

   template <typename TService>
   ServiceRegistration ServiceRegistration::dependsOn()
   {
      return dependsOn(typeid(TService));
   }

   //
   // ServiceRegistry Implementation
   //

   template <typename ServiceT>
   ServiceRegistration ServiceRegistry::add()
   {
      std::type_index id = typeid(ServiceT);
      addObject<ServiceT>(id);
      return ServiceRegistration(*this, id);
   }

   template <typename ServiceT>
   ServiceRegistration ServiceRegistry::add(ServiceT & service)
   {
      std::type_index id = typeid(ServiceT);
      addObject<ServiceT>(service, id);
      return ServiceRegistration(*this, id);
   }

   template <typename ServiceT>
   ServiceRegistration ServiceRegistry::add(std::unique_ptr<ServiceT> service)
   {
      std::type_index id = typeid(ServiceT);
      addObject<ServiceT>(std::move(service), id);
      return ServiceRegistration(*this, id);
   }

   template <typename ServiceT>
   ServiceT & ServiceRegistry::find()
   {
      if (!objectIsInitialised<ServiceT>())
      {
         // trigger dependency resolution and object construction if necessary
         initialiseObject(typeid(ServiceT), nullptr);
      }

      return findObject<ServiceT>();
   }

   template <typename ObjectT>
   ObjectT* ServiceRegistry::createObjectCallback(std::type_index id)
   {
      // init dependencies here because service initialisation is done in the constructor
      // and so dependencies need to be ready by then
      initialiseObjectDependencies(id);

      return new ObjectT(*this);
   }

   template <typename ObjectT>
   void ServiceRegistry::initialiseObjectCallback(ObjectT &, std::type_index id, void *)
   {
   }

} // namespace pcx

#endif // #ifndef PCX_SERVICE_REGISTRY_H
