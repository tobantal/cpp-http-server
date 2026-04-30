#include "adapters/secondary/SimpleRequest.hpp"

void SimpleRequest::setMethod(const std::string& method) {
    method_ = method;
}

void SimpleRequest::setPath(const std::string& path) {
    path_ = path;
}

void SimpleRequest::setIp(const std::string& ip) {
    ip_ = ip;
}

void SimpleRequest::setPort(int port) {
    port_ = port;
}

std::string SimpleRequest::getBody() const {
    return body_;
}

void SimpleRequest::setBody(const std::string& body) {
    body_ = body;
}

std::string SimpleRequest::getMethod() const {
    return method_;
}

std::string SimpleRequest::getIp() const {
    return ip_;
}

int SimpleRequest::getPort() const {
    return port_;
}