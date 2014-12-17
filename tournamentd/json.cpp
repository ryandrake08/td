#include "json.hpp"
#include <stddef.h> // Oops, cJSON.h requires stddef.h
#include "cjson/cJSON.h"
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <streambuf>
#include <string>

// Verify valid ptr
void json::check() const
{
    assert(this->ptr != nullptr);

    // Check root json pointer
    if(this->ptr == nullptr)
    {
        throw std::logic_error("json: uninitialized or invalid object");
    }
}

// Construct from raw cJSON pointer
json::json(cJSON* raw_ptr) : ptr(raw_ptr)
{
    check();
}

// Construct from json string
json::json(const std::string& str) : ptr(cJSON_Parse(str.c_str()))
{
    check();
}

// Construct from parent and object name
json::json(cJSON* parent, const std::string& name) : ptr(cJSON_Duplicate(cJSON_GetObjectItem(parent, name.c_str()), 1))
{
    check();
}

// Construct from parent json and object name
json::json(const json& parent, const std::string& name) : ptr(cJSON_Duplicate(cJSON_GetObjectItem(parent.ptr, name.c_str()), 1))
{
    check();
}

// Construct from file
json json::load(const std::string& filename)
{
    // Read file
    std::ifstream file(filename);
    if(file.fail())
    {
        throw std::runtime_error("json: " + filename + " not found");
    }

    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return json(str);
}

// Copy constructor duplicates
json::json(const json& other) : ptr(cJSON_Duplicate(other.ptr, 1))
{
    check();
}

// Assignment duplicates
json& json::operator=(const json& other)
{
    if(this != &other)
    {
        // Delete current cJSON
        cJSON_Delete(this->ptr);

        // Duplicate other one
        this->ptr = cJSON_Duplicate(other.ptr, 1);
    }

    check();
    return *this;
}

// Delete raw cJSON pointer
json::~json()
{
    cJSON_Delete(this->ptr);
}

// Is this json a cJSON_Object?
bool json::is_object() const
{
    return (this->ptr->type & 0xff) == cJSON_Object;
}

// Does this object contain a given child object?
bool json::has_object(const std::string& name) const
{
    return cJSON_GetObjectItem(this->ptr, name.c_str()) != nullptr;
}

// Specialized getters
template <>
int json::value<int>() const
{
    if((this->ptr->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error("json: value<int> not a number type");
    }
    return this->ptr->valueint;
}

template <>
std::string json::value<std::string>() const
{
    if((this->ptr->type & 0xff) != cJSON_String)
    {
        throw std::runtime_error("json: value<string> not a string type");
    }
    return this->ptr->valuestring;
}

template <>
double json::value<double>() const
{
    if((this->ptr->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error("json: value<double> not a number type");
    }
    return this->ptr->valuedouble;
}

template <>
bool json::value<bool>() const
{
    if((this->ptr->type & 0xff) == cJSON_True)
    {
        return true;
    }
    else if((this->ptr->type & 0xff) == cJSON_False)
    {
        return false;
    }
    else
    {
        throw std::runtime_error("json: value<bool> not a boolean type");
    }
}
