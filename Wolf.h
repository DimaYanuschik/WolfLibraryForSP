// Wolf.cpp
#include "pch.h"
#include <windows.h>
#include <random>
#include <iostream>

extern "C" __declspec(dllexport) int GetRandomEvent(double p1, double p2, double p3) {
    double p5 = 25.0;
    double p4 = 100.0 - (p1 + p2 + p3 + p5);

    // Проверяем, что p4 не отрицательное
    if (p4 < 0) {
        return -1; // Ошибка
    }

    // Накопительные вероятности
    double cumulativeProbabilities[5] = { p1, p2, p3, p4, p5 };
    for (int i = 1; i < 5; i++) {
        cumulativeProbabilities[i] += cumulativeProbabilities[i - 1];
    }

    // Генерируем случайное число от 0 до 100
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    double randomValue = dis(gen);

    // Определяем событие на основе случайного числа
    for (int i = 0; i < 5; i++) {
        if (randomValue <= cumulativeProbabilities[i]) {
            return i + 1; // Возвращаем событие (1-5)
        }
    }

    return 5; // На всякий случай, если что-то пойдет не так
}

extern "C" __declspec(dllexport) int GetRandomEvent5(double p1, double p2, double p3, double p4, double p5) {
    // Проверяем, что сумма вероятностей не превышает 100%
    double total = p1 + p2 + p3 + p4 + p5;
    if (total > 100.0) {
        return -1;
    }

    // Накопительные вероятности
    double cumulativeProbabilities[5] = { p1, p2, p3, p4, p5 };
    for (int i = 1; i < 5; i++) {
        cumulativeProbabilities[i] += cumulativeProbabilities[i - 1];
    }

    // Генерируем случайное число от 0 до total
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, total);
    double randomValue = dis(gen);

    // Определяем событие на основе случайного числа
    for (int i = 0; i < 5; i++) {
        if (randomValue <= cumulativeProbabilities[i]) {
            return i + 1; // Возвращаем событие (1-5)
        }
    }

    return 5; // На всякий случай, если что-то пойдет не так
}

extern "C" __declspec(dllexport) int CalculateProbabilities(double value1, double maxValue1, double range1,
    double value2, double maxValue2, double range2,
    double value3, double maxValue3, double range3) {
    double p1, p2, p3;
    double p5 = 25.0;

    // Вычисляем p1
    double percent1 = (value1 / maxValue1) * range1;
    p1 = percent1 > range1 ? range1 : percent1;

    // Вычисляем p2
    double percent2 = (value2 / maxValue2) * range2;
    p2 = percent2 > range2 ? range2 : percent2;

    // Вычисляем p3
    double percent3 = (value3 / maxValue3) * range3;
    p3 = percent3 > range3 ? range3 : percent3;

    // Считаем p4
    double total = p1 + p2 + p3;
    double p4 = 100.0 - total - p5;

    if (p4 < 0) {
        return -1; // Ошибка
    }

    // Возвращаем число от 1 до 5 на основе вероятностей
    return GetRandomEvent(p1, p2, p3);
}