#include "x/data/json/json_cpp_reader.h"

namespace x::data
{

JsonCppReader::Reader::Reader(const Json::Value* value, std::string* error_message, const ReadPolicy policy)
    : current_(value)
    , error_message_(error_message)
    , policy_(policy)
{
}

JsonCppReader::Reader& JsonCppReader::Reader::SetPolicy(const ReadPolicy policy)
{
    policy_ = policy;
    return *this;
}

JsonCppReader::Reader& JsonCppReader::Reader::SetDefaultPolicy()
{
    policy_ = ReadPolicy::RETURN_DEFAULT;
    return *this;
}

JsonCppReader::Reader& JsonCppReader::Reader::SetErrorPolicy()
{
    policy_ = ReadPolicy::RETURN_ERROR;
    return *this;
}

JsonCppReader::Reader& JsonCppReader::Reader::SetExceptionPolicy()
{
    policy_ = ReadPolicy::THROW_EXCEPTION;
    return *this;
}

JsonCppReader::Reader& JsonCppReader::Reader::SerErrorMessage(std::string* error_message)
{
    error_message_ = error_message;
    return *this;
}

JsonCppReader::Reader& JsonCppReader::Reader::ClearError()
{
    if (error_message_)
    {
        error_message_->clear();
    }
    return *this;
}

bool JsonCppReader::Reader::Error() const
{
    return policy_ != ReadPolicy::RETURN_ERROR || error_message_ == nullptr || !error_message_->empty();
}

const std::string& JsonCppReader::Reader::ErrorMessage() const
{
    static std::string empty;
    return error_message_ ? *error_message_ : empty;
}

JsonCppReader::Reader JsonCppReader::Reader::Field(const std::string& name) const
{
    if (!current_ || !current_->isObject() || !current_->isMember(name)) {
        return Reader(nullptr, error_message_, policy_);
    }
    return Reader(&(*current_)[name], error_message_, policy_);
}

const Json::Value* JsonCppReader::Reader::Raw() const
{
    if (current_) return current_;
    if (policy_  == ReadPolicy::THROW_EXCEPTION)
    {
        throw std::runtime_error("cannot read from null or non-object JSON");
    }
    if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
    {
        *error_message_ = "cannot read from null or non-object JSON";
    }
    static Json::Value empty;
    return &empty;
}

std::string JsonCppReader::Reader::ToString() const
{
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    return Json::writeString(builder, *Raw());
}

const char* JsonCppReader::Reader::get_json_type_name(const Json::ValueType type)
{
    switch (type) {
        case Json::nullValue: return "null";
        case Json::intValue: return "int";
        case Json::uintValue: return "uint";
        case Json::realValue: return "real";
        case Json::stringValue: return "string";
        case Json::booleanValue: return "bool";
        case Json::arrayValue: return "array";
        case Json::objectValue: return "object";
        default: return "unknown";
    }
}

JsonCppReader::JsonCppReader(const Json::Value* value)
    : root_(value)
{}

}
