#pragma once

#include "domain/IResponse.hpp"
#include "util/StringUtils.hpp"
#include <boost/beast/http.hpp>
#include <string>
#include <map>
#include <optional>

/**
 * @file BeastResponseAdapter.hpp
 * @brief Boost.Beast response adapter implementing IResponse
 * @author Anton Tobolkin
 */

/**
 * @struct BeastResponseAdapter
 * @brief IResponse implementation that wraps a Boost.Beast HTTP response
 */
struct BeastResponseAdapter : IResponse
{
    BeastResponseAdapter(boost::beast::http::response<boost::beast::http::string_body>& res)
        : res_(res) {}

    void setStatus(int code) override;
    void setStatus(HttpStatus status) override;
    void setBody(const std::string& body) override;
    void setHeader(const std::string& name, const std::string& value) override;
    void setCookie(const std::string& name,
                   const std::string& value,
                   const std::string& path = "/",
                   bool httpOnly = true,
                   bool secure = false,
                   int maxAge = -1) override;
    int getStatus() const override;
    std::string getBody() const override;
    std::map<std::string, std::string> getHeaders() const override;
    std::optional<std::string> getHeader(const std::string& name) const override;
    void setResult(int code,
                   const std::string& contentType,
                   const std::string& body) override;
    void setResult(HttpStatus status,
                   const std::string& contentType,
                   const std::string& body) override;
    void setTraceId(const std::string& id) override;

private:
    boost::beast::http::response<boost::beast::http::string_body>& res_;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};
