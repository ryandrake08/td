#include "process.hpp"
#include "logger.hpp"
#include <cerrno> // for errno
#include <cstdlib> // for exit()
#include <csignal> // for kill()
#include <system_error>
#include <unistd.h> /* for fork() */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for waitpid() */

struct process_impl
{
    pid_t pid;
};

process::process(const char* file, const char* const argv[], unsigned int sleep_s) : pimpl(new process_impl)
{
    if(this->pimpl != nullptr)
    {
        auto pid(fork());
        if(pid == 0)
        {
            // child process
            execvp(file, const_cast<char * const *>(argv));
            std::exit(127);
        }
        else if(pid == -1)
        {
            throw(std::system_error(errno, std::system_category(), "process: fork"));
        }
        else
        {
            this->pimpl->pid = pid;

            if(sleep_s != 0)
            {
                sleep(sleep_s);
            }

            logger(LOG_INFO) << "fork " << pid << ", exec " << file << ", sleep " << sleep_s << " seconds\n";
        }
    }
}

process::~process() throw()
{
    if(this->pimpl != nullptr)
    {
        auto killed(kill(this->pimpl->pid, SIGTERM));

        logger(LOG_INFO) << "kill: " << killed << '\n';

        auto wp(waitpid(this->pimpl->pid, 0, 0));

        logger(LOG_INFO) << "waitpid: " << wp << '\n';
        logger(LOG_INFO) << "killed " << this->pimpl->pid << '\n';

        delete pimpl;
    }
}

void process::signal(int signum)
{
    if(this->pimpl != nullptr)
    {
        if(kill(this->pimpl->pid, signum) == -1)
        {
            throw(std::system_error(errno, std::system_category(), "process: signal"));
        }
    }
}
