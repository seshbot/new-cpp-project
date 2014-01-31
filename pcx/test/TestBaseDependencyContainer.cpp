
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;

#include <set>
#include <list>
#include <functional>
#include <pcx/impl/BaseDependencyContainer.h>

#include <iostream>

using namespace pcx;

namespace
{
   struct TestContainer : public BaseDependencyContainer<TestContainer, std::string>
   {
      std::function<void()> initialiseObjectDependenciesCallback;
      std::vector<std::string> constructedIds;
      std::vector<std::string> initialisedIds;

      void initialiseObjectDependencies(std::string objectId)
      {
         initialisedIds.push_back(objectId);
         if (initialiseObjectDependenciesCallback) initialiseObjectDependenciesCallback();
      }

      template <typename ObjectT>
      ObjectT* createObject(std::string id)
      {
         constructedIds.push_back(id);
         return new ObjectT();
      }

      bool idWasConstructed(std::string objectId)
      {
         for (auto id : constructedIds)
         {
            if (id == objectId) return true;
         }
         return false;
      }

      bool idWasInitialised(std::string objectId)
      {
         for (auto id : initialisedIds)
         {
            if (id == objectId) return true;
         }
         return false;
      }
   };
}

BOOST_AUTO_TEST_CASE( ObjectContainer_different_registration_types )
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
      MockService3() : disposeFlag_(nullptr) { }
      ~MockService3() { if (nullptr == disposeFlag_) throw std::exception(); *disposeFlag_ = true; }
      bool * disposeFlag_;
      void SetDisposeFlag(bool* disposeFlag) { disposeFlag_ = disposeFlag; }
   };

   MockService1 refService(refDisposed);

   {
      TestContainer container;
      container.addObject(refService, "ref1");
      container.addObject(std::unique_ptr<MockService2>(new MockService2(ptrDisposed)), "ref2");
      container.addObject<MockService3>("ref3");
      container.findObject<MockService3>()->SetDisposeFlag(&registeredDisposed);

      BOOST_CHECK(nullptr != container.findObject<MockService2>());
      BOOST_CHECK(nullptr != container.findObject<MockService3>());

      BOOST_CHECK(!container.idWasConstructed("ref1"));
      BOOST_CHECK(!container.idWasConstructed("ref2"));
      BOOST_CHECK(container.idWasConstructed("ref3"));

      BOOST_CHECK(!container.idWasInitialised("ref1"));
      BOOST_CHECK(container.idWasInitialised("ref2"));
      BOOST_CHECK(container.idWasInitialised("ref3"));
   }
   BOOST_CHECK(!refDisposed);
   BOOST_CHECK(ptrDisposed);
   BOOST_CHECK(registeredDisposed);

   std::cout << "test ended!" << std::endl;
}

