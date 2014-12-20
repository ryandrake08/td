#pragma once
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
    explicit json(const std::vector<json>& values);

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

    // Get value
    template <typename T>
    T value() const;

    // Get value for name
    template <typename T>
    T value(const char* name) const;
    template <typename T>
    T value(const std::string& name) const { value<T>(name.c_str()); }

    // Set value for name
    template <typename T>
    json& set_value(const char* name, const T& value);
    template <typename T>
    json& set_value(const std::string& name, const T& value) { return set_value(name.c_str(), value); }

    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const json& object);
    friend std::istream& operator>>(std::istream& is, json& object);
};