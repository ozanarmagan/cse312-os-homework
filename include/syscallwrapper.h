#ifndef __MYOS__SYSCALLWRAPPER_H
#define __MYOS__SYSCALLWRAPPER_H

#include <common/types.h>
#include <syscalls.h>

using namespace myos::common;
using namespace myos;

static void sysprintDec32(uint32_t key)
{
    // Syscall to print decimal number
    asm("int $0x80" : : "a" (SYSCALL_PRINTDEC32), "b" (key));
}

static void sysprintf(char* str)
{
    // Syscall to print string
    asm("int $0x80" : : "a" (SYSCALL_PRINTF), "b" (str));
}

static inline uint32_t getpid()
{
    // Syscall to get process id
    uint32_t pid;
    asm("int $0x80" : "=c" (pid) : "a" (SYSCALL_GETPID));
    return pid;
}

static inline uint32_t getppid()
{
    // Syscall to get parent process id
    uint32_t ppid;
    asm("int $0x80" : "=c" (ppid) : "a" (SYSCALL_GETPPID));
    return ppid;
}

static inline void dynamicPriority()
{
    // Syscall to use dynamic priority for current process
    asm("int $0x80" : : "a" (SYSCALL_USE_DYNAMIC_PRIORITY));
}


// force compiler to inline the function
// This is needed to avoid stack corruption
__attribute__((always_inline))
static inline uint32_t fork()
{
    // Syscall to fork current process
    uint32_t pid;
    asm("int $0x80" : "=c" (pid) : "a" (SYSCALL_FORK));
    return pid;
}

// force compiler to inline the function
// This is needed to avoid stack corruption
__attribute__((always_inline))
static inline uint32_t forkWithPriority(uint32_t priority)
{
    // Syscall to fork current process with priority
    uint32_t pid;
    asm("int $0x80" : "=c" (pid) : "a" (SYSCALL_FORK2), "b" (priority));
    return pid;
}

static inline void exec(void* entrypoint, uint32_t argc, uint32_t* argv)
{
    // Syscall to execute a new program in current process
    asm volatile("int $0x80" : : "a" (SYSCALL_EXECVE), "b" (entrypoint), "c" (argc), "d" (argv));
}


static inline void exit()
{
    // Syscall to exit current process
    asm("int $0x80" : : "a" (SYSCALL_EXIT));
}

// force compiler to inline the function
__attribute__((always_inline))
static inline void wait(uint32_t pid)
{
    // Syscall to wait for a process to finish
    asm("int $0x80" : : "a" (SYSCALL_WAIT), "b" (pid));
}

// force compiler to inline the function
__attribute__((always_inline))
static inline char* checkKeyPress() {
    // Syscall to check if a key is pressed
    void* keyPress;
    asm("int $0x80" : "=b" (keyPress) : "a" (SYSCALL_GET_KB_INPUT));
    return (char*)keyPress;
}

// force compiler to inline the function
__attribute__((always_inline))
static inline bool checkMouseClick() {
    // Syscall to check if mouse is clicked
    bool mouseClick;
    asm("int $0x80" : "=b" (mouseClick) : "a" (SYSCALL_GET_MOUSE_INPUT));
    return mouseClick;
}


static inline uint32_t get_interrupt_count()
{
    // Syscall to get interrupt count
    uint32_t count;
    asm("int $0x80" : "=c" (count) : "a" (SYSCALL_GETTICKS));
    return count;
}

#endif