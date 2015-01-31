#pragma once
#include <iostream>
#include <streambuf>
#include <vector>

// streambuf for manipulating a vector-based buffer

template <typename T>
class basic_ivectorstreambuf : public std::basic_streambuf<T>
{
public:
    basic_ivectorstreambuf(const std::vector<T>& vec)
    {
        this->setg(const_cast<T*>(&vec.front()),
                   const_cast<T*>(&vec.front()),
                   const_cast<T*>(&vec.front() + vec.size()));
    }
};

typedef basic_ivectorstreambuf<char> ivectorstreambuf;

template <typename T>
class basic_ivectorstream : public std::basic_istream<T>
{
    basic_ivectorstreambuf<T> buffer;

public:
    basic_ivectorstream(const std::vector<T>& vec) : std::istream(&buffer), buffer(vec)
    {
    }
};

typedef basic_ivectorstream<char> ivectorstream;
