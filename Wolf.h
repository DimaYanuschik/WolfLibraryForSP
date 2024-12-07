// Wolf.cpp
#include "pch.h"
#include <windows.h>
#include <random>
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>


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
        return -1; // Ошибка
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

    // Если сумма вероятностей превышает 100%, возвращаем ошибку
    if (p4 < 0) {
        return -1; // Ошибка
    }

    // Возвращаем число от 1 до 5 на основе вероятностей
    return GetRandomEvent(p1, p2, p3);
}

// Функция для удаления из хранилища. В зависимости от параметра use_atomic,
// используется атомарное хранилище или обычное с мьютексом.
extern "C" __declspec(dllexport) bool DeleteFromStorage(
    std::atomic<int>& atomic_storage,
    int& mutex_storage,
    bool use_atomic,
    std::mutex& mtx
) {
    if (use_atomic) {
        int current = atomic_storage.load(std::memory_order_relaxed);
        if (current > 0) {
            atomic_storage.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }
        else {
            return false;
        }
    }
    else {
        std::lock_guard<std::mutex> lock(mtx);
        if (mutex_storage > 0) {
            --mutex_storage;
            return true;
        }
        else {
            return false;
        }
    }
}

// Функция для сброса количества элементов в хранилище
extern "C" __declspec(dllexport) void ResetStorageCount(std::atomic<int>& atomic_storage, int& mutex_storage, bool use_atomic) {
    if (use_atomic) {
        atomic_storage.store(0, std::memory_order_relaxed);
      
    }
    else {
        mutex_storage = 0;
    
    }
}

// Функция для возобновления работы потоков
extern "C" __declspec(dllexport) void ResumeThreads(
    std::mutex& cv_mtx,
    std::condition_variable& cv,
    std::atomic<bool>& should_terminate
) {
    std::unique_lock<std::mutex> lock(cv_mtx);
    should_terminate.store(false, std::memory_order_relaxed);
    cv.notify_all();
}

// Функция для удаления потока до окончания программы
__declspec(dllexport) void DeleteThread(int thread_id, std::chrono::steady_clock::time_point start_time,
    std::mutex& cv_mtx, std::atomic<bool>& should_terminate,
    std::condition_variable& cv, int simulation_duration_ms) {
    std::unique_lock<std::mutex> lock(cv_mtx);
    should_terminate.store(true, std::memory_order_relaxed);

    // Вычисление оставшегося времени в симуляции
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
    auto remaining_time = simulation_duration_ms - elapsed_time;

    if (remaining_time > 0) {
        cv.wait_for(lock, std::chrono::milliseconds(remaining_time), [&] {
            return !should_terminate.load(std::memory_order_relaxed);
            });
    }
}

// Статус блокировки потока (заблокирован или возрожден)
enum class BlockStatus {
    Blocked,
    Revived
};

// Функция для блокировки потока на случайное время
__declspec(dllexport) BlockStatus BlockThread(int thread_id, std::mutex& cv_mtx, std::atomic<bool>& should_terminate,
    std::condition_variable& cv) {
    std::unique_lock<std::mutex> lock(cv_mtx);
    should_terminate.store(true, std::memory_order_relaxed);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 5);

    // Ожидание случайное количество времени или до завершения потока
    bool revived = cv.wait_for(lock, std::chrono::milliseconds(dis(gen)), [&] {
        return !should_terminate.load(std::memory_order_relaxed);
        });

    return revived ? BlockStatus::Revived : BlockStatus::Blocked;
}


