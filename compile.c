#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int arraySize;

int *buildCaseValuesArr() {
    FILE *readFile;
    if ((readFile = fopen("switch.c", "r")) == NULL) {
        printf("Error opening switch.c");
        exit(1);
    }

    char buf[100];
    int i, next = 0;
    arraySize = 0;
    int *caseValuesArr = (int *) malloc(sizeof(int));
    while (fgets(buf, 100, readFile) != NULL) {
        char *token = strtok(buf, " : ;");
        while (token != NULL) {
            //takes case "number"
            if (next) {
                caseValuesArr = realloc(caseValuesArr, ++arraySize * sizeof(int));
                caseValuesArr[arraySize - 1] = atoi(token);
                next = 0;
            }

            if (strcmp(token, "case") == 0) {
                next = 1;
            }
            token = strtok(NULL, " :");
        }
    }

    fclose(readFile);
    return caseValuesArr;
}

int isInCaseValuesArr(int *CaseValuesArr, int num) {
    int i;
    for (i = 0; i < arraySize; i++) {
        if (CaseValuesArr[i] == num) {
            return 1;
        }
    }
    return 0;
}

void removeChar(char *s, int c) {

    int i, j = 0, n = strlen(s);
    for (i = 0; i < n; i++)
        if (s[i] != c)
            s[j++] = s[i];

    s[j] = '\0';
}

void prepend(char *s, const char *t) {
    size_t len = strlen(t);
    memmove(s + len, s, strlen(s) + 1);
    memcpy(s, t, len);
}

int setResult() {
    FILE *readFile;
    if ((readFile = fopen("switch.c", "r")) == NULL) {
        printf("Error opening switch.c");
        exit(1);
    }
    char buf[100];
    while (fgets(buf, 100, readFile) != NULL) {
        if (strchr(buf, '=') != NULL && strstr(buf, "long") != NULL) {
            int next = 0;
            char *token = strtok(buf, " : ;");
            while (token != NULL) {
                //takes case "number"
                if (next) {
                    removeChar(token, ';');
                    removeChar(token, '\n');
                    return atoi(token);
                }
                if (strcmp(token, "=") == 0) {
                    next = 1;
                }
                token = strtok(NULL, " :");
            }
        }
    }
    return 0;
}


int main() {

    FILE *writeFile;
    if ((writeFile = fopen("switch.s", "w")) == NULL) {
        printf("Error opening file");
        exit(1);
    }
    fprintf(writeFile, ".section .text\n");
    fprintf(writeFile, ".globl switch2\n");
    fprintf(writeFile, "switch2:\n");

    int *caseValuesArr = buildCaseValuesArr();
    int minCaseValue = 10000;
    int maxCaseValue = -10000;

    //find min and max values in switch
    int i;
    for (i = 0; i < arraySize; i++) {
        if (caseValuesArr[i] < minCaseValue) {
            minCaseValue = caseValuesArr[i];
        }
        if (caseValuesArr[i] > maxCaseValue) {
            maxCaseValue = caseValuesArr[i];
        }
    }

    int result = setResult();
    fprintf(writeFile, "movq $%d, %%rbx\n", result);
    //subtract min value in switch from "action" in rdx;
    fprintf(writeFile, "subq $%d, %%rdx\n", minCaseValue);
    //check if value is greater then maxCaseValue
    fprintf(writeFile, "cmpq $%d, %%rdx\n", maxCaseValue - minCaseValue);
    //if yes, jump to default case
    fprintf(writeFile, "ja .Ldef\n");
    //if not, jump to "jump table"
    fprintf(writeFile, "jmp *.JumpTable(,%%rdx,8)\n");

    //Array for all values between min and max
    int tempMinValue = minCaseValue, rangeArrSize = (maxCaseValue - minCaseValue) + 1;
    int rangeArr[rangeArrSize];
    for (i = 0; i < (maxCaseValue - minCaseValue) + 1; i++) {
        if (isInCaseValuesArr(caseValuesArr, tempMinValue)) {
            rangeArr[i] = i;
        } else {
            rangeArr[i] = -1;
        }
        tempMinValue++;
    }

    free(caseValuesArr);

    FILE *readFile;
    if ((readFile = fopen("switch.c", "r")) == NULL) {
        printf("Error opening switch.c");
        exit(1);
    }

    char buf[100];
    int foundCase = 0, twoPointersFlag;
    while (fgets(buf, 100, readFile) != NULL) {
        // operation statement
        if (strchr(buf, '=') != NULL && strstr(buf, "long") == NULL) {
            char *left, *right;
            char oper = '!';
            char *token = strtok(buf, " ;");
            while (token != NULL) {
                if (strstr(token, "*p1") != NULL) {
                    if (oper == '!') {
                        left = "(%rdi)";
                    } else {
                        right = "(%rdi)";
                    }
                } else if (strstr(token, "*p2") != NULL) {
                    if (oper == '!') {
                        left = "(%rsi)";
                    } else {
                        right = "(%rsi)";
                    }
                } else if (strstr(token, "result") != NULL) {
                    if (oper == '!') {
                        left = "%rbx";
                    } else {
                        right = "%rbx";
                    }
                } else if (strcmp(token, "=") == 0) {
                    oper = '=';
                } else if (strstr(token, "+=") != NULL) {
                    oper = '+';
                } else if (strstr(token, "-=") != NULL) {
                    oper = '-';
                } else if (strstr(token, "*=") != NULL) {
                    oper = '*';
                } else if (strstr(token, "<<=") != NULL) {
                    oper = '<';
                } else if (strstr(token, ">>=") != NULL) {
                    oper = '>';
                    //long result = x
                } else {
                    removeChar(token, ';');
                    removeChar(token, '\n');
                    prepend(token, "$");
                    if (oper == '!') {
                        left = token;
                    } else {
                        right = token;
                    }
                }

                token = strtok(NULL, " :");
            }
            twoPointersFlag = 0;
            if ((strchr(left, '(') != NULL) && (strchr(right, '(') != NULL)) {
                twoPointersFlag = 1;
            }

            switch (oper) {
                case '=':
                    if (twoPointersFlag) {
                        fprintf(writeFile, "movq %s, %%rcx \n", right);
                        fprintf(writeFile, "movq %%rcx, %s\n", left);
                    } else {
                        fprintf(writeFile, "movq %s, %s\n", right, left);
                    }
                    break;
                case '+':
                    if (twoPointersFlag) {
                        fprintf(writeFile, "movq %s, %%rcx \n", right);
                        fprintf(writeFile, "addq %%rcx, %s\n", left);
                    } else {
                        fprintf(writeFile, "addq %s, %s\n", right, left);
                    }
                    break;
                case '-':
                    if (twoPointersFlag) {
                        fprintf(writeFile, "movq %s, %%rcx \n", right);
                        fprintf(writeFile, "subq %%rcx, %s\n", left);
                    } else {
                        fprintf(writeFile, "subq %s, %s\n", right, left);
                    }
                    break;
                case '*':
                    if (twoPointersFlag) {
                        fprintf(writeFile, "movq %s, %%rcx \n", right);
                        fprintf(writeFile, "imulq %%rcx, %s\n", left);
                    } else {
                        fprintf(writeFile, "imulq %s, %s\n", right, left);
                    }
                    break;
                case '<':
                    if (strchr(right, '%')) {
                        fprintf(writeFile, "movq %s, %%rcx \n", right);
                        fprintf(writeFile, "shl %%cl, %s\n", left);
                    } else {

                        fprintf(writeFile, "shl %s, %s\n", right, left);
                    }
                    break;
                case '>':
                    if (strchr(right, '%')) {
                        fprintf(writeFile, "movq %s, %%rcx \n", right);
                        fprintf(writeFile, "shr %%cl, %s\n", left);
                    } else {
                        fprintf(writeFile, "shrq %s, %s\n", right, left);
                    }
                    break;
            }
            //case row
        } else if (strstr(buf, "case") != NULL) {
            char *token = strtok(buf, " : ;");
            while (token != NULL) {
                if (strcmp(token, "case") == 0) {
                    token = strtok(NULL, " :");
                    fprintf(writeFile, ".L%d:\n", rangeArr[atoi(token) - minCaseValue]);
                }
                token = strtok(NULL, " :");
            }
            //Break row
        } else if (strstr(buf, "break") != NULL) {
            fprintf(writeFile, "jmp .Done\n");
            //Default row
        } else if (strstr(buf, "default") != NULL) {
            fprintf(writeFile, ".Ldef:\n");
            //long result = x
        }
    }
    fclose(readFile);
    fprintf(writeFile, ".Done:\n");
    fprintf(writeFile, "movq %%rbx, %%rax\n");
    fprintf(writeFile, "ret\n");
    fprintf(writeFile, ".section .rodata\n");
    fprintf(writeFile, ".align 8\n");
    fprintf(writeFile, ".JumpTable:\n");
    for (i = 0; i < rangeArrSize; i++) {
        fprintf(writeFile, ".quad .L");
        if (rangeArr[i] == -1) {
            fprintf(writeFile, "def\n");
        } else {
            fprintf(writeFile, "%d\n", i);
        }

    }
    fclose(writeFile);
    return 0;
}