#pragma once

#include <zmq.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <thread>  // Add this line


namespace Binance {

class ZMQSubscriber {
public:
    explicit ZMQSubscriber(
        size_t queue_capacity = 1024,
        const std::string& endpoint = "tcp://127.0.0.1:5555"
    );
    
    ~ZMQSubscriber();
    
    // Copy/move semantics
    ZMQSubscriber(const ZMQSubscriber&) = delete;
    ZMQSubscriber& operator=(const ZMQSubscriber&) = delete;
    ZMQSubscriber(ZMQSubscriber&&) = delete;
    ZMQSubscriber& operator=(ZMQSubscriber&&) = delete;

    void start();
    void stop() noexcept;
    bool pop(std::pair<std::string, std::vector<uint8_t>>& data); // Updated to also return topic

private:
    void receive_loop();
    std::unique_ptr<boost::lockfree::spsc_queue<std::pair<std::string, std::vector<uint8_t>>>> queue_; // Updated to hold topic and payload
    std::string endpoint_;
    zmq::context_t context_;
    zmq::socket_t socket_;
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
};
} // namespace Binance