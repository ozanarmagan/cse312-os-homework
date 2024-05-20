#ifndef __MYOS__PROGRAM_H
#define __MYOS__PROGRAM_H


#include <common/types.h>
#include <syscallwrapper.h>

using namespace myos::common;

extern void printf(char* str);

// Collatz conjecture program
void collatz(uint32_t start);

// Long running task program
void long_running_task(uint32_t n);

// Binary search program
void binary_search(uint32_t* arr, uint32_t n, uint32_t key);

// Linear search program
void linear_search(uint32_t* arr, uint32_t n, uint32_t key);


// Helper function to compare two strings
bool strcmp(char* str1, char* str2);

// Helper function to find program by name
void* findProgram(char* str);


// Shell implementation
// The shell will take input from the user and execute the program
// if the program is found in the list of programs
// It will fork a new process and execute the program
void shell();

#endif