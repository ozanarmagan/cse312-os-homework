 
#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

namespace myos
{
    constexpr common::uint32_t SYSCALL_PRINTF = 4;
    constexpr common::uint32_t SYSCALL_FORK = 5;
    constexpr common::uint32_t SYSCALL_GETPID = 6;
    constexpr common::uint32_t SYSCALL_EXECVE = 7;
    constexpr common::uint32_t SYSCALL_EXIT = 8;
    constexpr common::uint32_t SYSCALL_WAIT = 9;
    constexpr common::uint32_t SYSCALL_GETPPID = 10;
    constexpr common::uint32_t SYSCALL_GETTICKS = 11;
    constexpr common::uint32_t SYSCALL_FORK2 = 12;
    constexpr common::uint32_t SYSCALL_PRINTDEC32 = 13;
    constexpr common::uint32_t SYSCALL_USE_DYNAMIC_PRIORITY = 14;
    constexpr common::uint32_t SYSCALL_GET_KB_INPUT = 15;
    constexpr common::uint32_t SYSCALL_GET_MOUSE_INPUT = 16;

    class SyscallHandler : public hardwarecommunication::InterruptHandler
    {
        TaskManager* taskManager;
    public:
        SyscallHandler(hardwarecommunication::InterruptManager* interruptManager, myos::common::uint8_t InterruptNumber, TaskManager* taskManager);
        ~SyscallHandler();
        
        virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);

    };
    
    
}


#endif