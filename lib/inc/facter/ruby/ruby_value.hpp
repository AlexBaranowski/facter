/**
 * @file
 * Declares the Ruby fact value.
 */
#pragma once

#include "api.hpp"
#include "fact.hpp"
#include "../facts/value.hpp"

namespace facter { namespace ruby {

    /**
     * Represents a value for a Ruby fact.
     */
    struct ruby_value : facter::facts::value
    {
        /**
         * Constructs a ruby_value.
         * @param value The Ruby value.
         */
        ruby_value(VALUE value);

        /**
         * Destructs a ruby_value.
         */
        ~ruby_value();

        /**
         * Prevents the ruby_value from being copied.
         */
        ruby_value(ruby_value const&) = delete;

        /**
         * Prevents the ruby_value from being copied.
         * @returns Returns this ruby_value.
         */
        ruby_value& operator=(ruby_value const&) = delete;

        /**
         * Moves the given ruby_value into this ruby_value.
         * @param other The ruby_value to move into this ruby_value.
         */
        ruby_value(ruby_value&& other);

        /**
         * Moves the given ruby_value into this ruby_value.
         * @param other The ruby_value to move into this ruby_value.
         * @return Returns this ruby_value.
         */
        ruby_value& operator=(ruby_value&& other);

        /**
         * Converts the value to a JSON value.
         * @param allocator The allocator to use for creating the JSON value.
         * @param value The returned JSON value.
         */
        virtual void to_json(rapidjson::Allocator& allocator, rapidjson::Value& value) const;

        /**
          * Writes the value to the given stream.
          * @param os The stream to write to.
          * @param quoted True if string values should be quoted or false if not.
          * @returns Returns the stream being written to.
          */
        virtual std::ostream& write(std::ostream& os, bool quoted = true) const;

        /**
          * Writes the value to the given YAML emitter.
          * @param emitter The YAML emitter to write to.
          * @returns Returns the given YAML emitter.
          */
        virtual YAML::Emitter& write(YAML::Emitter& emitter) const;

        /**
         * Gets the Ruby value.
         * @return Returns the Ruby value.
         */
        VALUE value() const;

     private:
        static void to_json(api const& ruby, VALUE value, rapidjson::Allocator& allocator, rapidjson::Value& json);
        static void write(api const& ruby, VALUE value, std::ostream& os, bool quoted);
        static void write(api const& ruby, VALUE value, YAML::Emitter& emitter);

        VALUE _value;
    };

}}  // namespace facter::ruby
