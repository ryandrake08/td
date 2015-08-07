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

    // Templated construction from container of any object - create vector, implicitly converting each element to json
    template <typename T>
    json(const T& it, const T& end) : json(std::vector<json>(it, end)) {}

    // Templated conversion to any object
    template <typename T>
    bool to(T& value) const;

    // Print to string
    std::string string(bool pretty=false) const;

    // Get value for name
    template <typename T>
    bool get_value(const char* name, T& value) const;
    template <typename T>
    bool get_value(const std::string& name, T& value) const { return get_value(name.c_str(), value); }
    template <typename T>
    bool get_values(const char* name, T& values) const
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
    void set_value(const char* name, const json& value);

    // I/O from streams
    void write(std::ostream& os) const;
    void read(std::istream& is);
};

// Stream operators
std::ostream& operator<<(std::ostream& os, const json& object);
std::istream& operator>>(std::istream& is, json& object);
