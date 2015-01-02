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

    // Need to call above constructor
    friend bool get_json_value(const cJSON* obj, json& value);
    friend bool get_json_array_value(const cJSON* obj, std::vector<json>& value);

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
    json(json&& other);

    // Assignment
    json& operator=(const json& other);
    json& operator=(json&& other);

    // Destruction
    ~json();

    // Templated construction from any object
    template <typename T>
    json(const T& value);

    // Print to string
    std::string string(bool pretty=false) const;

    // Is this json a cJSON_Object?
    bool is_object() const;

    // Does this object contain a given child object?
    bool has_object(const char* name) const;
    bool has_object(const std::string& name) const { return has_object(name.c_str()); }

    // Get value for name
    template <typename T>
    bool get_value(const char* name, T& value) const;
    template <typename T>
    bool get_value(const std::string& name, T& value) const { return get_value(name.c_str(), value); }
    template <typename T>
    bool get_value(const char* name, std::vector<T>& values) const
    {
        std::vector<json> array;
        if(this->get_value(name, array))
        {
            values.resize(array.size());
            std::copy(array.begin(), array.end(), values.begin());
            return true;
        }
        return false;
    }

    // Set value for name
    template <typename T>
    json& set_value(const char* name, const T& value);
    template <typename T>
    json& set_value(const std::string& name, const T& value) { return set_value(name.c_str(), value); }
    template <typename T>
    json& set_value(const char* name, const std::vector<T>& values)
    {
        std::vector<json> array;
        array.reserve(values.size());
        std::copy(values.begin(), values.end(), std::back_inserter(array));
        this->set_value(name, array);
        return *this;
    }

    // I/O from streams
    void write(std::ostream& os) const;
    void read(std::istream& is);
};

// Stream operators
std::ostream& operator<<(std::ostream& os, const json& object);
std::istream& operator>>(std::istream& is, json& object);
