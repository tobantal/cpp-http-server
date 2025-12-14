// BeastRequestAdapter.hpp
#pragma once
#include "IRequest.hpp"
#include <boost/beast/http.hpp>
#include <map>
#include <string>

/**
 * @file BeastRequestAdapter.hpp
 * @brief Адаптер для Boost.Beast HTTP запроса
 * @author Anton Tobolkin
 */
struct BeastRequestAdapter : IRequest
{
    BeastRequestAdapter(
        const boost::beast::http::request<boost::beast::http::string_body>& req,
        const std::string& clientIp)
        : req_(req), ip_(clientIp) {}

    std::string getPath() const override
    {
        auto target = std::string(req_.target());
        auto pos = target.find('?');
        return pos == std::string::npos ? target : target.substr(0, pos);
    }

    std::string getMethod() const override
    {
        return std::string(req_.method_string());
    }

    std::string getBody() const override
    {
        return req_.body();
    }

    std::map<std::string, std::string> getParams() const override
    {
        std::map<std::string, std::string> params;
        auto target = std::string(req_.target());
        auto pos = target.find('?');
        if (pos == std::string::npos)
            return params;

        std::string query = target.substr(pos + 1);
        size_t start = 0;
        while (start < query.size())
        {
            auto eq = query.find('=', start);
            auto amp = query.find('&', start);
            if (eq == std::string::npos)
                break;

            std::string key = query.substr(start, eq - start);
            std::string value = amp == std::string::npos
                                    ? query.substr(eq + 1)
                                    : query.substr(eq + 1, amp - eq - 1);

            params[key] = value;
            if (amp == std::string::npos)
                break;
            start = amp + 1;
        }
        return params;
    }
    
    std::map<std::string, std::string> getHeaders() const override
    {
        std::map<std::string, std::string> headers;
        
        for (auto const& field : req_)
        {
            std::string name = std::string(field.name_string());
            std::string value = std::string(field.value());
            headers[name] = value;
        }
        
        return headers;
    }
    
    std::string getIp() const override
    {
        return ip_;
    }

    int getPort() const override
    {
        return 80;
    }

private:
    const boost::beast::http::request<boost::beast::http::string_body>& req_;
    std::string ip_;
};
