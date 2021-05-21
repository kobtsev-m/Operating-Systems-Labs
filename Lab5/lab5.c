#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#define SUCCESS_STATUS (0)
#define WRONG_ARGUMENTS_NUMBER_ERROR (1)
#define FILE_OPEN_ERROR (2)
#define FILE_READ_ERROR (3)
#define FILE_CLOSE_ERROR (4)
#define LSEEK_ERROR (5)
#define STDOUT_WRITE_ERROR (6)
#define INVALID_VALUE_ERROR (7)
#define MEMORY_REALLOCATION_ERROR (8)

#define FILE_OPEN_ERROR_VALUE (-1)
#define FILE_READ_ERROR_VALUE (-1)
#define FILE_CLOSE_ERROR_VALUE (-1)
#define LSEEK_ERROR_VALUE (-1)
#define STDOUT_WRITE_ERROR_VALUE (-1)

#define LINE_END_SYMBOL ('\n')
#define TERMINAL_ZERO ('\0')
#define DECIMAL_SYSTEM (10)
#define TABLE_SIZE (256)
#define BUF_SIZE (100)
#define STOP_IDX (0)


int fillTable(int fileDescriptor, int *linesOffsets, int *linesLen, int *linesTotal) {
    char buffer[BUF_SIZE];
    int bufferSize =  BUF_SIZE;
    int currentOffset = 0;
    int lineIdx = 1;

    while (bufferSize != 0) {
        bufferSize = read(fileDescriptor, buffer, BUF_SIZE);
        if (bufferSize == FILE_READ_ERROR_VALUE) {
            perror("Error on reading text from file");
            return FILE_READ_ERROR;
        }
        for (int i = 0; i < bufferSize; ++i) {
            linesLen[lineIdx]++;
            currentOffset++;
            if (buffer[i] == LINE_END_SYMBOL) {
                linesOffsets[lineIdx] = currentOffset - linesLen[lineIdx];
                lineIdx++;
            }
        }
    }
    *linesTotal = lineIdx;

    return SUCCESS_STATUS;
}

int convertStrToLineIdx(char *str, int strSize, long long *lineIdx, int linesTotal) {
    if (strnlen(str, strSize) == 0) {
        fprintf(stderr, "Invalid number: string can't be empty\n");
        return INVALID_VALUE_ERROR;
    }

    errno = 0;
    char *endPtr = NULL;
    *lineIdx = strtoll(str, &endPtr, DECIMAL_SYSTEM);

    if (strnlen(endPtr, strSize) != 0) {
        fprintf(stderr, "Invalid number: string need to contain only digits\n");
        return INVALID_VALUE_ERROR;
    }
    if (errno == ERANGE && (*lineIdx == LONG_MAX || *lineIdx == LONG_MIN)) {
        perror("Invalid number");
        return INVALID_VALUE_ERROR;
    }
    if (*lineIdx > linesTotal || *lineIdx < 0) {
        fprintf(stderr, "Invalid number: no such line in file\n");
        return INVALID_VALUE_ERROR;
    }
    return SUCCESS_STATUS;
}

int getLineIdx(long long *lineIdx, int linesTotal) {
    char askForNumberText[27] = "Please, enter line number: ";
    char *inputValue = (char*) malloc(sizeof(char) * BUF_SIZE);
    memset(inputValue, TERMINAL_ZERO, sizeof(char) * BUF_SIZE);
    int inputIdx = 0;

    int writeRes = write(STDOUT_FILENO, askForNumberText, 27);
    if (writeRes == STDOUT_WRITE_ERROR_VALUE) {
        perror("Error on printing message for user");
        return STDOUT_WRITE_ERROR;
    }
    while (true) {
        int readRes = read(STDIN_FILENO, &inputValue[inputIdx], BUF_SIZE);
        if (readRes == FILE_READ_ERROR_VALUE) {
            perror("Error on reading line number");
            return FILE_READ_ERROR;
        }
        inputIdx = (int) strnlen(inputValue, inputIdx + BUF_SIZE);
        if (inputValue[inputIdx - 1] == LINE_END_SYMBOL) {
            inputValue[inputIdx - 1] = TERMINAL_ZERO;
            break;
        }
        char *tmp = (char*) realloc(inputValue, sizeof(char) * (inputIdx + BUF_SIZE));
        if (tmp == NULL) {
            perror("Error on realloc for input value");
            return MEMORY_REALLOCATION_ERROR;
        }
        inputValue = tmp;
    }

    int convertRes = convertStrToLineIdx(inputValue, inputIdx, lineIdx, linesTotal);
    free(inputValue);

    return convertRes;
}

int readLineFromFile(int fileDescriptor, char *line, int currOffset, int currLen) {
    int lseekRes = lseek(fileDescriptor, currOffset, SEEK_SET);
    if (lseekRes == LSEEK_ERROR_VALUE) {
        perror("Error on lseek");
        return LSEEK_ERROR;
    }
    int readRes = read(fileDescriptor, line, currLen);
    if (readRes == FILE_READ_ERROR_VALUE) {
        perror("Error on reading line from file");
        return FILE_READ_ERROR;
    }
    return SUCCESS_STATUS;
}

int printLine(char* line, int currLen) {
    int writeRes = write(STDOUT_FILENO, line, currLen);
    if (writeRes == STDOUT_WRITE_ERROR_VALUE) {
        perror("Error on printing line");
        return STDOUT_WRITE_ERROR;
    }
    return SUCCESS_STATUS;
}

int getLines(int fileDescriptor, int *linesOffsets, int *linesLen, int linesTotal) {

    long long lineIdx;

    while (true) {
        int getLineIdxRes = getLineIdx(&lineIdx, linesTotal);
        if (getLineIdxRes != SUCCESS_STATUS && getLineIdxRes != INVALID_VALUE_ERROR) {
            return getLineIdxRes;
        }
        if (getLineIdxRes == INVALID_VALUE_ERROR) {
            continue;
        }
        if (lineIdx == STOP_IDX) {
            break;
        }
        if (linesLen[lineIdx] == 0) {
            continue;
        }
        int currOffset = linesOffsets[lineIdx];
        int currLen = linesLen[lineIdx];
        char line[currLen];

        int readLineRes = readLineFromFile(fileDescriptor, line, currOffset, currLen);
        if (readLineRes != SUCCESS_STATUS) {
            return readLineRes;
        }
        int printLineRes = printLine(line, currLen);
        if (printLineRes != SUCCESS_STATUS) {
            return printLineRes;
        }
    }
    return SUCCESS_STATUS;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Wrong arguments number\n");
        return WRONG_ARGUMENTS_NUMBER_ERROR;
    }

    int linesOffsets[TABLE_SIZE]  = {0};
    int linesLen[TABLE_SIZE]  = {0};
    int linesTotal;

    int fileDescriptor = open(argv[1], O_RDONLY);
    if (fileDescriptor == FILE_OPEN_ERROR_VALUE) {
        perror("Error on file opening");
        return FILE_OPEN_ERROR;
    }

    int fillTableRes = fillTable(fileDescriptor, linesOffsets, linesLen, &linesTotal);
    if (fillTableRes != SUCCESS_STATUS) {
        return fillTableRes;
    }

    int getLinesRes = getLines(fileDescriptor, linesOffsets, linesLen, linesTotal);
    if (getLinesRes != SUCCESS_STATUS) {
        return getLinesRes;
    }

    int closeFileRes = close(fileDescriptor);
    if (closeFileRes == FILE_CLOSE_ERROR_VALUE) {
        perror("Error with closing the file");
        return FILE_CLOSE_ERROR;
    }

    return SUCCESS_STATUS;
}
