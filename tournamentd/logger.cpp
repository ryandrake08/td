#include "logger.hpp"
#include "outputdebugstringbuf.hpp"

template<> basic_logstream<char>::basic_logstream(const char* function, logger_level level) : basic_logstream(debugstreambuf(), function, level)
{
}

template<> basic_logstream<wchar_t>::basic_logstream(const char* function, logger_level level) : basic_logstream(wdebugstreambuf(), function, level)
{
}
