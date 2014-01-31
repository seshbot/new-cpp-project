#ifndef PCX_SERVICE_REGISTRY_H
#define PCX_SERVICE_REGISTRY_H

#include <string>
#include <exception>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <typeindex>

#include "Utils.h"

namespace pcx
{
   class ServiceRegistry;
   class ServiceRegistration
   {
   public:
      ServiceRegistration(std::type_info const & typeInfo, ServiceRegistry& ServiceRegistry);
      ServiceRegistration(ServiceRegistration const & other);

      template <typename TService>
      ServiceRegistration dependsOn()
      {
         return dependsOn(typeid(TService));
      }

   private:
      ServiceRegistration dependsOn(std::type_info const & typeInfo);

      class Impl;
      std::shared_ptr<Impl> impl_;
   };

   class ServiceRegistry
   {
   public:
      ~ServiceRegistry();

      void addDependency(std::type_info const & dependent, std::type_info const & dependsOn);

      template <typename TService>
      ServiceRegistration add()
      {
         return addImpl<TService>(
            [](ServiceRegistry& services)
            { 
               return new TService(services); 
            }, 
            [=](TService* service)
            { 
               delete service; 
            });
      }

      template <typename TService>
      ServiceRegistration add(std::unique_ptr<TService> service)
      {
         TService* rawPtr = service.release();
         try
         {
            return addImpl<TService>(
               [=](ServiceRegistry& services)
               { 
                  return rawPtr; 
               }, 
               [](TService* servicePtr)
               { 
                  delete servicePtr; 
               });
         }
         catch (...)
         {
            delete rawPtr;
            throw;
         }
      }

      template <typename TService>
      ServiceRegistration add(TService & service)
      {
         TService* servicePtr = &service;
         return addImpl<TService>(
            [=](ServiceRegistry& services)
            { 
               return servicePtr; 
            }, 
            [=](TService* service)
            { 
               // do nothing, we dont own this service
            });
      }

      template <typename TService>
      TService * findService()
      {
         return findService<TService>(typeid(TService));
      }

      template <typename TService>
      bool serviceExists() const
      {
         return serviceExists<TService>(typeid(TService));
      }

   private:
      void initialise(std::type_index const & typeIndex);

      template <typename TService, typename TFactory, typename TDeleter>
      ServiceRegistration addImpl(TFactory factory, TDeleter deleter)
      {
         auto const & typeInfo = typeid(TService);
         // delete existing data
         auto it = typedData_.find(typeInfo);
         if (it != typedData_.end())
            throw std::runtime_error((std::string("Service '") + demangle_name(typeInfo.name()) + std::string("' already registered")).c_str());

         typedData_[typeInfo] = createDataHolder<TService>(factory, deleter);

         return ServiceRegistration(typeInfo, *this);
      }

      template <typename TService>
      TService * findService(std::type_info const & typeInfo)
      {
         if (typedData_.find(typeInfo) == typedData_.end())
         {
            throw std::runtime_error((std::string("Service '") + demangle_name(typeInfo.name()) + std::string("' not registered")).c_str());
         }

         // wont do anything if already initialised
         initialise(typeInfo);

         auto* service = getDataHolderData<TService>(typedData_[typeInfo]);
         if (nullptr == service) throw std::runtime_error("Service not initialised");
         
         return service;
      }

      template <typename TService>
      bool serviceExists(std::type_info const & typeInfo) const
      {
         return typedData_.find(typeInfo) != typedData_.end();
      }

      struct Initialiser
      {
         virtual void* initialise(ServiceRegistry& services) = 0;
      };

      struct Deleter
      {
         virtual void destroy(void* ptr) = 0;
      };

      enum EState { Uninitialised, Initialising, Initialised };
      typedef std::tuple<void*, Initialiser*, Deleter*, std::string, EState> ServiceInfoT;
      typedef std::unordered_map<std::type_index, ServiceInfoT> TypedDataContainer;
      typedef std::unordered_set<std::type_index> TypedDataSetT;
      typedef std::unordered_map<std::type_index, TypedDataSetT> TypedDataDepenencyContainer;

      template <typename TService, typename TFactory, typename TDeleter>
      static ServiceInfoT createDataHolder(TFactory factory, TDeleter deleter)
      {
         struct TypedInitialiser : public Initialiser
         {
            TypedInitialiser(TFactory factory) : factory_(factory) { }
            virtual void* initialise(ServiceRegistry& services) override { return factory_(services); }
            TFactory factory_;
         };

         struct TypedDeleter : public Deleter
         {
            TDeleter deleter_;
            TypedDeleter(TDeleter deleter) : deleter_(deleter) { }
            virtual void destroy(void* ptr) override { deleter_(static_cast<TService*>(ptr)); }
         };

         return ServiceInfoT(nullptr, new TypedInitialiser(factory), new TypedDeleter(deleter), demangle_name(typeid(TService).name()), Uninitialised);
      }

      template <typename TService>
      static TService* getDataHolderData(ServiceInfoT& dataHolder)
      {
         return static_cast<TService*>(std::get<0>(dataHolder));
      }

      static void deleteDataHolder(ServiceInfoT& dataHolder)
      {
         auto ptr = std::get<0>(dataHolder);
         auto locator = std::get<1>(dataHolder);
         auto deleter = std::get<2>(dataHolder);
         deleter->destroy(ptr);

         // ignore 'abstract but non-virtual destructor' for Deleter and Initialiser
         // - we know what we're doing
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdelete-non-virtual-dtor"
         delete deleter;
         delete locator;
#pragma clang diagnostic pop
      }

      TypedDataContainer typedData_;
      TypedDataDepenencyContainer typedDataDependencies_;
   };

} // namespace pcx

#endif // #ifndef PCX_SERVICE_REGISTRY_H
