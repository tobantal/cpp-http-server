// BoostBeast/HttpClient.hpp
#pragma once
#include "IRequest.hpp"
#include "IResponse.hpp"

/**
 * @file IHttpClient.hpp
 * @brief HTTP клиент для отправки сообщений
 * @author Anton Tobolkin
 */
class IHttpClient
{
public:
    /**
     * @brief Отправить HTTP запрос
     * @param request IRequest с методом, IP, портом, путём, телом и заголовками
     * @param response IResponse для записи ответа
     * @return true если успешно, false в случае ошибки
     */
    virtual bool send(const IRequest& request, IResponse& response) = 0;
};
