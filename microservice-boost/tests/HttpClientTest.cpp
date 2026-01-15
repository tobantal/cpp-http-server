#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "HttpClient.hpp"
#include "SimpleRequest.hpp"
#include "SimpleResponse.hpp"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace beast = boost::beast;

// -----------------------------------------------------------------------------
//                 Лёгкий тестовый HTTP сервер (Beast)
// -----------------------------------------------------------------------------

void runTestHttpServer(std::atomic<bool>& ready)
{
    using namespace boost;

    try {
        asio::io_context ioc;
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8089));
        ready.store(true);

        tcp::socket socket(ioc);
        acceptor.accept(socket);

        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        http::response<http::string_body> res{http::status::ok, 11};
        res.set(http::field::server, "TestServer");
        res.set(http::field::content_type, "text/plain");
        res.body() = "HelloTest";
        res.prepare_payload();

        http::write(socket, res);

        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
    } catch (...) {
        // игнорируем ошибки тестового сервера
    }
}

// -----------------------------------------------------------------------------
//                                Т Е С Т
// -----------------------------------------------------------------------------

TEST(HttpClientTest, SendRealHttpRequest)
{
    std::atomic<bool> serverReady = false;

    std::thread serverThread([&] { runTestHttpServer(serverReady); });

    // Ждём поднятия сервера
    while (!serverReady.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    HttpClient client;
    
    // Используем SimpleRequest вместо TestRequest
    SimpleRequest request("GET", "/test", "", "127.0.0.1", 8089);
    SimpleResponse response;

    bool ok = client.send(request, response);

    serverThread.join();

    ASSERT_TRUE(ok);
    ASSERT_EQ(response.getStatus(), 200);
    ASSERT_EQ(response.getBody(), "HelloTest");

    auto headers = response.getHeaders();
    ASSERT_TRUE(headers.find("Server") != headers.end());
    ASSERT_EQ(headers["Server"], "TestServer");
}

TEST(HttpClientTest, RequestWithHeaders)
{
    std::atomic<bool> serverReady = false;

    std::thread serverThread([&] { runTestHttpServer(serverReady); });

    while (!serverReady.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    HttpClient client;
    
    SimpleRequest request("GET", "/test", "", "127.0.0.1", 8089);
    request.setHeader("Authorization", "Bearer test-token");
    request.setHeader("Accept", "application/json");
    
    SimpleResponse response;

    bool ok = client.send(request, response);

    serverThread.join();

    ASSERT_TRUE(ok);
    ASSERT_EQ(response.getStatus(), 200);
}
