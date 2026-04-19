#pragma once

#include "IHttpHandler.hpp"
#include "ILogger.hpp"
#include "NullLogger.hpp"
#include "HttpError.hpp"
#include "StringUtils.hpp"
#include <memory>
#include <vector>

class ChainHandler : public IHttpHandler
{
public:
    template <typename... Handlers>
    explicit ChainHandler(Handlers &&...handlers)
        : logger_(std::make_shared<NullLogger>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    template <typename... Handlers>
    ChainHandler(std::shared_ptr<ILogger> logger, Handlers &&...handlers)
        : logger_(logger ? std::move(logger) : std::make_shared<NullLogger>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    void handle(IRequest &req, IResponse &res) override;

private:
    std::vector<std::shared_ptr<IHttpHandler>> handlers_;
    std::shared_ptr<ILogger> logger_;

    void sendError(IResponse &res, int status, const std::string &message);
};