#pragma once
#include "IResponse.hpp"
#include <boost/beast/http.hpp>
#include <string>

/**
 * @file BeastResponseAdapter.hpp
 * @brief Адаптер для Boost.Beast HTTP ответа
 * @author Anton Tobolkin
 */
struct BeastResponseAdapter : IResponse {
    BeastResponseAdapter(boost::beast::http::response<boost::beast::http::string_body>& res)
        : res_(res) {}

    void setStatus(int code) override {
        res_.result(boost::beast::http::status(code));
    }

    void setBody(const std::string& body) override {
        res_.body() = body;
    }

    void setHeader(const std::string& name, const std::string& value) override {
        res_.set(name, value);
    }

private:
    boost::beast::http::response<boost::beast::http::string_body>& res_;
};
