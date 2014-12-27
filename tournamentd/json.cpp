#include "json.hpp"
#include <stddef.h> // Oops, cJSON.h requires stddef.h
#include "cjson/cJSON.h"
#include <algorithm>
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

void ensure_type(cJSON* object, int type)
{
    if((object->type & 0xff) != type)
    {
        throw std::runtime_error("object not of type: " + std::to_string(type));
    }
}

static cJSON* safe_GetArrayItem(cJSON* object, int i)
{
    ensure_type(object, cJSON_Array);
    auto obj(cJSON_GetArrayItem(object, i));
    if(obj == nullptr)
    {
        throw std::runtime_error("array item does not exist" + std::to_string(i));
    }
    return obj;
}

static int safe_valueint(cJSON* object)
{
    ensure_type(object, cJSON_Number);
    return object->valueint;
}

static double safe_valuedouble(cJSON* object)
{
    ensure_type(object, cJSON_Number);
    return object->valuedouble;
}

static const char* safe_valuestring(cJSON* object)
{
    ensure_type(object, cJSON_String);
    return object->valuestring;
}

static bool safe_valuebool(cJSON* object)
{
    if((object->type & 0xff) == cJSON_True)
    {
        return true;
    }
    else if((object->type & 0xff) == cJSON_False)
    {
        return false;
    }
    else
    {
        throw std::runtime_error("object not boolean");
    }
}

static unsigned int safe_valueuint(cJSON* object)
{
    ensure_type(object, cJSON_Number);
    if(object->valueint < std::numeric_limits<unsigned int>::min())
    {
        throw std::runtime_error("object has negative value");
    }
    return static_cast<unsigned int>(object->valueint);
}

static unsigned long safe_valueulong(cJSON* object)
{
    ensure_type(object, cJSON_Number);
    if(object->valueint < std::numeric_limits<unsigned long>::min())
    {
        throw std::runtime_error("object has negative value");
    }
    return static_cast<unsigned long>(object->valueint);
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
template <>
json::json(const std::vector<json>& values) : ptr(cJSON_CreateArray())
{
    check();
    for(auto item : values)
    {
        cJSON_AddItemToArray(this->ptr, cJSON_Duplicate(item.ptr, 1));
    }
}

template <>
json::json(const std::vector<int>& values) : ptr(cJSON_CreateIntArray(&values[0], static_cast<int>(values.size())))
{
    check();
}

template <>
json::json(const std::vector<float>& values) : ptr(cJSON_CreateFloatArray(&values[0], static_cast<int>(values.size())))
{
    check();
}

template <>
json::json(const std::vector<double>& values) : ptr(cJSON_CreateDoubleArray(&values[0], static_cast<int>(values.size())))
{
    check();
}

template <>
json::json(const std::vector<std::string>& values)
{
    std::vector<const char*> tmp(values.size());
    // workaround for clang using lambda:
    // should be able to pass mem_fn(&string::c_str) to transform
    std::transform(values.begin(), values.end(), tmp.begin(), [](const std::string& str) { return str.c_str(); });
    ptr = cJSON_CreateStringArray(&tmp[0], static_cast<int>(tmp.size()));
    check();
}

template <>
json::json(const std::vector<unsigned long>& values)
{
    std::vector<int> tmp(values.begin(), values.end());
    ptr = cJSON_CreateIntArray(&tmp[0], static_cast<int>(tmp.size()));
    check();
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

// Specialized getters
template <>
bool json::get_value(const char* name, int& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        value = safe_valueint(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<long>(const char* name, long& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        value = safe_valueint(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<unsigned int>(const char* name, unsigned int& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        value = safe_valueuint(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<unsigned long>(const char* name, unsigned long& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        value = safe_valueulong(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<std::string>(const char* name, std::string& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        value = safe_valuestring(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<double>(const char* name, double& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        value = safe_valuedouble(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<bool>(const char* name, bool& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        value = safe_valuebool(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<json>(const char* name, json& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        ensure_type(obj, cJSON_Object);
        value = json::dup(obj);
        return true;
    }
    else
    {
        return false;
    }
}

template <>
bool json::get_value<std::vector<json>>(const char* name, std::vector<json>& value) const
{
    auto obj(cJSON_GetObjectItem(this->ptr, name));
    if(obj != nullptr)
    {
        auto size(cJSON_GetArraySize(this->ptr));

        value.resize(size);

        for(int i=0; i<size; i++)
        {
            value[i] = json::dup(safe_GetArrayItem(this->ptr, i));
        }

        return true;
    }
    else
    {
        return false;
    }
}

// Specialized setters
template <>
json& json::set_value<int>(const char* name, const int& value)
{
    cJSON_AddNumberToObject(this->ptr, name, value);

    return *this;
}

template <>
json& json::set_value<long>(const char* name, const long& value)
{
    if(value > std::numeric_limits<int>::max())
    {
        throw std::runtime_error("unsigned long value would overflow json int");
    }

    cJSON_AddNumberToObject(this->ptr, name, static_cast<int>(value));

    return *this;
}

template <>
json& json::set_value<unsigned int>(const char* name, const unsigned int& value)
{
    if(value > std::numeric_limits<int>::max())
    {
        throw std::runtime_error("unsigned int value would overflow json int");
    }

    cJSON_AddNumberToObject(this->ptr, name, static_cast<int>(value));

    return *this;
}

template <>
json& json::set_value<unsigned long>(const char* name, const unsigned long& value)
{
    if(value > std::numeric_limits<int>::max())
    {
        throw std::runtime_error("unsigned long value would overflow json int");
    }

    cJSON_AddNumberToObject(this->ptr, name, static_cast<int>(value));

    return *this;
}

template <>
json& json::set_value<long long>(const char* name, const long long& value)
{
    if(value > std::numeric_limits<int>::max())
    {
        throw std::runtime_error("unsigned long value would overflow json int");
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
json& json::set_value<const char*>(const char* name, const char* const& value)
{
    cJSON_AddStringToObject(this->ptr, name, value);

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

// I/O from streams
void json::write(std::ostream& os) const
{
    auto buffer(cJSON_PrintUnformatted(this->ptr));
    os << buffer;
    free(buffer);
}

void json::read(std::istream& is)
{
    std::string buffer;
    is >> buffer;

    if(this->ptr != nullptr)
    {
        // Delete current cJSON
        cJSON_Delete(this->ptr);
    }

    // Parse into new cJSON
    this->ptr = cJSON_Parse(buffer.c_str());

    check();
}

std::ostream& operator<<(std::ostream& os, const json& object)
{
    object.write(os);
    return os;
}

std::istream& operator>>(std::istream& is, json& object)
{
    object.read(is);
    return is;
}