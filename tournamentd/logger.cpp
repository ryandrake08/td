#include "logger.hpp"
#include "datetime.hpp"
#include "outputdebugstringbuf.hpp"
#include <fstream>

logger_internal& logger_internal::instance()
{
    static logger_internal instance;
    return instance;
}

logger_internal::logger_internal() : mask(-1)
{
	redirect_debug_output();
}

void logger_internal::set_enabled(std::initializer_list<logger_level> logs)
{
    this->mask = 0;
    for(auto& level : logs)
    {
        this->mask |= 1 << level;
    }
}

std::ostream& logger_internal::get_stream(const char* function, logger_level level)
{
    if((1 << level) & this->mask)
    {
        static const char* level_string[] = { " DEBUG ", " INFO ", " WARNING ", " ERROR " };
        return std::clog << datetime::local << datetime::now() << level_string[level] << function << ": ";
    }
    else
    {
        static std::ostream nullstream(nullptr);
        return nullstream;
    }
}
