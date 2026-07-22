#include "MatchingEngine.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[]) {
    std::string filename = (argc > 1) ? argv[1] : "orders.txt";

    std::vector<OrderMessage> messages;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open " << filename << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        OrderMessage msg{};
        if (type == "Add") {
            msg.type = OrderMessage::Type::Add;
            iss >> msg.orderId >> msg.isBuy >> msg.shares >> msg.price;
        } else if (type == "Cancel") {
            msg.type = OrderMessage::Type::Cancel;
            iss >> msg.orderId;
        } else if (type == "Modify") {
            msg.type = OrderMessage::Type::Modify;
            iss >> msg.orderId >> msg.shares >> msg.price;
        } else {
            continue;
        }
        messages.push_back(msg);
    }

    std::cout << "Loaded " << messages.size() << " orders from " << filename << "\n";

    MatchingEngine engine;
    std::thread matchingThread([&engine]() { engine.run(); });

    auto start = std::chrono::high_resolution_clock::now();

    for (const auto& msg : messages)
        while (!engine.submit(msg)) {}

    engine.stop();
    matchingThread.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Processed " << messages.size() << " orders in " << ms << " ms\n";
    if (ms > 0)
        std::cout << "Throughput: " << (messages.size() * 1000 / ms) << " orders/sec\n";

    return 0;
}
