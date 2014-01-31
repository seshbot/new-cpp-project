
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;

#include <utility>
#include <pcx/MessageBus.h>

using namespace pcx;

BOOST_AUTO_TEST_SUITE( MessageBusSuite )

BOOST_AUTO_TEST_CASE( basicSubPub )
{
   struct Event1 { int num; Event1(int n) : num(n) { } };

   MessageBus bus;

   bool called = false;
   int calledNum = 0;
   bus.subscribe<Event1>([&](void* sender, Event1 const & evt)
   {
      called = true;
      calledNum = evt.num;
   });

   BOOST_CHECK( !called );
   BOOST_CHECK( calledNum == 0 );
   bus.publish<Event1>(nullptr, Event1(10));
   BOOST_CHECK( called );
   BOOST_CHECK( calledNum == 10 );
}

BOOST_AUTO_TEST_SUITE_END()
