#include <iostream>

#include <boost/log/trivial.hpp>

int main(int argc, char* argv[])
{
   (void)argc; (void)argv; // avoid 'unreferenced formal parameter' warnings

   BOOST_LOG_TRIVIAL(debug) << "Starting gui..." << std::endl;
}

