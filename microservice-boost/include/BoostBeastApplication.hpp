#pragma once
#include "IWebApplication.hpp"
#include "IHttpHandler.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <map>
#include <optional>

class IRequest;
class IResponse;

/**
 * @brief Структура для возврата результата поиска handler'а
 * @version 2.0
 * 
 * Содержит найденный handler и паттерн, по которому он был найден.
 * Используется для передачи информации о маршруте в handleRequest().
 */
struct HandlerMatch {
    std::shared_ptr<IHttpHandler> handler;  ///< Найденный handler
    std::string pattern;                     ///< Паттерн, по которому найден handler
};

/**
 * @class BoostBeastApplication
 * @brief HTTP-сервер на основе Boost.Beast/Asio
 * @version 2.0
 * @author Anton Tobolkin
 */
class BoostBeastApplication : public IWebApplication
{
public:
    BoostBeastApplication();
    virtual ~BoostBeastApplication();

    void start() override;
    void stop();
    void loadEnvironment(int argc, char* argv[]) override;

protected:
    /**
     * @brief Разделитель между методом и паттерном в ключе хэндлера.
     * 
     * Примеры ключей:
     *   "GET:/api/v1/orders"      - exact match
     *   "GET:/api/v1/orders/*"    - wildcard match
     *   "POST:/api/v1/auth/login" - exact match
     *
     * ВАЖНО: При переходе на Express.js стиль path parameters (:orderId)
     * символ ':' создаст неоднозначность в ключе: "GET:/api/v1/orders/:orderId"
     * Первый ':' — разделитель, второй ':' — начало параметра.
     * Текущий код find(':') найдёт первый, что корректно.
     * Но для читаемости рекомендуется заменить на '|' или '#' при рефакторинге.
     */
    static constexpr char HANDLER_KEY_DELIMITER = ':';

    std::map<std::string, std::shared_ptr<IHttpHandler>> handlers_;
    
    /**
     * @brief Найти handler для указанного метода и пути
     * @param method HTTP метод (GET, POST, etc.)
     * @param path Путь запроса
     * @return HandlerMatch с handler'ом и паттерном или nullopt если не найден
     * 
     * @note Сначала ищет точное совпадение, затем по wildcard паттернам.
     */
    std::optional<HandlerMatch> findHandler(const std::string& method, const std::string& path);
    
    /**
     * @brief Сформировать ключ для хранения handler'а
     * @param method HTTP метод
     * @param pattern Паттерн маршрута
     * @return Ключ вида "METHOD:pattern"
     */
    std::string getHandlerKey(const std::string& method, const std::string& pattern) const;

private:
    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    bool running_;

    void handleSession(boost::asio::ip::tcp::socket socket);
    void loadJsonToEnvironment(const nlohmann::json& j, const std::string& prefix = "");
    void handleBeastRequest(
        const boost::beast::http::request<boost::beast::http::string_body>& req,
        boost::beast::http::response<boost::beast::http::string_body>& res,
        const std::string& clientIp);
    
    void handleRequest(IRequest& req, IResponse& res);
};
