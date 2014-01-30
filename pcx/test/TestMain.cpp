
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
    framework::master_test_suite().p_name.value = "pcx master test suite";

    return 0;
}
