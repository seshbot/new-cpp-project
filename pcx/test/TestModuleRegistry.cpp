
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;

#include <set>
#include <pcx/ModuleRegistry.h>

using namespace pcx;

namespace
{
   struct TestModule : public Module
   {

   };
}

BOOST_AUTO_TEST_SUITE( ModuleRegistrySuite )

BOOST_AUTO_TEST_CASE( ModuleRegistry_different_registration_types )
{
   bool ptrDisposed = false;
   bool refDisposed = false;
   bool registeredDisposed = false;
   
   struct MockModule1 : public TestModule
   {
      bool & disposeFlag_;
      MockModule1(bool& disposeFlag) : disposeFlag_(disposeFlag) { }
      ~MockModule1() { disposeFlag_ = true; }
   };

   struct MockModule2 : public TestModule
   {
      bool & disposeFlag_;
      MockModule2(bool& disposeFlag) : disposeFlag_(disposeFlag) { }
      ~MockModule2() { disposeFlag_ = true; }
   };

   struct MockModule3 : public TestModule
   {
      MockModule3() : disposeFlag_(nullptr) { }
      ~MockModule3() { if (nullptr == disposeFlag_) throw std::exception(); *disposeFlag_ = true; }
      bool * disposeFlag_;
      void SetDisposeFlag(bool* disposeFlag) { disposeFlag_ = disposeFlag; }
   };

   MockModule1 refModule(refDisposed);

   {
      ModuleRegistry modules;
      modules.add(refModule, "mod1");
      modules.add(std::unique_ptr<MockModule2>(new MockModule2(ptrDisposed)), "mod2");
      modules.add<MockModule3>("mod3");
      modules.find<MockModule3>().SetDisposeFlag(&registeredDisposed);

      modules.find<MockModule2>();
      modules.find<MockModule3>();
   }
   BOOST_CHECK(!refDisposed);
   BOOST_CHECK(ptrDisposed);
   BOOST_CHECK(registeredDisposed);
}

BOOST_AUTO_TEST_CASE( ModuleRegistry_simple )
{
   struct MockModule1 : public TestModule { };
   struct MockModule2 : public TestModule { };
   struct MockModule3 : public TestModule { };
   struct MockModule4 : public TestModule { };

   ModuleRegistry modules;
   modules.add<MockModule2>("mod2").withDependency("mod1");
   modules.add(std::unique_ptr<MockModule1>(new MockModule1()), "mod1");
   modules.add<MockModule3>("mod3").withDependency("mod1");
   modules.add<MockModule4>("mod4").withDependency("mod2");

   modules.find<MockModule1>();
   modules.find<MockModule2>();
   modules.find<MockModule3>();
   modules.find<MockModule4>();
}

BOOST_AUTO_TEST_CASE( ModuleRegistry_dep_failure )
{
   struct MockModule1 : public TestModule { };
   struct MockModule2 : public TestModule { };
   struct MockModule3 : public TestModule { };

   ModuleRegistry modules;
   modules.add<MockModule2>("mod2").withDependency("mod1");

   try
   {
      // MockModule1 dependency not yet satisfied
      modules.find<MockModule2>();
      BOOST_CHECK(false);
   }
   catch (...)
   {
   }
}

BOOST_AUTO_TEST_CASE( ModuleRegistry_dep_cycle )
{
   struct MockModule1 : public TestModule { };
   struct MockModule2 : public TestModule { };
   struct MockModule3 : public TestModule { };

   ModuleRegistry modules;
   modules.add<MockModule2>("mod2").withDependency("mod1");
   modules.add<MockModule1>("mod1").withDependency("mod3");
   modules.add<MockModule3>("mod3").withDependency("mod2");

   try
   {
      // not yet registered
      modules.find<MockModule1>();
      BOOST_CHECK(false);
   }
   catch (...)
   {
   }
}

BOOST_AUTO_TEST_CASE( ModuleRegistry_dep_ordering )
{
   static int counter = 0;
   struct MockModule2 : public TestModule { MockModule2()
   {
      BOOST_CHECK(++counter == 2);
   }};
   struct MockModule5 : public TestModule { MockModule5()
   {
      BOOST_CHECK(++counter == 5);
   }};
   struct MockModule1 : public TestModule { MockModule1()
   {
      BOOST_CHECK(++counter == 1);
   }};
   struct MockModule4 : public TestModule { MockModule4()
   {
      BOOST_CHECK(++counter == 4);
   }};
   struct MockModule3 : public TestModule { MockModule3()
   {
      BOOST_CHECK(++counter == 3);
   }};

   // deps 3->2->1  5->4
   ModuleRegistry modules;
   modules.add<MockModule3>("mod3").withDependency("mod2");
   modules.add<MockModule2>("mod2").withDependency("mod1");
   modules.add<MockModule4>("mod4");
   modules.add<MockModule1>("mod1");
   modules.add<MockModule5>("mod5").withDependency("mod4");

   modules.find<MockModule3>();
   BOOST_CHECK(counter == 3);

   modules.find<MockModule4>();
   BOOST_CHECK(counter == 4);
   modules.find<MockModule5>();
   BOOST_CHECK(counter == 5);
}

BOOST_AUTO_TEST_SUITE_END()
