#include "settings/RabbitMQSettings.hpp"
#include "ports/output/IEnvironment.hpp"
#include <cstdlib>
#include <sstream>

RabbitMQSettings::RabbitMQSettings(std::shared_ptr<IEnvironment> env)
    : host_(kDefaultHost), port_(kDefaultPort), user_(kDefaultUser),
      password_(kDefaultPassword), exchange_(kDefaultExchange), queueName_("")
{
    std::string envHost = getEnvOrDefault("RABBITMQ_HOST", "");
    if (!envHost.empty())
    {
        host_ = envHost;
    }
    else if (env)
    {
        try { host_ = env->get<std::string>("rabbitmq.host"); }
        catch (const std::exception &) { host_ = kDefaultHost; }
    }

    int envPort = getEnvOrDefaultInt("RABBITMQ_PORT", 0);
    if (envPort > 0)
    {
        port_ = envPort;
    }
    else if (env)
    {
        try { port_ = env->get<int>("rabbitmq.port"); }
        catch (const std::exception &) { port_ = kDefaultPort; }
    }

    std::string envUser = getEnvOrDefault("RABBITMQ_USER", "");
    if (!envUser.empty())
    {
        user_ = envUser;
    }
    else if (env)
    {
        try { user_ = env->get<std::string>("rabbitmq.user"); }
        catch (const std::exception &) { user_ = kDefaultUser; }
    }

    std::string envPassword = getEnvOrDefault("RABBITMQ_PASSWORD", "");
    if (!envPassword.empty())
    {
        password_ = envPassword;
    }
    else if (env)
    {
        try { password_ = env->get<std::string>("rabbitmq.password"); }
        catch (const std::exception &) { password_ = kDefaultPassword; }
    }

    std::string envExchange = getEnvOrDefault("RABBITMQ_EXCHANGE", "");
    if (!envExchange.empty())
    {
        exchange_ = envExchange;
    }
    else if (env)
    {
        try { exchange_ = env->get<std::string>("rabbitmq.exchange"); }
        catch (const std::exception &) { exchange_ = kDefaultExchange; }
    }

    const char *envQueue = std::getenv("RABBITMQ_QUEUE_NAME");
    if (envQueue && envQueue[0] != '\0')
    {
        queueName_ = envQueue;
    }
    else if (env)
    {
        try { queueName_ = env->get<std::string>("rabbitmq.queueName"); }
        catch (const std::exception &) { queueName_ = ""; }
    }
}

std::string RabbitMQSettings::getConnectionString() const
{
    std::ostringstream oss;
    oss << "amqp://" << user_ << ":" << password_ << "@" << host_ << ":" << port_ << "/";
    return oss.str();
}

std::string RabbitMQSettings::getEnvOrDefault(const char *name, const std::string &defaultValue)
{
    const char *value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}

int RabbitMQSettings::getEnvOrDefaultInt(const char *name, int defaultValue)
{
    const char *value = std::getenv(name);
    if (value)
    {
        try { return std::stoi(value); }
        catch (const std::exception &) { return defaultValue; }
    }
    return defaultValue;
}