#include <program.h>

using namespace myos::common;
using namespace myos;


// Collatz conjecture program
void collatz(uint32_t start) {
    sysprintf("Collatz conjecture\n");
    sysprintf("Starting collatz conjecture for ");
    sysprintDec32(start);
    sysprintf("\n");
    // run collatz conjecture for each number from start to 0
    while(start != 1) {
        uint32_t start_ = start;
        sysprintf("Collatz conjecture for ");
        sysprintDec32(start);
        sysprintf(": ");
        while(start_ != 1) {
            sysprintDec32(start_);
            sysprintf(", ");
            if(start_ % 2 == 0) {
                start_ = start_ / 2;
            } else {
                start_ = 3 * start_ + 1;
            }
        }
        sysprintDec32(1);
        sysprintf("\n");
        start--;
    }
    exit();
}

// Long running task program
void long_running_task(uint32_t n) {
    printf("Long running task for n: ");
    sysprintDec32(n);
    printf("\n");
    uint32_t res = 0;
    for(uint32_t i = 0; i < n; i++) {
        for(uint32_t j = 0; j < n; j++) {
            res += i * j;
        }
    }

    printf("Long running task finished, result: ");
    sysprintDec32(res);
    printf("\n");
    exit();
}

// Binary search program
void binary_search(uint32_t* arr, uint32_t n, uint32_t key) {
    printf("Binary search\n");
    // first sort the array
    while(1) {
        bool swapped = false;
        for(uint32_t i = 0; i < n - 1; i++) {
            if(arr[i] > arr[i + 1]) {
                uint32_t temp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = temp;
                swapped = true;
            }
        }
        if(!swapped) {
            break;
        }
    }

    uint32_t low = 0, high = n - 1;
    while(low <= high) {
        uint32_t mid = low + (high - low) / 2;
        if(arr[mid] == key) {
            printf("BINARY SEARCH: Key found at index ");
            sysprintDec32(mid);
            printf("\n");
            exit();
        } else if(arr[mid] < key) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    printf("BINARY SEARCH: Key not found\n");
    exit();
}

// Linear search program
void linear_search(uint32_t* arr, uint32_t n, uint32_t key) {
    printf("Linear search\n");
    for(uint32_t i = 0; i < n; i++) {
        if(arr[i] == key) {
            printf("LINEAR SEARCH: Key found at index ");
            sysprintDec32(i);
            printf("\n");
            exit();
        }
    }
    printf("LINEAR SEARCH: Key not found\n");
    exit();
}


// Helper function to compare two strings
bool strcmp(char* str1, char* str2) {
    while(*str1 != '\0' && *str2 != '\0') {
        if(*str1 != *str2) {
            return false;
        }
        str1++;
        str2++;
    }
    if(*str1 == '\0' && *str2 == '\0') {
        return true;
    }
    return false;
}

// Helper function to find program by name
void* findProgram(char* str)  {
    if(strcmp(str, "collatz")) {
        return (void*)collatz;
    } else if(strcmp(str, "long-running-task")) {
        return (void*)long_running_task;
    } else if(strcmp(str, "binary-search")) {
        return (void*)binary_search;
    } else if(strcmp(str, "linear-search")) {
        return (void*)linear_search;
    }
    return nullptr;
}


// Shell implementation
// The shell will take input from the user and execute the program
// if the program is found in the list of programs
// It will fork a new process and execute the program
void shell() {
    // buffer to store input
    char buffer[1024];
    int count = 0;
    printf("Welcome to MyOS shell\n");
    printf(">");
    // infinite loop to take input
    while(1) {
        // check if a key is pressed
        char* input = checkKeyPress();
        if(input != nullptr) {
            // print the input
            printf(input);
            for(int i = 0;i < 1024; i++) {
                // check for end of the input
                if(input[i] == '\0') {
                    break;
                }
                // check for backspace
                if(input[i] == '\b') {
                    if(count > 0) {
                        buffer[--count] = '\0';
                    }
                    continue;
                }
                // write to buffer
                buffer[count++] = input[i];

                // if enter is pressed
                if(input[i] == '\n') {
                    buffer[count - 1] = '\0';
                    // find the program
                    auto program = findProgram(buffer);
                    buffer[0] = '\0';
                    count = 0;

                    // if program is found
                    if(program != nullptr) {
                        auto pid = fork();
                        if(pid == 0) {
                            if(program == (void*)collatz) {
                                uint32_t argc = 1;
                                uint32_t argv[1] = {100};
                                exec(program, argc, argv);
                            } else if(program == (void*)long_running_task) {
                                uint32_t argc = 1;
                                uint32_t argv[1] = {10000};
                                exec(program, argc, argv);
                            } else {
                                uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
                                uint32_t argc = 3;
                                uint32_t argv[3] = {(uint32_t)arr, 10, 110};
                                exec(program, argc, argv);
                            }
                        } 
                        // wait for the child process to finish
                        wait(pid);
                    } else {
                        // if program is not found
                        printf("Program not found\n");
                    }
                    // print the prompt
                    printf(">");
                }
            }
        }
    }

}
