#include <pcx/ServiceRegistry.h>

#include <boost/lexical_cast.hpp>


namespace pcx
{
   class ServiceRegistration::Impl
   {
   public:
      Impl(std::type_info const & typeInfo, ServiceRegistry& ServiceRegistry)
         : typeInfo_(&typeInfo)
         , ServiceRegistry_(ServiceRegistry)
      { }

      void DependsOn(std::type_info const & typeInfo)
      {
         ServiceRegistry_.AddDependency(*typeInfo_, typeInfo);
      }

   private:
      std::type_info const * typeInfo_;
      ServiceRegistry& ServiceRegistry_;

   };


   //
   // ServiceRegistration
   //
   //

   ServiceRegistration::ServiceRegistration(std::type_info const & typeInfo, ServiceRegistry& ServiceRegistry)
      : impl_(new Impl(typeInfo, ServiceRegistry))
   {
   }


   ServiceRegistration::ServiceRegistration(ServiceRegistration const & other)
      : impl_(other.impl_)
   {
   }

   ServiceRegistration ServiceRegistration::DependsOn(std::type_info const & typeInfo)
   {
      impl_->DependsOn(typeInfo);
      return ServiceRegistration(*this);
   }


   //
   // ServiceRegistry
   //
   //

   ServiceRegistry::~ServiceRegistry()
   {
      auto it = typedData_.begin();
      auto itEnd = typedData_.end();
      for (; it != itEnd; ++it)
      {
         DeleteDataHolder(it->second);
      }
   }

   //void ServiceRegistry::Initialise()
   //{
   //   std::vector<std::type_index> toInitialise;
   //   {
   //      auto it = typedData_.begin();
   //      auto itEnd = typedData_.end();

   //      for (; it != itEnd; ++it)
   //      {
   //         toInitialise.push_back(it->first);
   //      }
   //   }

   //   std::unordered_set<std::type_index> initialised;

   //   while (!toInitialise.empty())
   //   {
   //      auto size = toInitialise.size();

   //      auto itNewEnd = std::remove_if(toInitialise.begin(), toInitialise.end(), [&](std::type_index const & typeIndex) -> bool
   //      {
   //         auto count = typedDataDependencies_.size();
   //         auto& first = *typedDataDependencies_.begin();
   //         // if any dependencies not initialised, don't remove yet
   //         auto& dependencies = typedDataDependencies_[typeIndex];
   //         for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ++depIt)
   //         {
   //            if (initialised.find(*depIt) == initialised.end())
   //            {
   //               // dependency not found yet
   //               return false;
   //            }
   //         }

   //         // all dependencies satisfied, initialise this one
   //         auto& dataHolder = typedData_[typeIndex];
   //         auto factory = std::get<1>(dataHolder);
   //         auto* newService = factory->Initialise(*this);
   //         std::get<0>(dataHolder) = newService;

   //         initialised.insert(typeIndex);
   //         return true;
   //      });

   //      toInitialise.erase(itNewEnd, toInitialise.end());

   //      if (size == toInitialise.size()) 
   //      {
   //         throw std::exception(
   //           (std::string("Could not initialise ") + 
   //            boost::lexical_cast<std::string>(size) + 
   //            std::string(" services due to missing dependencies")).c_str());
   //      }
   //   }
   //}


   void ServiceRegistry::Initialise(std::type_index const & typeIndex)
   {
      auto it = typedData_.find(typeIndex);
      if (it == typedData_.end())
      {
         throw std::runtime_error((std::string("Cannot initialise service '") + typeIndex.name() + "': service not registered").c_str());
      }

      auto& dataHolder = it->second;

      //
      // drop out if already initialised
      //
      auto state = std::get<4>(dataHolder);
      if (Initialised == state) return;
      if (Initialising == state) 
      {
         throw std::runtime_error((std::string("Service dependency loop detected with service ") + typeIndex.name()).c_str());
      }

      //
      // initialise dependencies
      //

      std::get<4>(dataHolder) = Initialising;

      auto& dependencies = typedDataDependencies_[typeIndex];
      for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ++depIt)
      {
         Initialise(*depIt);
      }

      //
      // initialise this service
      //

      auto factory = std::get<1>(dataHolder);
      auto* newService = factory->Initialise(*this);
      std::get<0>(dataHolder) = newService;

      std::get<4>(dataHolder) = Initialised;
   }

   void ServiceRegistry::AddDependency(std::type_info const & dependent, std::type_info const & dependsOn)
   {
      typedDataDependencies_[dependent].insert(dependsOn);
   }

} // namespace pcx

