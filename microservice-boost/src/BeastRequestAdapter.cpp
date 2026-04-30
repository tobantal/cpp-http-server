#include "adapters/primary/BeastRequestAdapter.hpp"
#include "util/StringUtils.hpp"
#include "util/Uuid7Generator.hpp"
#include "util/PathParamExtractor.hpp"

BeastRequestAdapter::BeastRequestAdapter(
    const boost::beast::http::request<boost::beast::http::string_body>& req,
    const std::string& clientIp,
    int port,
    std::shared_ptr<IIdGenerator> idGenerator)
    : req_(req), ip_(clientIp), port_(port), body_(req.body()),
      idGenerator_(idGenerator ? std::move(idGenerator) : std::make_shared<Uuid7Generator>()) {}

std::string BeastRequestAdapter::getPath() const
{
    auto target = std::string(req_.target());
    auto pos = target.find('?');
    std::string path = pos == std::string::npos ? target : target.substr(0, pos);
    return StringUtils::urlDecode(path);
}

std::vector<std::string> BeastRequestAdapter::getPathSegments() const
{
    return StringUtils::splitPath(getPath());
}

std::string BeastRequestAdapter::getPathPattern() const
{
    return pathPattern_;
}

void BeastRequestAdapter::setPathPattern(const std::string& pattern)
{
    pathPattern_ = pattern;
}

std::optional<std::string> BeastRequestAdapter::getPathParam(size_t index) const
{
    return PathParamExtractor::getByIndex(getPath(), pathPattern_, index);
}

std::map<std::string, std::string> BeastRequestAdapter::getQueryParams() const
{
    if (cachedQueryParams_.has_value())
    {
        return cachedQueryParams_.value();
    }

    std::map<std::string, std::string> params = queryParams_;

    auto target = std::string(req_.target());
    auto pos = target.find('?');
    if (pos == std::string::npos)
    {
        cachedQueryParams_ = params;
        return params;
    }

    std::string query = target.substr(pos + 1);
    size_t start = 0;
    while (start < query.size())
    {
        auto eq = query.find('=', start);
        auto amp = query.find('&', start);
        if (eq == std::string::npos)
            break;

        std::string key = StringUtils::urlDecode(query.substr(start, eq - start));
        std::string value = amp == std::string::npos
                                ? StringUtils::urlDecode(query.substr(eq + 1))
                                : StringUtils::urlDecode(query.substr(eq + 1, amp - eq - 1));

        if (params.find(key) == params.end())
        {
            params[key] = value;
        }
        if (amp == std::string::npos)
            break;
        start = amp + 1;
    }
    cachedQueryParams_ = params;
    return params;
}

std::optional<std::string> BeastRequestAdapter::getQueryParam(const std::string& name) const
{
    auto params = getQueryParams();
    auto it = params.find(name);
    if (it != params.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void BeastRequestAdapter::setQueryParam(const std::string& name, const std::string& value)
{
    queryParams_[name] = value;
}

std::map<std::string, std::string> BeastRequestAdapter::getParams() const
{
    return getQueryParams();
}

std::map<std::string, std::string> BeastRequestAdapter::getHeaders() const
{
    std::map<std::string, std::string> headers = headers_;

    for (auto const& field : req_)
    {
        std::string name = std::string(field.name_string());
        std::string value = std::string(field.value());
        if (headers.find(name) == headers.end())
        {
            headers[name] = value;
        }
    }

    return headers;
}

std::optional<std::string> BeastRequestAdapter::getHeader(const std::string& name) const
{
    std::string nameLower = StringUtils::toLower(name);
    for (const auto& [key, value] : headers_)
    {
        if (StringUtils::toLower(key) == nameLower)
        {
            return value;
        }
    }

    for (auto const& field : req_)
    {
        std::string fieldName = std::string(field.name_string());
        if (StringUtils::toLower(fieldName) == nameLower)
        {
            return std::string(field.value());
        }
    }

    return std::nullopt;
}

void BeastRequestAdapter::setHeader(const std::string& name, const std::string& value)
{
    headers_[name] = value;
}

void BeastRequestAdapter::setHeaders(const std::map<std::string, std::string>& headers)
{
    for (const auto& [name, value] : headers)
    {
        headers_[name] = value;
    }
}

std::string BeastRequestAdapter::getBody() const
{
    return body_;
}

void BeastRequestAdapter::setBody(const std::string& body)
{
    body_ = body;
}

std::string BeastRequestAdapter::getMethod() const
{
    return std::string(req_.method_string());
}

std::string BeastRequestAdapter::getIp() const
{
    return ip_;
}

int BeastRequestAdapter::getPort() const
{
    return port_;
}

std::optional<std::string> BeastRequestAdapter::getBearerToken() const
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

bool BeastRequestAdapter::isJson() const
{
    auto contentType = getContentType();
    return contentType.find("json") != std::string::npos;
}

std::string BeastRequestAdapter::getContentType() const
{
    auto ct = getHeader("Content-Type");
    return ct.value_or("");
}

void BeastRequestAdapter::setAttribute(const std::string& name, const std::string& value)
{
    attributes_[name] = value;
}

std::optional<std::string> BeastRequestAdapter::getAttribute(const std::string& name) const
{
    auto it = attributes_.find(name);
    if (it != attributes_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void BeastRequestAdapter::setObject(const std::string& name, std::shared_ptr<IEnvironment> obj)
{
    objects_[name] = obj;
}

std::optional<std::shared_ptr<IEnvironment>> BeastRequestAdapter::getObject(const std::string& name) const
{
    auto it = objects_.find(name);
    if (it != objects_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

std::string BeastRequestAdapter::getTraceId()
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

void BeastRequestAdapter::setTraceId(const std::string& id)
{
    setHeader("X-Trace-ID", id);
}