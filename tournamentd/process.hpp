#pragma once

class process_impl;

// Wrapper for execvp that forks and executes a process
class process
{
    // Using pimpl pattern to hide implementation details
    process_impl* pimpl;

public:
    process(const char* file, const char* const argv[], unsigned int sleep_s=0);
    ~process() throw();

    // copying not allowed
    process(const process&) = delete;
    process& operator= (const process&) = delete;

    // Send process a signal
    void signal(int signum);
};