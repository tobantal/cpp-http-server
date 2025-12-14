#include "settings/DbSettings.hpp"
#include <iostream>

/**
 * @file DbSettings.cpp
 * @brief Реализация адаптера настроек БД
 * @author Anton Tobolkin
 */

DbSettings::DbSettings(std::shared_ptr<IEnvironment> env)
{
    try {
        host_ = env->get<std::string>("db.host");
    } catch (...) {
        throw std::runtime_error("Missing required setting: db.host");
    }

    try {
        port_ = env->get<int>("db.port");
    } catch (...) {
        throw std::runtime_error("Missing required setting: db.port");
    }

    try {
        name_ = env->get<std::string>("db.name");
    } catch (...) {
        throw std::runtime_error("Missing required setting: db.name");
    }

    try {
        user_ = env->get<std::string>("db.user");
    } catch (...) {
        throw std::runtime_error("Missing required setting: db.user");
    }

    try {
        password_ = env->get<std::string>("db.password");
    } catch (...) {
        throw std::runtime_error("Missing required setting: db.password");
    }
}

std::string DbSettings::getHost() const
{
    return host_;
}

int DbSettings::getPort() const
{
    return port_;
}

std::string DbSettings::getName() const
{
    return name_;
}

std::string DbSettings::getUser() const
{
    return user_;
}

std::string DbSettings::getPassword() const
{
    return password_;
}