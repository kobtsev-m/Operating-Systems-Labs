#include <unistd.h>
#include <stdio.h>

#define SUCCESS_STATUS (0)
#define WRONG_ARGUMENTS_NUMBER_ERROR (1)
#define FILE_OPEN_ERROR (2)
#define SETUID_ERROR (3)
#define SETUID_ERROR_VALUE (-1)

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Wrong arguments number");
        return WRONG_ARGUMENTS_NUMBER_ERROR;
    }

    // Создание указателя на файл
    FILE *file;

    // Вывод действительного и эффективного идентификаторов процесса
    printf("Real user id: %d\nEffective user id: %d\n", getuid(), geteuid());

    // Первая попытка чтения файла
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error on first file open");
        return FILE_OPEN_ERROR;
    }
    fclose(file);

    // Установка эффективого индентификатора равным действительному
    int setuidRes = setuid(getuid());
    if (setuidRes == SETUID_ERROR_VALUE) {
        perror("Error on setuid");
        return SETUID_ERROR;
    }

    // Вывод идентификаторов после использования функции setuid
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
