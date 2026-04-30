#include "domain/HttpClientError.hpp"

bool HttpClientResult::ok() const {
    return error == HttpClientError::None;
}

std::string httpClientErrorToString(HttpClientError e) {
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