
#include <multitasking.h>

using namespace myos;
using namespace myos::common;

extern void printf(char* str);
extern void printfHex(uint8_t key);
extern void printDec32(uint32_t key);
extern void printfHex16(uint16_t key);
extern void printDec32(uint32_t key);

extern "C" void* _getCPUState();

GlobalDescriptorTable* Task::gdt = nullptr;

// Task class implementation
Task::Task(void entrypoint(), common::uint32_t priority)
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
    cpustate -> eip = (uint32_t)entrypoint;
    cpustate -> cs = gdt->CodeSegmentSelector();
    // cpustate -> ss = ;
    cpustate -> eflags = 0x202;

    pid = pidCounter++;
    this->priority = priority;
}


// Empty task constructor
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
CPUState* TaskManager::RoundRobinScheduler(CPUState* cpustate)
{  
    // print process table
    //printTasks();

    auto startingIndex = currentTask;
    ++currentTask;
    currentTask %= numTasks;
    // find the next task to run
    while(tasks[currentTask].state != TaskState::READY)
    {
        ++currentTask;
        currentTask %= numTasks;
        // if we have checked all the tasks and none of them are ready, return the current task
        if(currentTask == startingIndex)
            return cpustate;
    }

    // if we have found a task to run, change the state of the current task to ready
    // and update the cpustate of the current task
    if(startingIndex >= 0) {
        tasks[startingIndex].cpustate = cpustate;
        if(tasks[startingIndex].state == TaskState::RUNNING)
            tasks[startingIndex].state = TaskState::READY;
    }
    


    // change the state of the selected task to running and return its cpustate
    tasks[currentTask].state = TaskState::RUNNING;
    return tasks[currentTask].cpustate;
}


// Pick the next task to run with preemptive scheduling
// The task with the highest priority is selected among the ready tasks
CPUState* TaskManager::PreemptiveScheduler(CPUState* cpustate)
{  
    common::uint32_t selectedTask = -1;
    int lastTask = currentTask;
    ++currentTask;
    currentTask %= numTasks;
    common::uint32_t startingIndex = currentTask;
    while(true)
    {
        if(tasks[currentTask].state == TaskState::READY && (selectedTask == -1 || tasks[currentTask].priority < tasks[selectedTask].priority)) {
            selectedTask = currentTask;
        }
        ++currentTask;
        currentTask %= numTasks;
        if(currentTask == startingIndex) {
            // if we have checked all the tasks and none of them are ready, return the current task
            if(selectedTask == -1 && lastTask != -1) {
                selectedTask = lastTask;
                break;
            } else if(selectedTask != -1) {
                break;
            }
        }
    }

    tasks[selectedTask].state = TaskState::RUNNING;
    currentTask = selectedTask;

    if(lastTask >= 0) {
        tasks[lastTask].cpustate = cpustate;
        if(tasks[lastTask].state == TaskState::RUNNING)
            tasks[lastTask].state = TaskState::READY;
    }

    return tasks[currentTask].cpustate;
}


// Pick the next task to run with preemptive scheduling
CPUState* TaskManager::PreemptiveSchedulerWithDynamicPriority(CPUState* cpustate)
{  
    common::uint32_t selectedTask = -1;
    int lastTask = currentTask;
    ++currentTask;
    currentTask %= numTasks;
    common::uint32_t startingIndex = currentTask;
    while(true)
    {
        if(tasks[currentTask].state == TaskState::READY && (selectedTask == -1 || tasks[currentTask].priority < tasks[selectedTask].priority)) {
            selectedTask = currentTask;
        }
        ++currentTask;
        currentTask %= numTasks;
        if(currentTask == startingIndex) {
            // if we have checked all the tasks and none of them are ready, return the current task
            if(selectedTask == -1 && lastTask != -1) {
                selectedTask = lastTask;
                break;
            } else if(selectedTask != -1) {
                break;
            }
        }
    }

    // check if we have a task with dynamic priority
    if(timerInterruptCounter % 5 == 0 && taskWithDynamicPriority != -1 && tasks[taskWithDynamicPriority].state != TaskState::ZOMBIE && tasks[taskWithDynamicPriority].priority > 0) {
        printf("Changing priority of process ");
        printDec32(tasks[taskWithDynamicPriority].pid);
        printf(" from ");
        printDec32(tasks[taskWithDynamicPriority].priority);
        printf(" to ");
        printDec32(0);
        printf("\n");
        tasks[taskWithDynamicPriority].priority = 0;
        if(selectedTask != taskWithDynamicPriority) {
            selectedTask = taskWithDynamicPriority;
        }
    }
        
    tasks[selectedTask].state = TaskState::RUNNING;
    currentTask = selectedTask;

    if(lastTask >= 0) {
        tasks[lastTask].cpustate = cpustate;
        if(tasks[lastTask].state == TaskState::RUNNING)
            tasks[lastTask].state = TaskState::READY;
    }

    return tasks[currentTask].cpustate;
}

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    if(numTasks <= 0)
        return cpustate;


    timerInterruptCounter++;
    // call the scheduler based on the scheduler type
    if(schedulerType == SchedulerType::ROUNDROBIN)
        return RoundRobinScheduler(cpustate);
    else if(schedulerType == SchedulerType::PREEMPTIVE)
        return PreemptiveScheduler(cpustate);
    else if(schedulerType == SchedulerType::PREEMPTIVE_DYNAMIC_PRIORITY)
        return PreemptiveSchedulerWithDynamicPriority(cpustate);
}

uint32_t Task::pidCounter = 1;


// Exit the current task
void TaskManager::ExitTask()
{
    if(numTasks <= 0)
        return;

    // change the state of the current task to zombie
    if(currentTask >= 0) {
        tasks[currentTask].state = TaskState::ZOMBIE;
    }

    // find the parent of the current task
    Task* parent = nullptr;
    for(int i = 0; i < numTasks; ++i)
    {
        if(tasks[i].pid == tasks[currentTask].ppid)
        {
            parent = &tasks[i];
            break;
        }
    }

    // check if the parent is blocked on the current task
    if(parent) {
        if(parent->state == TaskState::BLOCKED && (parent->blocking_pid == tasks[currentTask].pid || (parent->blocking_pid == WAIT_ALL_CHILDREN && !isThereAliveChild(parent->pid)))) {
            parent->state = TaskState::READY;
            parent->blocking_pid = NOT_BLOCKED;
        }
    }
}


// Fork the current task
common::uint32_t TaskManager::ForkTask(CPUState* cpustate, common::int32_t priority)
{   

    if(numTasks <= 0)
        return -1;

    if(numTasks >= 256)
        return -1;
    tasks[numTasks].pid = Task::pidCounter++;
    tasks[numTasks].ppid = tasks[currentTask].pid;
    tasks[numTasks].state = TaskState::READY;
    if(priority == -1) {
        tasks[numTasks].priority = tasks[currentTask].priority;
    } else {
        tasks[numTasks].priority = priority;
    }

    for(int i = 0; i < 4096; i++) {
        tasks[numTasks].stack[i] = tasks[currentTask].stack[i];
    }

    // calculate new stack offset
    common::uint32_t currentTaskOffset = (((common::uint32_t)cpustate - (common::uint32_t) tasks[currentTask].stack));
    tasks[numTasks].cpustate = (CPUState*)(((common::uint32_t) tasks[numTasks].stack) + currentTaskOffset);
    tasks[numTasks].cpustate->ecx = 0;

    numTasks++;
    return tasks[numTasks - 1].pid;
}

// wait for a task to finish
int TaskManager::WaitTask(uint32_t pid)
{
    // check if there are any tasks
    if(numTasks <= 0)
        return -1;

    // check if the pid is waiting for all children
    if(pid == WAIT_ALL_CHILDREN) {
        // check if there are any alive children
        if(isThereAliveChild(tasks[currentTask].pid)) {
            tasks[currentTask].state = TaskState::BLOCKED;
            tasks[currentTask].blocking_pid = WAIT_ALL_CHILDREN;
            return 0;
        } else {
            return -1;
        }
    }
    
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

    // check if the task is already a zombie
    if(task->state == TaskState::ZOMBIE) {
        return 0;
    }

    tasks[currentTask].state = TaskState::BLOCKED;
    tasks[currentTask].blocking_pid = pid;
    return 0;
}

// check if there are any alive children of a task
bool TaskManager::isThereAliveChild(common::uint32_t pid) {
    for(int i = 0; i < numTasks; ++i)
    {
        if(tasks[i].ppid == pid && tasks[i].state != TaskState::ZOMBIE)
        {
            return true;
        }
    }
    return false;
}

// Load a new program to the current process
uint32_t TaskManager::ExecveTask(void* entrypoint, common::uint32_t argc, common::uint32_t* argv, CPUState* cpustate) {
    if(numTasks <= 0)
        return -1;

    // Get address of the stack
    common::uint32_t* stackStartingAddress = (common::uint32_t*)(((uint32_t)cpustate) + sizeof(CPUState) - 4);
    for(int i = argc - 1; i >= 0; --i) {
        stackStartingAddress[i] = (uint32_t)argv[i];
    }
    cpustate->eax = 0;
    cpustate->ebx = 0;
    cpustate->ecx = 0;
    cpustate->edx = 0;
    cpustate->esi = 0;
    cpustate->edi = 0;
    cpustate->ebp = 0;
    cpustate->eflags = 0x202;
    cpustate->cs = Task::gdt->CodeSegmentSelector();
    cpustate->eip = (uint32_t)entrypoint;

    return (uint32_t)cpustate;
}

void TaskManager::printTasks()
{
    for(int i = 0; i < numTasks; ++i)
    {
        // Print beautiful task information
        printf("-----------------------------\n");
        printf("PID: ");
        printDec32(tasks[i].pid);
        printf("\nPPID: ");
        printDec32(tasks[i].ppid);
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
        printf("PRIORITY: ");
        printDec32(tasks[i].priority);
        printf("\n");
    }
}

void TaskManager::handleKeyPress(char key) {
    pressedKeys[numPressedKeys++] = key;
    pressedKeys[numPressedKeys] = '\0';
}

char* TaskManager::readKeyPresses() {
    if(numPressedKeys == 0)
        return nullptr;

    char* keys = new char[numPressedKeys + 1];
    for(int i = 0; i < numPressedKeys; ++i) {
        keys[i] = pressedKeys[i];
    }
    numPressedKeys = 0;
    pressedKeys[numPressedKeys] = '\0';
    return keys;
}