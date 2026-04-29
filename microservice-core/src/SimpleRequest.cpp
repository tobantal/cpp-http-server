#include "adapters/secondary/SimpleRequest.hpp"

std::string SimpleRequest::getPath() const
{
    return path_;
}

std::vector<std::string> SimpleRequest::getPathSegments() const
{
    std::vector<std::string> segments;
    std::string segment;

    for (char ch : path_)
    {
        if (ch == '/')
        {
            if (!segment.empty())
            {
                segments.push_back(segment);
                segment.clear();
            }
        }
        else if (ch == '?')
        {
            break;
        }
        else
        {
            segment += ch;
        }
    }

    if (!segment.empty())
    {
        segments.push_back(segment);
    }

    return segments;
}

std::string SimpleRequest::getPathPattern() const
{
    return pathPattern_;
}

void SimpleRequest::setPathPattern(const std::string& pattern)
{
    pathPattern_ = pattern;
}

std::optional<std::string> SimpleRequest::getPathParam(size_t index) const
{
    return PathParamExtractor::getByIndex(path_, pathPattern_, index);
}

std::map<std::string, std::string> SimpleRequest::getQueryParams() const
{
    return queryParams_;
}

std::optional<std::string> SimpleRequest::getQueryParam(const std::string& name) const
{
    auto it = queryParams_.find(name);
    if (it != queryParams_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void SimpleRequest::setQueryParam(const std::string& name, const std::string& value)
{
    queryParams_[name] = value;
}

std::map<std::string, std::string> SimpleRequest::getParams() const
{
    return getQueryParams();
}

std::map<std::string, std::string> SimpleRequest::getHeaders() const
{
    return headers_;
}

std::optional<std::string> SimpleRequest::getHeader(const std::string& name) const
{
    std::string nameLower = StringUtils::toLower(name);
    for (const auto& [key, value] : headers_)
    {
        if (StringUtils::toLower(key) == nameLower)
        {
            return value;
        }
    }
    return std::nullopt;
}

void SimpleRequest::setHeader(const std::string& name, const std::string& value)
{
    headers_[name] = value;
}

void SimpleRequest::setHeaders(const std::map<std::string, std::string>& headers)
{
    for (const auto& [name, value] : headers)
    {
        headers_[name] = value;
    }
}

std::string SimpleRequest::getBody() const
{
    return body_;
}

void SimpleRequest::setBody(const std::string& body)
{
    body_ = body;
}

std::string SimpleRequest::getMethod() const
{
    return method_;
}

std::string SimpleRequest::getIp() const
{
    return ip_;
}

int SimpleRequest::getPort() const
{
    return port_;
}

std::optional<std::string> SimpleRequest::getBearerToken() const
{
    auto auth = getHeader("Authorization");
    if (!auth)
    {
        return std::nullopt;
    }

    const std::string bearerPrefix = "Bearer ";
    if (auth->length() > bearerPrefix.length() &&
        auth->substr(0, bearerPrefix.length()) == bearerPrefix)
    {
        return auth->substr(bearerPrefix.length());
    }

    return std::nullopt;
}

bool SimpleRequest::isJson() const
{
    auto contentType = getContentType();
    return contentType.find("json") != std::string::npos;
}

std::string SimpleRequest::getContentType() const
{
    auto ct = getHeader("Content-Type");
    return ct.value_or("");
}

void SimpleRequest::setAttribute(const std::string& name, const std::string& value)
{
    attributes_[name] = value;
}

std::optional<std::string> SimpleRequest::getAttribute(const std::string& name) const
{
    auto it = attributes_.find(name);
    if (it != attributes_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void SimpleRequest::setObject(const std::string& name, std::shared_ptr<IEnvironment> obj)
{
    objects_[name] = std::move(obj);
}

std::optional<std::shared_ptr<IEnvironment>> SimpleRequest::getObject(const std::string& name) const
{
    auto it = objects_.find(name);
    if (it != objects_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

std::string SimpleRequest::getTraceId()
{
    auto header = getHeader("X-Trace-ID");
    if (header)
    {
        return *header;
    }
    std::string id = idGenerator_->generate();
    setHeader("X-Trace-ID", id);
    return id;
}

void SimpleRequest::setTraceId(const std::string& id)
{
    setHeader("X-Trace-ID", id);
}

void SimpleRequest::setMethod(const std::string& method)
{
    method_ = method;
}

void SimpleRequest::setPath(const std::string& path)
{
    path_ = path;
}

void SimpleRequest::setIp(const std::string& ip)
{
    ip_ = ip;
}

void SimpleRequest::setPort(int port)
{
    port_ = port;
}