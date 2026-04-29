#pragma once

#include "ports/input/IHttpErrorHandler.hpp"
#include "util/StringUtils.hpp"

/**
 * @file HttpErrorSender.hpp
 * @brief Default HTTP error handler that sends JSON error responses
 * @author Anton Tobolkin
 */

/**
 * @class HttpErrorSender
 * @brief Sends JSON-formatted error responses
 *
 * Formats: {"error": "message"}
 * Sets traceId from IResponse header "X-Trace-ID" if present.
 */
class HttpErrorSender : public IHttpErrorHandler
{
public:
    /**
     * @brief Send JSON error response
     * @param res HTTP response
     * @param e HTTP error
     */
    void handleError(IResponse &res, const HttpError &e) override;
};