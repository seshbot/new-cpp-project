#include <iostream>

#include <pcx/Logging.h>

#include <pcx/Configuration.h>
#include <pcx/ModuleRegistry.h>
#include <pcx/ServiceRegistry.h>

#include <SFML/Graphics.hpp>

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

      sf::RenderWindow app(sf::VideoMode(800, 600), "sample-cpp-game-project");

      while (app.isOpen()) {
         sf::Event Event;
         while (app.pollEvent(Event)) {
            if (Event.type == sf::Event::Closed)
               app.close();
         }
         app.clear(sf::Color::Black);
         app.display();
      }
   }
   catch (std::exception & ex)
   {
      LOG(error) << "Unexpected error starting application - " << ex.what() << std::endl;
   }
}

