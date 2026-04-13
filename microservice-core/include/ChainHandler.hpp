#pragma once

#include "IHttpHandler.hpp"
#include "HttpError.hpp"
#include <memory>
#include <vector>
#include <iostream>

class ChainHandler : public IHttpHandler
{
public:
    template <typename... Handlers>
    explicit ChainHandler(Handlers &&...handlers)
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    void handle(IRequest &req, IResponse &res) override
    {
        for (auto &h : handlers_)
        {
            try
            {
                h->handle(req, res);
            }
            catch (const HttpError &e)
            {
                sendError(res, e.statusCode(), e.message());
                return;
            }
            catch (const std::exception &e)
            {
                std::cerr << "[ChainHandler] Unhandled exception: " << e.what() << std::endl;
                sendError(res, 500, "Internal server error");
                return;
            }
        }

        if (res.getStatus() == 0)
        {
            std::cerr << "[ChainHandler] CRITICAL ERROR: chain finished, but response status not set" << std::endl;
            sendError(res, 500, "Internal server error");
        }
    }

private:
    std::vector<std::shared_ptr<IHttpHandler>> handlers_;

    void sendError(IResponse &res, int status, const std::string &message)
    {
        res.setResult(status, "application/json",
                      R"({"error": ")" + message + R"("})");
    }
};