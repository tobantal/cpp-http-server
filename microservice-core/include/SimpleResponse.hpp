#pragma once

#include "IResponse.hpp"
#include <string>
#include <map>

/**
 * @file SimpleResponse.hpp
 * @brief Простая реализация IResponse
 * @author Anton Tobolkin
 */
class SimpleResponse : public IResponse
{
public:
    SimpleResponse(int status = 200, const std::string& body = "")
        : status_(status), body_(body)
    {
    }

    void setStatus(int code) override
    {
        status_ = code;
    }

    void setBody(const std::string& body) override
    {
        body_ = body;
    }

    void setHeader(const std::string& name, const std::string& value) override
    {
        headers_[name] = value;
    }

    int getStatus() const { return status_; }
    std::string getBody() const { return body_; }
    std::map<std::string, std::string> getHeaders() const { return headers_; }

private:
    int status_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};
