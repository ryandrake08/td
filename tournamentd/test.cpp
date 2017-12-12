#include "tests.hpp"
#include <stdexcept>
#include <iostream>

#define DO_TEST(test) failures += test();

int main(int /* argc */, char * /* argv */ [])
{
    int failures(0);

    try
    {
        DO_TEST(test_json);
        DO_TEST(test_socketstream);

        std::cerr << "tests finished with " << failures << " failures\n";
    }
    catch(const std::exception& e)
    {
        std::cerr << "exception thrown during tests: " << e.what() << std::endl;
        failures = -1;
    }

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
