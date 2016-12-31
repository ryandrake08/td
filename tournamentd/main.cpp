#include "runloop.hpp"
#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        // Run the runloop
        runloop r;
        return r.run(argc, argv);
    }
    catch(const std::exception& e)
    {
        // Catch-all exception handler.
        // If we've managed to get here,
        // we've encountered a fatal error
        // that couldn't be handled upstream.
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
