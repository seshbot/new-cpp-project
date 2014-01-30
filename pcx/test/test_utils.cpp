// test_utils.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <set>
#include <algorithm>
#include <utility>
#include <utils/utils/IndexPool.h>
#include <utils/utils/ServiceLocator.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/icl/interval_set.hpp>

using namespace game::utils;

BOOST_AUTO_TEST_CASE( overFillPool )
{
   auto list = IndexPool(10);

   long idx;
   for (int i = 0; i < 10; i++)
   {
      idx = list.Allocate();
   }

   try
   {
      list.Allocate();
      BOOST_CHECK( false );
   }
   catch (...)
   {
   }

   list.Free(idx);
   BOOST_CHECK(idx == list.Allocate());
}


BOOST_AUTO_TEST_CASE( reallocation )
{
   auto list = IndexPool(10);

   for (int i = 0; i < 10; i++)
   {
      list.Allocate();
   }

   std::set<long> indexes;
   for (size_t i = 1; i < 10; i += 2)
   {
      indexes.insert(i);
   }
   
   std::for_each(indexes.begin(), indexes.end(), [&](long idx)
   {
      list.Free(idx);
   });

   std::set<long> remainingIndexes;
   list.ForEach([&](long idx)
   {
      BOOST_CHECK(remainingIndexes.insert(idx).second);
   });

   BOOST_CHECK(10 == remainingIndexes.size() + indexes.size());

   {
      auto it = indexes.begin();
      for (; it != indexes.end(); ++it)
      {
         BOOST_CHECK(remainingIndexes.find(*it) == remainingIndexes.end());
      }
   }

   std::set<long> reallocIndexes;
   for (size_t i = 0; i < indexes.size(); ++i)
   {
      reallocIndexes.insert(list.Allocate());
   }

   BOOST_CHECK(indexes.size() == reallocIndexes.size());

   {
      auto it1 = indexes.begin();
      auto it2 = reallocIndexes.begin();
      for (; it1 != indexes.end(); ++it1, ++it2)
      {
         BOOST_CHECK(*it1 == *it2);
      }
   }
}


BOOST_AUTO_TEST_CASE( serviceLocator_different_registration_types )
{
   bool ptrDisposed = false;
   bool refDisposed = false;
   bool registeredDisposed = false;
   
   struct MockService1 
   { 
      bool & disposeFlag_; 
      MockService1(bool& disposeFlag) : disposeFlag_(disposeFlag) { } 
      ~MockService1() { disposeFlag_ = true; } 
   };

   struct MockService2
   {
      bool & disposeFlag_; 
      MockService2(bool& disposeFlag) : disposeFlag_(disposeFlag) { } 
      ~MockService2() { disposeFlag_ = true; } 
   };

   struct MockService3
   { 
      MockService3(ServiceLocator& services) : disposeFlag_(nullptr) { } 
      ~MockService3() { if (nullptr == disposeFlag_) throw std::exception(); *disposeFlag_ = true; } 
      bool * disposeFlag_; 
      void SetDisposeFlag(bool* disposeFlag) { disposeFlag_ = disposeFlag; }
   };

   MockService1 refService(refDisposed);

   {
      ServiceLocator services;
      services.Register(refService);
      services.Register(std::unique_ptr<MockService2>(new MockService2(ptrDisposed)));
      services.Register<MockService3>();
      services.GetService<MockService3>()->SetDisposeFlag(&registeredDisposed);

      BOOST_CHECK(nullptr != services.GetService<MockService2>());
      BOOST_CHECK(nullptr != services.GetService<MockService3>());
   }
   BOOST_CHECK(!refDisposed);
   BOOST_CHECK(ptrDisposed);
   BOOST_CHECK(registeredDisposed);
}

BOOST_AUTO_TEST_CASE( serviceLocator_simple )
{
   struct MockService1 { MockService1() { } };
   struct MockService2 { MockService2(ServiceLocator& services) { } };
   struct MockService3 { MockService3(ServiceLocator& services) { } };
   struct MockService4 { MockService4(ServiceLocator& services) { } };

   ServiceLocator services;
   services.Register<MockService2>().DependsOn<MockService1>();
   services.Register(std::unique_ptr<MockService1>(new MockService1()));
   services.Register<MockService3>().DependsOn<MockService1>();
   services.Register<MockService4>().DependsOn<MockService2>();
   BOOST_CHECK(nullptr != services.GetService<MockService1>());
   BOOST_CHECK(nullptr != services.GetService<MockService2>());
   BOOST_CHECK(nullptr != services.GetService<MockService3>());
   BOOST_CHECK(nullptr != services.GetService<MockService4>());
}

BOOST_AUTO_TEST_CASE( serviceLocator_dep_failure )
{
   struct MockService1 { MockService1() { } };
   struct MockService2 { MockService2(ServiceLocator& services) { } };
   struct MockService3 { MockService3(ServiceLocator& services) { } };

   ServiceLocator services;
   services.Register<MockService2>().DependsOn<MockService1>();

   try 
   {
      // MockService1 dependency not yet satisfied
      services.GetService<MockService2>();
      BOOST_CHECK(false);
   }
   catch (...)
   {
   }
}

BOOST_AUTO_TEST_CASE( serviceLocator_dep_cycle )
{
   struct MockService1 { MockService1(ServiceLocator& services) { } };
   struct MockService2 { MockService2(ServiceLocator& services) { } };
   struct MockService3 { MockService3(ServiceLocator& services) { } };

   ServiceLocator services;
   services.Register<MockService2>().DependsOn<MockService1>();
   services.Register<MockService1>().DependsOn<MockService3>();
   services.Register<MockService3>().DependsOn<MockService2>();

   try 
   {
      // not yet registered
      services.GetService<MockService1>();
      BOOST_CHECK(false);
   }
   catch (...)
   {
   }
}

BOOST_AUTO_TEST_CASE( serviceLocator_dep_ordering )
{
   static int counter = 0;
   struct MockService2 { MockService2(ServiceLocator& services) 
   {
      BOOST_CHECK(++counter == 2);
   }};
   struct MockService5 { MockService5(ServiceLocator& services) 
   {
      BOOST_CHECK(++counter == 5);
   }};
   struct MockService1 { MockService1(ServiceLocator& services) 
   {
      BOOST_CHECK(++counter == 1);
   }};
   struct MockService4 { MockService4(ServiceLocator& services) 
   {
      BOOST_CHECK(++counter == 4);
   }};
   struct MockService3 { MockService3(ServiceLocator& services) 
   {
      BOOST_CHECK(++counter == 3);
   }};

   // deps 3->2->1  5->4
   ServiceLocator services;
   services.Register<MockService3>().DependsOn<MockService2>();
   services.Register<MockService2>().DependsOn<MockService1>();
   services.Register<MockService4>();
   services.Register<MockService1>();
   services.Register<MockService5>().DependsOn<MockService4>();

   services.GetService<MockService3>();
   BOOST_CHECK(counter == 3);

   services.GetService<MockService4>();
   BOOST_CHECK(counter == 4);
   services.GetService<MockService5>();
   BOOST_CHECK(counter == 5);
}

