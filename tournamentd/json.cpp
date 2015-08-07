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
    if((double)from < (double)std::numeric_limits<Td>::lowest())
    {
        throw std::out_of_range("value " + std::to_string(from) + " would underflow " + std::to_string(std::numeric_limits<Td>::lowest()));
    }

    if((double)from > (double)std::numeric_limits<Td>::max())
    {
        throw std::out_of_range("value " + std::to_string(from) + " would overflow " + std::to_string(std::numeric_limits<Td>::max()));
    }
    return static_cast<Td>(from);
}

template <typename T>
static bool get_int_value(const cJSON* obj, T& value)
{
    if(obj != nullptr)
    {
        ensure_type(obj, cJSON_Number);
        value = bounds_checking_cast<long long int,T>(obj->valueint);
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
            value = true;
        }
        else if((obj->type & 0xff) == cJSON_False)
        {
            value = false;
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
        // Uncomment this to limit this to returning only Objects
        // ensure_type(obj, cJSON_Object);
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
            get_json_value(item, value[i]);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool get_array_of_int_value(const cJSON* obj, std::vector<int>& value)
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
            get_int_value(item, value[i]);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool get_array_of_double_value(const cJSON* obj, std::vector<double>& value)
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
            get_double_value(item, value[i]);
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

// Construct from primatives
template <>
json::json(const int& value) : ptr(check(cJSON_CreateInt(bounds_checking_cast<int,long long int>(value))))
{
}

template <>
json::json(const unsigned int& value) : ptr(check(cJSON_CreateInt(bounds_checking_cast<unsigned int,long long int>(value))))
{
}

template <>
json::json(const long& value) : ptr(check(cJSON_CreateInt(bounds_checking_cast<long,long long int>(value))))
{
}

template <>
json::json(const unsigned long& value) : ptr(check(cJSON_CreateInt(bounds_checking_cast<unsigned long,long long int>(value))))
{
}

template <>
json::json(const long long& value) : ptr(check(cJSON_CreateInt(value)))
{
}

template <>
json::json(const unsigned long long& value) : ptr(check(cJSON_CreateInt(bounds_checking_cast<unsigned long long,double>(value))))
{
}

template <>
json::json(const double& value) : ptr(check(cJSON_CreateDouble(value)))
{
}

template <>
json::json(const char* const& value) : ptr(check(cJSON_CreateString(value)))
{
}

template <>
json::json(const std::string& value) : ptr(check(cJSON_CreateString(value.c_str())))
{
}

template <>
json::json(const bool& value) : ptr(check(cJSON_CreateBool(value != 0)))
{
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

// Templated conversion to any object
template <>
bool json::to(int& value) const
{
    if(this->ptr != nullptr)
    {
        ensure_type(this->ptr, cJSON_Number);
        value = bounds_checking_cast<long long int,int>(this->ptr->valueint);
        return true;
    }
    else
    {
        return false;
    }
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

template <>
bool json::get_value(const char* name, std::vector<int>& value) const
{
    return ::get_array_of_int_value(cJSON_GetObjectItem(this->ptr, name), value);
}

template <>
bool json::get_value(const char* name, std::vector<double>& value) const
{
    return ::get_array_of_double_value(cJSON_GetObjectItem(this->ptr, name), value);
}

// Specialized setters
void json::set_value(const char* name, const json& value)
{
    cJSON_AddItemToObject(this->ptr, name, cJSON_Duplicate(value.ptr, 1));
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