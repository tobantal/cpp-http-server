#pragma once

#include "domain/INameable.hpp"
#include "domain/IRequest.hpp"
#include "domain/IResponse.hpp"
#include <string>

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
 * Inherits INameable for logging (handler name in ChainHandler logs).
 */
class IHttpHandler : public INameable
{
public:
    /**
     * @brief Handle an HTTP request
     * @param req HTTP request
     * @param res HTTP response
     */
    virtual void handle(IRequest &req, IResponse &res) = 0;

    /**
     * @brief Get handler name for logging and metrics
     * @return Handler name (default: "UnnamedHandler")
     */
    std::string name() const override;
};