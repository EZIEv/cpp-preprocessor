# Препроцессор директив #include

Этот проект реализует препроцессор для обработки директив `#include` в файлах C++. Программа раскрывает `#include` директивы, вставляя содержимое соответствующих заголовочных файлов в выходной файл.

### Функциональность

- Обнаружение и обработка относительных (`"filename"`) и абсолютных (`<filename>`) путей к файлам.

- Поиск заголовочных файлов в указанных include-директориях.

- Рекурсивная обработка `#include` директив.

- Вывод предупреждений при отсутствии заголовочного файла.

### Структура кода

- `Preprocess()` — рекурсивная функция обработки файлов, раскрывающая #include директивы.

- `IsFileInDir()` — проверяет наличие файла в той же директории, что и исходный файл.

- `FindFileInIncludeDirectories()` — ищет файл в заданных include-директориях.

- `GetFileContents()` — читает содержимое файла в строку.

- `Test()` — набор тестов для проверки работы препроцессора.

### Требования

Компилятор с поддержкой C++17 и выше

Поддержка библиотеки `<filesystem>`

### Пример работы

Входной файл a.cpp:
```
// комментарий перед include
#include "dir1/b.h"
// текст между b.h и c.h
#include "dir1/d.h"
```

Выходной файл a.in:
```
// комментарий перед include
// текст из b.h перед include
// текст из c.h перед include
// std1
// текст из c.h после include
// текст из b.h после include
// текст между b.h и c.h
// текст из d.h перед include
// std2
// текст из d.h после include
```