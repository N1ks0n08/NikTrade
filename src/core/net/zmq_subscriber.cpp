#include "zmq_subscriber.hpp"
#include <chrono>
#include <iostream>
#include <thread>

namespace Binance {

ZMQSubscriber::ZMQSubscriber(size_t queue_capacity, const std::string& endpoint)
    : queue_(std::make_unique<boost::lockfree::spsc_queue<std::pair<std::string, std::vector<uint8_t>>>>(queue_capacity)), // Updated to hold topic and payload
      endpoint_(endpoint),
      context_(1),
      socket_(context_, ZMQ_SUB)
{
    socket_.connect(endpoint_);
    socket_.set(zmq::sockopt::subscribe, "");  // Updated setsockopt
}

ZMQSubscriber::~ZMQSubscriber() {
    stop();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void ZMQSubscriber::start() {
    if (!running_.exchange(true)) {
        worker_thread_ = std::thread(&ZMQSubscriber::receive_loop, this);
    }
}

void ZMQSubscriber::stop() noexcept {
    running_.store(false, std::memory_order_release);
}

bool ZMQSubscriber::pop(std::pair<std::string, std::vector<uint8_t>>& data) {
    return queue_->pop(data);
}

void ZMQSubscriber::receive_loop() {
    zmq::pollitem_t items[] = {{socket_, 0, ZMQ_POLLIN, 0}};
    
    while (running_.load(std::memory_order_acquire)) {
        try {
            // Update poll to use chrono
            zmq::poll(items, 1, std::chrono::milliseconds(100));
            
            if (items[0].revents & ZMQ_POLLIN) {
                // Receive topic frame first
                zmq::message_t topic_msg;
                if (!socket_.recv(topic_msg, zmq::recv_flags::none)) continue;

                // Receive payload frame second
                zmq::message_t payload_msg;
                if (!socket_.recv(payload_msg, zmq::recv_flags::none)) continue;

                // Convert topic to string
                std::string topic_str(static_cast<char*>(topic_msg.data()), topic_msg.size());
                
                // Copy payload into vector
                const auto* data = static_cast<uint8_t*>(payload_msg.data());
                std::vector<uint8_t> buffer(data, data + payload_msg.size());

                // Optional: print for debugging
                //fmt::print("[ZMQSubscriber] Received topic: {}\n", topic_str);

                // Push to queue (blocks if full)
                // NOTE: EDIT THIS LATER ON TO DROP OLD MESSAGES IF QUEUE IS FULL
                std::pair<std::string, std::vector<uint8_t>> item{topic_str, std::move(buffer)};
                while (running_ && !queue_->push(std::move(item))) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        } catch (const zmq::error_t& e) {
            // Ignore ETERM; it means context is terminated
            if (e.num() != ETERM) {
                std::cerr << "ZMQ error: " << e.what() << "\n";
            }
            break; // Exit loop on error
        }
    }
}

} // namespace Binance