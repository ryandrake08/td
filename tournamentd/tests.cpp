#include "json.hpp"
#include "socketstream.hpp"
#include <vector>
#include <string>
#include <iostream>

#define test_assert(expression, description)                  \
    if((expression) != true) {                                \
        std::cerr << __func__ << ": " << description << '\n'; \
        return 1;                                             \
    }

// json.cpp tests

int test_json()
{
    static std::string test_json = "{ \"testnumber\" : 1, \"teststring\" : \"stringdata\" }";
    json test(test_json);

    test_assert(test.is_object(), "test json not of object type");
    test_assert(json(test, "testnumber").value<int>() == 1, "test object number child not correctly output as int");
    test_assert(json(test, "testnumber").value<double>() == 1.0, "test object number child not correctly output as double");
    test_assert(json(test, "teststring").value<std::string>() == "stringdata", "test object string child not correctly output as string");

    return 0;
}

// socketstream.cpp test

int test_socketstream()
{
    // todo
    return 0;
}
