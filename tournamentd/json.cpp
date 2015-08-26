#include "json.hpp"
#include <stddef.h> // Oops, cJSON.h requires stddef.h
#include "cjson/cJSON.h"
#include <algorithm>
#include <deque>
#include <fstream>
#include <limits>
#include <memory>
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
json::json(const unsigned long long& value) : ptr(check(cJSON_CreateInt(bounds_checking_cast<unsigned long long,long long int>(value))))
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
int json::value() const
{
    ensure_type(check(this->ptr), cJSON_Number);
    return bounds_checking_cast<long long int,int>(this->ptr->valueint);
}

template <>
unsigned int json::value() const
{
    ensure_type(check(this->ptr), cJSON_Number);
    return bounds_checking_cast<long long int,unsigned int>(this->ptr->valueint);
}

template <>
long json::value() const
{
    ensure_type(check(this->ptr), cJSON_Number);
    return bounds_checking_cast<long long int,long>(this->ptr->valueint);
}

template <>
unsigned long json::value() const
{
    ensure_type(check(this->ptr), cJSON_Number);
    return bounds_checking_cast<long long int,unsigned long>(this->ptr->valueint);
}

template <>
long long json::value() const
{
    ensure_type(check(this->ptr), cJSON_Number);
    return this->ptr->valueint;
}

template <>
unsigned long long json::value() const
{
    ensure_type(check(this->ptr), cJSON_Number);
    return bounds_checking_cast<long long int,unsigned long long>(this->ptr->valueint);
}

template <>
double json::value() const
{
    ensure_type(check(this->ptr), cJSON_Number);
    return this->ptr->valuedouble;
}

template <>
std::string json::value() const
{
    ensure_type(check(this->ptr), cJSON_String);
    return this->ptr->valuestring;
}

template <>
bool json::value() const
{
    if((check(this->ptr)->type & 0xff) == cJSON_True)
    {
        return true;
    }
    else if((check(this->ptr)->type & 0xff) == cJSON_False)
    {
        return false;
    }
    else
    {
        throw std::invalid_argument("object not of type bool");
    }
}

template <>
std::vector<json> json::value() const
{
    ensure_type(check(this->ptr), cJSON_Array);
    std::vector<json> ret;
    for(int i=0; i<cJSON_GetArraySize(this->ptr); i++)
    {
        auto item(cJSON_GetArrayItem(this->ptr, i));
        if(item == nullptr)
        {
            throw std::out_of_range("array item does not exist: " + std::to_string(i));
        }
        ret.push_back(json(const_cast<cJSON*>(cJSON_Duplicate(item, 1))));
    }
    return ret;
}

// Printer
std::string json::string(bool pretty) const
{
    std::unique_ptr<char, decltype(std::free)*> buf { (pretty ? cJSON_Print(this->ptr) : cJSON_PrintUnformatted(this->ptr)), std::free };
    return std::string(buf.get());
}

// Generic JSON getter
bool json::get_value(const char* name, json& value) const
{
    auto item(cJSON_GetObjectItem(this->ptr, name));
    if(item != nullptr)
    {
        value = json(cJSON_Duplicate(item, 1));
        return true;
    }
    return false;
}

// Generic JSON setter
void json::set_value(const char* name, const json& value)
{
    cJSON_AddItemToObject(this->ptr, name, cJSON_Duplicate(value.ptr, 1));
}

std::ostream& operator<<(std::ostream& os, const json& object)
{
    os << object.string();
    return os;
}

std::istream& operator>>(std::istream& is, json& object)
{
    std::string buffer;
    is >> buffer;
    object = json::eval(buffer);
    return is;
}