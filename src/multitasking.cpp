
#include <multitasking.h>

using namespace myos;
using namespace myos::common;

extern void printf(char* str);
extern void printfHex(uint8_t key);
extern void printfHex32(uint32_t key);
extern void printfHex16(uint16_t key);

extern "C" void* _getCPUState();

GlobalDescriptorTable* Task::gdt = nullptr;

Task::Task(void entrypoint())
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    /*
    cpustate -> gs = 0;
    cpustate -> fs = 0;
    cpustate -> es = 0;
    cpustate -> ds = 0;
    */
    
    // cpustate -> error = 0;    
    //cpustate -> esp = ((uint32_t)cpustate) - 8;
    printf("entrypoint: ");
    printfHex32((uint32_t)entrypoint);
    printf("\n");
    cpustate -> eip = (uint32_t)entrypoint;
    cpustate -> cs = gdt->CodeSegmentSelector();
    // cpustate -> ss = ;
    cpustate -> eflags = 0x202;

    pid = pidCounter++;
    
}


Task::Task()
{
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    state = TaskState::EMPTY;
}



Task Task::operator=(const Task& task) {
    for(int i = 0; i < 4096; ++i)
        stack[i] = task.stack[i];
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    pid = task.pid;
    state = TaskState::READY;
    return *this;
}

Task::~Task()
{
}

        
TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = -1;
}

TaskManager::~TaskManager()
{
}

bool TaskManager::AddTask(const Task& task)
{
    if(numTasks >= 256)
        return false;
    tasks[numTasks++] = task;
    return true;
}


// Pick the next task to run with round-robin scheduling
CPUState* TaskManager::PickNextTask()
{  
    // printf("Picking next task\n");
    // printf("Current task is: ");
    // printfHex32(tasks[currentTask].pid);
    // printf("\nState: ");
    //        switch(tasks[currentTask].state)
    //     {
    //         case TaskState::READY:
    //             printf("READY");
    //             break;
    //         case TaskState::RUNNING:
    //             printf("RUNNING");
    //             break;
    //         case TaskState::ZOMBIE:
    //             printf("ZOMBIE");
    //             break;
    //         case TaskState::BLOCKED:
    //             printf("BLOCKED");
    //             break;
    //     }
    // printf("\n");
    // printf("Current task index: ");
    // printfHex32(currentTask);
    // printf("\n");


    while(tasks[currentTask].state != TaskState::READY)
    {
        if(tasks[currentTask].state == TaskState::BLOCKED) {
            // check if the wait is over
            auto blocking_pid = tasks[currentTask].blocking_pid;
            bool block_finished = false;
            for(int i = 0; i < numTasks; ++i)
            {
                if(tasks[i].pid == blocking_pid && tasks[i].state == TaskState::ZOMBIE)
                {
                    tasks[currentTask].state = TaskState::READY;
                    tasks[currentTask].blocking_pid = 0;
                    block_finished = true;
                    break;
                }
            }
            if(block_finished) {
                continue;
            }
        }
        ++currentTask;
        currentTask %= numTasks;
    }
    tasks[currentTask].state = TaskState::RUNNING;
    static int i = 0;
    // if(i++ <7)
    // {
    //    printf("Current task: ");
    //    printfHex32((uint32_t)tasks[currentTask].cpustate->eip);
    //    printf(" PID: ");
    //      printfHex32(tasks[currentTask].pid);
    //     printf(" EAX: ");
    //     printfHex32(tasks[currentTask].cpustate->eax);
    //     printf(" EBX: ");
    //     printfHex32(tasks[currentTask].cpustate->ebx);
    //     printf(" ECX: ");
    //     printfHex32(tasks[currentTask].cpustate->ecx);
    //     printf("\n");
    // }


    // printf("Next EIP: ");
    // printfHex32(tasks[currentTask].cpustate->eip);
    // printf("\n");

    return tasks[currentTask].cpustate;
}

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    //printf("Scheduling\n");
    //printTasks();
    if(numTasks <= 0)
        return cpustate;

    if(currentTask >= 0 && tasks[currentTask].state == TaskState::RUNNING) {
        tasks[currentTask].cpustate = cpustate;
        tasks[currentTask].state = TaskState::READY;
    }

    ++currentTask;
    currentTask %= numTasks;


    return PickNextTask();
}

uint32_t Task::pidCounter = 1;

void TaskManager::ExitTask()
{
    if(numTasks <= 0)
        return;
    
    if(currentTask >= 0 && tasks[currentTask].state == TaskState::RUNNING) {
        tasks[currentTask].state = TaskState::ZOMBIE;
    }
}


common::uint32_t TaskManager::ForkTask(CPUState* cpustate)
{   

    if(numTasks <= 0)
        return -1;

    if(numTasks >= 256)
        return -1;

    tasks[numTasks].pid = Task::pidCounter++;
    tasks[numTasks].ppid = tasks[currentTask].pid;
    tasks[numTasks].state = TaskState::READY;


    for(int i = 0; i < 4096; i++) {
        tasks[numTasks].stack[i] = tasks[currentTask].stack[i];
    }

    common::uint32_t currentTaskOffset = (((common::uint32_t)cpustate - (common::uint32_t) tasks[currentTask].stack));
    tasks[numTasks].cpustate = (CPUState*)(((common::uint32_t) tasks[numTasks].stack) + currentTaskOffset);
    tasks[numTasks].cpustate->ecx = 0;

    numTasks++;
    return tasks[numTasks - 1].pid;
}


int TaskManager::WaitTask(int pid)
{
    // check if there are any tasks
    if(numTasks <= 0)
        return -1;
    
    Task* task = nullptr;

    // find the task with the given pid
    for(int i = 0; i < numTasks; ++i)
    {
        if(tasks[i].pid == pid)
        {
            task = &tasks[i];
            break;
        }
    }

    // check if the task exists
    if(!task) {
        printf("Task not found\n");
        return -1;
    }

    // check if the task is a child of the current task
    if(task->ppid != tasks[currentTask].pid) {
        printf("Task is not a child of the current task\n");
        return -1;
    }

    if(task->state == TaskState::ZOMBIE) {
        return 0;
    }

    tasks[currentTask].state = TaskState::BLOCKED;
    tasks[currentTask].blocking_pid = pid;
    return 0;
}

uint32_t TaskManager::ExecveTask(void entrypoint())
{
    if(numTasks <= 0)
        return -1;


    tasks[currentTask].cpustate->ecx = 0;
    tasks[currentTask].cpustate->edx = 0;
    tasks[currentTask].cpustate->esi = 0;
    tasks[currentTask].cpustate->edi = 0;
    tasks[currentTask].cpustate->ebp = 0;
    tasks[currentTask].cpustate->eflags = 0x202;
    tasks[currentTask].cpustate->cs = Task::gdt->CodeSegmentSelector();
    tasks[currentTask].cpustate->eip = (uint32_t)entrypoint;

    return (uint32_t)tasks[currentTask].cpustate;
}

uint32_t TaskManager::ExecveTask(void entrypoint(uint32_t), uint32_t arg, CPUState* cpustate) {
    if(numTasks <= 0)
        return -1;

    cpustate->eax = arg;
    cpustate->ecx = 0;
    cpustate->edx = 0;
    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;
    cpustate->eflags = 0x202;
    cpustate->cs = Task::gdt->CodeSegmentSelector();
    cpustate->eip = (uint32_t)entrypoint;
    *(uint32_t*)(((uint32_t)cpustate) + sizeof(CPUState) - 4) = arg;
    return (uint32_t)cpustate;
}

void TaskManager::printTasks()
{
    for(int i = 0; i < numTasks; ++i)
    {
        // Print beautiful task information
        printf("-----------------------------\n");
        printf("PID: ");
        printfHex32(tasks[i].pid);
        printf("\nPPID: ");
        printfHex32(tasks[i].ppid);
        printf("\nState: ");
        switch(tasks[i].state)
        {
            case TaskState::READY:
                printf("READY");
                break;
            case TaskState::RUNNING:
                printf("RUNNING");
                break;
            case TaskState::ZOMBIE:
                printf("ZOMBIE");
                break;
            case TaskState::BLOCKED:
                printf("BLOCKED");
                break;
        }
        printf("\n");
        printf("EAX: ");
        printfHex32(tasks[i].cpustate->eax);
        printf("\n");
        printf("EBX: ");
        printfHex32(tasks[i].cpustate->ebx);
        printf("\n");
        printf("\n-----------------------------\n");
    }
}