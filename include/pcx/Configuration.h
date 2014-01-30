#ifndef PCX_CONFIGURATION_H
#define PCX_CONFIGURATION_H

#include <boost/signals2.hpp>
#include <vector>
#include <string>
#include <memory>

namespace pcx
{
   class IOption
   {
   public:
      virtual ~IOption() { }

      virtual std::string const & name() const = 0;
      virtual bool isSet() const = 0;

      virtual std::string stringValue() const = 0;
      virtual bool booleanValue() const = 0;
      virtual long integerValue() const = 0;
      virtual double doubleValue() const = 0;

      std::string stringValue(std::string const & defaultValue) const { return isSet() ? stringValue() : defaultValue; }
      bool booleanValue(bool defaultValue) const { return isSet() ? booleanValue() : defaultValue; }
      long integerValue(long defaultValue) const { return isSet() ? integerValue() : defaultValue; }
      double doubleValue(double defaultValue) const { return isSet() ? doubleValue() : defaultValue; }
   };

   class ISection
   {
   public:
      virtual ~ISection() { }

      virtual ISection & section(std::string const & name) = 0;
      virtual ISection const & section(std::string const & name) const = 0;
      virtual std::vector<std::string> sectionNames() const = 0;
      virtual bool sectionExists(std::string const & name) const = 0;

      virtual IOption & option(std::string const & name) = 0;
      virtual IOption const & option(std::string const & name) const = 0;
      virtual std::vector<std::string> optionNames() const = 0;
      virtual bool optionExists(std::string const & name) const = 0;

      std::string stringValue(std::string const & name, std::string const & defaultValue) const
      { return optionExists(name) ? option(name).stringValue(defaultValue) : defaultValue; }
      std::string stringValue(std::string const & name) const
      { return option(name).stringValue(); }
      bool booleanValue(std::string const & name, bool defaultValue) const
      { return optionExists(name) ? option(name).booleanValue(defaultValue) : defaultValue; }
      bool booleanValue(std::string const & name) const
      { return option(name).booleanValue(); }
      long integerValue(std::string const & name, long defaultValue) const
      { return optionExists(name) ? option(name).integerValue(defaultValue) : defaultValue; }
      long integerValue(std::string const & name) const
      { return option(name).integerValue(); }
      double doubleValue(std::string const & name, double defaultValue) const
      { return optionExists(name) ? option(name).doubleValue(defaultValue) : defaultValue; }
      double doubleValue(std::string const & name) const
      { return option(name).doubleValue(); }

      boost::signals2::signal<void()> UpdatedSignal;
   };

   class IConfiguration
   {
   public:
      virtual ~IConfiguration() { }

      virtual ISection & section(std::string const & name) = 0;
      virtual ISection const & section(std::string const & name) const = 0;
      virtual std::vector<std::string> sectionNames() const = 0;
      virtual bool sectionExists(std::string const & name) const = 0;

      virtual IOption & option(std::string const & name) = 0;
      virtual IOption const & option(std::string const & name) const = 0;
      virtual std::vector<std::string> optionNames() const = 0;
      virtual bool optionExists(std::string const & name) const = 0;

      boost::signals2::signal<void()> UpdatedSignal;
   };

   //
   // factory functions
   //

   std::unique_ptr<IConfiguration> createFileConfiguration(std::string filename);

} // namespace pcx

#endif // #ifndef PCX_CONFIGURATION_H
