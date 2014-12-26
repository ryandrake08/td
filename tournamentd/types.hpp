#pragma once
#include "json.hpp"
#include <cstddef>
#include <limits>
#include <stdexcept>

class tournament_error : public std::runtime_error
{
public:
    explicit tournament_error(const char* what_arg) : std::runtime_error(what_arg) {}
};

typedef std::size_t player_id;
static const player_id no_player = std::numeric_limits<std::size_t>::max();