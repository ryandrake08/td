#pragma once
#include <string>

// Forward-declare
struct cJSON;

class json
{
    cJSON* ptr;

    // Verify valid ptr
    void check() const;

    // Construct from raw cJSON pointer
    explicit json(cJSON* raw_ptr);

    // Construct from parent raw cJSON and object name
    json(cJSON* parent, const std::string& name);

public:
    // Construct from json string
    explicit json(const std::string& str);

    // Construct from parent json and object name
    json(const json& parent, const std::string& name);

    // Construct from file
    static json load(const std::string& filename);

    // Copy construction
    json(const json& other);

    // Assignment
    json& operator=(const json& other);

    // Destruction
    ~json();

    // Is this json a cJSON_Object?
    bool is_object() const;

    // Does this object contain a given child object?
    bool has_object(const std::string& name) const;

    // Get value
    template <typename T>
    T value() const;
};