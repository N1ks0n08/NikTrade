#pragma once

#include <functional> /// for callbacks (passing a function as an argument)
#include <zmq.hpp> // C++ binding for ZeroMQ
#include <string>
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <atomic>
#include "../flatbuffers/Binance/binance_bookticker_generated.h"

// Forward declare FlatBuffers namespace
namespace Binance {
    struct BookTicker; // forward declaration to reduce compile time
}

// ZMQSubscriber pushes decoded BookTicker messages into a lock-free queue
class ZMQSubscriber {
public:
    // type alias Callback for a function that takes a pointer to BookTicker and returns nothing
    // allows users of the subscriber to define custom handling functions for incoming messages
    using Callback = std::function<void(const Binance::BookTicker*)>;

    // setup subscriber to connect to the endpoint at default local TCP port 5555
    ZMQSubscriber(boost::lockfree::spsc_queue<const Binance::BookTicker*>* queue,
                  const std::string& endpoint = "tcp://127.0.0.1:5555");
    // Destructor ensures resources (socket/context) are cleanly closed
    ~ZMQSubscriber();

    // start subscriber loop
    void run();

    // Stop subscriber gracefully
    void stop();

private:
    void receiveLoop();

    std::string endpoint_; // ZeroMQ endpoint string
    zmq::context_t context_; //ZeroMQ context
    zmq::socket_t socket_; // Actual subscriber scoket
    std::atomic<bool> running_; // Boolean flag controllling the main receive loop
    std::thread workerThread_;
    boost::lockfree::spsc_queue<const Binance::BookTicker*>* queue_; // external queue
};
