#include <pcx/ServiceRegistry.h>
#include <pcx/Logging.h>


namespace pcx
{
   class ServiceRegistration::Impl
   {
   public:
      Impl(std::type_info const & typeInfo, ServiceRegistry& ServiceRegistry)
         : typeInfo_(&typeInfo)
         , ServiceRegistry_(ServiceRegistry)
      { }

      void dependsOn(std::type_info const & typeInfo)
      {
         ServiceRegistry_.addDependency(*typeInfo_, typeInfo);
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

   ServiceRegistration ServiceRegistration::dependsOn(std::type_info const & typeInfo)
   {
      impl_->dependsOn(typeInfo);
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
         deleteDataHolder(it->second);
      }
   }

   void ServiceRegistry::initialise(std::type_index const & typeIndex)
   {
      auto it = typedData_.find(typeIndex);
      if (it == typedData_.end())
      {
         throw std::runtime_error((std::string("Cannot initialise service '") + demangle_name(typeIndex.name()) + "': service not registered").c_str());
      }

      auto& dataHolder = it->second;

      //
      // drop out if already initialised
      //
      auto state = std::get<4>(dataHolder);
      if (Initialised == state) return;
      if (Initialising == state) 
      {
         throw std::runtime_error((std::string("Service dependency loop detected with service '") + demangle_name(typeIndex.name()) + "'").c_str());
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
            LOG(error) << std::string("Cannot initialise service '") + demangle_name(typeIndex.name()) + "' - "
                          + "error initialising dependent service: " + ex.what();

            throw;
         }
      }

      //
      // initialise this service
      //

      auto factory = std::get<1>(dataHolder);
      auto* newService = factory->initialise(*this);
      std::get<0>(dataHolder) = newService;

      std::get<4>(dataHolder) = Initialised;
   }

   void ServiceRegistry::addDependency(std::type_info const & dependent, std::type_info const & dependsOn)
   {
      typedDataDependencies_[dependent].insert(dependsOn);
   }

} // namespace pcx

