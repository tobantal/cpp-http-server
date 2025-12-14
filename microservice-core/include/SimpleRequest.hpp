#pragma once

#include "IRequest.hpp"
#include <string>
#include <map>

/**
 * @file SimpleRequest.hpp
 * @brief Простая реализация IRequest для исходящих запросов
 * @author Anton Tobolkin
 */
struct SimpleRequest : IRequest
{
    SimpleRequest(const std::string& method,
                  const std::string& path,
                  const std::string& body,
                  const std::string& ip,
                  int port,
                  const std::map<std::string, std::string>& headers = {})
        : method_(method), path_(path), body_(body), ip_(ip), port_(port), headers_(headers)
    {
    }

    std::string getPath() const override { return path_; }
    std::string getMethod() const override { return method_; }
    std::string getBody() const override { return body_; }
    std::string getIp() const override { return ip_; }
    int getPort() const override { return port_; }
    
    std::map<std::string, std::string> getParams() const override
    {
        return {};
    }
    
    std::map<std::string, std::string> getHeaders() const override
    {
        return headers_;
    }

private:
    std::string method_;
    std::string path_;
    std::string body_;
    std::string ip_;
    int port_;
    std::map<std::string, std::string> headers_;
};
