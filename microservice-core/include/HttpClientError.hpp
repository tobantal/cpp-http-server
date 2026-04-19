#pragma once

#include <string>
#include <cstdint>

enum class HttpClientError : uint8_t
{
    None,
    DnsFailed,
    ConnectTimeout,
    ConnectionRefused,
    WriteTimeout,
    ReadTimeout,
    UnknownError
};

struct HttpClientResult
{
    HttpClientError error = HttpClientError::None;
    std::string errorMessage;

    bool ok() const { return error == HttpClientError::None; }
};

inline std::string httpClientErrorToString(HttpClientError e)
{
    switch (e)
    {
    case HttpClientError::None: return "none";
    case HttpClientError::DnsFailed: return "dns_failed";
    case HttpClientError::ConnectTimeout: return "connect_timeout";
    case HttpClientError::ConnectionRefused: return "connection_refused";
    case HttpClientError::WriteTimeout: return "write_timeout";
    case HttpClientError::ReadTimeout: return "read_timeout";
    case HttpClientError::UnknownError: return "unknown_error";
    default: return "unknown";
    }
}