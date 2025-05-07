#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <vector>
#include <Windows.h>

class TaskThread {
public:
    TaskThread() : stopFlag(false), taskCompleted(false) {
        workerThread = std::thread(&TaskThread::threadLoop, this);
    }

    ~TaskThread() {
        {
            std::unique_lock<std::mutex> lock(taskMutex);
            stopFlag = true;
            taskCondition.notify_one();
        }
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void addTask(const std::function<void()>& task) {
        {
            std::unique_lock<std::mutex> lock(taskMutex);
            currentTask = task;
            taskAvailable = true;
            taskCompleted = false; // Reset the flag before adding a new task
        }
        taskCondition.notify_one();
    }

    void waitForTaskCompletion() {
        std::unique_lock<std::mutex> lock(taskMutex);
        taskCompletionCondition.wait(lock, [this]() { return taskCompleted; });
    }

private:
    void threadLoop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(taskMutex);
                taskCondition.wait(lock, [this]() { return taskAvailable || stopFlag; });

                if (stopFlag && !taskAvailable) {
                    break;
                }

                task = currentTask;
                taskAvailable = false;
            }
            task(); // Execute the task
            {
                std::unique_lock<std::mutex> lock(taskMutex);
                taskCompleted = true;
            }
            taskCompletionCondition.notify_one(); // Notify task completion
        }
    }

    std::thread workerThread;
    std::function<void()> currentTask;
    std::mutex taskMutex;
    std::condition_variable taskCondition;
    std::condition_variable taskCompletionCondition;
    std::atomic<bool> stopFlag;
    bool taskAvailable = false;
    bool taskCompleted = false;
};

int main() {
    TaskThread taskThread;

    taskThread.addTask([]() { std::cout << "Task 1 executed\n"; });
    Sleep(10000);
    taskThread.waitForTaskCompletion();

    taskThread.addTask([]() { std::cout << "Task 2 executed\n"; });
    taskThread.waitForTaskCompletion();

    taskThread.addTask([]() { std::cout << "Task 3 executed\n"; });
    taskThread.waitForTaskCompletion();

    return 0;
}