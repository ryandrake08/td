#pragma once
#include <functional>
#include <string>
#include <vector>

// Forward-declare
struct cJSON;

class json
{
    cJSON* ptr;

    // Verify valid ptr
    void check() const;

    // Construct from raw cJSON pointer
    explicit json(cJSON* raw_ptr);
    static json dup(cJSON* raw_ptr);

public:
    // Construct an empty object
    json();

    // Construct from json string
    explicit json(const char* str);
    explicit json(const std::string& str);

    // Construct from file
    static json load(const char* filename);
    static json load(const std::string& filename);

    // Construct from an array
    template <typename T>
    explicit json(const std::vector<T>& values);

    // Copy/move construction
    json(const json& other);
    json(json&& other);

    // Assignment
    json& operator=(const json& other);
    json& operator=(json&& other);

    // Destruction
    ~json();

    // Is this json a cJSON_Object?
    bool is_object() const;

    // Does this object contain a given child object?
    bool has_object(const char* name) const;
    bool has_object(const std::string& name) const;

    // Perform a function on each array element
    void for_each(const std::function<void(const json&,int)>& func) const;
    void for_each(const char* name, const std::function<void(const json&,int)>& func) const;
    void for_each(const std::string& name, const std::function<void(const json&,int)>& func) const;

    // Get value
    template <typename T>
    T value() const;

    // Get value for name
    template <typename T>
    bool get_value(const char* name, T& value) const;
    template <typename T>
    bool get_value(const std::string& name, T& value) const { return get_value(name.c_str(), value); }

    // Set value for name
    template <typename T>
    json& set_value(const char* name, const T& value);
    template <typename T>
    json& set_value(const std::string& name, const T& value) { return set_value(name.c_str(), value); }

    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const json& object);
    friend std::istream& operator>>(std::istream& is, json& object);
};