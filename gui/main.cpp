#include <iostream>

#include <pcx/Logging.h>

#include <pcx/Configuration.h>
#include <pcx/ModuleRegistry.h>
#include <pcx/ServiceRegistry.h>

namespace
{
   struct RenderModule : public pcx::Module
   {
      virtual void startup(pcx::IConfiguration const & config, pcx::ServiceRegistry & services) {}
      virtual void shutdown() {}
      virtual void restart(pcx::IConfiguration const & config, pcx::ServiceRegistry & services) {}

      virtual void update(double timeSinceLast) {}
   };

   struct UserControlModule : public pcx::Module
   {
      virtual void startup(pcx::IConfiguration const & config, pcx::ServiceRegistry & services) {}
      virtual void shutdown() {}
      virtual void restart(pcx::IConfiguration const & config, pcx::ServiceRegistry & services) {}

      virtual void update(double timeSinceLast) {}
   };
}


int main(int argc, char* argv[])
{
   (void)argc; (void)argv; // avoid 'unreferenced formal parameter' warnings

   try
   {
      LOG(debug) << "Starting gui..." << std::endl;

      auto config = pcx::createFileConfiguration("client.cfg");
      pcx::ModuleRegistry modules {};
      pcx::ServiceRegistry services {};

      modules.add<RenderModule>("render");
      modules.add<UserControlModule>("user-control").withDependency("render");

      modules.startup(*config, services);
   }
   catch (std::exception & ex)
   {
      LOG(error) << "Unexpected error starting application - " << ex.what() << std::endl;
   }
}

