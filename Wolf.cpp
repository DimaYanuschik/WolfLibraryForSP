#include "pch.h"
#include "Wolf.h"

int GetRandomEvent(double p1, double p2, double p3) {
    double p5 = 25.0;
    double p4 = 100.0 - (p1 + p2 + p3 + p5);

    // ���������, ��� p4 �� �������������
    if (p4 < 0) {
        return -1; // ������
    }

    // ������������� �����������
    double cumulativeProbabilities[5] = { p1, p2, p3, p4, p5 };
    for (int i = 1; i < 5; i++) {
        cumulativeProbabilities[i] += cumulativeProbabilities[i - 1];
    }

    // ���������� ��������� ����� �� 0 �� 100
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    double randomValue = dis(gen);

    // ���������� ������� �� ������ ���������� �����
    for (int i = 0; i < 5; i++) {
        if (randomValue <= cumulativeProbabilities[i]) {
            return i + 1; // ���������� ������� (1-5)
        }
    }

    return 5; // �� ������ ������, ���� ���-�� ������ �� ���
}


extern "C" __declspec(dllexport) int GetRandomEvent5(double p1, double p2, double p3, double p4, double p5) {
    // ���������, ��� ����� ������������ �� ��������� 100%
    double total = p1 + p2 + p3 + p4 + p5;
    if (total > 100.0) {
        return -1; // ������
    }

    // ������������� �����������
    double cumulativeProbabilities[5] = { p1, p2, p3, p4, p5 };
    for (int i = 1; i < 5; i++) {
        cumulativeProbabilities[i] += cumulativeProbabilities[i - 1];
    }

    // ���������� ��������� ����� �� 0 �� total
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, total);
    double randomValue = dis(gen);

    // ���������� ������� �� ������ ���������� �����
    for (int i = 0; i < 5; i++) {
        if (randomValue <= cumulativeProbabilities[i]) {
            return i + 1; // ���������� ������� (1-5)
        }
    }

    return 5; // �� ������ ������, ���� ���-�� ������ �� ���
}

extern "C" __declspec(dllexport) int CalculateProbabilities(double value1, double maxValue1, double range1,
    double value2, double maxValue2, double range2,
    double value3, double maxValue3, double range3) {
    double p1, p2, p3;
    double p5 = 25.0;

    // ��������� p1
    double percent1 = (value1 / maxValue1) * range1;
    p1 = percent1 > range1 ? range1 : percent1;

    // ��������� p2
    double percent2 = (value2 / maxValue2) * range2;
    p2 = percent2 > range2 ? range2 : percent2;

    // ��������� p3
    double percent3 = (value3 / maxValue3) * range3;
    p3 = percent3 > range3 ? range3 : percent3;

    // ������� p4
    double total = p1 + p2 + p3;
    double p4 = 100.0 - total - p5;

    // ���� ����� ������������ ��������� 100%, ���������� ������
    if (p4 < 0) {
        return -1; // ������
    }

    // ���������� ����� �� 1 �� 5 �� ������ ������������
    return GetRandomEvent(p1, p2, p3);
}

extern "C" __declspec(dllexport) int GetQueueSize(std::queue<Request> requestQueue, std::mutex & queueMutex) {
    std::lock_guard<std::mutex> lock(queueMutex);
    return static_cast<int>(requestQueue.size());
}

extern "C" __declspec(dllexport) bool GetOperatorStatus(int operatorId, std::vector<std::shared_ptr<std::atomic<bool>>>&operatorFlags, std::mutex & operatorsMutex) {
    std::lock_guard<std::mutex> lock(operatorsMutex);
    if (operatorId >= 1 && operatorId <= operatorFlags.size()) {
        return !operatorFlags[operatorId - 1]->load();
    }
    return false;
}

extern "C" __declspec(dllexport) bool RemoveSpecificRequest(int requestId, std::queue<Request>&requestQueue, std::mutex & queueMutex) {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::queue<Request> tempQueue;
    bool found = false;
    while (!requestQueue.empty()) {
        Request req = requestQueue.front();
        requestQueue.pop();
        if (req.id != requestId) {
            tempQueue.push(req);
        }
        else {
            found = true;
        }
    }
    requestQueue = tempQueue;
    return found;
}
