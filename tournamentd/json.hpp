#pragma once
#include <functional>
#include <string>
#include <vector>

// Forward-declare
struct cJSON;

class json
{
    cJSON* ptr;

    // Construct from raw cJSON pointer
    explicit json(cJSON* raw_ptr);

public:
    // Construct an empty object
    json();

    // Construct from json string
    static json eval(const char* str);
    static json eval(const std::string& str) { return eval(str.c_str()); }

    // Construct from file
    static json load(const char* filename);
    static json load(const std::string& filename) { return load(filename.c_str()); }

    // Copy/move construction
    json(const json& other);
    json(json&& other) noexcept;

    // Assignment
    json& operator=(const json& other);
    json& operator=(json&& other) noexcept;

    // Destruction
    ~json();

    // Templated construction from any object
    template <typename T>
    explicit json(const T& value);

    // Templated construction from container of any object - create vector, implicitly converting each element to json
    template <typename T>
    explicit json(const T& it, const T& end) : json(std::vector<json>(it, end)) {}

    // Templated conversion to any object
    template <typename T>
    T value() const;
    template <typename T>
    operator T() const { return this->value<T>(); }

    // Print to string
    std::string string(bool pretty=false) const;

    // Get json value for name
    bool get_value(const char* name, json& value) const;

    // Get value for name by way of an intermediate json item
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

    // Get collection for name, by way of intermediate json items
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

    // Set json value for name
    void set_json_value(const char* name, const json& value);

    // Set value for name
    template <typename T>
    void set_value(const char* name, const T& value)
    {
        set_json_value(name, json(value));
    }
};

// Stream operators
std::ostream& operator<<(std::ostream& os, const json& object);
std::istream& operator>>(std::istream& is, json& object);
