#pragma once

#include "domain/IRequest.hpp"
#include "domain/IResponse.hpp"

/**
 * @file IHttpHandler.hpp
 * @brief Interface for HTTP request handlers
 * @author Anton Tobolkin
 */

/**
 * @class IHttpHandler
 * @brief Interface for processing HTTP requests
 *
 * Each handler is responsible for processing a specific endpoint.
 * Registered in IoC by method and path, e.g., "GET:/api/users"
 */
class IHttpHandler
{
public:
    virtual ~IHttpHandler() = default;

    /**
     * @brief Handle an HTTP request
     * @param req HTTP request
     * @param res HTTP response
     */
    virtual void handle(IRequest &req, IResponse &res) = 0;
};
