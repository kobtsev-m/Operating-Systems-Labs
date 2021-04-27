#include <unistd.h>
#include <stdio.h>

#define SUCCESS_STATUS 0
#define FILE_OPEN_ERROR 1
#define SETUID_ERROR 2

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Wrong arguments number");
    }

    // Создание указателя на файл
    FILE *file;

    // Вывод действительного и эффективного id пользователя
    // Действительный id - id пользователя, вызвавшего процесс
    // Эффективный id - обычно совпадает c действительным, используется для проверки
    // прав доступа, может отличаться от дествительного, если установлен бит setuid
    // на исполняемом файле и файл не принадлежит пользователю
    printf("Real user id: %d\nEffective user id: %d\n", getuid(), geteuid());

    // Первая попытка чтения файла
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error on first file open");
        return FILE_OPEN_ERROR;
    }
    fclose(file);

    // Устанавливка действительного id владельца текущего процесса
    int res = setuid(getuid());
    if (res == -1) {
        perror("Error on setuid");
        return SETUID_ERROR;
    }

    // Вывод идентификаторов после setuid
    printf("New real user id: %d\nNew effective user id: %d\n", getuid(), geteuid());

    // Вторая попытка чтения файла
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error on second file open");
        return FILE_OPEN_ERROR;
    }
    fclose(file);

    return SUCCESS_STATUS;
}
