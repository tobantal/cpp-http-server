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
    }
}

// -----------------------------------------------------------------------------
//                                Т Е С Т
// -----------------------------------------------------------------------------

TEST(HttpClientTest, SendRealHttpRequest)
{
    std::atomic<bool> serverReady = false;

    std::thread serverThread([&] { runTestHttpServer(serverReady); });

    while (!serverReady.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    HttpClient client;
    
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

TEST(HttpClientTest, ConnectTimeoutOnUnreachableHost)
{
    setenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS", "500", 1);

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "192.0.2.1", 80);
    SimpleResponse response;

    auto start = std::chrono::steady_clock::now();
    bool ok = client.send(request, response);
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    ASSERT_FALSE(ok);
    ASSERT_EQ(response.getStatus(), 500);
    ASSERT_TRUE(elapsedMs >= 400 && elapsedMs < 5000)
        << "Connect timeout should be ~500ms, took " << elapsedMs << "ms";
}

TEST(HttpClientTest, ConnectTimeoutRespectsEnvVariable)
{
    setenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS", "300", 1);

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "192.0.2.1", 80);
    SimpleResponse response;

    auto start = std::chrono::steady_clock::now();
    bool ok = client.send(request, response);
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    ASSERT_FALSE(ok);
    ASSERT_EQ(response.getStatus(), 500);
    ASSERT_TRUE(elapsedMs >= 200 && elapsedMs < 5000)
        << "Custom timeout should be ~300ms, took " << elapsedMs << "ms";
}

TEST(HttpClientTest, InvalidHostReturnsError)
{
    setenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS", "1000", 1);

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "this.host.does.not.exist.invalid", 80);
    SimpleResponse response;

    bool ok = client.send(request, response);

    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    ASSERT_FALSE(ok);
    ASSERT_EQ(response.getStatus(), 500);
}

TEST(HttpClientTest, DefaultConnectTimeoutIs5000)
{
    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "127.0.0.1", 59999);
    SimpleResponse response;

    auto start = std::chrono::steady_clock::now();
    bool ok = client.send(request, response);
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    ASSERT_FALSE(ok);
    ASSERT_TRUE(elapsedMs < 15000)
        << "Default connect timeout should be ~5000ms, took " << elapsedMs << "ms";
}
