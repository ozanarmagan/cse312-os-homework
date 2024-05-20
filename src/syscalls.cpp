
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
void printDec32(uint32_t key);

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
            return (uint32_t)taskManager->Schedule((CPUState*)cpu);
            break;
        case 6:
            cpu->ecx = taskManager->GetCurrentTask()->getPID();
            break;
        case 7:
            return taskManager->ExecveTask((void*)cpu->ebx, cpu->ecx, (uint32_t*)cpu->edx, (CPUState*)cpu);
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
            cpu->ecx = taskManager->GetTimerInterruptCounter();
            break;
        case 12: 
            cpu->ecx = taskManager->ForkTask(cpu, cpu->ebx);
            esp = (uint32_t)taskManager->Schedule((CPUState*)cpu);
            break;
        case 13:
            printDec32(cpu->ebx);
            break;
        case 14: 
            taskManager->setTaskWithDynamicPriority(taskManager->GetCurrentTask()->getPID());
            break;
        case 15:
            cpu->ebx = (uint32_t)taskManager->readKeyPresses();
            break;
        case 16:
            cpu->ebx = (uint32_t)taskManager->isMouseClicked();
            break;
        default:
            break;
    }

    
    return esp;
}

