#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <json/json.h>

namespace x::data
{

class JsonCppReader {
public:
    class Reader {
    public:
        // 读取策略
        enum class ReadPolicy {
            RETURN_DEFAULT,     // 字段不存在时返回默认值
            RETURN_ERROR,       // 字段不存在时返回错误信息
            THROW_EXCEPTION,    // 字段不存在时抛出异常
        };

        // 默认构造函数，创建空Reader
        explicit Reader(
            const Json::Value* value = nullptr,
            std::string* error_message = nullptr,
            ReadPolicy policy = ReadPolicy::RETURN_ERROR
        );
        // 移动构造函数
        Reader(Reader&&) = default;
        // 拷贝构造函数
        Reader(const Reader&) = default;
        Reader& operator=(Reader&&) noexcept = default;
        Reader& operator=(const Reader&) = default;

        // 设置读取策略
        Reader& SetPolicy(ReadPolicy policy);
        Reader& SetDefaultPolicy();
        Reader& SetErrorPolicy();
        Reader& SetExceptionPolicy();

        // 错误信息设置
        Reader& SerErrorMessage(std::string* error_message);
        Reader& ClearError();
        [[nodiscard]] bool Error() const;
        [[nodiscard]] const std::string& ErrorMessage() const;

        struct ProcessPathFunctor
        {
            const Json::Value*& current;
            explicit ProcessPathFunctor(const Json::Value*& node)
                : current(node)
            {}

            template <typename T>
            void operator()(T&& arg) const
            {
                if (!current) {
                    return;
                }

                if constexpr (std::is_integral_v<std::decay_t<T>>) {
                    if (current->isArray() && arg >= 0 && static_cast<size_t>(arg) < current->size()) {
                        current = &(*current)[arg];
                    } else {
                        current = nullptr;
                    }
                }
                else if constexpr (
                    std::is_same_v<std::decay_t<T>, std::string> ||
                    std::is_same_v<std::decay_t<T>, const char*> ||
                    std::is_same_v<std::decay_t<T>, char*>)
                {
                    if (current->isObject() && current->isMember(arg)) {
                        current = &(*current)[arg];
                    } else {
                        current = nullptr;
                    }
                }
                else if constexpr (std::is_convertible_v<T, std::string_view>) {
                    if (const std::string key(arg); current->isObject() && current->isMember(key)) {
                        current = &(*current)[key];
                    } else {
                        current = nullptr;
                    }
                }
                else {
                    current = nullptr;
                }
            }
        };

        template<typename... Args>
        Reader At(Args&&... args) const {
            if (!current_) {
                return Reader(nullptr, error_message_, policy_);
            }

            const Json::Value* current = current_;
            // 依次处理路径参数
            (ProcessPathFunctor(current)(std::forward<Args>(args)), ...);

            return Reader(current, error_message_, policy_);
        }

        template<typename... Args>
        Reader& Move(Args&&... args) {
            if (!current_) {
                return *this;
            }

            const Json::Value* new_current = current_;
            // 依次处理路径参数
            (ProcessPathFunctor(new_current)(std::forward<Args>(args)), ...);

            current_ = new_current;
            return *this;
        }

        // 读取一行数据，返回元组
        template<typename... Types, typename... Names>
        auto Line(Names&&... names) const -> std::tuple<Types...> {
            static_assert(sizeof...(Types) == sizeof...(Names),
                         "number of types must match number of names");

            if (!current_ || !current_->isObject()) {
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error("cannot read from null or non-object JSON");
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = "cannot read from null or non-object JSON";
                }
                return std::tuple<Types...>();
            }

            return std::make_tuple(extract_value<Types>(std::forward<Names>(names))...);
        }

        // 读取一行数据，返回包含 optional 的元组
        template<typename... Types, typename... Names>
        auto TryLine(Names&&... names) const -> std::tuple<std::optional<Types>...> {
            static_assert(sizeof...(Types) == sizeof...(Names),
                         "number of types must match number of names");

            if (!current_ || !current_->isObject()) {
                return std::tuple<std::optional<Types>...>{};
            }

            return std::make_tuple(try_extract_value<Types>(std::forward<Names>(names))...);
        }

        template<typename T>
        T Scalar() const {
            if (!current_ || (!current_->isBool() && !current_->isNumeric() & !current_->isString())) {
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error("can only read from bool, number, string JSON");
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_) {
                    *error_message_ = "can only read from bool, number, string JSON";
                }
                return T{};
            }

            return extract<T>(*current_, "scalar");
        }

        template<typename T>
        std::optional<T> TryScalar() const {
            if (!current_ || (!current_->isBool() && !current_->isNumeric() & !current_->isString())) {
                return std::nullopt;
            }

            try {
                return extract<T>(*current_, "scalar");
            } catch(...) {
                return std::nullopt;
            }
        }

        // 检查字段是否存在
        [[nodiscard]] bool Has(const std::string& name) const { return current_ && current_->isObject() && current_->isMember(name); }
        // 检查整个Reader是否有效
        [[nodiscard]] bool IsValid() const { return current_ != nullptr; }
        // 获取指定字段的Reader
        [[nodiscard]] Reader Field(const std::string& name) const;
        // 检查Reader是否有效（bool转换）
        explicit operator bool() const { return current_ != nullptr; }
        // 获取原始指针
        [[nodiscard]] const Json::Value* Raw() const;
        // 序列化为字符串
        [[nodiscard]] std::string ToString() const;
        // 获取读取策略
        [[nodiscard]] ReadPolicy GetPolicy() const { return policy_; }

    private:
        const Json::Value* current_ = nullptr;
        std::string* error_message_ = nullptr;
        ReadPolicy policy_ = ReadPolicy::RETURN_ERROR;

        // 获取JSON值的类型名称
        static const char* get_json_type_name(Json::ValueType type);

        // 获取期望类型的名称
        template<typename T>
        static const char* get_expected_type_name() {
            if constexpr (std::is_same_v<T, short>) return "short";
            else if constexpr (std::is_same_v<T, unsigned short>) return "ushort";
            else if constexpr (std::is_same_v<T, int>) return "int";
            else if constexpr (std::is_same_v<T, unsigned>) return "uint";
            else if constexpr (std::is_same_v<T, long>) return "long";
            else if constexpr (std::is_same_v<T, unsigned long>) return "ulong";
            else if constexpr (std::is_same_v<T, long long>) return "longlong";
            else if constexpr (std::is_same_v<T, unsigned long long>) return "ulonglong";
            else if constexpr (std::is_same_v<T, double>) return "double";
            else if constexpr (std::is_same_v<T, float>) return "float";
            else if constexpr (std::is_same_v<T, bool>) return "bool";
            else if constexpr (std::is_same_v<T, const char*>) return "const char*";
            else if constexpr (std::is_same_v<T, std::string_view>) return "string_view";
            else if constexpr (std::is_same_v<T, std::string>) return "string";
            else if constexpr (std::is_same_v<T, const std::string&>) return "const string&";
            else return "unknown";
        }

        // 提取单个值（根据策略处理）
        template<typename T>
        T extract_value(const std::string& name) const {
            if (!current_ || !current_->isObject()) {
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error("cannot read from null or non-object JSON");
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = "cannot read from null or non-object JSON";
                }
                return T{};
            }

            if (!current_->isMember(name)) {
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error("field '" + name + "' not found");
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = "field '" + name + "' not found";
                }
                return T{};
            }

            const Json::Value& value = (*current_)[name];
            return extract<T>(value, name);
        }

        // 尝试提取值，返回optional
        template<typename T>
        std::optional<T> try_extract_value(const std::string& name) const {
            if (!current_ || !current_->isObject() || !current_->isMember(name)) {
                return std::nullopt;
            }

            const Json::Value& value = (*current_)[name];
            try {
                return extract<T>(value, name);
            } catch(...) {
                return std::nullopt;
            }
        }

        // 类型提取实现
        template<typename T>
        T extract(const Json::Value& value, const std::string& field_name) const {
            if constexpr (std::is_same_v<T, const char*>) {
                if (!value.isString()) {
                    if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                        throw std::runtime_error(
                            "type mismatch for field '" + field_name + "', expected const char*, got " +
                            get_json_type_name(value.type()));
                    }
                    if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                    {
                        *error_message_ = "type mismatch for field '" + field_name + "', expected const char*, got " +
                            get_json_type_name(value.type());
                    }
                    return "";
                }
                return value.asCString();
            }
            else if constexpr (std::is_same_v<T, std::string_view>) {
                if (!value.isString()) {
                    if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                        throw std::runtime_error(
                            "type mismatch for field '" + field_name + "', expected string_view, got " +
                            get_json_type_name(value.type()));
                    }
                    if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                    {
                        *error_message_ = "type mismatch for field '" + field_name + "', expected string_view, got " +
                            get_json_type_name(value.type());
                    }
                    return std::string_view{};
                }
                return std::string_view(value.asCString());
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                if (!value.isString()) {
                    if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                        throw std::runtime_error(
                            "type mismatch for field '" + field_name + "', expected string, got " +
                            get_json_type_name(value.type()));
                    }
                    if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                    {
                        *error_message_ = "type mismatch for field '" + field_name + "', expected string, got " +
                            get_json_type_name(value.type());
                    }
                    return T{};
                }
                return value.asString();
            }
            else if constexpr (
                std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> ||
                std::is_same_v<T, short> || std::is_same_v<T, unsigned short>
            ) {
                if (value.isInt()) return static_cast<T>(value.asInt());
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0;
            }
            else if constexpr (std::is_same_v<T, int>) {
                if (value.isInt()) return value.asInt();
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0;
            }
            else if constexpr (std::is_same_v<T, unsigned int>) {
                if (value.isUInt()) return value.asUInt();
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0;
            }
            else if constexpr (std::is_same_v<T, long> || std::is_same_v<T, long long>) {
                if (value.isInt64()) return static_cast<T>(value.asInt64());
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0;
            }
            else if constexpr (std::is_same_v<T, unsigned long> || std::is_same_v<T, unsigned long long>) {
                if (value.isUInt64()) return static_cast<T>(value.asUInt64());
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0;
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                if (value.isInt64()) return value.asInt64();
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0;
            }
            else if constexpr (std::is_same_v<T, uint64_t>) {
                if (value.isUInt64()) return value.asUInt64();
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0;
            }
            else if constexpr (std::is_same_v<T, double>) {
                if (value.isDouble()) return value.asDouble();
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0.0;
            }
            else if constexpr (std::is_same_v<T, float>) {
                if (value.isDouble()) return static_cast<float>(value.asDouble());
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return 0.0;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                if (value.isBool()) return value.asBool();
                if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                    throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
                }
                if (policy_ == ReadPolicy::RETURN_ERROR && error_message_)
                {
                    *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                        field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
                }
                return false;
            }
            if (policy_ == ReadPolicy::THROW_EXCEPTION) {
                throw std::runtime_error(std::format("type mismatch for field '{}', expected {}, got {}",
                    field_name, get_expected_type_name<T>(), get_json_type_name(value.type())));
            }
            if (policy_ == ReadPolicy::RETURN_ERROR && error_message_) {
                *error_message_ = std::format("type mismatch for field '{}', expected {}, got {}",
                    field_name, get_expected_type_name<T>(), get_json_type_name(value.type()));
            }
            return T{};
        }
    };

    explicit JsonCppReader(const Json::Value* value);

    [[nodiscard]] Reader CreateReader() { return Reader(root_, &error_message_); }

    void ClearError() { error_message_.clear(); }
    [[nodiscard]] bool Error() const { return !error_message_.empty(); }
    [[nodiscard]] const std::string& ErrorMessage() const { return error_message_; }

    // 快捷方式：直接读取
    template<typename... Types, typename... Names>
    auto Line(Names&&... names) -> std::tuple<Types...> {
        return CreateReader().Line<Types...>(std::forward<Names>(names)...);
    }

private:
    const Json::Value* root_ = nullptr;
    std::string error_message_;
};

}