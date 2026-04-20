#pragma once

#include "IHttpHandler.hpp"
#include "ILogger.hpp"
#include "NullLogger.hpp"
#include "HttpError.hpp"
#include "StringUtils.hpp"
#include <memory>
#include <vector>

/**
 * @class ChainHandler
 * @brief Middleware chain — выполняет обработчиков последовательно
 *
 * Выполняет каждый handler в порядке добавления. При выбросе HttpError
 * возвращает соответствующий статус. При std::exception — 500.
 * Автоматически извлекает/генерирует X-Trace-ID и прокидывает в ответ.
 */
class ChainHandler : public IHttpHandler
{
public:
    /**
     * @brief Создать цепочку с NullLogger по умолчанию
     * @param handlers Обработчики (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    explicit ChainHandler(Handlers &&...handlers)
        : logger_(std::make_shared<NullLogger>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    /**
     * @brief Создать цепочку с заданным логером
     * @param logger Логер (если nullptr — используется NullLogger)
     * @param handlers Обработчики (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    ChainHandler(std::shared_ptr<ILogger> logger, Handlers &&...handlers)
        : logger_(logger ? std::move(logger) : std::make_shared<NullLogger>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    /**
     * @brief Выполнить цепочку обработчиков
     * @param req HTTP-запрос
     * @param res HTTP-ответ
     *
     * Извлекает/генерирует trace ID, выполняет обработчиков последовательно.
     * По завершении устанавливает X-Trace-ID в ответ. При ошибке —
     * логирует с [traceId] и возвращает JSON-ответ с ошибкой.
     */
    void handle(IRequest &req, IResponse &res) override;

private:
    std::vector<std::shared_ptr<IHttpHandler>> handlers_;
    std::shared_ptr<ILogger> logger_;

    void sendError(IResponse &res, int status, const std::string &message);
};