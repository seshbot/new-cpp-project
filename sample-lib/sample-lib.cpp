#include <boost/log/trivial.hpp>

int sampleLibFunction()
{
   BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
   return 42;
}
