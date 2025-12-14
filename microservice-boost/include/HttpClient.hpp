#pragma once

#include "IHttpClient.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>

/**
 * @file HttpClient.hpp
 * @brief HTTP клиент для общения между микросервисами
 * @author Anton Tobolkin
 */
class HttpClient : public IHttpClient
{
public:
    /**
     * @brief Отправить HTTP запрос
     * @param request IRequest с методом, IP, портом, путём, телом и заголовками
     * @param response IResponse для записи ответа
     * @return true если успешно, false в случае ошибки
     */
    bool send(const IRequest& request, IResponse& response) override;
};
