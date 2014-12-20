#include "json.hpp"
#include <stddef.h> // Oops, cJSON.h requires stddef.h
#include "cjson/cJSON.h"
#include <cassert>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <vector>

// Verify valid ptr
void json::check() const
{
    assert(this->ptr != nullptr);

    // Check root json pointer
    if(this->ptr == nullptr)
    {
        throw std::logic_error("uninitialized or invalid object");
    }
}

static cJSON* safe_GetObjectItem(cJSON* object, const char* string)
{
    auto obj(cJSON_GetObjectItem(object, string));
    if(obj == nullptr)
    {
        throw std::runtime_error(std::string("child does not exist: ") + string);
    }
    return obj;
}

static cJSON* safe_GetArrayItem(cJSON* array, int i)
{
    auto obj(cJSON_GetArrayItem(array, i));
    if(obj == nullptr)
    {
        throw std::runtime_error("array item does not exist");
    }
    return obj;
}

// Construct an empty object
json::json() : ptr(cJSON_CreateObject())
{
    check();
}

// Construct from raw cJSON pointer
json::json(cJSON* raw_ptr) : ptr(raw_ptr)
{
    check();
}

json json::dup(cJSON* raw_ptr)
{
    return json(cJSON_Duplicate(raw_ptr, 1));
}

// Construct from json string
json::json(const char* str) : ptr(cJSON_Parse(str))
{
    check();
}
json::json(const std::string& str) : json(str.c_str())
{
}

// Construct from an array
json::json(const std::vector<json>& values) : ptr(cJSON_CreateArray())
{
    check();
    for(auto item : values)
    {
        cJSON_AddItemToArray(this->ptr, cJSON_Duplicate(item.ptr, 1));
    }
}

// Construct from file
json json::load(const char* filename)
{
    // Read file
    std::ifstream file(filename);
    if(file.fail())
    {
        throw std::runtime_error(std::string("file not found: ") + filename);
    }

    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return json(str.c_str());
}
json json::load(const std::string& filename)
{
    return json::load(filename.c_str());
}

// Copy constructor duplicates
json::json(const json& other) : ptr(cJSON_Duplicate(other.ptr, 1))
{
    check();
}

// Move constructor moves
json::json(json&& other) : ptr(other.ptr)
{
    other.ptr = nullptr;
    check();
}

// Copy assignment duplicates
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

// Copy assignment duplicates
json& json::operator=(json&& other)
{
    if(this != &other)
    {
        // Delete current cJSON
        cJSON_Delete(this->ptr);

        // Copy the other one
        this->ptr = other.ptr;
        other.ptr = nullptr;
    }

    check();
    return *this;
}

// Delete raw cJSON pointer
json::~json()
{
    if(this->ptr != nullptr)
    {
        cJSON_Delete(this->ptr);
    }
}

// Is this json a cJSON_Object?
bool json::is_object() const
{
    return (this->ptr->type & 0xff) == cJSON_Object;
}

// Does this object contain a given child object?
bool json::has_object(const char* name) const
{
    return cJSON_GetObjectItem(this->ptr, name) != nullptr;
}
bool json::has_object(const std::string& name) const
{
    return this->has_object(name.c_str());
}

// Specialized getters
template <>
int json::value<int>() const
{
    if((this->ptr->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error("value<int> for object not a number type");
    }
    return this->ptr->valueint;
}

template <>
unsigned long json::value<unsigned long>() const
{
    if((this->ptr->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error("value<unsigned long> for object not a number type");
    }
    if(this->ptr->valueint < std::numeric_limits<unsigned long>::min())
    {
        throw std::runtime_error("value<unsigned long> for object with negative value");
    }
    return static_cast<unsigned long>(this->ptr->valueint);
}

template <>
std::string json::value<std::string>() const
{
    if((this->ptr->type & 0xff) != cJSON_String)
    {
        throw std::runtime_error("value<string> for object not a string type");
    }
    return this->ptr->valuestring;
}

template <>
double json::value<double>() const
{
    if((this->ptr->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error("value<double> for object not a number type");
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
        throw std::runtime_error("value<bool> for object not a boolean type");
    }
}

template <>
std::vector<json> json::value<std::vector<json>>() const
{
    if((this->ptr->type & 0xff) != cJSON_Array)
    {
        throw std::runtime_error("value<vector> for object not an array type");
    }

    std::vector<json> ret;
    auto size(cJSON_GetArraySize(this->ptr));
    for(int i=0; i<size; i++)
    {
        ret.push_back(json::dup(safe_GetArrayItem(this->ptr, i)));
    }
    return ret;
}


template <>
int json::value<int>(const char* name) const
{
    auto obj(safe_GetObjectItem(this->ptr, name));
    if((obj->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error(std::string("not a number type: ") + name);
    }
    return obj->valueint;
}

template <>
unsigned long json::value<unsigned long>(const char* name) const
{
    auto obj(safe_GetObjectItem(this->ptr, name));
    if((obj->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error(std::string("not a number type: ") + name);
    }
    if(obj->valueint < std::numeric_limits<unsigned long>::min())
    {
        throw std::runtime_error("value<unsigned long> for object with negative value");
    }
    return static_cast<unsigned long>(obj->valueint);
}

template <>
std::string json::value<std::string>(const char* name) const
{
    auto obj(safe_GetObjectItem(this->ptr, name));
    if((obj->type & 0xff) != cJSON_String)
    {
        throw std::runtime_error(std::string("not a string type: ") + name);
    }
    return obj->valuestring;
}

template <>
double json::value<double>(const char* name) const
{
    auto obj(safe_GetObjectItem(this->ptr, name));
    if((obj->type & 0xff) != cJSON_Number)
    {
        throw std::runtime_error(std::string("not a number type: ") + name);
    }
    return obj->valuedouble;
}

template <>
bool json::value<bool>(const char* name) const
{
    auto obj(safe_GetObjectItem(this->ptr, name));
    if((obj->type & 0xff) == cJSON_True)
    {
        return true;
    }
    else if((obj->type & 0xff) == cJSON_False)
    {
        return false;
    }
    else
    {
        throw std::runtime_error(std::string("not a boolean type: ") + name);
    }
}

template <>
json json::value<json>(const char* name) const
{
    auto obj(safe_GetObjectItem(this->ptr, name));
    if((obj->type & 0xff) != cJSON_Object)
    {
        throw std::runtime_error(std::string("not an object type: ") + name);
    }
    return json::dup(obj);
}

template <>
std::vector<json> json::value<std::vector<json>>(const char* name) const
{
    auto obj(safe_GetObjectItem(this->ptr, name));
    if((obj->type & 0xff) != cJSON_Array)
    {
        throw std::runtime_error(std::string("not an array type: ") + name);
    }

    std::vector<json> ret;
    auto size(cJSON_GetArraySize(obj));
    for(int i=0; i<size; i++)
    {
        ret.push_back(json::dup(safe_GetArrayItem(obj, i)));
    }
    return ret;
}

// Specialized setters
template <>
json& json::set_value<int>(const char* name, const int& value)
{
    cJSON_AddNumberToObject(this->ptr, name, value);

    return *this;
}

template <>
json& json::set_value<unsigned long>(const char* name, const unsigned long& value)
{
    if(value > std::numeric_limits<int>::max())
    {
        throw std::runtime_error("value<unsigned long> for object with negative value");
    }

    cJSON_AddNumberToObject(this->ptr, name, static_cast<int>(value));

    return *this;
}

template <>
json& json::set_value<std::string>(const char* name, const std::string& value)
{
    cJSON_AddStringToObject(this->ptr, name, value.c_str());

    return *this;
}

template <>
json& json::set_value<double>(const char* name, const double& value)
{
    cJSON_AddNumberToObject(this->ptr, name, value);

    return *this;
}

template <>
json& json::set_value<bool>(const char* name, const bool& value)
{
    if(value)
    {
        cJSON_AddTrueToObject(this->ptr, name);
    }
    else
    {
        cJSON_AddFalseToObject(this->ptr, name);
    }

    return *this;
}

template <>
json& json::set_value<json>(const char* name, const json& value)
{
    cJSON_AddItemToObject(this->ptr, name, cJSON_Duplicate(value.ptr, 1));

    return *this;
}

template <>
json& json::set_value<std::vector<json>>(const char* name, const std::vector<json>& values)
{
    return set_value<json>(name, json(values));
}

std::ostream& operator<<(std::ostream& os, const json& object)
{
    auto buffer(cJSON_PrintUnformatted(object.ptr));
    os << buffer;
    free(buffer);
    return os;
}

std::istream& operator>>(std::istream& is, json& object)
{
    std::string buffer;
    is >> buffer;
    object = json(buffer);
    return is;
}