#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "HttpClient.hpp"
#include "SimpleRequest.hpp"
#include "SimpleResponse.hpp"
#include "HttpClientError.hpp"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace beast = boost::beast;

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

TEST(HttpClientTest, SendRealHttpRequest)
{
    std::atomic<bool> serverReady = false;

    std::thread serverThread([&] { runTestHttpServer(serverReady); });

    while (!serverReady.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "127.0.0.1", 8089);
    SimpleResponse response;

    auto result = client.send(request, response);

    serverThread.join();

    ASSERT_TRUE(result.ok());
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

    auto result = client.send(request, response);

    serverThread.join();

    ASSERT_TRUE(result.ok());
    ASSERT_EQ(response.getStatus(), 200);
}

TEST(HttpClientTest, ConnectTimeoutOnUnreachableHost)
{
    setenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS", "500", 1);

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "192.0.2.1", 80);
    SimpleResponse response;

    auto start = std::chrono::steady_clock::now();
    auto result = client.send(request, response);
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error, HttpClientError::ConnectTimeout);
    EXPECT_EQ(response.getStatus(), 200);
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
    auto result = client.send(request, response);
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error, HttpClientError::ConnectTimeout);
    ASSERT_TRUE(elapsedMs >= 200 && elapsedMs < 5000)
        << "Custom timeout should be ~300ms, took " << elapsedMs << "ms";
}

TEST(HttpClientTest, DnsFailedOnInvalidHost)
{
    setenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS", "1000", 1);

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "this.host.does.not.exist.invalid", 80);
    SimpleResponse response;

    auto result = client.send(request, response);

    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error, HttpClientError::DnsFailed);
}

TEST(HttpClientTest, ConnectionRefusedOnClosedPort)
{
    HttpClient client;

    SimpleRequest request("GET", "/test", "", "127.0.0.1", 59999);
    SimpleResponse response;

    auto result = client.send(request, response);

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error, HttpClientError::ConnectionRefused);
}

TEST(HttpClientTest, DefaultConnectTimeoutIs5000)
{
    unsetenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");

    HttpClient client;

    SimpleRequest request("GET", "/test", "", "127.0.0.1", 59999);
    SimpleResponse response;

    auto start = std::chrono::steady_clock::now();
    auto result = client.send(request, response);
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    ASSERT_FALSE(result.ok());
    ASSERT_TRUE(elapsedMs < 15000)
        << "Default connect timeout should be ~5000ms, took " << elapsedMs << "ms";
}

TEST(HttpClientTest, ResponseNotMutatedOnError)
{
    HttpClient client;

    SimpleRequest request("GET", "/test", "", "127.0.0.1", 59999);
    SimpleResponse response;
    response.setStatus(201);
    response.setBody("original");

    auto result = client.send(request, response);

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(response.getStatus(), 201);
    EXPECT_EQ(response.getBody(), "original");
}

TEST(HttpClientTest, HttpClientResultOk)
{
    HttpClientResult result;
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(result.error, HttpClientError::None);
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST(HttpClientTest, HttpClientResultError)
{
    HttpClientResult result{HttpClientError::ReadTimeout, "read timeout"};
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, HttpClientError::ReadTimeout);
    EXPECT_EQ(result.errorMessage, "read timeout");
}

TEST(HttpClientTest, HttpClientErrorToString)
{
    EXPECT_EQ(httpClientErrorToString(HttpClientError::None), "none");
    EXPECT_EQ(httpClientErrorToString(HttpClientError::DnsFailed), "dns_failed");
    EXPECT_EQ(httpClientErrorToString(HttpClientError::ConnectTimeout), "connect_timeout");
    EXPECT_EQ(httpClientErrorToString(HttpClientError::ConnectionRefused), "connection_refused");
    EXPECT_EQ(httpClientErrorToString(HttpClientError::WriteTimeout), "write_timeout");
    EXPECT_EQ(httpClientErrorToString(HttpClientError::ReadTimeout), "read_timeout");
    EXPECT_EQ(httpClientErrorToString(HttpClientError::UnknownError), "unknown_error");
}