#pragma once

#include <IHttpHandler.hpp>

class HealthHandler : public IHttpHandler
{
public:
    void handle(IRequest &req, IResponse &res) override
    {
        res.setResult(200, "application/json", R"({"status": "ok"})");
    }
};
