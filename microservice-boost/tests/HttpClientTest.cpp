#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "HttpClient.hpp"
#include "IRequest.hpp"
#include "IResponse.hpp"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace beast = boost::beast;

// --- Тестовые реализации IRequest/IResponse ---------------------------------

class TestRequest : public IRequest {
public:
    std::string method = "GET";
    std::string ip = "127.0.0.1";
    int port = 8089;
    std::string path = "/test";
    std::map<std::string, std::string> headers{};
    std::string body_;
    std::map<std::string, std::string> params{};

    std::string getMethod() const override { return method; }
    std::string getIp() const override { return ip; }
    int getPort() const override { return port; }
    std::string getPath() const override { return path; }
    std::map<std::string, std::string> getHeaders() const override { return headers; }
    std::string getBody() const override { return body_; }

    std::map<std::string, std::string> getParams() const override { return params; }
};


class TestResponse : public IResponse {
public:
    int status = 0;
    std::string body;
    std::map<std::string, std::string> headers{};

    void setStatus(int s) override { status = s; }
    void setBody(const std::string& b) override { body = b; }
    void setHeader(const std::string& name, const std::string& value) override {
        headers[name] = value;
    }

    int getStatus() const { return status; }
    std::string getBody() const { return body; }
    std::map<std::string, std::string> getHeaders() const { return headers; }
};


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
    TestRequest request;
    TestResponse response;

    bool ok = client.send(request, response);

    serverThread.join();

    ASSERT_TRUE(ok);
    ASSERT_EQ(response.getStatus(), 200);
    ASSERT_EQ(response.getBody(), "HelloTest");

    auto headers = response.getHeaders();
    ASSERT_TRUE(headers.find("Server") != headers.end());
    ASSERT_EQ(headers["Server"], "TestServer");
}
