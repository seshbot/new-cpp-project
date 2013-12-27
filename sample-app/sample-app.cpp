#include <iostream>

#include "sample-lib.h"

int main(int argc, char* argv[])
{
   (void)argc; (void)argv; // avoid 'unreferenced formal parameter' warnings

   std::cout << "sample application!\n";

   std::cout << " - sample lib says '" << sampleLibFunction() << "'\n";
}

