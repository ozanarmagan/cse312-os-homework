 
#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <gdt.h>

namespace myos
{


    constexpr common::uint32_t WAIT_ALL_CHILDREN = 257;
    constexpr common::uint32_t NOT_BLOCKED = 258;

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
        common::uint32_t pid, ppid, blocking_pid = NOT_BLOCKED;
        enum TaskState state = TaskState::READY;
        common::uint32_t priority = 0;
    public:
        static common::uint32_t pidCounter;
        static GlobalDescriptorTable* gdt;
        Task(void entrypoint(), common::uint32_t priority = 0);
        Task();
        common::uint32_t getPID() {
            return pid;
        }
        common::uint32_t getPPID() {
            return ppid;
        }
        common::uint32_t getPriority() {
            return priority;
        }
        Task operator=(const Task& task);
        ~Task();
    };
    
    // 
    class TaskManager
    {
    public:
        enum class SchedulerType
        {
            ROUNDROBIN,
            PREEMPTIVE,
            PREEMPTIVE_DYNAMIC_PRIORITY
        };
    private:
        Task tasks[256];
        int numTasks;
        int currentTask;
        SchedulerType schedulerType = SchedulerType::ROUNDROBIN;
        CPUState* RoundRobinScheduler(CPUState* cpustate);
        CPUState* PreemptiveScheduler(CPUState* cpustate);
        CPUState* PreemptiveSchedulerWithDynamicPriority(CPUState* cpustate);
        common::uint32_t timerInterruptCounter = 0;
        bool isThereAliveChild(common::uint32_t pid);
        int taskWithDynamicPriority = -1;
        char pressedKeys[1024];
        common::uint32_t numPressedKeys = 0;
        bool mouseClicked = false;
    public:
        TaskManager();
        ~TaskManager();
        bool AddTask(const Task& task);
        CPUState* Schedule(CPUState* cpustate);
        void ExitTask();
        common::uint32_t ForkTask(CPUState* cpustate, common::int32_t priority = -1);
        int WaitTask(common::uint32_t taskID);
        common::uint32_t ExecveTask(void* entrypoint, common::uint32_t argc, common::uint32_t* argv, CPUState* cpustate);
        int GetCurrentTaskID() {
            return currentTask;
        }
        Task* GetCurrentTask() {
            return &tasks[currentTask];
        }
        common::uint32_t GetTimerInterruptCounter() {
            return timerInterruptCounter;
        }
        void SetSchedulerType(SchedulerType schedulerType) {
            this->schedulerType = schedulerType;
        }
        SchedulerType GetSchedulerType() {
            return schedulerType;
        }
        void setTaskWithDynamicPriority(common::uint32_t pid) {
            for(int i = 0; i < numTasks; i++)
                if(tasks[i].getPID() == pid)
                {
                    taskWithDynamicPriority = i;
                    break;
                }
        }
        void handleKeyPress(char key);
        char* readKeyPresses();
        void handleMouseClick() {
            mouseClicked = true;
        }
        bool isMouseClicked() {
            bool temp = mouseClicked;
            mouseClicked = false;
            return temp;
        }
        void printTasks();
    };
    
    
    
}


#endif