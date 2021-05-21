#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#define SUCCESS_STATUS (0)
#define WRONG_ARGUMENTS_NUMBER_ERROR (1)
#define FILE_OPEN_ERROR (2)
#define FILE_READ_ERROR (3)
#define FILE_CLOSE_ERROR (4)
#define LSEEK_ERROR (5)
#define STDOUT_WRITE_ERROR (6)
#define INVALID_VALUE_ERROR (7)

#define FILE_OPEN_ERROR_VALUE (-1)
#define FILE_READ_ERROR_VALUE (-1)
#define FILE_CLOSE_ERROR_VALUE (-1)
#define LSEEK_ERROR_VALUE (-1)
#define STDOUT_WRITE_ERROR_VALUE (-1)

#define LINE_END_SYMBOL ('\n')
#define DECIMAL_SYSTEM (10)
#define INPUT_SIZE (100)
#define BUF_SIZE (100)
#define TABLE_SIZE (256)
#define STOP_LINE_IDX (0)


int fillTable(int fileDescriptor, int *linesOffsets, int *linesLen, int *linesNumber) {
    char buffer[BUF_SIZE];
    int bufferSize =  BUF_SIZE;

    int currentOffset = 0;
    int offsetIdx = 0;
    int lineIdx = 0;

    while (bufferSize > 0) {
        bufferSize = read(fileDescriptor, buffer, BUF_SIZE);
        if (bufferSize == FILE_READ_ERROR_VALUE) {
            perror("Error on reading text from file");
            return FILE_READ_ERROR;
        }
        for (int i = 0; i < bufferSize; ++i) {
            currentOffset++;
            linesLen[lineIdx]++;
            if (buffer[i] == LINE_END_SYMBOL) {
                linesOffsets[offsetIdx] = currentOffset - linesLen[lineIdx];
                offsetIdx++;
                lineIdx++;
            }
        }
    }
    *linesNumber = lineIdx;

    return SUCCESS_STATUS;
}

int getLineIdx(long long int *lineIdx, int linesNumber) {
    char prependText[27] = "Please, enter line number: ";
    char inputValue[INPUT_SIZE];

    int writeInFileRes = write(STDOUT_FILENO, prependText, 27);
    if (writeInFileRes == STDOUT_WRITE_ERROR_VALUE) {
        perror("Error on printing message for user");
        return STDOUT_WRITE_ERROR;
    }

    int readFromFileRes = read(STDIN_FILENO, inputValue, INPUT_SIZE);
    if (readFromFileRes == FILE_READ_ERROR_VALUE) {
        perror("Error on reading line");
        return FILE_READ_ERROR;
    }

    errno = 0;
    char *endPtr = NULL;
    *lineIdx = strtoll(inputValue, &endPtr, DECIMAL_SYSTEM);

    if (inputValue == endPtr) {
        printf("Invalid number: string can't contain characters\n");
        return INVALID_VALUE_ERROR;
    }
    if (errno == ERANGE && (*lineIdx == LONG_MAX || *lineIdx == LONG_MIN)) {
        perror("Invalid number");
        return INVALID_VALUE_ERROR;
    }
    if (*lineIdx > linesNumber || lineIdx < 0) {
        printf("Invalid number: no such line in file\n");
        return INVALID_VALUE_ERROR;
    }

    return SUCCESS_STATUS;
}

int readLineFromFile(int fileDescriptor, char *line, int currOffset, int currLen) {
    int lseekRes = lseek(fileDescriptor, currOffset, SEEK_SET);
    if (lseekRes == LSEEK_ERROR_VALUE) {
        perror("Error on lseek");
        return LSEEK_ERROR;
    }
    int readFromFileRes = read(fileDescriptor, line, currLen);
    if (readFromFileRes == FILE_READ_ERROR_VALUE) {
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

int getLines(int fileDescriptor, int *linesOffsets, int *linesLen, int linesNumber) {
    int currOffset, currLen;
    long long int lineIdx;

    while (true) {
        int getLineIdxRes = getLineIdx(&lineIdx, linesNumber);
        if (getLineIdxRes == INVALID_VALUE_ERROR) {
            continue;
        }
        if (lineIdx == STOP_LINE_IDX) {
            break;
        }
        if (linesLen[lineIdx] != 0) {
            currOffset = linesOffsets[lineIdx];
            currLen = linesLen[lineIdx];
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
    }

    return SUCCESS_STATUS;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Wrong arguments number");
        return WRONG_ARGUMENTS_NUMBER_ERROR;
    }

    int linesOffsets[TABLE_SIZE]  = {0};
    int linesLen[TABLE_SIZE]  = {0};
    int linesNumber;

    int fileDescriptor = open(argv[1], O_RDONLY);
    if (fileDescriptor == FILE_OPEN_ERROR_VALUE) {
        perror("Error on file opening");
        return FILE_OPEN_ERROR;
    }

    int fillTableRes = fillTable(fileDescriptor, linesOffsets, linesLen, &linesNumber);
    if (fillTableRes != SUCCESS_STATUS) {
        return fillTableRes;
    }

    int getLinesRes = getLines(fileDescriptor, linesOffsets, linesLen, linesNumber);
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
