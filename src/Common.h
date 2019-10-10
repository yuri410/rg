#pragma once

#include <vector>
#include <string>
#include <optional>
#include <glm/glm.hpp>
#include <cstdint>

using string = std::string;

typedef uint32_t uint;

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


template <typename C>
bool contains(const C& c, const typename C::value_type& e)
{
    return std::find(c.begin(), c.end(), e) != c.end();
}

template <typename C, typename F>
bool contains_if(const C& c, F matcher)
{
    return std::find_if(c.begin(), c.end(), matcher) != c.end();
}