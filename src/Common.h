#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

using string = std::string;

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