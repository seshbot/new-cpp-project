#ifndef PCX_IMPL_TYPED_OBJECT_REGISTRY_H
#define PCX_IMPL_TYPED_OBJECT_REGISTRY_H

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
   class TypedObjectRegistry;

   class ObjectRegistration
   {
   public:
      ObjectRegistration(std::type_info const & typeInfo, TypedObjectRegistry& registry);

      template <typename TObject>
      ObjectRegistration withDependency()
      {
         return withDependency(typeid(TObject));
      }

   private:
      ObjectRegistration withDependency(std::type_info const & typeInfo);

      std::type_info const * typeInfo_;
      TypedObjectRegistry& registry_;
   };

   class TypedObjectRegistry
   {
   public:
      ~TypedObjectRegistry();

      void addDependency(std::type_info const & dependent, std::type_info const & dependency);

      template <typename TObject>
      ObjectRegistration add()
      {
         return addImpl<TObject>(
            [](TypedObjectRegistry& registry)
            { 
               return new TObject(registry);
            }, 
            [=](TObject* object)
            { 
               delete object;
            },
            EState::Uninitialised);
      }

      // add and take ownership of an object
      template <typename TObject>
      ObjectRegistration add(std::unique_ptr<TObject> object)
      {
         TObject* rawPtr = object.release();
         try
         {
            return addImpl<TObject>(
               [=](TypedObjectRegistry& registry)
               { 
                  return rawPtr; 
               }, 
               [](TObject* objectPtr)
               { 
                  delete objectPtr;
               },
               EState::Initialised);
         }
         catch (...)
         {
            // technically if we throw we havent taken ownership
            // so perhaps we shouldnt do this?
            delete rawPtr;
            throw;
         }
      }

      // add an object we do not own
      template <typename TObject>
      ObjectRegistration add(TObject & object)
      {
         TObject* objectPtr = &object;
         return addImpl<TObject>(
            [=](TypedObjectRegistry& registry)
            { 
               return objectPtr;
            }, 
            [=](TObject* object)
            { 
               // do nothing, we dont own this object
            },
            EState::Initialised);
      }

      template <typename TObject>
      TObject * findObject()
      {
         return findObject<TObject>(typeid(TObject));
      }

      template <typename TObject>
      bool objectExists() const
      {
         return objectExists<TObject>(typeid(TObject));
      }

   private:
      struct Initialiser
      {
         virtual void* initialise(TypedObjectRegistry& objects) = 0;
      };

      struct Deleter
      {
         virtual void destroy(void* ptr) = 0;
      };

      enum EState { Uninitialised, Initialising, Initialised };
      typedef std::tuple<void*, Initialiser*, Deleter*, std::string, EState> ObjectInfoT;
      typedef std::unordered_map<std::type_index, ObjectInfoT> TypedDataContainer;
      typedef std::unordered_set<std::type_index> TypedDataSetT;
      typedef std::unordered_map<std::type_index, TypedDataSetT> TypedDataDepenencyContainer;

      void initialise(std::type_index const & typeIndex);

      template <typename TObject, typename TFactory, typename TDeleter>
      ObjectRegistration addImpl(TFactory factory, TDeleter deleter, EState initialState)
      {
         auto const & typeInfo = typeid(TObject);
         // delete existing data
         auto it = typedData_.find(typeInfo);
         if (it != typedData_.end())
            throw std::runtime_error((std::string("Object of type '") + std::string(typeInfo.name()) + std::string("' already registered")).c_str());

         typedData_[typeInfo] = createDataHolder<TObject>(factory, deleter, initialState);

         return ObjectRegistration(typeInfo, *this);
      }

      template <typename TObject>
      TObject * findObject(std::type_info const & typeInfo)
      {
         if (typedData_.find(typeInfo) == typedData_.end())
         {
            throw std::runtime_error((std::string("Object of type '") + typeInfo.name() + std::string("' not registered")).c_str());
         }

         // wont do anything if already initialised
         initialise(typeInfo);

         auto* object = getDataHolderData<TObject>(typedData_[typeInfo]);
         if (nullptr == object) throw std::runtime_error("Cannot find object instance - not yet initialised");
         
         return object;
      }

      template <typename TObject>
      bool objectExists(std::type_info const & typeInfo) const
      {
         return typedData_.find(typeInfo) != typedData_.end();
      }

      template <typename TObject, typename TFactory, typename TDeleter>
      static ObjectInfoT createDataHolder(TFactory factory, TDeleter deleter, EState initialState)
      {
         struct TypedInitialiser : public Initialiser
         {
            TypedInitialiser(TFactory factory) : factory_(factory) { }
            virtual void* initialise(TypedObjectRegistry& objects) override { return factory_(objects); }
            TFactory factory_;
         };

         struct TypedDeleter : public Deleter
         {
            TDeleter deleter_;
            TypedDeleter(TDeleter deleter) : deleter_(deleter) { }
            virtual void destroy(void* ptr) override { deleter_(static_cast<TObject*>(ptr)); }
         };

         return ObjectInfoT(nullptr, new TypedInitialiser(factory), new TypedDeleter(deleter), typeid(TObject).name(), initialState);
      }

      template <typename TObject>
      static TObject* getDataHolderData(ObjectInfoT& dataHolder)
      {
         return static_cast<TObject*>(std::get<0>(dataHolder));
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
      TypedDataDepenencyContainer typedDataDependencies_;
   };

} // namespace pcx

#endif // #ifndef PCX_IMPL_TYPED_OBJECT_REGISTRY_H
