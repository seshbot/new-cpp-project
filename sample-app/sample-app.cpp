#include <iostream>

#include <boost/log/trivial.hpp>
#include <sample-lib/sample-lib.h>

int main(int argc, char* argv[])
{
   (void)argc; (void)argv; // avoid 'unreferenced formal parameter' warnings

   std::cout << "sample application!" << std::endl;

   auto sampleReturn = sampleLibFunction();

   BOOST_LOG_TRIVIAL(debug) << " - sample lib says '" << sampleReturn << "'" << std::endl;
}

