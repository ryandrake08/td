#include <iostream>
#include <sstream>
#include <vector>

#if defined (_WIN32)
#include <Windows.h>

template<typename TChar, typename TTraits = std::char_traits<TChar>>
class basic_outputdebugstringbuf : public std::basic_stringbuf<TChar, TTraits>
{
public:
	explicit basic_outputdebugstringbuf() : _buffer(256)
	{
		setg(nullptr, nullptr, nullptr);
		setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
	}

	virtual ~basic_outputdebugstringbuf() throw()
	{
	}

	static_assert(std::is_same<TChar, char>::value || std::is_same<TChar, wchar_t>::value, "basic_outputdebugstringbuf only supports char and wchar_t types");

	virtual int sync() try
	{
		MessageOutputer<TChar, TTraits>()(pbase(), pptr());
		setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
		return 0;
	}
	catch (...)
	{
		return -1;
	}

	virtual int_type overflow(int_type c = TTraits::eof())
	{
		auto syncRet = sync();
		if (c != TTraits::eof()) {
			_buffer[0] = c;
			setp(_buffer.data(), _buffer.data() + 1, _buffer.data() + _buffer.size());
		}
		return syncRet == -1 ? TTraits::eof() : 0;
	}

private:
	std::vector<TChar> _buffer;

	template<typename TChar, typename TTraits>
	struct MessageOutputer;

	template<>
	struct MessageOutputer<char, std::char_traits<char>>
	{
		template<typename TIterator>
		void operator()(TIterator begin, TIterator end) const
		{
			std::string s(begin, end);
			OutputDebugStringA(s.c_str());
		}
	};

	template<>
	struct MessageOutputer<wchar_t, std::char_traits<wchar_t>>
	{
		template<typename TIterator>
		void operator()(TIterator begin, TIterator end) const
		{
			std::wstring s(begin, end);
			OutputDebugStringW(s.c_str());
		}
	};
};

typedef basic_outputdebugstringbuf<char> outputdebugstringbuf;
typedef basic_outputdebugstringbuf<wchar_t> woutputdebugstringbuf;

void redirect_debug_output()
{
	if (IsDebuggerPresent())
	{
		static outputdebugstringbuf charDebugOutput;
		std::cerr.rdbuf(&charDebugOutput);
		std::clog.rdbuf(&charDebugOutput);

		static woutputdebugstringbuf wcharDebugOutput;
		std::wcerr.rdbuf(&wcharDebugOutput);
		std::wclog.rdbuf(&wcharDebugOutput);
	}
}

#else

void redirect_debug_output()
{
}

#endif