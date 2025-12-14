#pragma once

#include "IEnvironment.hpp"
#include <memory>
#include <iostream>

/**
 * @file IWebApplication.hpp
 * @brief Базовый интерфейс веб-приложения
 * @author Anton Tobolkin
 */

/**
 * @class IWebApplication
 * @brief Интерфейс веб-приложения с Template Method паттерном
 * 
 * Паттерн Template Method:
 * - run() - шаблонный метод (не виртуальный)
 * - loadEnvironment() - загрузка конфигурации из окружения
 * - configureInjection() - настройка DI
 * - start() - запуск сервера
 */
class IWebApplication
{
public:
    IWebApplication() = default;
    virtual ~IWebApplication() = default;

    /**
     * @brief Запустить приложение (Template Method)
     * 
     * Вызывает последовательно:
     * 1. loadEnvironment(argc, argv) - парсинг аргументов и загрузка конфигурации
     * 2. configureInjection() - настройка DI контейнера
     * 3. start() - запуск HTTP сервера
     * 
     * @param argc Количество аргументов командной строки
     * @param argv Массив аргументов командной строки
     */
    virtual void run(int argc, char* argv[])
    {
        loadEnvironment(argc, argv);
        configureInjection();
        start();
    }

protected:
    /**
     * @brief Загрузить конфигурацию из окружения
     * 
     * PRODUCTION-LIKE РЕАЛИЗАЦИЯ ДОЛЖНА ВКЛЮЧАТЬ:
     * ================================================
     * 
     * 1. ПАРСИНГ АРГУМЕНТОВ КОМАНДНОЙ СТРОКИ:
     *    - Флаги запуска (--port=8080, --host=0.0.0.0, --env=production)
     *    - Путь к конфигурационному файлу (--config=/etc/app/config.yaml)
     *    - Уровень логирования (--log-level=info)
     *    - Режим работы (--mode=cluster)
     * 
     * 2. ЗАГРУЗКА ПЕРЕМЕННЫХ ОКРУЖЕНИЯ (12-Factor App):
     *    - APP_PORT, APP_HOST
     *    - DATABASE_URL, REDIS_URL
     *    - JWT_SECRET, API_KEY
     *    - LOG_LEVEL, ENVIRONMENT (dev/staging/prod)
     * 
     * 3. ЧТЕНИЕ КОНФИГУРАЦИОННЫХ ФАЙЛОВ:
     *    - YAML/JSON/TOML конфигурация
     *    - Секреты из файлов (Docker secrets, Kubernetes secrets)
     *    - Профили конфигурации (dev.yaml, prod.yaml)
     * 
     * 4. ЗАГРУЗКА SSL/TLS СЕРТИФИКАТОВ:
     *    - Пути к сертификатам (cert.pem, key.pem)
     *    - CA bundle для проверки клиентских сертификатов
     *    - Валидация сертификатов (проверка срока действия)
     * 
     * 5. ИНТЕГРАЦИЯ С ВНЕШНИМИ СЕРВИСАМИ:
     *    - Consul/etcd для distributed configuration
     *    - HashiCorp Vault для секретов
     *    - AWS Systems Manager Parameter Store
     *    - Azure Key Vault, Google Cloud Secret Manager
     * 
     * 6. ВАЛИДАЦИЯ КОНФИГУРАЦИИ:
     *    - Проверка обязательных параметров
     *    - Валидация форматов (URL, email, IP)
     *    - Проверка доступности ресурсов (порты, файлы)
     *    - Early fail если конфигурация невалидна
     * 
     * @param argc Количество аргументов командной строки
     * @param argv Массив аргументов командной строки
     */
    virtual void loadEnvironment(int argc, char* argv[]) = 0;

    /**
     * @brief Настроить DI контейнер (Boost.DI injector)
     */
    virtual void configureInjection() = 0;

    /**
     * @brief Запустить HTTP сервер
     * 
     * Использует параметры из env_ (host, port)
     */
    virtual void start() = 0;

    std::shared_ptr<IEnvironment> env_; ///< Конфигурация окружения
};