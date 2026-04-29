/**
 * @file HttpErrorSender.cpp
 * @brief HttpErrorSender implementation
 * @author Anton Tobolkin
 */

#include "handler/HttpErrorSender.hpp"

/**
 * @brief Handle HTTP error - send JSON error response
 * @param res HTTP response
 * @param e HttpError with status code and message
 *
 * Sets 503 with JSON body {"error": "message"} and preserves X-Trace-ID.
 */
void HttpErrorSender::handleError(IResponse &res, const HttpError &e)
{
    auto traceId = res.getHeader("X-Trace-ID");
    res.setResult(e.statusCode(), "application/json",
                  R"({"error": ")" + StringUtils::escapeJson(e.message()) + R"("})");
    if (traceId)
    {
        res.setTraceId(*traceId);
    }
}