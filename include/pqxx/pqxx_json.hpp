/* Implements to_json for varius pqxx return types.
* By implementing to_json for a return type, we can initialize a json using a value or variable,
* of type having said return type, by passing said value or variable to the json constructor.
* No extra syntax is needed!
* For example: suppose we have a result res that is already defined and a json my_json.
* Then, since to_json is defined for result objects below,
* we can get the result set from res in my_json by doing my_json = res;
*/

#ifndef PQXX_H_JSON
#define PQXX_H_JSON
#include "pqxx/internal/result_iter.hxx"
#include "pqxx/result.hxx"
#include "pqxx/transaction_base.hxx"
#include <tuple>

// include json.hpp to use the nlohmann:json data type as json type
#include <nlohmann/json.hpp>

using json = nlohmann::json;

template <typename... types>
void higher_order_tuple_func(std::tuple<types...>& tup, int& row_index, json& tr_base_json) {
    int column_index = 0;
    
    // capture column_index by reference so that it is changed (in this case, incremented) each time column_callback is called
    auto column_callback = [&](const auto& elem) {
        tr_base_json["data"][row_index][column_index++] = elem;
    };
    
    /* recursively call column_callback on each column using the comma operator inside parentheses,
    * where the columns comprise col_args, which the tuple tup is mapped to in std::apply below
    */
    auto tr_base_json_callback = [column_callback](const types&... col_args) {
        (column_callback(col_args), ...);
    };
    std::apply(tr_base_json_callback, tup);
}

namespace pqxx { 
    void to_json(json& result_json, const result& res);
    void to_json(json& result_json, const result&& res);

    void to_json(json& result_json, const row& res_row);
    void to_json(json& result_json, const row&& res_row);

    template <typename ...types>
    /* to_json for the return value of a pqxx::work::query(); the return value is an iterator to
    * the underlying result set, which is composed of a number of std::tuple<types...>s
    */
    void to_json(json& tr_base_json, pqxx::internal::result_iteration<types...>& iter_result) {
        int cur_row_index = -1;
        for (auto it = iter_result.begin(); it != iter_result.end(); ++it) {
            higher_order_tuple_func(*it, ++cur_row_index, tr_base_json);
        }
        tr_base_json["status-code"] = 200;
    }

    template <typename ...types>
    void to_json(json& tr_base_json, pqxx::internal::result_iteration<types...>&& iter_result) {
        to_json(tr_base_json, iter_result);
    }
}
#endif
