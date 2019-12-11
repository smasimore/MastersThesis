/**
 * Shared enum class hash for using unordered_maps keyed off of an enum class.
 */

#ifndef ENUM_CLASS_HASH_HPP
#define ENUM_CLASS_HASH_HPP

#include <cstring>

/*
 * In order to use an enum class as the key for an unordered_map, we need to 
 * define a hash function that maps the key type to size_t. Hash function 
 * copied from:
 * https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
 */
struct EnumClassHash
{
    template <typename T>
    std::size_t operator () (T t) const
    {
        return static_cast<std::size_t> (t);
    }
};


#endif
