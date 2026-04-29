/**
 * @file SimpleResponse.cpp
 * @brief SimpleResponse implementation
 * @author Anton Tobolkin
 */

#include "adapters/secondary/SimpleResponse.hpp"

/**
 * @brief Construct SimpleResponse with status and body
 * @param status HTTP status code (default 200)
 * @param body Response body (default empty)
 */
SimpleResponse::SimpleResponse(int status, const std::string& body)
    : status_(status), body_(body)
{
}

/**
 * @brief Set HTTP status code
 * @param code Status code (e.g., 200, 404, 500)
 */
void SimpleResponse::setStatus(int code)
{
    status_ = code;
}

/**
 * @brief Set status from enum
 * @param status HttpStatus enum value
 */
void SimpleResponse::setStatus(HttpStatus status)
{
    status_ = toInt(status);
}

void SimpleResponse::setBody(const std::string& body)
{
    body_ = body;
}

void SimpleResponse::setHeader(const std::string& name, const std::string& value)
{
    headers_[name] = value;
}

void SimpleResponse::setCookie(const std::string& name,
                                const std::string& value,
                                const std::string& path,
                                bool httpOnly,
                                bool secure,
                                int maxAge)
{
    std::string cookie = name + "=" + value;
    if (!path.empty())
    {
        cookie += "; Path=" + path;
    }
    if (maxAge >= 0)
    {
        cookie += "; Max-Age=" + std::to_string(maxAge);
    }
    if (httpOnly)
    {
        cookie += "; HttpOnly";
    }
    if (secure)
    {
        cookie += "; Secure";
    }
    headers_["Set-Cookie"] = cookie;
}

int SimpleResponse::getStatus() const
{
    return status_;
}

std::string SimpleResponse::getBody() const
{
    return body_;
}

std::map<std::string, std::string> SimpleResponse::getHeaders() const
{
    return headers_;
}

std::optional<std::string> SimpleResponse::getHeader(const std::string& name) const
{
    std::string nameLower = StringUtils::toLower(name);
    for (const auto& [key, val] : headers_)
    {
        if (StringUtils::toLower(key) == nameLower)
        {
            return val;
        }
    }
    return std::nullopt;
}

void SimpleResponse::setResult(int code,
                               const std::string& contentType,
                               const std::string& body)
{
    setStatus(code);
    setHeader("Content-Type", contentType);
    setBody(body);
}

void SimpleResponse::setResult(HttpStatus status,
                               const std::string& contentType,
                               const std::string& body)
{
    setResult(toInt(status), contentType, body);
}

void SimpleResponse::setTraceId(const std::string& id)
{
    setHeader("X-Trace-ID", id);
}