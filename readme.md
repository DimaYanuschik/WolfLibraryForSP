# Wolf Library

## Описание

Wolf Library — это динамическая библиотека (DLL), предоставляющая функции для генерации случайных событий на основе заданных вероятностей.
Библиотека включает в себя функции для расчета вероятностей и получения случайных событий, что может быть полезно в играх или других приложениях,
где требуется случайный выбор.

## Установка

1. Скачайте файлы:
   - `Wolf_DLL_.dll`
   - `Wolf_DLL.lib`
   - `Wolf.h`
   - `framework.h`
   - `pch.h`

2. Добавьте их в ваш проект.

## Использование

### Подключение библиотеки

Чтобы использовать Wolf Library в вашем проекте:

1. Добавьте файл `Wolf.lib` в качестве зависимости:
   - В Visual Studio: 
     - Перейдите в свойства проекта.
     - В разделе **Linker** -> **Input** добавьте `Wolf_DLL.lib` в **Additional Dependencies**.
     - Убедитесь, что путь к директории с `Wolf_DLL.lib` указан в **Linker** -> **General** -> **Additional Library Directories**.
   
2. Добавьте путь к заголовочному файлу `Wolf.h`:
   - В разделе **C/C++** -> **General** -> **Additional Include Directories**.
  
3. Скопируйте **Wolf.h** в католог, в котором содержится файл .exe вашей программый.

### Примеры использования

#### Пример 1: Получение случайного события с тремя вероятностями(вероятности 4 и 5 подсчитываются автоматически)

```cpp
#include "Wolf.h"
#include <iostream>

int main() {
    double p1 = 10.0; // Вероятность события 1
    double p2 = 20.0; // Вероятность события 2
    double p3 = 30.0; // Вероятность события 3

    int event = GetRandomEvent(p1, p2, p3);
    std::cout << "Случайное событие: " << event << std::endl;

    return 0;
}
```

#### Пример 2: Получение случайного события с пятью вероятностями

```cpp
#include "Wolf.h"
#include <iostream>

int main() {
    double p1 = 15.0; // Вероятность события 1
    double p2 = 25.0; // Вероятность события 2
    double p3 = 30.0; // Вероятность события 3
    double p4 = 10.0; // Вероятность события 4
    double p5 = 20.0; // Вероятность события 5

    int event = GetRandomEvent5(p1, p2, p3, p4, p5);
    std::cout << "Случайное событие: " << event << std::endl;

    return 0;
}
```

#### Пример 3: Расчет вероятностей и получение события

```cpp
#include "Wolf.h"
#include <iostream>

int main() {
    double value1 = 50.0, maxValue1 = 100.0, range1 = 40.0;
    double value2 = 25.0, maxValue2 = 100.0, range2 = 30.0;
    double value3 = 10.0, maxValue3 = 100.0, range3 = 20.0;

    int event = CalculateProbabilities(value1, maxValue1, range1, value2, maxValue2, range2, value3, maxValue3, range3);
    std::cout << "Calculated random event: " << event << std::endl;

    return 0;
}
```

## Описание функций

1. int GetRandomEvent(double p1, double p2, double p3)
    Описание: Генерирует случайное событие на основе трех вероятностей(вероятность 4 и 5 подсчитываются автоматически).
    Параметры:
    p1, p2, p3: Вероятности для событий 1, 2 и 3 соответственно.
    Возвращает: Номер события (1-5) или -1 в случае ошибки.

2. int GetRandomEvent5(double p1, double p2, double p3, double p4, double p5)
    Описание: Генерирует случайное событие на основе пяти вероятностей.
    Параметры:
    p1, p2, p3, p4, p5: Вероятности для событий 1-5.
    Возвращает: Номер события (1-5) или -1 в случае ошибки.

3. int CalculateProbabilities(double value1, double maxValue1, double range1, double value2, double maxValue2, double range2, double value3, double maxValue3, double range3)
    Описание: Рассчитывает вероятности на основе переданных значений и диапазонов.
    Параметры:
    value1, maxValue1, range1: Значение, максимальное значение и диапазон для первого события.
    value2, maxValue2, range2: Аналогично для второго события.
    value3, maxValue3, range3: Аналогично для третьего события.
    Возвращает: Номер случайного события или -1 в случае ошибки.
