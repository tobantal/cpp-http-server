#include "HttpClient.hpp"
#include <iostream>

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

bool HttpClient::send(const IRequest& request, IResponse& response)
{
    try
    {
        std::string portStr = std::to_string(request.getPort());

        std::cout << "[HttpClient] Sending " << request.getMethod() 
                  << " " << request.getIp() << ":" << portStr << request.getPath() << std::endl;

        asio::io_context ioc;
        tcp::resolver resolver(ioc);
        auto results = resolver.resolve(request.getIp(), portStr);

        beast::tcp_stream stream(ioc);
        stream.connect(results);

        // Формируем HTTP запрос
        http::request<http::string_body> req;
        req.method(http::string_to_verb(request.getMethod()));
        req.target(request.getPath());
        req.version(11);

        // Базовые хэдеры
        req.set(http::field::host, request.getIp());
        req.set(http::field::user_agent, "microservices/1.0");

        // Копируем хэдеры из IRequest
        for (const auto& [key, value] : request.getHeaders())
        {
            req.set(key, value);
        }

        // Устанавливаем body если есть
        std::string body = request.getBody();
        if (!body.empty())
        {
            req.body() = body;
            req.set(http::field::content_length, std::to_string(body.length()));
        }

        req.prepare_payload();

        // Отправляем запрос
        http::write(stream, req);

        // Читаем ответ
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        // Закрываем соединение
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        std::cout << "[HttpClient] Received status: " << res.result_int() << std::endl;

        // Заполняем response
        response.setStatus(res.result_int());
        response.setBody(res.body());
        
        for (const auto& field : res)
        {
            response.setHeader(std::string(field.name_string()), std::string(field.value()));
        }

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[HttpClient] Error: " << e.what() << std::endl;
        response.setStatus(500);
        response.setBody("Internal Server Error");
        return false;
    }
}
