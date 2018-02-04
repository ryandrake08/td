#include "json.hpp"
#include <stddef.h> // Oops, cJSON.h requires stddef.h
#include "cJSON/cJSON.h"
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
        throw std::runtime_error("invalid json");
    }
    return obj;
}

static void ensure_type(cJSON* object, int type)
{
    if((object->type & 0xff) != type)
    {
        throw std::invalid_argument("object not of type: " + std::to_string(type));
    }
}

static bool identical(cJSON* j0, cJSON* j1)
{
    // compare type
    if((j0->type & 0xff) != (j1->type & 0xff))
    {
        return false;
    }

    // compare valuestring
    if(j0->valuestring != nullptr && j0->valuestring != nullptr)
    {
        // both strings exist and are different
        if(strcmp(j0->valuestring, j1->valuestring))
        {
            return false;
        }
    }
    else if(j0->valuestring != nullptr || j1->valuestring != nullptr)
    {
        // one string is null and the other one is not
        return false;
    }

    // compare valueint
    if(j0->valueint != j1->valueint)
    {
        return false;
    }

    // compare valuedouble
    if(j0->valuedouble != j1->valuedouble)
    {
        return false;
    }

    // compare valuestring
    if(j0->string != nullptr && j0->string != nullptr)
    {
        // both strings exist and are different
        if(strcmp(j0->string, j1->string))
        {
            return false;
        }
    }
    else if(j0->string != nullptr || j1->string != nullptr)
    {
        // one string is null and the other one is not
        return false;
    }

    // ensure number of sub-objects is identical
    auto s0(cJSON_GetArraySize(j0));
    auto s1(cJSON_GetArraySize(j1));
    if(s0 != s1)
    {
        return false;
    }

    // deep compare subobjects
    for(auto i(0); i<s0; i++)
    {
        if(!identical(cJSON_GetArrayItem(j0, i), cJSON_GetArrayItem(j1, i)))
        {
            return false;
        }
    }

    return true;
}

template <typename Ts, typename Td>
static Td bounds_checking_cast(Ts from)
{
    if(static_cast<double>(from) < static_cast<double>(std::numeric_limits<Td>::lowest()))
    {
        throw std::out_of_range("value " + std::to_string(from) + " would underflow " + std::to_string(std::numeric_limits<Td>::lowest()));
    }

    if(static_cast<double>(from) > static_cast<double>(std::numeric_limits<Td>::max()))
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
json::json(json&& other) noexcept : ptr(check(other.ptr))
{
    other.ptr = nullptr;
}

// Copy assignment duplicates
json& json::operator=(const json& other)
{
    auto tmp(other);
    std::swap(this->ptr, tmp.ptr);
    return *this;
}

// Move assignment moves
json& json::operator=(json&& other) noexcept
{
    auto tmp(std::move(other));
    std::swap(this->ptr, tmp.ptr);
    return *this;
}

// Delete raw cJSON pointer
json::~json()
{
    cJSON_Delete(this->ptr);
}

// Validity (is underlying pointer set?)
bool json::valid() const
{
    return this->ptr != nullptr;
}

// Equality (do underlying pointers match)
bool json::equal(const json& other) const
{
    return this->ptr == other.ptr;
}

// Identical (does data match)
bool json::identical(const json& other) const
{
    return ::identical(this->ptr, other.ptr);
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
json::json(const double& value) : ptr(check(cJSON_CreateNumber(value)))
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
json::json(const bool& value) : ptr(check(cJSON_CreateBool(value ? cJSON_True : cJSON_False)))
{
}

// Construct from arrays
template <>
json::json(const std::vector<json>& value) : ptr(check(cJSON_CreateArray()))
{
    for(const auto& item : value)
    {
        cJSON_AddItemToArray(check(this->ptr), cJSON_Duplicate(item.ptr, 1));
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
    auto type(check(this->ptr)->type & 0xff);
    if(type == cJSON_True)
    {
        return true;
    }
    else if(type == cJSON_False)
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
        ret.push_back(json(cJSON_Duplicate(item, 1)));
    }
    return ret;
}

// Generic JSON getter
bool json::get_value(const char* name) const
{
    return cJSON_GetObjectItem(check(this->ptr), name) != nullptr;
}

// Generic JSON getter
bool json::get_value(const char* name, json& value) const
{
    auto item(cJSON_GetObjectItem(check(this->ptr), name));
    if(item != nullptr)
    {
        value = json(cJSON_Duplicate(item, 1));
        return true;
    }
    return false;
}

// Set json value for name
void json::set_json_value(const char* name, const json& value)
{
    cJSON_AddItemToObject(check(this->ptr), name, cJSON_Duplicate(value.ptr, 1));
}

// true if json is null or is an empty object
bool json::empty() const
{
    auto type(check(this->ptr)->type & 0xff);
    if(type == cJSON_NULL)
    {
        return true;
    }
    else if(type != cJSON_Array && type != cJSON_Object)
    {
        return false;
    }
    else
    {
        return cJSON_GetArraySize(this->ptr) == 0;
    }
}

// Printer
std::string json::print(bool pretty) const
{
    check(this->ptr);
    std::unique_ptr<char, decltype(std::free)*> buf { (pretty ? cJSON_Print(this->ptr) : cJSON_PrintUnformatted(this->ptr)), std::free };
    return std::string(buf.get());
}

static inline int mode_iword()
{
    static int i = std::ios_base::xalloc();
    return i;
}

std::ostream& json::nopretty(std::ostream& os)
{
    os.iword(mode_iword()) = 0;
    return os;
}

std::ostream& json::pretty(std::ostream& os)
{
    os.iword(mode_iword()) = 1;
    return os;
}

std::ostream& operator<<(std::ostream& os, const json& object)
{
    os << object.print(os.iword(mode_iword()) != 0);
    return os;
}

std::istream& operator>>(std::istream& is, json& object)
{
    std::string buffer;
    is >> buffer;
    object = json::eval(buffer);
    return is;
}

