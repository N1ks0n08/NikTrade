#include "zmq_subscriber.hpp"
#include <fmt/core.h>
#include <flatbuffers/flatbuffers.h>
#include <iostream>

ZMQSubscriber::ZMQSubscriber(boost::lockfree::spsc_queue<const Binance::BookTicker*>* queue,
                             const std::string& endpoint)
    : endpoint_(endpoint),
      context_(1),
      socket_(context_, zmq::socket_type::sub),
      running_(false),
      queue_(queue)
{
    socket_.connect(endpoint_);
    socket_.set(zmq::sockopt::subscribe, ""); // subscribe to all topics
    fmt::print("ZMQ Subscriber connected to {}\n", endpoint_);
}

ZMQSubscriber::~ZMQSubscriber() {
    stop();
}

void ZMQSubscriber::run() {
    running_ = true;
    workerThread_ = std::thread(&ZMQSubscriber::receiveLoop, this);
}

void ZMQSubscriber::stop() {
    running_ = false;
    if (workerThread_.joinable()) workerThread_.join();
    socket_.close();
    context_.close();
}

void ZMQSubscriber::receiveLoop() {
    while (running_) {
        try {
            zmq::message_t topicMsg;
            zmq::message_t payloadMsg;

            socket_.recv(topicMsg, zmq::recv_flags::none);
            socket_.recv(payloadMsg, zmq::recv_flags::none);

            auto payload_ptr = payloadMsg.data<const uint8_t>();
            auto bookTicker = Binance::GetBookTicker(payload_ptr);

            // push pointer into lock-free queue
            while (!queue_->push(bookTicker)) {
                // queue full, drop message (or optionally spin/wait)
            }

        } catch (const zmq::error_t& e) {
            fmt::print("ZMQ error: {}\n", e.what());
        }
    }
}
