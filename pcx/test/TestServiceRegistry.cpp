
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;

#include <set>
#include <pcx/IndexPool.h>
#include <pcx/ServiceRegistry.h>

using namespace pcx;

BOOST_AUTO_TEST_SUITE( ServiceRegistrySuite )

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


BOOST_AUTO_TEST_CASE( ServiceRegistry_different_registration_types )
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
      MockService3(ServiceRegistry& services) : disposeFlag_(nullptr) { }
      ~MockService3() { if (nullptr == disposeFlag_) throw std::exception(); *disposeFlag_ = true; }
      bool * disposeFlag_;
      void SetDisposeFlag(bool* disposeFlag) { disposeFlag_ = disposeFlag; }
   };

   MockService1 refService(refDisposed);

   {
      ServiceRegistry services;
      services.add(refService);
      services.add(std::unique_ptr<MockService2>(new MockService2(ptrDisposed)));
      services.add<MockService3>();
      services.findService<MockService3>()->SetDisposeFlag(&registeredDisposed);

      BOOST_CHECK(nullptr != services.findService<MockService2>());
      BOOST_CHECK(nullptr != services.findService<MockService3>());
   }
   BOOST_CHECK(!refDisposed);
   BOOST_CHECK(ptrDisposed);
   BOOST_CHECK(registeredDisposed);
}

BOOST_AUTO_TEST_CASE( ServiceRegistry_simple )
{
   struct MockService1 { MockService1() { } };
   struct MockService2 { MockService2(ServiceRegistry& services) { } };
   struct MockService3 { MockService3(ServiceRegistry& services) { } };
   struct MockService4 { MockService4(ServiceRegistry& services) { } };

   ServiceRegistry services;
   services.add<MockService2>().dependsOn<MockService1>();
   services.add(std::unique_ptr<MockService1>(new MockService1()));
   services.add<MockService3>().dependsOn<MockService1>();
   services.add<MockService4>().dependsOn<MockService2>();
   BOOST_CHECK(nullptr != services.findService<MockService1>());
   BOOST_CHECK(nullptr != services.findService<MockService2>());
   BOOST_CHECK(nullptr != services.findService<MockService3>());
   BOOST_CHECK(nullptr != services.findService<MockService4>());
}

BOOST_AUTO_TEST_CASE( ServiceRegistry_dep_failure )
{
   struct MockService1 { MockService1() { } };
   struct MockService2 { MockService2(ServiceRegistry& services) { } };
   struct MockService3 { MockService3(ServiceRegistry& services) { } };

   ServiceRegistry services;
   services.add<MockService2>().dependsOn<MockService1>();

   try
   {
      // MockService1 dependency not yet satisfied
      services.findService<MockService2>();
      BOOST_CHECK(false);
   }
   catch (...)
   {
   }
}

BOOST_AUTO_TEST_CASE( ServiceRegistry_dep_cycle )
{
   struct MockService1 { MockService1(ServiceRegistry& services) { } };
   struct MockService2 { MockService2(ServiceRegistry& services) { } };
   struct MockService3 { MockService3(ServiceRegistry& services) { } };

   ServiceRegistry services;
   services.add<MockService2>().dependsOn<MockService1>();
   services.add<MockService1>().dependsOn<MockService3>();
   services.add<MockService3>().dependsOn<MockService2>();

   try
   {
      // not yet registered
      services.findService<MockService1>();
      BOOST_CHECK(false);
   }
   catch (...)
   {
   }
}

BOOST_AUTO_TEST_CASE( ServiceRegistry_dep_ordering )
{
   static int counter = 0;
   struct MockService2 { MockService2(ServiceRegistry& services)
   {
      BOOST_CHECK(++counter == 2);
   }};
   struct MockService5 { MockService5(ServiceRegistry& services)
   {
      BOOST_CHECK(++counter == 5);
   }};
   struct MockService1 { MockService1(ServiceRegistry& services)
   {
      BOOST_CHECK(++counter == 1);
   }};
   struct MockService4 { MockService4(ServiceRegistry& services)
   {
      BOOST_CHECK(++counter == 4);
   }};
   struct MockService3 { MockService3(ServiceRegistry& services)
   {
      BOOST_CHECK(++counter == 3);
   }};

   // deps 3->2->1  5->4
   ServiceRegistry services;
   services.add<MockService3>().dependsOn<MockService2>();
   services.add<MockService2>().dependsOn<MockService1>();
   services.add<MockService4>();
   services.add<MockService1>();
   services.add<MockService5>().dependsOn<MockService4>();

   services.findService<MockService3>();
   BOOST_CHECK(counter == 3);

   services.findService<MockService4>();
   BOOST_CHECK(counter == 4);
   services.findService<MockService5>();
   BOOST_CHECK(counter == 5);
}

BOOST_AUTO_TEST_SUITE_END()
