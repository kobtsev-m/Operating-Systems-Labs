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
#define SELECT_ERROR (9)

#define FILE_OPEN_ERROR_VALUE (-1)
#define FILE_READ_ERROR_VALUE (-1)
#define FILE_CLOSE_ERROR_VALUE (-1)
#define LSEEK_ERROR_VALUE (-1)
#define STDOUT_WRITE_ERROR_VALUE (-1)
#define SELECT_ERROR_VALUE (-1)
#define SELECT_NO_REACTION_VALUE (0)

#define REQUIRED_ARGS_NUMBER (2)
#define LINE_END_SYMBOL ('\n')
#define TERMINAL_ZERO ('\0')
#define DECIMAL_SYSTEM (10)
#define INIT_TABLE_SIZE (100)
#define BUF_SIZE (100)
#define STOP_IDX (0)
#define TIMEOUT_SEC (5)
#define TIMEOUT_USEC (0)
#define MAX_FD (0)

typedef struct TableRow {
    int length;
    int offset;
} TableRow;

int fillTable(int fileDescriptor, TableRow **linesTable, int *linesTotal) {
    char buffer[BUF_SIZE];
    int bufferSize = BUF_SIZE;
    int lineIdx = 1;
    int linesLimit = *linesTotal;
    int currLen = 0;
    int totalOffset = 0;

    while (bufferSize != 0) {
        bufferSize = read(fileDescriptor, buffer, BUF_SIZE);
        if (bufferSize == FILE_READ_ERROR_VALUE) {
            perror("Error on reading text from file");
            return FILE_READ_ERROR;
        }
        for (int i = 0; i < bufferSize; ++i) {
            currLen++;
            totalOffset++;
            if (buffer[i] == LINE_END_SYMBOL) {
                (*linesTable)[lineIdx].length = currLen;
                (*linesTable)[lineIdx].offset = totalOffset - currLen;
                lineIdx++;
                currLen = 0;
            }
            if (lineIdx == linesLimit) {
                linesLimit *= 2;
                TableRow *tmp = (TableRow*) realloc(*linesTable, sizeof(TableRow) * linesLimit);
                if (tmp == NULL) {
                    perror("Error on lines table realloc");
                    return MEMORY_REALLOCATION_ERROR;
                }
                *linesTable = tmp;
            }
        }
    }
    *linesTotal = lineIdx - 1;
    return SUCCESS_STATUS;
}

int convertStrToLineIdx(char *str, int strSize, long long *lineIdx, int linesTotal) {
    if (strnlen(str, strSize) == 0) {
        fprintf(stderr, "Invalid line number: input value is empty\n");
        return INVALID_VALUE_ERROR;
    }

    errno = 0;
    char *endPtr = NULL;
    *lineIdx = strtoll(str, &endPtr, DECIMAL_SYSTEM);

    if (strnlen(endPtr, strSize) != 0) {
        fprintf(stderr, "Invalid line number: input value is not a number\n");
        return INVALID_VALUE_ERROR;
    }
    if (errno == ERANGE && (*lineIdx == LONG_MAX || *lineIdx == LONG_MIN)) {
        perror("Invalid line number");
        return INVALID_VALUE_ERROR;
    }
    if (*lineIdx > linesTotal || *lineIdx < 0) {
        fprintf(stderr, "Invalid line number: no such line in file\n");
        return INVALID_VALUE_ERROR;
    }
    return SUCCESS_STATUS;
}

int getLineIdx(long long *lineIdx, int linesTotal) {
    char *inputValue = (char*) malloc(sizeof(char) * BUF_SIZE);
    int inputIdx = 0;

    while (true) {
        int readRes = read(STDIN_FILENO, &inputValue[inputIdx], BUF_SIZE);
        if (readRes == FILE_READ_ERROR_VALUE) {
            perror("Error on reading line number");
            free(inputValue);
            return FILE_READ_ERROR;
        }
        inputIdx += readRes;
        if (inputValue[inputIdx - 1] == LINE_END_SYMBOL) {
            inputValue[inputIdx - 1] = TERMINAL_ZERO;
            break;
        }
        char *tmp = (char*) realloc(inputValue, sizeof(char) * (inputIdx + BUF_SIZE));
        if (tmp == NULL) {
            perror("Error on realloc for input value");
            free(inputValue);
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
        perror("Error on lssek while printing sigle line");
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

int getCurrLine(int fileDescriptor, int currOffset, int currLen) {
    char line[currLen];
    int readLineRes = readLineFromFile(fileDescriptor, line, currOffset, currLen);
    if (readLineRes != SUCCESS_STATUS) {
        return readLineRes;
    }
    int printLineRes = printLine(line, currLen);
    if (printLineRes != SUCCESS_STATUS) {
        return printLineRes;
    }
    return SUCCESS_STATUS;
}

int printEntireFile(int fileDescriptor, TableRow *linesTable, int linesTotal) {
    for (int i = 1; i < linesTotal + 1; ++i) {
        int getCurrLineRes = getCurrLine(
            fileDescriptor,
            linesTable[i].offset,
            linesTable[i].length
        );
        if (getCurrLineRes != SUCCESS_STATUS) {
            return getCurrLineRes;
        }
    }
    return EXIT_SUCCESS;
}

int waitForInputValue(bool *isInputValue) {
    char timeoutInfoText[35] = "Five seconds to enter line number: ";
    char timeoutText[26] = "\nTime is over. Your file:\n";
    fd_set readDescriptors;
    struct timeval timeout;

    int writeRes = write(STDOUT_FILENO, timeoutInfoText, 35);
    if (writeRes == STDOUT_WRITE_ERROR_VALUE) {
        perror("Error on showing timeout info text");
        return STDOUT_WRITE_ERROR;
    }

    FD_ZERO(&readDescriptors);
    FD_SET(STDIN_FILENO, &readDescriptors);

    do {
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = TIMEOUT_USEC;

        int selectRes = select(MAX_FD + 1, &readDescriptors, NULL, NULL, &timeout);
        if (selectRes == SELECT_ERROR_VALUE) {
            perror("Error on select");
            return SELECT_ERROR;
        }

        if (selectRes == SELECT_NO_REACTION_VALUE) {
            writeRes = write(STDOUT_FILENO, timeoutText, 26);
            if (writeRes == STDOUT_WRITE_ERROR_VALUE) {
                perror("Error on printing timeout message");
                return STDOUT_WRITE_ERROR;
            }
            *isInputValue = false;
            return SUCCESS_STATUS;
        }
    } while (!FD_ISSET(STDIN_FILENO, &readDescriptors));

    *isInputValue = true;
    return SUCCESS_STATUS;
}

int getLines(int fileDescriptor, TableRow *linesTable, int linesTotal) {

    long long lineIdx;
    bool isInputValue;

    while (true) {
        int waitingInputRes = waitForInputValue(&isInputValue);
        if (waitingInputRes != SUCCESS_STATUS) {
            return waitingInputRes;
        }
        if (!isInputValue) {
            int printFileRes = printEntireFile(fileDescriptor, linesTable, linesTotal);
            if (printFileRes != SUCCESS_STATUS) {
                return printFileRes;
            }
            return SUCCESS_STATUS;
        }
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
        int getCurrLineRes = getCurrLine(
            fileDescriptor,
            linesTable[lineIdx].offset,
            linesTable[lineIdx].length
        );
        if (getCurrLineRes != SUCCESS_STATUS) {
            return getCurrLineRes;
        }
    }
    return SUCCESS_STATUS;
}

int main(int argc, char *argv[]) {

    if (argc != REQUIRED_ARGS_NUMBER) {
        fprintf(stderr, "Wrong arguments number\n");
        return WRONG_ARGUMENTS_NUMBER_ERROR;
    }

    int fileDescriptor = open(argv[1], O_RDONLY);
    if (fileDescriptor == FILE_OPEN_ERROR_VALUE) {
        perror("Error on file opening");
        return FILE_OPEN_ERROR;
    }

    TableRow *linesTable = (TableRow*) malloc(sizeof(TableRow) * INIT_TABLE_SIZE);
    int linesTotal = INIT_TABLE_SIZE;

    int fillTableRes = fillTable(fileDescriptor, &linesTable, &linesTotal);
    if (fillTableRes != SUCCESS_STATUS) {
        free(linesTable);
        int closeFileRes = close(fileDescriptor);
        if (closeFileRes == FILE_CLOSE_ERROR_VALUE) {
            perror("Error on file closing");
            return FILE_CLOSE_ERROR;
        }
        return fillTableRes;
    }

    int getLinesRes = getLines(fileDescriptor, linesTable, linesTotal);
    if (getLinesRes != SUCCESS_STATUS) {
        free(linesTable);
        int closeFileRes = close(fileDescriptor);
        if (closeFileRes == FILE_CLOSE_ERROR_VALUE) {
            perror("Error on file closing");
            return FILE_CLOSE_ERROR;
        }
        return getLinesRes;
    }

    free(linesTable);
    int closeFileRes = close(fileDescriptor);
    if (closeFileRes == FILE_CLOSE_ERROR_VALUE) {
        perror("Error on file closing");
        return FILE_CLOSE_ERROR;
    }
    return SUCCESS_STATUS;
}
