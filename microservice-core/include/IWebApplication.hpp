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
     */
    virtual void start() = 0;

    std::shared_ptr<IEnvironment> env_; ///< Конфигурация окружения
};
