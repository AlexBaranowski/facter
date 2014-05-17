#include <facter/facts/scalar_value.hpp>
#include <rapidjson/document.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace rapidjson;
using namespace YAML;

namespace facter { namespace facts {

    template <>
    void scalar_value<string>::to_json(Allocator& allocator, rapidjson::Value& value) const
    {
        value.SetString(_value.c_str(), _value.size());
    }

    template <>
    void scalar_value<int64_t>::to_json(Allocator& allocator, rapidjson::Value& value) const
    {
        value.SetInt64(_value);
    }

    template <>
    void scalar_value<bool>::to_json(Allocator& allocator, rapidjson::Value& value) const
    {
        value.SetBool(_value);
    }

    template <>
    Emitter& scalar_value<string>::write(Emitter& emitter) const
    {
        // Unfortunately, yaml-cpp doesn't handle quoting strings automatically that well
        // For instance, if the string is an integer, no quotes are written out
        // This will cause someone parsing the YAML to see the type as an integer and
        // not as a string.
        emitter << DoubleQuoted << _value;
        return emitter;
    }

    template struct scalar_value<string>;
    template struct scalar_value<int64_t>;
    template struct scalar_value<bool>;

}}  // namespace facter::facts
