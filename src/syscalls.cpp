
#include <syscalls.h>
 
using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;
 
SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber, TaskManager* taskManager) :    
InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset()), taskManager(taskManager)
{
}

SyscallHandler::~SyscallHandler()
{
}


void printf(char*);
void printfHex32(uint32_t key);

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;
    
    switch(cpu->eax)
    {
        case 4:
            printf((char*)cpu->ebx);
            break;
        case 5:
            cpu->ecx = taskManager->ForkTask(cpu);
            esp = (uint32_t)taskManager->Schedule((CPUState*)cpu);
            break;
        case 6:
            cpu->ecx = taskManager->GetCurrentTask()->getPID();
            break;
        case 7:
            return taskManager->ExecveTask((void(*)())cpu->ebx);
            break;
        case 8:
            taskManager->ExitTask();
            esp = (uint32_t)taskManager->Schedule((CPUState*)esp);
            break;
        case 9:
            cpu->ecx = taskManager->WaitTask(cpu->ebx);
            esp = (uint32_t)taskManager->Schedule((CPUState*)esp);
            break;
        case 10:
            cpu->ecx = taskManager->GetCurrentTask()->getPPID();
            break;  
        case 11:
            return taskManager->ExecveTask((void(*)(uint32_t))cpu->ebx, cpu->ecx, (CPUState*)cpu);
            break;
        default:
            break;
    }

    
    return esp;
}

