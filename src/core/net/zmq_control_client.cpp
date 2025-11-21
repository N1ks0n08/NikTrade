#include "zmq_control_client.hpp"
#include <iostream>

ZMQControlClient::ZMQControlClient(const std::string& endpoint)
    : context_(1), socket_(context_, ZMQ_REQ)
{
    socket_.connect(endpoint);
}

ZMQControlClient::~ZMQControlClient() {
    socket_.close();
    context_.close();
}

bool ZMQControlClient::requestHistoricalKlines(const std::string& symbol, int timeoutMs) {
    zmq::message_t request(symbol.c_str(), symbol.size());
    if (!socket_.send(request, zmq::send_flags::none)) {
        std::cerr << "Failed to send request\n";
        return false;
    }

    // Poll for a reply
    zmq::pollitem_t items[] = { {socket_, 0, ZMQ_POLLIN, 0} };
    zmq::poll(items, 1, timeoutMs);

    if (items[0].revents & ZMQ_POLLIN) {
        zmq::message_t reply;
        socket_.recv(reply);
        std::string replyStr(static_cast<char*>(reply.data()), reply.size());
        return replyStr == "OK";
    }

    std::cerr << "Timeout waiting for control reply\n";
    return false;
}
