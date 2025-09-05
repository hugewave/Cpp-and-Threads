#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

std::queue<std::vector<unsigned char>> buffer;
const unsigned int MAX_BUFFER_SIZE = 10;
std::mutex mtx;
std::condition_variable cv_produce, cv_consume;
bool done = false;

void producer(int id, int num_items) {
    // Random generator setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1000, 4000);
    for (int i = 0; i < num_items; ++i) {
        int len = dist(gen);
        std::vector<unsigned char> data(len);
        for (size_t j = 0; j < data.size(); ++j) {
            data[j] = static_cast<unsigned char>((i + j) % 256);
        }
        std::unique_lock<std::mutex> lock(mtx);
        if (buffer.size() < MAX_BUFFER_SIZE) {
            buffer.push(data);
            std::cout << "Producer " << id << " produced: length=" << data.size() << std::endl;
            cv_consume.notify_one();
        } else {
            std::cout << "Producer " << id << " discarded: length=" << data.size() << " (buffer full)" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::unique_lock<std::mutex> lock(mtx);
    done = true;
    cv_consume.notify_all();
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_consume.wait(lock, [] { return !buffer.empty() || done; });
        if (!buffer.empty()) {
            std::vector<unsigned char> item = buffer.front();
            buffer.pop();
            std::cout << "Consumer " << id << " consumed: [";
            for (auto v : item) std::cout << (int)v << " ";
            std::cout << "]" << std::endl;
            // No need to notify producer here
        } else if (done) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() {
    std::thread prod1(producer, 1, 20);
    std::thread cons1(consumer, 1);
    std::thread cons2(consumer, 2);

    prod1.join();
    cons1.join();
    cons2.join();

    return 0;
}