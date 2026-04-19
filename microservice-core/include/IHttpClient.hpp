#pragma once

#include "IRequest.hpp"
#include "IResponse.hpp"
#include "HttpClientError.hpp"

class IHttpClient
{
public:
    virtual ~IHttpClient() = default;

    virtual HttpClientResult send(const IRequest& request, IResponse& response) = 0;
};