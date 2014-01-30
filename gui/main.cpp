#include <iostream>

#include <pcx/Logging.h>

#include <pcx/Configuration.h>
#include <pcx/ModuleRegistry.h>
#include <pcx/ServiceRegistry.h>

int main(int argc, char* argv[])
{
   (void)argc; (void)argv; // avoid 'unreferenced formal parameter' warnings

   try
   {
      LOG(debug) << "Starting gui..." << std::endl;

      auto config = pcx::createFileConfiguration("client.cfg");
      pcx::ModuleRegistry modules {};
      pcx::ServiceRegistry services {};
      modules.startup(*config, services);
   }
   catch (std::exception & ex)
   {
      LOG(error) << "Unexpected error starting application - " << ex.what() << std::endl;
   }
}

