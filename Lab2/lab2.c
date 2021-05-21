#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SUCCESS_STATUS (0)
#define PUTENV_ERROR (1)
#define PUTENV_ERROR_VALUE (-1)

int main() {
    // Создание переменной типа time_t (числовой тип из библотеки time.h)
    time_t now;
    // Создание структуры для последущего хранения времени
    struct tm *sp;

    // Установка переменной окружения TZ для последующего опеределения
    // времени в этом часовом поясе
    int putenvRes = putenv("TZ=America/Los_Angeles");
    if (putenvRes == PUTENV_ERROR_VALUE) {
        perror("Error while setting Time Zone");
        return PUTENV_ERROR;
    }

    // Запись текущего времени в переменную now - значение в секундах, считая
    // от timestamp (01.01.1970 00:00)
    time(&now);

    // Вывод текущего времени в формате (Month Week Day H:M:S Year).
    // Функция ctime как раз преобразует время в секундах из переменной now
    // в строку с учетом часового пояса из перменной TZ
    printf("%s", ctime(&now));

    // Функция localtime возвращает указатель на структуру, в которой время
    // хранится в полях (т.е. tm_mday - день, tm_min - минута). Время
    // представляется как локальное (т.е для текущего часового пояса)
    sp = localtime(&now);

    // Вывод времени на основе полей полученной структуры
    printf(
        "%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1,
        sp->tm_mday,
        sp->tm_year + 1900,
        sp->tm_hour,
        sp->tm_min,
        tzname[sp->tm_isdst]
    );

    return SUCCESS_STATUS;
}
