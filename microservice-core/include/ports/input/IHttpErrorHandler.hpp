#pragma once

#include "domain/IResponse.hpp"
#include "domain/error/HttpError.hpp"

/**
 * @file IHttpErrorHandler.hpp
 * @brief Interface for HTTP error handling
 * @author Anton Tobolkin
 */

/**
 * @struct IHttpErrorHandler
 * @brief Interface for handling HTTP errors
 *
 * Implementations format error responses (e.g., JSON, XML)
 * and set appropriate status codes. traceId is read from IResponse::getHeader("X-Trace-ID").
 */
struct IHttpErrorHandler
{
    virtual ~IHttpErrorHandler() = default;

    /**
     * @brief Handle an HTTP error by writing error response
     * @param res HTTP response to write error into
     * @param e HTTP error with status code and message
     */
    virtual void handleError(IResponse &res, const HttpError &e) = 0;
};