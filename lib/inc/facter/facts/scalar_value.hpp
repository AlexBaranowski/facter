/**
 * @file
 * Declares the fact value for scalar values like strings and integers.
 */
#pragma once

#include "value.hpp"
#include <cstdint>
#include <string>
#include <iostream>
#include <yaml-cpp/yaml.h>

namespace facter { namespace facts {

    /**
     * Represents a simple scalar value.
     * This type can be moved but cannot be copied.
     * @tparam T The underlying scalar type.
     */
    template <typename T>
    struct scalar_value : value
    {
        /**
         * Constructs a scalar_value.
         * @param value The scalar value to move into this object.
         */
        explicit scalar_value(T&& value) :
            _value(std::move(value))
        {
        }

        /**
         * Constructs a scalar_value.
         * @param value The scalar value to copy into this object.
         */
        explicit scalar_value(T const& value) :
            _value(value)
        {
        }

        /**
         * Prevents the scalar_value from being copied.
         */
        scalar_value(scalar_value const&) = delete;

        /**
         * Prevents the scalar_value from being copied.
         * @returns Returns this scalar_value.
         */
        scalar_value& operator=(scalar_value const&) = delete;

        /**
         * Moves the given scalar_value into this scalar_value.
         * @param other The scalar_value to move into this scalar_value.
         */
        // Visual Studio 12 still doesn't allow default for move constructor.
        scalar_value(scalar_value&& other)
        {
            *this = std::move(other);
        }

        /**
         * Moves the given scalar_value into this scalar_value.
         * @param other The scalar_value to move into this scalar_value.
         * @return Returns this scalar_value.
         */
        // Visual Studio 12 still doesn't allow default for move assignment.
        scalar_value& operator=(scalar_value&& other)
        {
            value::operator=(static_cast<struct value&&>(other));
            if (this != &other) {
                _value = std::move(other._value);
            }
            return *this;
        }

        /**
         * Converts the value to a JSON value.
         * @param allocator The allocator to use for creating the JSON value.
         * @param value The returned JSON value.
         */
        virtual void to_json(rapidjson::Allocator& allocator, rapidjson::Value& value) const;

        /**
         * Gets the underlying scalar value.
         * @return Returns the underlying scalar value.
         */
        T const& value() const { return _value; }

        /**
          * Writes the value to the given stream.
          * @param os The stream to write to.
          * @param quoted True if string values should be quoted or false if not.
          * @returns Returns the stream being written to.
          */
        virtual std::ostream& write(std::ostream& os, bool quoted = true) const
        {
            os << _value;
            return os;
        }

        /**
          * Writes the value to the given YAML emitter.
          * @param emitter The YAML emitter to write to.
          * @returns Returns the given YAML emitter.
          */
        virtual YAML::Emitter& write(YAML::Emitter& emitter) const
        {
            emitter << _value;
            return emitter;
        }

     private:
        T _value;
    };

    // Declare the specializations for JSON output
    template <>
    void scalar_value<std::string>::to_json(rapidjson::Allocator& allocator, rapidjson::Value& value) const;
    template <>
    void scalar_value<int64_t>::to_json(rapidjson::Allocator& allocator, rapidjson::Value& value) const;
    template <>
    void scalar_value<bool>::to_json(rapidjson::Allocator& allocator, rapidjson::Value& value) const;
    template <>
    void scalar_value<double>::to_json(rapidjson::Allocator& allocator, rapidjson::Value& value) const;

    // Declare the specializations for YAML output
    template <>
    YAML::Emitter& scalar_value<std::string>::write(YAML::Emitter& emitter) const;

    // Declare the specializations for stream output
    template <>
    std::ostream& scalar_value<bool>::write(std::ostream& os, bool quoted) const;
    template <>
    std::ostream& scalar_value<std::string>::write(std::ostream& os, bool quoted) const;

    // Declare the common instantiations as external; defined in scalar_value.cc
    extern template struct scalar_value<std::string>;
    extern template struct scalar_value<int64_t>;
    extern template struct scalar_value<bool>;
    extern template struct scalar_value<double>;

    // Typedef the common instantiation
    /**
     * Represents a string fact value.
     */
    typedef scalar_value<std::string> string_value;
    /**
     * Represents an integer fact value.
     */
    typedef scalar_value<int64_t> integer_value;
    /**
     * Represents a boolean fact value.
     */
    typedef scalar_value<bool> boolean_value;
    /**
     * Represents a double fact value.
     */
    typedef scalar_value<double> double_value;

}}  // namespace facter::facts
