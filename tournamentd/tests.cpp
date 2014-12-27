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

    int testnumber(0);
    test_assert(test.get_value<int>("testnumber", testnumber) == true, "test object number child not correctly output as int");
    test_assert(testnumber == 1, "test object int child not correctly interpreted");

    double testdouble(0.0);
    test_assert(test.get_value<double>("testnumber", testdouble) == true, "test object number child not correctly output as double");
    test_assert(testdouble == 1.0, "test object double child not correctly interpreted");

    std::string teststring;
    test_assert(test.get_value<std::string>("teststring", teststring) == true, "test object string child not correctly output as string");
    test_assert(teststring == "stringdata", "test object string child not correctly interpreted");

    return 0;
}

// socketstream.cpp test

int test_socketstream()
{
    // todo
    return 0;
}
