// Wolf.cpp
#include "pch.h"
#include <windows.h>
#include <random>
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <sstream>


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

void logEvent(const std::string& message, const std::string& folderPath) {
    std::ofstream logFile(folderPath + "/" + LOG_FILE_NAME, std::ios::app); // Открываем в режиме добавления
    if (logFile) {
        logFile << message << std::endl;
    }
}

extern "C" __declspec(dllexport) const char* getFilesWithPrefix(const char* folderPath, const char* prefix) {
    static std::string result;  // Статическая строка, чтобы она сохраняла значение между вызовами.
    result.clear();  // Очищаем предыдущий результат.

    try {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                const std::string fileName = entry.path().filename().string();
                if (fileName.substr(0, strlen(prefix)) == prefix) {
                    if (!result.empty()) {
                        result += " ";  // Добавляем пробел, если строка не пустая
                    }
                    result += entry.path().string();
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error&) {
        return nullptr;  // Возвращаем nullptr в случае ошибки
    }

    return result.empty() ? nullptr : result.c_str();  // Возвращаем C-строку
}

extern "C" __declspec(dllexport) int isCarActive(const std::string & filePath) {
    std::ifstream file(filePath);
    if (!file) {
        return -1; // Ошибка открытия файла
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("inactive moves:", 0) == 0) { // Начинается ли строка с "inactive moves:"
            int inactiveMoves = std::stoi(line.substr(15));
            return (inactiveMoves == 0) ? 1 : 0; // Возвращаем 1, если 0 ходов, иначе 0
        }
    }

    return 1; // Строки "inactive moves: ..." нет, машина активна
}
extern "C" __declspec(dllexport) bool isWolfActive(const std::string & folderPath) {
    std::string wolfFilePath = folderPath + "/" + WOLF_FILE_NAME;
    std::ifstream wolfFile(wolfFilePath);
    if (!wolfFile) {
        return true; // Если файл отсутствует, волк активен
    }

    std::string line;
    while (std::getline(wolfFile, line)) {
        if (line.rfind("inactive moves:", 0) == 0) {
            int inactiveMoves = std::stoi(line.substr(15));
            return inactiveMoves == 0;
        }
    }
    return true; // Если строки "inactive moves:" нет, волк активен
}

int decrementInactiveMoves(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        return -1; // Ошибка открытия файла
    }

    std::vector<std::string> lines;
    std::string line;
    bool updated = false;

    while (std::getline(file, line)) {
        if (line.rfind("inactive moves:", 0) == 0) {
            int inactiveMoves = std::stoi(line.substr(15));
            if (inactiveMoves > 0) {
                --inactiveMoves;
            }
            if (inactiveMoves > 0) {
                lines.push_back("inactive moves: " + std::to_string(inactiveMoves));
            }
            updated = true;
        }
        else {
            lines.push_back(line);
        }
    }
    file.close();

    if (!updated) {
        return 0; // Не было строки "inactive moves:"
    }

    // Перезаписываем файл с обновлёнными данными
    std::ofstream outFile(filePath);
    if (outFile) {
        for (const auto& l : lines) {
            outFile << l << "\n";
        }
        return 1; // Успешное обновление
    }

    return -1; // Ошибка записи
}
void decrementInactiveMoves(const char* folderPath, const char* carFilePrefix) {
    // Уменьшаем неактивные ходы волка
    std::string wolfFilePath = std::string(folderPath) + "/" + WOLF_FILE_NAME;
    decrementInactiveMoves(wolfFilePath);

    // Получаем файлы с заданным префиксом
    const char* carFilesStr = getFilesWithPrefix(folderPath, carFilePrefix);
    if (carFilesStr) {
        std::stringstream ss(carFilesStr);
        std::string carFile;
        while (std::getline(ss, carFile, ' ')) {
            if (!isCarActive(carFile)) {
                decrementInactiveMoves(carFile.c_str());
            }
        }
    }
}

int EatCar(const char* folderPath, const char* carFilePrefix) {
    if (!isWolfActive(folderPath)) {
        return 0; // Волк неактивен
    }

    const char* filesStr = getFilesWithPrefix(folderPath, carFilePrefix);
    if (!filesStr) {
        return 0; // Нет подходящих файлов
    }

    std::vector<std::string> files;
    std::stringstream ss(filesStr);
    std::string file;
    while (std::getline(ss, file, ' ')) {
        files.push_back(file);
    }

    if (files.empty()) {
        return 0; // Нет подходящих файлов
    }

    std::srand(std::time(nullptr)); // Инициализируем генератор случайных чисел
    size_t randomIndex = std::rand() % files.size();
    if (std::filesystem::remove(files[randomIndex])) {
        logEvent("Event: Wolf ate car. Car ID: " + files[randomIndex], folderPath);
        return 1; // Успешное удаление
    }
    return -1; // Ошибка удаления
}
int EatDriver(const char* folderPath, const char* carFilePrefix) {
    if (!isWolfActive(folderPath)) {
        return 0; // Волк неактивен
    }

    const char* filesStr = getFilesWithPrefix(folderPath, carFilePrefix);
    if (!filesStr) {
        return 0; // Нет подходящих файлов
    }

    std::vector<std::string> files;
    std::stringstream ss(filesStr);
    std::string file;
    while (std::getline(ss, file, ' ')) {
        files.push_back(file);
    }

    if (files.empty()) {
        return 0; // Нет подходящих файлов
    }

    std::srand(std::time(nullptr));
    size_t randomIndex = std::rand() % files.size();
    int inactiveMoves = std::rand() % 10 + 1; // Случайное число от 1 до 10

    std::ofstream File(files[randomIndex], std::ios::app); // Открываем файл в режиме добавления
    if (File) {
        File << "inactive moves: " << inactiveMoves << "\n";
        logEvent("Event: Wolf ate driver. Car ID: " + files[randomIndex], folderPath);
        return 1; // Успешное обновление
    }

    return -1; // Ошибка открытия файла
}
int EatHalfBudget(const std::string& folderPath, const std::string& CommonFilePath) {
    if (!isWolfActive(folderPath)) {
        return 0; // Волк неактивен
    }

    // Проверяем, существует ли файл
    if (!std::filesystem::exists(CommonFilePath)) {
        return -1;
    }
    std::ifstream inputFile(CommonFilePath);
    if (!inputFile) {
        return -1; // Ошибка открытия файла для чтения
    }

    std::string line;
    //std::vector<std::string> lines;
    //bool budgetFound = false;
    //double budget = 0.0;
    int budget = 0;

    // Читаем файл построчно и ищем строку с бюджетом
    /*while (std::getline(inputFile, line)) {
        if (line.rfind("budget:", 0) == 0) {
            budget = std::stod(line.substr(7));
            budget /= 2; // Уменьшаем бюджет вполовину
            line = "budget: " + std::to_string(budget);
            budgetFound = true;
        }
        lines.push_back(line);
    }*/
    std::getline(inputFile, line);
    budget = std::stoi(line);
    budget = budget / 2; // Уменьшаем бюджет вполовину
    line = std::to_string(budget);

    inputFile.close();

    /*if (!budgetFound) {
        return 0; // Бюджет не найден в файле
    }*/

    // Перезаписываем файл с обновлёнными данными
    std::ofstream outputFile(CommonFilePath);
    if (!outputFile) {
        return -1; // Ошибка открытия файла для записи
    }

    /*for (const auto& l : lines) {
        outputFile << l << "\n";
    }*/
    outputFile << line << "\n";
    outputFile.close();
    logEvent("Event: EatHalfBudget | Remaining Budget: " + std::to_string(budget), folderPath);
    return 1;

}
bool CarHitWolf(const std::string& folderPath) {
    const std::string wolfFileName = WOLF_FILE_NAME;
    std::string wolfFilePath = folderPath + "/" + wolfFileName;

    std::srand(std::time(nullptr)); // Инициализируем генератор случайных чисел
    int inactiveMoves = std::rand() % 10 + 1; // Случайное число от 1 до 10

    std::ofstream wolfFile(wolfFilePath, std::ios::trunc); // Перезаписываем файл
    if (wolfFile) {
        wolfFile << "inactive moves: " << inactiveMoves << "\n";
        logEvent("Event: Car hit wolf. Wolf inactive for " + std::to_string(inactiveMoves) + " moves.", folderPath);
        return true;
    }
    return false;
}
bool SellWolfSkin(const std::string& folderPath, const std::string& commonFilePath) {
    const std::string wolfFileName = WOLF_FILE_NAME;
    std::string wolfFilePath = folderPath + "/" + wolfFileName;

    // Генерируем случайное число ходов, на которые волк будет неактивен
    std::srand(std::time(nullptr));
    int inactiveMoves = std::rand() % 20 + 10; // Случайное число от 10 до 30

    // Перезаписываем файл волка, чтобы он стал неактивным
    std::ofstream wolfFile(wolfFilePath, std::ios::trunc);
    if (!wolfFile) {
        return false; // Ошибка записи
    }
    wolfFile << "inactive moves: " << inactiveMoves << "\n";
    wolfFile.close();

    // Увеличиваем бюджет
    std::ifstream inputFile(commonFilePath);
    if (!inputFile) {
        return false; // Ошибка чтения файла бюджета
    }

    int budget = 0;
    inputFile >> budget;
    inputFile.close();

    int incr = std::rand() % 20 + 300;
    budget += incr; // Добавляем фиксированную сумму за продажу шкуры

    std::ofstream outputFile(commonFilePath);
    if (!outputFile) {
        return false; // Ошибка записи бюджета
    }
    outputFile << budget << "\n";
    outputFile.close();

    logEvent("Event: Car hit wolf and its skin was sold. Wolf inactive for " + std::to_string(inactiveMoves) + " moves. Budget increased by " + std::to_string(incr), folderPath);
    return true;
}

extern "C" __declspec(dllexport) int ExecuteRandomEvent(
    const std::string & folderPath,
    const std::string & carFilePrefix,
    const std::string & filePath,
    double p1, double p2, double p3, double p4, double p5)
{
    if (!isWolfActive(folderPath)) {
        logEvent("Wolf inactive", folderPath);
        decrementInactiveMoves(folderPath.c_str(), carFilePrefix.c_str());
        return 0;
    }
    decrementInactiveMoves(folderPath.c_str(), carFilePrefix.c_str());
    // Получаем случайное событие (1-5)
    int event = GetRandomEvent5(p1, p2, p3, p4, p5);

    // В зависимости от события выполняем соответствующую функцию
    int i;
    switch (event) {
    case 1:
        i = EatCar(folderPath.c_str(), carFilePrefix.c_str());
        if (i == 1)
            return 1;
        else return i;
    case 2:
        i = EatDriver(folderPath.c_str(), carFilePrefix.c_str());
        if (i == 1)
            return 2;
        else return i;
    case 3:
        i = EatHalfBudget(folderPath, filePath);
        if (i == 1)
            return 3;
        else return i;
    case 4:
        i = CarHitWolf(folderPath);
        if (i)
            return 4;
        else return i;
    case 5:
        i = SellWolfSkin(folderPath, filePath);
        if (i)
            return 5;
        else return i;
    default:
        return -1; // Ошибка
    }
}
