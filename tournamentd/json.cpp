#include "json.hpp"
#include <stddef.h> // Oops, cJSON.h requires stddef.h
#include "cjson/cJSON.h"
#include <algorithm>
#include <deque>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <unordered_set>
#include <vector>

// Verify valid ptr
static cJSON* check(cJSON* obj)
{
    // Throw if pointer is null
    if(obj == nullptr)
    {
        throw std::logic_error("empty or invalid json");
    }
    return obj;
}

static void ensure_type(const cJSON* object, int type)
{
    if((object->type & 0xff) != type)
    {
        throw std::invalid_argument("object not of type: " + std::to_string(type));
    }
}

template <typename Ts, typename Td>
static Td bounds_checking_cast(Ts from)
{
#if 0
    if(from < std::numeric_limits<Td>::min())
    {
        throw std::out_of_range("value " + std::to_string(from) + " would underflow " + std::to_string(std::numeric_limits<Td>::min()));
    }

    if(from > std::numeric_limits<Td>::max())
    {
        throw std::out_of_range("value " + std::to_string(from) + " would overflow " + std::to_string(std::numeric_limits<Td>::max()));
    }
#endif
    return static_cast<Td>(from);
}

template <typename T>
static bool get_int_value(const cJSON* obj, T& value)
{
    if(obj != nullptr)
    {
        ensure_type(obj, cJSON_Number);
        value = bounds_checking_cast<int,T>(obj->valueint);
        return true;
    }
    else
    {
        return false;
    }
}

template <typename T>
static bool get_double_value(const cJSON* obj, T& value)
{
    if(obj != nullptr)
    {
        ensure_type(obj, cJSON_Number);
        value = bounds_checking_cast<double,T>(obj->valuedouble);
        return true;
    }
    else
    {
        return false;
    }
}

template <typename T>
static bool get_string_value(const cJSON* obj, T& value)
{
    if(obj != nullptr)
    {
        ensure_type(obj, cJSON_String);
        value = obj->valuestring;
        return true;
    }
    else
    {
        return false;
    }
}

static bool get_bool_value(const cJSON* obj, bool& value)
{
    if(obj != nullptr)
    {
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
            throw std::invalid_argument("object not of type boolean");
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool get_json_value(const cJSON* obj, json& value)
{
    if(obj != nullptr)
    {
        ensure_type(obj, cJSON_Object);
        value = json(cJSON_Duplicate(const_cast<cJSON*>(obj), 1));
        return true;
    }
    else
    {
        return false;
    }
}

bool get_json_array_value(const cJSON* obj, std::vector<json>& value)
{
    if(obj != nullptr)
    {
        ensure_type(obj, cJSON_Array);
        auto size(cJSON_GetArraySize(const_cast<cJSON*>(obj)));
        value.resize(size);

        for(int i=0; i<size; i++)
        {
            auto item(cJSON_GetArrayItem(const_cast<cJSON*>(obj), i));
            if(item == nullptr)
            {
                throw std::out_of_range("array item does not exist: " + std::to_string(i));
            }
            value[i] = json(cJSON_Duplicate(const_cast<cJSON*>(item), 1));
        }

        return true;
    }
    else
    {
        return false;
    }
}

// Construct an empty object
json::json() : ptr(check(cJSON_CreateObject()))
{
}

// Construct from raw cJSON pointer
json::json(cJSON* raw_ptr) : ptr(check(raw_ptr))
{
}

// Construct from json string
json json::eval(const char* str)
{
    return json(check(cJSON_Parse(str)));
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

    return eval(str.c_str());
}

// Copy constructor duplicates
json::json(const json& other) : ptr(check(cJSON_Duplicate(other.ptr, 1)))
{
}

// Move constructor moves
json::json(json&& other) : ptr(check(other.ptr))
{
    other.ptr = nullptr;
}

// Copy assignment duplicates
json& json::operator=(const json& other)
{
    if(this != &other)
    {
        // Delete current cJSON
        cJSON_Delete(this->ptr);

        // Duplicate other one
        this->ptr = check(cJSON_Duplicate(other.ptr, 1));
    }

    check(this->ptr);
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
        this->ptr = check(other.ptr);
        other.ptr = nullptr;
    }
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

// Construct from arrays
template <>
json::json(const std::vector<json>& values) : ptr(check(cJSON_CreateArray()))
{
    for(const auto& item : values)
    {
        cJSON_AddItemToArray(this->ptr, cJSON_Duplicate(item.ptr, 1));
    }
}

template <>
json::json(const std::vector<int>& values) : ptr(check(cJSON_CreateIntArray(&values[0], static_cast<int>(values.size()))))
{
}

template <>
json::json(const std::vector<float>& values) : ptr(check(cJSON_CreateFloatArray(&values[0], static_cast<int>(values.size()))))
{
}

template <>
json::json(const std::vector<double>& values) : ptr(check(cJSON_CreateDoubleArray(&values[0], static_cast<int>(values.size()))))
{
}

template <>
json::json(const std::vector<const char*>& values) : ptr(check(cJSON_CreateStringArray(const_cast<const char**>(&values[0]), static_cast<int>(values.size()))))
{
}

template <>
json::json(const std::vector<std::string>& values)
{
    std::vector<const char*> tmp(values.size());
    // workaround for clang using lambda:
    // should be able to pass mem_fn(&string::c_str) to transform
    //  std::transform(values.begin(), values.end(), tmp.begin(), std::mem_fn(&std::string::c_str));
    //  std::transform(values.begin(), values.end(), tmp.begin(), std::bind(&std::string::c_str, std::placeholders::_1));
    std::transform(values.begin(), values.end(), tmp.begin(), [](const std::string& str) { return str.c_str(); });
    this->ptr = check(cJSON_CreateStringArray(&tmp[0], static_cast<int>(tmp.size())));
}

template <>
json::json(const std::vector<unsigned long>& values)
{
    std::vector<int> tmp(values.begin(), values.end());
    this->ptr = check(cJSON_CreateIntArray(&tmp[0], static_cast<int>(tmp.size())));
}

template <>
json::json(const std::deque<unsigned long>& values)
{
    std::vector<int> tmp(values.begin(), values.end());
    this->ptr = check(cJSON_CreateIntArray(&tmp[0], static_cast<int>(tmp.size())));
}

template <>
json::json(const std::unordered_set<unsigned long>& values)
{
    std::vector<int> tmp(values.begin(), values.end());
    this->ptr = check(cJSON_CreateIntArray(&tmp[0], static_cast<int>(tmp.size())));
}

std::string json::string(bool pretty) const
{
    std::string ret;

    if(pretty)
    {
        auto buffer(cJSON_Print(this->ptr));
        ret = buffer;
        free(buffer);
    }
    else
    {
        auto buffer(cJSON_PrintUnformatted(this->ptr));
        ret = buffer;
        free(buffer);
    }

    return ret;
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
    return ::get_int_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, long& value) const
{
    return ::get_int_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, unsigned int& value) const
{
    return ::get_int_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, unsigned long& value) const
{
    return ::get_int_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, double& value) const
{
    return ::get_double_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, std::string& value) const
{
    return ::get_string_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, bool& value) const
{
    return ::get_bool_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, json& value) const
{
    return ::get_json_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, std::vector<json>& value) const
{
    return ::get_json_array_value(cJSON_GetObjectItem(this->ptr, name), value);
}

// Specialized setters
template <>
json& json::set_value(const char* name, const int& value)
{
    cJSON_AddNumberToObject(this->ptr, name, value);
    return *this;
}

template <>
json& json::set_value(const char* name, const long& value)
{
    cJSON_AddNumberToObject(this->ptr, name, (bounds_checking_cast<long,int>(value)));
    return *this;
}

template <>
json& json::set_value(const char* name, const unsigned int& value)
{
    cJSON_AddNumberToObject(this->ptr, name, (bounds_checking_cast<unsigned int,int>(value)));
    return *this;
}

template <>
json& json::set_value(const char* name, const unsigned long& value)
{
    cJSON_AddNumberToObject(this->ptr, name, (bounds_checking_cast<unsigned long,int>(value)));
    return *this;
}

template <>
json& json::set_value(const char* name, const long long& value)
{
    cJSON_AddNumberToObject(this->ptr, name, (bounds_checking_cast<long long,int>(value)));
    return *this;
}


template <>
json& json::set_value(const char* name, const std::string& value)
{
    cJSON_AddStringToObject(this->ptr, name, value.c_str());
    return *this;
}

template <>
json& json::set_value(const char* name, const char* const& value)
{
    cJSON_AddStringToObject(this->ptr, name, value);
    return *this;
}

template <>
json& json::set_value(const char* name, const double& value)
{
    cJSON_AddNumberToObject(this->ptr, name, value);
    return *this;
}

template <>
json& json::set_value(const char* name, const bool& value)
{
    cJSON_AddBoolToObject(this->ptr, name, value);
    return *this;
}

template <>
json& json::set_value(const char* name, const json& value)
{
    cJSON_AddItemToObject(this->ptr, name, cJSON_Duplicate(value.ptr, 1));
    return *this;
}

template <>
json& json::set_value(const char* name, const std::vector<json>& values)
{
    return this->set_value(name, json(values));
}

// I/O from streams
void json::write(std::ostream& os) const
{
    os << this->string();
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
    this->ptr = check(cJSON_Parse(buffer.c_str()));
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