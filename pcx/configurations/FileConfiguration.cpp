#include "FileConfiguration.h"

#include <pcx/Logging.h>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <map>
#include <stdexcept>


namespace
{
   class FileOption : public pcx::IOption
   {
   public:
      FileOption(boost::property_tree::ptree::value_type & option)
         : name_(option.first), option_(&option)
      {
      }

      FileOption(std::string const & name)
         : name_(name), option_(nullptr)
      {
      }

      virtual std::string const & name() const
      {
         return name_;
      }

      virtual bool isSet() const
      {
         return nullptr != option_;
      }

      virtual std::string stringValue() const
      {
         return option_ ? option_->second.get_value<std::string>() : "";
      }

      virtual bool booleanValue() const
      {
         return option_ ? option_->second.get_value<bool>() : false;
      }

      virtual long integerValue() const
      {
         return option_ ? option_->second.get_value<long>() : 0;
      }

      virtual double doubleValue() const
      {
         return option_ ? option_->second.get_value<double>() : 0.0;
      }

   private:
      std::string name_;
      boost::property_tree::ptree::value_type const * option_;
   };

   class FileSection : public pcx::ISection
   {
   public:
      FileSection(boost::property_tree::ptree & tree)
      {
         using namespace boost::property_tree;

         std::vector<std::string> sections;

         BOOST_FOREACH(ptree::value_type & v, tree)
         {
            if (!v.second.empty())
            {
               sections_.insert(std::make_pair(v.first, FileSection(v)));
            }
            else
            {
               options_.insert(std::make_pair(v.first, FileOption(v)));
            }
         }
      }

      FileSection(boost::property_tree::ptree::value_type & section)
      {
         using namespace boost::property_tree;

         std::vector<std::string> sections;

         BOOST_FOREACH(ptree::value_type & v, section.second)
         {
            if (!v.second.empty())
            {
               sections_.insert(std::make_pair(v.first, FileSection(v)));
            }
            else
            {
               options_.insert(std::make_pair(v.first, FileOption(v)));
            }
         }
      }

      FileSection(FileSection && other)
         : sections_(std::move(other.sections_)), options_(std::move(other.options_))
      {
      }

      virtual ISection & section(std::string const & name)
      {
         auto it = sections_.find(name);
         if (sections_.end() == it)
            throw std::runtime_error(std::string("cannot find section '") + name + std::string("'"));

         return it->second;
      }

      virtual ISection const & section(std::string const & name) const
      {
         auto it = sections_.find(name);
         if (sections_.end() == it)
            throw std::runtime_error(std::string("cannot find section '") + name + std::string("'"));

         return it->second;
      }

      virtual std::vector<std::string> sectionNames() const
      {
         auto sections = sections_ | boost::adaptors::map_keys;
         return std::vector<std::string>(sections.begin(), sections.end());
      }

      virtual bool sectionExists(std::string const & name) const
      {
         return sections_.find(name) != sections_.end();
      }

      virtual pcx::IOption & option(std::string const & name)
      {
         auto it = options_.find(name);
         if (options_.end() == it)
            throw std::runtime_error(std::string("cannot find option '") + name + std::string("'"));

         return it->second;
      }

      virtual pcx::IOption const & option(std::string const & name) const
      {
         auto it = options_.find(name);
         if (options_.end() == it)
            throw std::runtime_error(std::string("cannot find option '") + name + std::string("'"));

         return it->second;
      }

      virtual std::vector<std::string> optionNames() const
      {
         auto options = options_ | boost::adaptors::map_keys;
         return std::vector<std::string>(options.begin(), options.end());
      }

      virtual bool optionExists(std::string const & name) const
      {
         return options_.find(name) != options_.end();
      }

   private:
      FileSection(FileSection const & other);
      FileSection & operator=(FileSection const & other);

      std::map<std::string, FileSection> sections_;
      std::map<std::string, FileOption> options_;
   };
}


namespace pcx
{
   namespace impl
   {
      FileConfiguration::FileConfiguration(std::string const & filename)
      {
         LOG(info) << "parsing configuration file '" << filename << "'";

         //read_xml(filename, ptree);
         //root_ = new FileSection(*ptree.begin());
         read_info(filename, ptree_);
         root_ = new FileSection(ptree_);
      }

      FileConfiguration::~FileConfiguration()
      {
         delete root_;
      }

      ISection & FileConfiguration::section(std::string const & name)
      {
         return root_->section(name);
      }

      ISection const & FileConfiguration::section(std::string const & name) const
      {
         return root_->section(name);
      }

      std::vector<std::string> FileConfiguration::sectionNames() const
      {
         return root_->sectionNames();
      }

      bool FileConfiguration::sectionExists(std::string const & name) const
      {
         return root_->sectionExists(name);
      }

      IOption & FileConfiguration::option(std::string const & name)
      {
         return root_->option(name);
      }

      IOption const & FileConfiguration::option(std::string const & name) const
      {
         return root_->option(name);
      }

      std::vector<std::string> FileConfiguration::optionNames() const
      {
         return root_->optionNames();
      }

      bool FileConfiguration::optionExists(std::string const & name) const
      {
         return root_->optionExists(name);
      }

   } // namespace impl
} // namespace pcx

