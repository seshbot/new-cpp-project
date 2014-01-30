// test_utils.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma warning ( disable : 4996 )
#include <utility>
#include <utils/utils/EventBus.h>

#include <boost/test/unit_test.hpp>

using namespace game::utils;

BOOST_AUTO_TEST_CASE( basicSubPub )
{
   struct Event1 { int num; Event1(int n) : num(n) { } };

   EventBus bus;

   bool called = false;
   int calledNum = 0;
   bus.Subscribe<Event1>([&](Event1 const & evt)
   {
      called = true;
      calledNum = evt.num;
   });

   BOOST_CHECK( !called );
   BOOST_CHECK( calledNum == 0 );
   bus.Publish<Event1>(Event1(10));
   BOOST_CHECK( called );
   BOOST_CHECK( calledNum == 10 );
}
