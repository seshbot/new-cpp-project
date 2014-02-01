#ifndef PCX_DEPENDENCY_CONTAINER_H
#define PCX_DEPENDENCY_CONTAINER_H

#include <string>
#include <exception>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <typeindex>

#include <boost/lexical_cast.hpp>

#include <pcx/Utils.h>

namespace pcx
{
   /**
    * @brief The BaseLazyFactory class encapsulates functionality relating to
    * on-demand object initialisation.
    * All objects added must have an identifier. When required the inheriting class
    * will be required to create and then initialise the requisite object via the
    * createObjectCallback<T>(id, ctx) and initialiseObjectCallback(obj, id, ctx) methods.
    *
    * Use like this:
    * class MyContainer : public BaseDependencyContainer<MyContainer, std::string> {
    *    template <typename ObjectT>
    *    ObjectT* createObjectCallback(std::string id, void* context) {
    *       // only called for non-existent objects (addObject<T>(), not addObject<T>(obj))
    *       // create object of type ObjectT for this container
    *       // perhaps ensuring dependencies are met by calling
    *       // initialiseObject(id, context)
    *    }
    *    template <typename ObjectT>
    *    ObjectT* initialiseObjectCallback(ObjectT & object, std::string id, void* context) {
    *       // called for all objects in dependency order
    *       // context contains whatever information was passed into initialiseObject(id, ctx)
    *    }
    * };
    *
    * MyContainer cont;
    * cont.addObject<MyService>("svc1");
    * cont.addObject<MyOtherService>("svc2");
    *
    * cont.initialiseObject("svc1", nullptr);
    *
    * cont.findObject<MyService>();
    * // will invoke MyContainer::createObject<MyService>("svc1")
    */
   template <typename ContainerT, typename IdentifierT>
   class BaseLazyFactory
   {
   protected:
      virtual ~BaseLazyFactory();

      template <typename ObjectT>
      void addObject(IdentifierT objectId)
      {
         auto * container = static_cast<ContainerT*>(this);
         addObjectImpl<ObjectT>(objectId,
            [=](void * context)
            {
               auto * newObject = container->template createObjectCallback<ObjectT>(objectId, context);
               container->template initialiseObjectCallback<ObjectT>(*newObject, objectId, context);
               return newObject;
            },
            [=](ObjectT* object)
            {
               delete object;
            });
      }

      template <typename ObjectT>
      void addObject(ObjectT & object, IdentifierT objectId)
      {
         auto * container = static_cast<ContainerT*>(this);
         ObjectT* objectPtr = &object;
         addObjectImpl<ObjectT>(objectId,
            [=](void * context)
            {
               container->template initialiseObjectCallback<ObjectT>(*objectPtr, objectId, context);
               return objectPtr;
            },
            [=](ObjectT* object)
            {
               // do nothing, we dont own this object
            });
      }

      template <typename ObjectT>
      void addObject(std::unique_ptr<ObjectT> object, IdentifierT objectId)
      {
         auto * container = static_cast<ContainerT*>(this);
         ObjectT* rawPtr = object.release();
         try
         {
            addObjectImpl<ObjectT>(objectId,
               [=](void * context)
               {
                  container->template initialiseObjectCallback<ObjectT>(*rawPtr, objectId, context);
                  return rawPtr;
               },
               [](ObjectT* object)
               {
                  delete object;
               });
         }
         catch (...)
         {
            delete rawPtr;
            throw;
         }
      }

      template <typename ObjectT>
      ObjectT & findObject()
      {
         return *static_cast<ObjectT*>(findObjectImpl(typeid(ObjectT)));
      }

      template <typename ObjectT>
      ObjectT & findObject(IdentifierT id)
      {
         auto typeIndex = getTypeForId(id);

         if (typeIndex != typeid(ObjectT))
            throw std::runtime_error(
               std::string("Error finding object - registered object of unexpected type '") +
               demangle_name(typeIndex.name()) + "', expected '" +
               demangle_name(typeid(ObjectT).name()) + "'");

         return *static_cast<ObjectT*>(findObjectImpl(typeIndex));
      }

      void initialiseObject(IdentifierT id, void * context)
      {
         auto typeIndex = getTypeForId(id);
         initialiseObjectImpl(typeIndex, context);
      }

   private:
      struct Initialiser
      {
         virtual void* initialise(void * context) = 0;
      };

      struct Deleter
      {
         virtual void destroy(void* ptr) = 0;
      };

      enum EState { Uninitialised, Initialising, Initialised };
      typedef std::tuple<void*, Initialiser*, Deleter*, std::string, EState, IdentifierT> ObjectInfoT;
      typedef std::unordered_map<std::type_index, ObjectInfoT> TypedDataContainer;

      void initialiseObjectImpl(std::type_index const & typeIndex, void * context);

      template <typename ObjectT, typename TFactory, typename TDeleter>
      void addObjectImpl(IdentifierT objectId, TFactory factory, TDeleter deleter)
      {
         auto const & typeInfo = typeid(ObjectT);
         // delete existing data
         auto it = typedData_.find(typeInfo);
         if (it != typedData_.end())
            throw std::runtime_error((std::string("Object '") + demangle_name(typeInfo.name()) + std::string("' already registered")).c_str());

         typedData_[typeInfo] = createDataHolder<ObjectT>(objectId, factory, deleter, Uninitialised);
      }

      std::type_index getTypeForId(IdentifierT id)
      {
         for (auto pair : typedData_)
         {
            auto itId = std::get<5>(pair.second);
            if (itId == id)
            {
               return pair.first;
            }
         }

         throw std::runtime_error((std::string("Object ID '") + boost::lexical_cast<std::string>(id) + std::string("' not registered")).c_str());
      }

      void * findObjectImpl(std::type_index const & typeIndex)
      {
         if (typedData_.find(typeIndex) == typedData_.end())
         {
            throw std::runtime_error((std::string("Object '") + demangle_name(typeIndex.name()) + std::string("' not registered")).c_str());
         }

         auto* object = getDataHolderData(typedData_[typeIndex]);
         if (nullptr == object) throw std::runtime_error(std::string("Cannot find object of type '") + demangle_name(typeIndex.name()) + "' - object has not yet been initialised");

         return object;
      }

      template <typename ObjectT, typename TFactory, typename TDeleter>
      static ObjectInfoT createDataHolder(IdentifierT objectId, TFactory factory, TDeleter deleter, EState initialState)
      {
         struct TypedInitialiser : public Initialiser
         {
            TypedInitialiser(TFactory factory) : factory_(factory) { }
            virtual void* initialise(void * context) { return factory_(context); }
            TFactory factory_;
         };

         struct TypedDeleter : public Deleter
         {
            TDeleter deleter_;
            TypedDeleter(TDeleter deleter) : deleter_(deleter) { }
            virtual void destroy(void* ptr) { deleter_(static_cast<ObjectT*>(ptr)); }
         };

         return ObjectInfoT(nullptr, new TypedInitialiser(factory), new TypedDeleter(deleter), demangle_name(typeid(ObjectT).name()), initialState, objectId);
      }

      static void* getDataHolderData(ObjectInfoT& dataHolder)
      {
         return std::get<0>(dataHolder);
      }

      static void deleteDataHolder(ObjectInfoT& dataHolder)
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

   public:

      // this is for debugging purposes only, hence the unweildy signature
      // callback(id, data_type_info, state, data)
      void forEachObject(std::function<void(IdentifierT, std::string, std::string, void*)> callback)
      {
         for (auto pair : typedData_)
         {
            auto & dataHolder = pair.second;
            auto & data       = std::get<0>(dataHolder);
            auto & typeName   = std::get<3>(dataHolder);
            auto & state      = std::get<4>(dataHolder);
            auto & id         = std::get<5>(dataHolder);

            std::string stateStr =
               state == Initialised ? "initialised" :
               state == Uninitialised ? "uninitialised" :
               state == Initialising ? "initialising" :
               "UNKNOWN!";
            callback(id, typeName, stateStr, data);
         }
      }
   };


   //
   // implementation
   //

   template <typename ContainerT, typename IdentifierT>
   BaseLazyFactory<ContainerT, IdentifierT>::~BaseLazyFactory()
   {
      auto it = typedData_.begin();
      auto itEnd = typedData_.end();
      for (; it != itEnd; ++it)
      {
         deleteDataHolder(it->second);
      }
   }

   template <typename ContainerT, typename IdentifierT>
   void BaseLazyFactory<ContainerT, IdentifierT>::initialiseObjectImpl(std::type_index const & typeIndex, void * context)
   {
      // ObjectInfoT members:
      //  - 0: data  (void*)
      //  - 1: init  (Initialiser*)
      //  - 2: delt  (Deleter*)
      //  - 3: type_name (std::string)
      //  - 4: state (EState)
      //  - 5: id    (IdentifierT)

      auto it = typedData_.find(typeIndex);
      if (it == typedData_.end())
      {
         throw std::runtime_error((std::string("Cannot initialise object of type '") + demangle_name(typeIndex.name()) + "': object type not registered").c_str());
      }

      auto& dataHolder = it->second;

      //
      // drop out if already initialised
      //
      auto state = std::get<4>(dataHolder);
      if (Initialised == state) return;
      if (Initialising == state)
      {
         throw std::runtime_error((std::string("Object dependency loop detected with object of type '") + demangle_name(typeIndex.name()) + "'").c_str());
      }

      //
      // initialise object
      //

      std::get<4>(dataHolder) = Initialising;

      auto factory = std::get<1>(dataHolder);
      auto* newObject = factory->initialise(context);
      std::get<0>(dataHolder) = newObject;

      //
      // initialisation complete
      //

      std::get<4>(dataHolder) = Initialised;
   }

} // namespace pcx

#endif // #ifndef PCX_IMPL_TYPED_OBJECT_REGISTRY_H
