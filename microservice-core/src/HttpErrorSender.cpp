#include "handler/HttpErrorSender.hpp"
#include "util/StringUtils.hpp"

void HttpErrorSender::handleError(IResponse &res, const HttpError &e) {
    auto traceId = res.getHeader("X-Trace-ID");
    res.setResult(e.statusCode(), "application/json",
                  R"({"error": ")" + StringUtils::escapeJson(e.message()) + R"("})");
    if (traceId)
    {
        res.setTraceId(*traceId);
    }
}