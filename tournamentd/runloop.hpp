#pragma once

class runloop
{
    // signal handling
    static void signal_handler(int signum);

public:
    // create a runloop
    runloop();

    // destroy a runloop
    ~runloop();

    // run the runloop, given argc and argv from main()
    int run(int argc, const char* const argv[]);
};
