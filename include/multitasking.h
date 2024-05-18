 
#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <gdt.h>

namespace myos
{

    enum class TaskState
    {
        EMPTY,
        RUNNING,
        READY,
        BLOCKED,
        ZOMBIE // terminated
    };
    
    struct CPUState
    {
        common::uint32_t eax;
        common::uint32_t ebx;
        common::uint32_t ecx;
        common::uint32_t edx;

        common::uint32_t esi;
        common::uint32_t edi;
        common::uint32_t ebp;
        /*
        common::uint32_t gs;
        common::uint32_t fs;
        common::uint32_t es;
        common::uint32_t ds;
        */
        common::uint32_t error;

        common::uint32_t eip;
        common::uint32_t cs;
        common::uint32_t eflags;
        common::uint32_t esp;
        common::uint32_t ss;        
    } __attribute__((packed));
    
    
    class Task
    {
    friend class TaskManager;
    private:
        common::uint8_t stack[4096]; // 4 KiB
        CPUState* cpustate;
        common::uint32_t pid, ppid, blocking_pid;
        enum TaskState state = TaskState::READY;
    public:
        static common::uint32_t pidCounter;
        static GlobalDescriptorTable* gdt;
        Task(void entrypoint());
        Task();
        common::uint32_t getPID() {
            return pid;
        }
        common::uint32_t getPPID() {
            return ppid;
        }
        Task operator=(const Task& task);
        ~Task();
    };
    
    // 
    class TaskManager
    {
    private:
        Task tasks[256];
        int numTasks;
        int currentTask;
        CPUState* PickNextTask();
    public:
        TaskManager();
        ~TaskManager();
        bool AddTask(const Task& task);
        CPUState* Schedule(CPUState* cpustate);
        void ExitTask();
        common::uint32_t ForkTask(CPUState* cpustate);
        int WaitTask(int taskID);
        common::uint32_t ExecveTask(void (entrypoint)());
        common::uint32_t ExecveTask(void (entrypoint)(common::uint32_t), common::uint32_t arg, CPUState* cpustate);
        int GetCurrentTaskID() {
            return currentTask;
        }
        Task* GetCurrentTask() {
            return &tasks[currentTask];
        }
        void printTasks();
    };
    
    
    
}


#endif