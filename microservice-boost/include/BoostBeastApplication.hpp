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

class IRequest;
class IResponse;

class BoostBeastApplication : public IWebApplication
{
public:
    BoostBeastApplication();
    virtual ~BoostBeastApplication();

    void start() override;
    void stop();
    void loadEnvironment(int argc, char* argv[]) override;

protected:
    std::map<std::string, std::shared_ptr<IHttpHandler>> handlers_;
    
    std::shared_ptr<IHttpHandler> findHandler(const std::string& method, const std::string& path);
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
