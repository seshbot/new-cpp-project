#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <string>
#include <memory>
#include <vector>

#include <boost/filesystem.hpp>    // to stop link errors

#pragma warning ( push )
#pragma warning ( disable : 4996 ) // sprintf unsafe
#include <boost/log/trivial.hpp>
#include <boost/log/utility/init/to_file.hpp>
#include <boost/log/utility/init/to_console.hpp>
#pragma warning ( pop )

#define LOG(lvl) BOOST_LOG_TRIVIAL(lvl)

namespace utils
{
namespace logging
{

class Logger
{
public:
   enum ELevel { LevelTrace, LevelDebug, LevelInfo, LevelWarn, LevelError, LevelFatal };
   static const int NumLogLevels = 6;

   Logger();
   virtual ~Logger() { }

   void EnableLevel(ELevel level, bool enable);
   void SetLevel(ELevel level);

   bool IsEnabled(ELevel level) const;

   void EnableTrace(bool enable);
   void EnableDebug(bool enable);
   void EnableInfo(bool enable);
   void EnableWarn(bool enable);
   void EnableError(bool enable);
   void EnableFatal(bool enable);

   void Debug(std::string const & msg) const;
   void Error(std::string const & msg) const;
   void Fatal(std::string const & msg) const;
   void Info(std::string const & msg) const;
   void Warn(std::string const & msg) const;
   void Trace(std::string const & msg) const;

   void Log(ELevel level, std::string const & msg) const;

private:
   virtual void OnLog(ELevel level, std::string const & msg) const = 0;
   virtual void OnLevelEnabled(ELevel level, bool enabled) { }

   std::vector<bool> mEnabledLevels;
};

std::unique_ptr<Logger> GetLogger();

class NullLogger : public Logger
{
private:
   virtual void OnLog(ELevel level, std::string const & msg) const { }
};


}

}

#endif 