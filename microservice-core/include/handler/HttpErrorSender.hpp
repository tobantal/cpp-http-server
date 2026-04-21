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
    void handleError(IResponse &res, const HttpError &e) override
    {
        auto traceId = res.getHeader("X-Trace-ID");
        res.setResult(e.statusCode(), "application/json",
                      R"({"error": ")" + StringUtils::escapeJson(e.message()) + R"("})");
        if (traceId)
        {
            res.setTraceId(*traceId);
        }
    }
};