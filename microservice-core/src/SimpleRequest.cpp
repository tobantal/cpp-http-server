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