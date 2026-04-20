#pragma once

#include "domain/IRequest.hpp"
#include "domain/IResponse.hpp"
#include "domain/HttpClientError.hpp"

/**
 * @file IHttpClient.hpp
 * @brief Interface for HTTP client
 * @author Anton Tobolkin
 */

/**
 * @class IHttpClient
 * @brief Interface for sending HTTP requests
 */
class IHttpClient
{
public:
    virtual ~IHttpClient() = default;

    /**
     * @brief Send an HTTP request
     * @param request HTTP request to send
     * @param response HTTP response to populate
     * @return Result indicating success or error type
     */
    virtual HttpClientResult send(const IRequest& request, IResponse& response) = 0;
};
