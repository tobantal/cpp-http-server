#include "adapters/primary/BeastResponseAdapter.hpp"

void BeastResponseAdapter::setStatus(int code)
{
    res_.result(boost::beast::http::status(code));
}

void BeastResponseAdapter::setStatus(HttpStatus status)
{
    res_.result(boost::beast::http::status(toInt(status)));
}

void BeastResponseAdapter::setBody(const std::string& body)
{
    res_.body() = body;
}

void BeastResponseAdapter::setHeader(const std::string& name, const std::string& value)
{
    res_.set(name, value);
}

void BeastResponseAdapter::setCookie(const std::string& name,
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
    res_.set(boost::beast::http::field::set_cookie, cookie);
}

int BeastResponseAdapter::getStatus() const
{
    return res_.result_int();
}

std::string BeastResponseAdapter::getBody() const
{
    return res_.body();
}

std::map<std::string, std::string> BeastResponseAdapter::getHeaders() const
{
    std::map<std::string, std::string> headers;

    for (auto const& field : res_)
    {
        std::string name = std::string(field.name_string());
        std::string value = std::string(field.value());
        headers[name] = value;
    }

    return headers;
}

std::optional<std::string> BeastResponseAdapter::getHeader(const std::string& name) const
{
    std::string nameLower = StringUtils::toLower(name);

    for (auto const& field : res_)
    {
        std::string fieldName = std::string(field.name_string());
        if (StringUtils::toLower(fieldName) == nameLower)
        {
            return std::string(field.value());
        }
    }

    return std::nullopt;
}

void BeastResponseAdapter::setResult(int code,
                                       const std::string& contentType,
                                       const std::string& body)
{
    setStatus(code);
    setHeader("Content-Type", contentType);
    setBody(body);
}

void BeastResponseAdapter::setResult(HttpStatus status,
                                       const std::string& contentType,
                                       const std::string& body)
{
    setResult(toInt(status), contentType, body);
}

void BeastResponseAdapter::setTraceId(const std::string& id)
{
    setHeader("X-Trace-ID", id);
}
