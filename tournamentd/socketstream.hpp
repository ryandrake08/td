#pragma once
#include "socket.hpp"
#include <algorithm>
#include <iostream>
#include <streambuf>
#include <utility>

// streambuf for manipulating a character-based socket

template <typename T>
class basic_socketstreambuf : public std::basic_streambuf<T>
{
    typedef T char_type;
    typedef std::basic_streambuf<char_type> buf_type;
    typedef typename buf_type::int_type int_type;
    typedef typename std::basic_streambuf<char_type>::traits_type traits_type;

    common_socket sock;
    static const int char_size = sizeof(char_type);
    static const std::size_t SIZE = 8192;
    char_type* ibuf;
    char_type* obuf;

public:
    explicit basic_socketstreambuf(const common_socket& s) : sock(s)
    {
        this->ibuf = new char_type[SIZE];
        this->obuf = new char_type[SIZE];
        buf_type::setg(ibuf, ibuf, ibuf);
        buf_type::setp(obuf, obuf + (SIZE - 1));
    }

    ~basic_socketstreambuf() override
    {
        try
        {
            if(this->ibuf != nullptr)
            {
                delete[] this->ibuf;
            }
            if(this->obuf != nullptr)
            {
                this->sync();
                delete[] this->obuf;
            }
        }
        catch(...)
        {
        }
    }

    basic_socketstreambuf(basic_socketstreambuf&& other) noexcept : sock(other.sock), ibuf(other.ibuf), obuf(other.obuf)
    {
        other.ibuf = nullptr;
        other.obuf = nullptr;
    }

    basic_socketstreambuf& operator=(basic_socketstreambuf&& other) noexcept
    {
        this->sock = other.sock;
        this->ibuf = other.ibuf;
        this->obuf = other.obuf;
        other.ibuf = nullptr;
        other.obuf = nullptr;
    }

private:
    ptrdiff_t output_buffer()
    {
        auto num(buf_type::pptr() - buf_type::pbase());
        if(num > 0)
        {
            if(this->sock.send(obuf, num * char_size) != num)
            {
                return traits_type::eof();
            }
            buf_type::pbump(static_cast<int>(-num));
        }
        return num;
    }

    int_type overflow(int_type c) override
    {
        if(c != traits_type::eof())
        {
            *buf_type::pptr() = static_cast<char_type>(c);
            buf_type::pbump(1);
        }

        if(output_buffer() == traits_type::eof())
        {
            return traits_type::eof();
        }
        return c;
    }

    int sync() override
    {
        if(output_buffer() == traits_type::eof())
        {
            return traits_type::eof();
        }
        return 0;
    }

    int_type underflow() override
    {
        if(buf_type::gptr() < buf_type::egptr())
        {
            return *buf_type::gptr();
        }

        auto num(this->sock.recv(ibuf, SIZE * char_size));
        if(num == 0)
        {
            return traits_type::eof();
        }

        buf_type::setg(ibuf, ibuf, ibuf + num);
        return *buf_type::gptr();
    }

    std::streamsize showmanyc() override
    {
        char buf;
        return this->sock.peek(&buf, 1);
    }
};

typedef basic_socketstreambuf<char> socketstreambuf;
typedef basic_socketstreambuf<wchar_t> wsocketstreambuf;

template <typename T>
class basic_socketstream : public std::basic_iostream<T>
{
    basic_socketstreambuf<T> buf;
public:
    explicit basic_socketstream(const common_socket& s) : std::basic_iostream<T>(&buf), buf(s) {}
    basic_socketstream(basic_socketstream&& other) noexcept : std::basic_iostream<T>(&buf), buf(std::move(other.buf)) {}
    basic_socketstream& operator=(basic_socketstream&& other) noexcept { buf = other.buf; }
};

typedef basic_socketstream<char> socketstream;
typedef basic_socketstream<wchar_t> wsocketstream;
