#pragma once
#include <string>
#include <zmq.hpp>

class ZMQControlClient {
public:
    ZMQControlClient(const std::string& endpoint);
    ~ZMQControlClient();

    // Sends a request and optionally waits for a reply
    bool requestHistoricalKlines(const std::string& symbol, int timeoutMs = 500);

private:
    zmq::context_t context_;
    zmq::socket_t socket_;
};
