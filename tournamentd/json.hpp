#pragma once
#include <functional>
#include <string>
#include <vector>

// Forward-declare
struct cJSON;

class json
{
    cJSON* ptr;

    // construct from raw cJSON pointer
    explicit json(cJSON* raw_ptr);

public:
    // construct an empty object
    json();

    // construct from json string
    static json eval(const char* str);
    static json eval(const std::string& str) { return eval(str.c_str()); }

    // construct from file
    static json load(const char* filename);
    static json load(const std::string& filename) { return load(filename.c_str()); }

    // copy/move construction
    json(const json& other);
    json(json&& other) noexcept;

    // Assignment
    json& operator=(const json& other);
    json& operator=(json&& other) noexcept;

    // destruction
    ~json();

    // valid (is underlying pointer set?)
    bool valid() const;

    // equal (do underlying pointers match)
    bool equal(const json& other) const;
    bool operator==(const json& other) const { return equal(other); };

    // identical (does data match)
    bool identical(const json& other) const;

    // templated construction from any object
    template <typename T>
    explicit json(const T& value);

    // templated construction from container of any object - create vector, implicitly converting each element to json
    template <typename T>
    explicit json(const T& it, const T& end) : json(std::vector<json>(it, end)) {}

    // templated conversion to any object
    template <typename T>
    T value() const;

    // return whether json value exists for name
    bool get_value(const char* name) const;

    // get json value for name
    bool get_value(const char* name, json& value) const;

    // get value for name by way of an intermediate json item
    template <typename T>
    bool get_value(const char* name, T& value) const
    {
        json item;
        if(this->get_value(name, item))
        {
            value = item.value<T>();
            return true;
        }
        return false;
    }

    // get collection for name, by way of intermediate json items
    template <typename T>
    bool get_values(const char* name, T& values) const
    {
        std::vector<json> array;
        if(this->get_value(name, array))
        {
            values = T(array.begin(), array.end());
            return true;
        }
        return false;
    }

    // set json value for name
    void set_json_value(const char* name, const json& value);

    // set value for name
    template <typename T>
    void set_value(const char* name, const T& value)
    {
        set_json_value(name, json(value));
    }

    // true if json is null or is an empty object
    bool empty() const;

    // print to string
    std::string print(bool pretty=false) const;

    // declare ostream operator as friend
    friend std::ostream& operator<<(std::ostream& os, const json& object);

    // stream insertion manipulators: pretty print or no (default)
    std::ostream& pretty(std::ostream& os);
    std::ostream& nopretty(std::ostream& os);
};

// Stream operators
std::ostream& operator<<(std::ostream& os, const json& object);
std::istream& operator>>(std::istream& is, json& object);
