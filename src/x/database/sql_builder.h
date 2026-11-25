#pragma once

#include <format>
#include <string_view>
#include <string>
#include <vector>

namespace x::database
{

template <class RowType>
class SqlBuilder
{
    int64_t id_;
    std::vector<std::string_view> fields_;
    std::vector<std::string_view> placeholders_;
    std::vector<std::string> values_;
    std::string sql_;

public:
    SqlBuilder();

    [[nodiscard]] std::string& sql() { return sql_; }
    [[nodiscard]] const std::string& sql() const noexcept { return sql_; }
    [[nodiscard]] const std::vector<std::string>& values() const noexcept { return values_; }

    SqlBuilder& Clear();
    SqlBuilder& Select(const RowType& row);
    SqlBuilder& Insert(const RowType& row);
    SqlBuilder& Update(const RowType& row);
    SqlBuilder& Returning(const std::initializer_list<std::string_view>& column_list);

private:
    void FillSqlFields(const RowType& row);
};

template <class RowType>
SqlBuilder<RowType>::SqlBuilder()
    : id_(0)
{
    fields_.reserve(RowType::Column::total_count);
    placeholders_.reserve(RowType::Column::total_count);
    values_.reserve(RowType::Column::total_count);
    size_t count = 0;
    for (int i = 0; i < RowType::Column::total_count; ++i)
    {
        count += RowType::Column::columns[i].size() + 1;
    }
    count += 64;
    sql_.reserve(count);
}

template <class RowType>
SqlBuilder<RowType>& SqlBuilder<RowType>::Clear()
{
    id_ = 0;
    fields_.clear();
    placeholders_.clear();
    values_.clear();
    sql_.clear();
    return *this;
}

template <class RowType>
SqlBuilder<RowType>& SqlBuilder<RowType>::Select(const RowType& row)
{
    sql_.append("SELECT ");
    for (int i = 0; i < RowType::Column::total_count; ++i)
    {
        if (row.has(i))
        {
            sql_.append(RowType::Column::columns[i]).append(",");
        }
    }
    sql_.back() = ' ';
    sql_.append("FROM ").append(RowType::table_name);
    return *this;
}

template <class RowType>
SqlBuilder<RowType>& SqlBuilder<RowType>::Insert(const RowType& row)
{
    FillSqlFields(row);
    sql_.append("INSERT INTO ").append(RowType::table_name).append(" (");
    for (size_t i = 0; i < values_.size(); i++)
    {
        sql_.append(fields_[i]).append(",");
    }
    sql_.back() = ')';
    sql_.append(" VALUES (");
    for (size_t i = 0; i < values_.size(); i++)
    {
        sql_.append(placeholders_[i]).append(",");
    }
    sql_.back() = ')';
    return *this;
}

template <class RowType>
SqlBuilder<RowType>& SqlBuilder<RowType>::Update(const RowType& row)
{
    FillSqlFields(row);
    sql_.append("UPDATE ").append(RowType::table_name).append(" SET ");
    for (size_t i = 0; i < values_.size(); ++i)
    {
        sql_.append(fields_[i]).append("=").append(placeholders_[i]).append(",");
    }
    sql_.back() = ' ';
    sql_.append(std::format("WHERE id=${}", values_.size() + 1));
    values_.push_back(std::format("{}", id_));
    return *this;
}

template <class RowType>
SqlBuilder<RowType>& SqlBuilder<RowType>::Returning(const std::initializer_list<std::string_view>& column_list)
{
    sql_.append(" RETURNING ");
    if (empty(column_list))
    {
        for (int i = 0; i < RowType::Column::total_count; i++)
        {
            sql_.append(RowType::Column::columns[i]).append(",");
        }
    }
    else
    {
        for (const auto& column : column_list)
        {
            sql_.append(column).append(",");
        }
    }
    sql_.pop_back();
    return *this;
}

template <class RowType>
void SqlBuilder<RowType>::FillSqlFields(const RowType& row)
{
    for (int i = 0; i < RowType::Column::total_count; ++i)
    {
        if (row.has(i))
        {
            if (RowType::Column::columns[i] == RowType::Column::id)
            {
                id_ = row.id();
            }
            else
            {
                size_t index = values_.size();
                fields_.push_back(RowType::Column::columns[i]);
                placeholders_.push_back(RowType::Column::placeholders[index]);
                values_.emplace_back(row.String(i));
            }
        }
    }
}

}