
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;

#include <set>
#include <list>
#include <functional>
#include <pcx/impl/BaseLazyFactory.h>

#include <iostream>

using namespace pcx;

namespace
{
   struct TestFactory : public BaseLazyFactory<TestFactory, std::string>
   {
      std::function<void()> initialiseObjectDependenciesCallback;
      std::vector<std::string> constructedIds;
      std::vector<std::string> initialisedIds;

      template <typename ObjectT>
      ObjectT* createObjectCallback(std::string id)
      {
         constructedIds.push_back(id);
         return new ObjectT();
      }

      template <typename ObjectT>
      void initialiseObjectCallback(ObjectT&, std::string id, void*)
      {
         initialisedIds.push_back(id);
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

      using BaseLazyFactory::addObject;
      using BaseLazyFactory::initialiseObject;
      using BaseLazyFactory::findObject;
   };
}

BOOST_AUTO_TEST_CASE( LazyFactory_different_registration_types )
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
      TestFactory container;
      container.addObject(refService, "ref1");
      container.addObject(std::unique_ptr<MockService2>(new MockService2(ptrDisposed)), "ref2");
      container.addObject<MockService3>("ref3");

      container.initialiseObject("ref3", nullptr);
      container.initialiseObject("ref2", nullptr);
      container.initialiseObject("ref1", nullptr);

      container.findObject<MockService3>().SetDisposeFlag(&registeredDisposed);

      container.findObject<MockService2>();
      container.findObject<MockService3>();

      BOOST_CHECK(!container.idWasConstructed("ref1"));
      BOOST_CHECK(!container.idWasConstructed("ref2"));
      BOOST_CHECK(container.idWasConstructed("ref3"));

      BOOST_CHECK(container.idWasInitialised("ref1"));
      BOOST_CHECK(container.idWasInitialised("ref2"));
      BOOST_CHECK(container.idWasInitialised("ref3"));
   }
   BOOST_CHECK(!refDisposed);
   BOOST_CHECK(ptrDisposed);
   BOOST_CHECK(registeredDisposed);

   std::cout << "test ended!" << std::endl;
}

