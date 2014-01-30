#include <pcx/impl/TypedObjectRegistry.h>
#include <pcx/Logging.h>

#include <boost/lexical_cast.hpp>


namespace pcx
{
   //
   // ObjectRegistration
   //
   //

   ObjectRegistration::ObjectRegistration(std::type_info const & typeInfo, TypedObjectRegistry& registry)
      : typeInfo_(&typeInfo)
      , registry_(registry)
   {
   }

   ObjectRegistration ObjectRegistration::withDependency(std::type_info const & typeInfo)
   {
      registry_.addDependency(*typeInfo_, typeInfo);
      return *this;
   }


   //
   // TypedObjectRegistry
   //
   //

   TypedObjectRegistry::~TypedObjectRegistry()
   {
      auto it = typedData_.begin();
      auto itEnd = typedData_.end();
      for (; it != itEnd; ++it)
      {
         deleteDataHolder(it->second);
      }
   }

   void TypedObjectRegistry::initialise(std::type_index const & typeIndex)
   {
      auto it = typedData_.find(typeIndex);
      if (it == typedData_.end())
      {
         throw std::runtime_error((std::string("Cannot initialise object of type '") + typeIndex.name() + "': object type not registered").c_str());
      }

      auto& dataHolder = it->second;

      //
      // drop out if already initialised
      //
      auto state = std::get<4>(dataHolder);
      if (Initialised == state) return;
      if (Initialising == state) 
      {
         throw std::runtime_error((std::string("Object dependency loop detected with object of type '") + typeIndex.name() + "'").c_str());
      }

      //
      // initialise dependencies
      //

      std::get<4>(dataHolder) = Initialising;

      auto& dependencies = typedDataDependencies_[typeIndex];
      for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ++depIt)
      {
         try
         {
            initialise(*depIt);
         } catch (std::exception & ex)
         {
            LOG(error) << std::string("Cannot initialise object of type '") + typeIndex.name() + "' - "
                          + "error initialising dependent object: " + ex.what();

            throw;
         }
      }

      //
      // initialise this object
      //

      auto factory = std::get<1>(dataHolder);
      auto* newObject = factory->initialise(*this);
      std::get<0>(dataHolder) = newObject;

      std::get<4>(dataHolder) = Initialised;
   }

   void TypedObjectRegistry::addDependency(std::type_info const & dependent, std::type_info const & dependency)
   {
      typedDataDependencies_[dependent].insert(dependency);
   }

} // namespace pcx

