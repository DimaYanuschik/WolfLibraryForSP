#pragma once
#include <Windows.h>
#include <random>
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <queue>


// Структура запроса
struct Request {
    int id;
    std::string message;
};

// функции для достпупа в другом проекте, который будет использовать нашу dll

extern "C" __declspec(dllexport) int GetRandomEvent(double p1, double p2, double p3);

extern "C" __declspec(dllexport) int GetRandomEvent5(double p1, double p2, double p3, double p4, double p5);

extern "C" __declspec(dllexport) int CalculateProbabilities(double value1, double maxValue1, double range1,
    double value2, double maxValue2, double range2,
    double value3, double maxValue3, double range3);

extern "C" __declspec(dllexport) int GetQueueSize(std::queue<Request> requestQueue, std::mutex & queueMutex);
extern "C" __declspec(dllexport) bool GetOperatorStatus(int operatorId, std::vector<std::shared_ptr<std::atomic<bool>>>&operatorFlags, std::mutex & operatorsMutex);
extern "C" __declspec(dllexport) bool RemoveSpecificRequest(int requestId, std::queue<Request>&requestQueue, std::mutex & queueMutex);