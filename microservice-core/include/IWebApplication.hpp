#pragma once

#include "IEnvironment.hpp"
#include "IHttpHandler.hpp"
#include "ChainHandler.hpp"
#include <memory>
#include <string>

/**
 * @file IWebApplication.hpp
 * @brief Базовый интерфейс веб-приложения
 * @version 2.1
 * @author Anton Tobolkin
 */
class IWebApplication
{
public:
    IWebApplication() = default;
    virtual ~IWebApplication() = default;

    /**
     * @brief Запустить приложение (Template Method)
     */
    virtual void run(int argc, char *argv[])
    {
        loadEnvironment(argc, argv);
        configureInjection();
        start();
    }

    // =========================================================================
    // ROUTING API (public)
    // =========================================================================

    /**
     * @brief Зарегистрировать цепочку middleware для endpoint'а
     * @param method HTTP метод
     * @param pattern URL паттерн
     * @param handlers Обработчики (выполняются последовательно)
     *
     * @example
     *   registerEndpoint("GET", "/api/orders/*",
     *       authMiddleware,
     *       loggingMiddleware,
     *       orderByIdHandler);
     */
    template <typename... Handlers>
    void registerEndpoint(const std::string &method,
                          const std::string &pattern,
                          Handlers &&...handlers)
    {
        registerHandler(method, pattern,
                        std::make_shared<ChainHandler>(std::forward<Handlers>(handlers)...));
    }

protected:
    virtual void loadEnvironment(int argc, char *argv[]) = 0;
    virtual void configureInjection() = 0;
    virtual void start() = 0;

    /**
     * @brief Зарегистрировать обработчик (внутренний метод)
     * @param method HTTP метод
     * @param pattern URL паттерн
     * @param handler Обработчик
     *
     * @note Используется из registerEndpoint().
     *       Для регистрации handlers используйте registerEndpoint().
     */
    virtual void registerHandler(
        const std::string &method,
        const std::string &pattern,
        std::shared_ptr<IHttpHandler> handler) = 0;

    std::shared_ptr<IEnvironment> env_;
};
