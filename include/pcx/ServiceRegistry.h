#ifndef PCX_SERVICE_LOCATOR_H
#define PCX_SERVICE_LOCATOR_H

#include <string>
#include <exception>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <typeindex>


namespace pcx
{
   class ServiceRegistry;
   class ServiceRegistration
   {
   public:
      ServiceRegistration(std::type_info const & typeInfo, ServiceRegistry& ServiceRegistry);
      ServiceRegistration(ServiceRegistration const & other);

      template <typename TService>
      ServiceRegistration DependsOn()
      {
         return DependsOn(typeid(TService));
      }

   private:
      ServiceRegistration DependsOn(std::type_info const & typeInfo);

      class Impl;
      std::shared_ptr<Impl> impl_;
   };

   class ServiceRegistry
   {
   public:
      ~ServiceRegistry();

      void AddDependency(std::type_info const & dependent, std::type_info const & dependsOn);

      template <typename TService>
      ServiceRegistration Register()
      {
         return RegisterImpl<TService>(
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
      ServiceRegistration Register(std::unique_ptr<TService> service)
      {
         TService* rawPtr = service.release();
         try
         {
            return RegisterImpl<TService>(
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
      ServiceRegistration Register(TService & service)
      {
         TService* servicePtr = &service;
         return RegisterImpl<TService>(
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
      TService * GetService()
      {
         return GetService<TService>(typeid(TService));
      }

      template <typename TService>
      bool ServiceExists() const
      {
         return ServiceExists<TService>(typeid(TService));
      }

   private:
      void Initialise(std::type_index const & typeIndex);

      template <typename TService, typename TFactory, typename TDeleter>
      ServiceRegistration RegisterImpl(TFactory factory, TDeleter deleter)
      {
         auto const & typeInfo = typeid(TService);
         // delete existing data
         auto it = typedData_.find(typeInfo);
         if (it != typedData_.end())
            throw std::runtime_error((std::string("Service '") + std::string(typeInfo.name()) + std::string("' already registered")).c_str());

         typedData_[typeInfo] = CreateDataHolder<TService>(factory, deleter);

         return ServiceRegistration(typeInfo, *this);
      }

      template <typename TService>
      TService * GetService(std::type_info const & typeInfo)
      {
         if (typedData_.find(typeInfo) == typedData_.end())
         {
            throw std::runtime_error((std::string("Service '") + typeInfo.name() + std::string("' not registered")).c_str());
         }

         // wont do anything if already initialised
         Initialise(typeInfo);

         auto* service = GetDataHolderData<TService>(typedData_[typeInfo]);
         if (nullptr == service) throw std::runtime_error("Service not initialised");
         
         return service;
      }

      template <typename TService>
      bool ServiceExists(std::type_info const & typeInfo) const
      {
         return typedData_.find(typeInfo) != typedData_.end();
      }

      struct Initialiser
      {
         virtual void* Initialise(ServiceRegistry& services) = 0;
      };

      struct Deleter
      {
         virtual void Delete(void* ptr) = 0;
      };

      enum EState { Uninitialised, Initialising, Initialised };
      typedef std::tuple<void*, Initialiser*, Deleter*, std::string, EState> ServiceInfoT;
      typedef std::unordered_map<std::type_index, ServiceInfoT> TypedDataContainer;
      typedef std::unordered_set<std::type_index> TypedDataSetT;
      typedef std::unordered_map<std::type_index, TypedDataSetT> TypedDataDepenencyContainer;

      template <typename TService, typename TFactory, typename TDeleter>
      static ServiceInfoT CreateDataHolder(TFactory factory, TDeleter deleter)
      {
         struct TypedInitialiser : public Initialiser
         {
            TypedInitialiser(TFactory factory) : factory_(factory) { }
            virtual void* Initialise(ServiceRegistry& services) { return factory_(services); }
            TFactory factory_;
         };

         struct TypedDeleter : public Deleter
         {
            TDeleter deleter_;
            TypedDeleter(TDeleter deleter) : deleter_(deleter) { }
            virtual void Delete(void* ptr) { deleter_(static_cast<TService*>(ptr)); }
         };

         return ServiceInfoT(nullptr, new TypedInitialiser(factory), new TypedDeleter(deleter), typeid(TService).name(), Uninitialised);
      }

      template <typename TService>
      static TService* GetDataHolderData(ServiceInfoT& dataHolder)
      {
         return static_cast<TService*>(std::get<0>(dataHolder));
      }

      static void DeleteDataHolder(ServiceInfoT& dataHolder)
      {
         auto ptr = std::get<0>(dataHolder);
         auto locator = std::get<1>(dataHolder);
         auto deleter = std::get<2>(dataHolder);
         deleter->Delete(ptr);

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

#endif // #ifndef PCX_SERVICE_LOCATOR_H
