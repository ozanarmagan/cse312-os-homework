#include <microkernel.h>
#include <syscallwrapper.h>
#include <multitasking.h>
#include <program.h>

using namespace myos;
using namespace myos::common;


extern uint32_t rand();

// The init process will initialize the Process Table and ready queue
// and will start collatz conjecture program and long running task program 3 times
void initProcessPartA() {
    int i = 0;
    while(i < 3) {
        // fork a new process for collatz conjecture
        uint32_t pid = fork();
        if(pid == 0)
        {
            // execute collatz conjecture program in child process
            uint32_t argc = 1;
            uint32_t* argv = new uint32_t[1];
            argv[0] = 100;
            exec((void*)collatz, argc, argv);
        }
        // fork a new process for long running task
        uint32_t pid2 = fork();
        if(pid2 == 0)
        {
            // execute long running task program in child process
            uint32_t argc = 1;
            uint32_t* argv = new uint32_t[1];
            argv[0] = 100;
            exec((void*)long_running_task, argc, argv);
        }
        // repeat the process 3 times
        i++;
    }

    sysprintf("Waiting for all children to finish\n");
    // wait for all children to finish
    wait(WAIT_ALL_CHILDREN);
    sysprintf("all children finished\n");
    // Prevent the init process from exiting
    while(1);
}


// The first strategy is randomly choosing one of the programs and
// loading it into memory 10 times (Same program 10 different processes),
// start them and will enter an infinite loop until all the processes terminate.
void initProcessFirstStrategy() {
    printf("First Strategy\n");
    // generate random number between 0 and 3
    // if 0 pick collatz conjecture
    // if 1 pick long running task
    // if 2 pick binary search
    // if 3 pick linear search
    uint32_t random = rand() % 4;

    // load random program 10 times
    uint32_t i = 0;
    uint32_t pid;
    while(i < 10) {
        pid = fork();
        if(pid == 0) {
            if(random == 0) {
                // execute collatz conjecture program
                uint32_t argc = 1;
                uint32_t* argv = new uint32_t[1];
                argv[0] = 100;
                exec((void*)collatz, argc, argv);
            } else if(random == 1) {
                // execute long running task program
                uint32_t argc = 1;
                uint32_t *argv = new uint32_t[1];
                argv[0] = 10000;
                exec((void*)long_running_task, argc, argv);
            } else if(random == 2) {
                // execute binary search program
                uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
                uint32_t argc = 3;
                uint32_t *argv = new uint32_t[3];
                argv[0] = (uint32_t)arr;
                argv[1] = 10;
                argv[2] = 110;
                exec((void*)binary_search, argc, argv);
            } else {
                // execute linear search program
                uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
                uint32_t argc = 3;
                uint32_t *argv = new uint32_t[3];
                argv[0] = (uint32_t)arr;
                argv[1] = 10;
                argv[2] = 110;;
                exec((void*)linear_search, argc, argv);
            }
        }
        // repeat the process 10 times
        i++;
    }

    // wait for all children to finish
    printf("Waiting for all children to finish\n");
    wait(WAIT_ALL_CHILDREN);
    printf("All children finished\n");
    while(1);
}


// Second Strategy is choosing 2 out 4 programs randomly and loading
// each program 3 times start them and will enter an infinite loop until all
// the processes terminate.
void initProcessSecondStrategy() {
    uint32_t random1 = rand() % 4;
    uint32_t random2 = rand() % 4;

    // make sure random2 is different from random1
    while(random2 == random1) {
        random2 = rand() % 4;
    }

    uint32_t i = 0;
    while(i < 3) {
        uint32_t pid1 = fork();
        if(pid1 == 0) {
            if(random1 == 0) {
                uint32_t argc = 1;
                uint32_t argv[1] = {100};
                exec((void*)collatz, argc, argv);
            } else if(random1 == 1) {
                uint32_t argc = 1;
                uint32_t argv[1] = {100};
                exec((void*)long_running_task, argc, argv);
            } else if(random1 == 2) {
                uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
                uint32_t argc = 3;
                uint32_t argv[3] = {(uint32_t)arr, 10, 110};
                exec((void*)binary_search, argc, argv);
            } else {
                uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
                uint32_t argc = 3;
                uint32_t argv[3] = {(uint32_t)arr, 10, 110};
                exec((void*)linear_search, argc, argv);
            }
        }
        uint32_t pid2 = fork();
        if(pid2 == 0) {
            if(random2 == 0) {
                uint32_t argc = 1;
                uint32_t argv[1] = {100};
                exec((void*)collatz, argc, argv);
            } else if(random2 == 1) {
                uint32_t argc = 1;
                uint32_t argv[1] = {100};
                exec((void*)long_running_task, argc, argv);
            } else if(random2 == 2) {
                uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
                uint32_t argc = 3;
                uint32_t argv[3] = {(uint32_t)arr, 10, 110};
                exec((void*)binary_search, argc, argv);
            } else {
                uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
                uint32_t argc = 3;
                uint32_t argv[3] = {(uint32_t)arr, 10, 110};
                exec((void*)linear_search, argc, argv);
            }
        }
        i++;
    }

    // wait for all children to finish
    printf("Waiting for all children to finish\n");
    wait(WAIT_ALL_CHILDREN);
    printf("All children finished\n");
    while(1);
}


// Third Strategy, init process will initialize Process Table and ready
// queue, let the Collatz program is in the ready queue with the lowest
// priority, and after the 5th interrupt (we expect that the collatz lasts very
// longer than this of course), remaining programs will arrive as their
// priorities are the same.
void initProcessThirdStrategy() {
    // this will fork this process with lower priority
    auto pid = forkWithPriority(1);
    if(pid == 0) {
        uint32_t argc = 1;
        uint32_t argv[1] = {100};
        exec((void*)collatz, argc, argv);
    }
    uint32_t initial_count = get_interrupt_count();
    // wait for 5 interrupts
    while(get_interrupt_count() - initial_count < 5);
    printf("5 interrupts passed\n");
    printf("Starting other programs\n");
    pid = fork();
    if(pid == 0) {
        uint32_t argc = 1;
        uint32_t argv[1] = {100};
        exec((void*)long_running_task, argc, argv);
    }
    uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};    
    pid = fork();
    if(pid == 0) {
        uint32_t argc = 3;
        uint32_t argv[3] = {(uint32_t)arr, 10, 110};
        exec((void*)binary_search, argc, argv);
    }
    pid = fork();
    if(pid == 0) {
        uint32_t argc = 3;
        uint32_t argv[3] = {(uint32_t)arr, 10, 110};
        exec((void*)linear_search, argc, argv);
    }

    // wait for all children to finish
    printf("Waiting for all children to finish\n");
    wait(WAIT_ALL_CHILDREN);
}

// Dynamic Priority Strategy: the init process initializes the Process Table
// and ready queue. The Collatz program is initially placed in the ready
// queue with a priority lower than the other programs. For every timer
// interrupt, the OS evaluates the execution status of the Collatz program.
// If the Collatz program is still executing after a certain number of
// interrupts (let's say after every 5th interrupt), the OS dynamically adjusts
// its priority to be higher than the other programs in the ready queue. The
// remaining programs maintain their initial priorities throughout execution.
// The OS continues to handle timer interrupts and adjust priorities
// dynamically based on the execution status of the Collatz program. Once
// the Collatz program completes its execution or is preempted by another
// process, the OS resumes executing the remaining programs in the ready
// queue based on their priorities. This strategy allows the OS to
// dynamically prioritize the Collatz program based on its execution
// progress, ensuring efficient resource allocation, and potentially speeding
// up the completion of the Collatz program without compromising the
// execution of other programs.
void initProcessDynamicPriority() {
    auto pid = forkWithPriority(2);
    if(pid == 0) {
        // set dynamic priority for current process
        dynamicPriority();
        uint32_t argc = 1;
        uint32_t argv[1] = {100};
        exec((void*)collatz, argc, argv);
    }
    pid = forkWithPriority(1);
    if(pid == 0) {
        uint32_t argc = 1;
        uint32_t argv[1] = {100};
        exec((void*)long_running_task, argc, argv);
    }
    uint32_t arr[10] = {10, 20, 80, 30, 60, 50, 110, 100, 130, 170};
    pid = forkWithPriority(1);
    if(pid == 0) {
        uint32_t argc = 3;
        uint32_t argv[3] = {(uint32_t)arr, 10, 110};
        exec((void*)binary_search, argc, argv);
    }
    pid = forkWithPriority(1);
    if(pid == 0) {
        uint32_t argc = 3;
        uint32_t argv[3] = {(uint32_t)arr, 10, 110};
        exec((void*)linear_search, argc, argv);
    }

    // wait for all children to finish
    printf("Waiting for all children to finish\n");
    wait(WAIT_ALL_CHILDREN);
    exit();
}

// In this strategy, the operating system randomly chooses one of the programs and loads
// it into memory multiple times, each time creating a new process. These processes then
// enter an infinite loop, awaiting interactive input events such as mouse clicks or
// keyboard presses. Upon receiving an input event, the process reacts accordingly before
// returning to its idle state.
void initProcessRandomProcessSpawn() {
    uint32_t random = rand() % 4;
    void* randomProcess;
    // randomly choose one of the programs
    if(random == 0) {
        randomProcess = (void*)collatz;
    } else if(random == 1) {
        randomProcess = (void*)long_running_task;
    } else if(random == 2) {
        randomProcess = (void*)binary_search;
    } else {
        randomProcess = (void*)linear_search;
    }

    uint32_t pid = fork();
    if(pid == 0) {
        // wait for keyboard press
        printf("Press any key to start random process 1\n");
        char* keys;
        while((keys = checkKeyPress()) == nullptr);
        printf("Starting random process 1\n");
        printf("Pressed key: ");
        printf(keys);
        printf("\n");
        uint32_t argc = 1;
        uint32_t argv[1] = {100};
        exec(randomProcess, argc, argv);
    }
    
    pid = fork();
    if(pid == 0) {
        // wait for mouse click
        printf("Click anywhere to start random process 2\n");
        while(!checkMouseClick());
        printf("Starting random process 2\n");
        uint32_t argc = 1;
        uint32_t argv[1] = {100};
        exec(randomProcess, argc, argv);
    }

    // wait for all children to finish
    printf("Waiting for all children to finish\n");
    wait(WAIT_ALL_CHILDREN);
    printf("All children finished\n");
    while(1);
}

void initProcessInputPriority() {
    uint32_t pid = fork();
    if(pid == 0) {
        uint32_t argc = 0;
        uint32_t* argv = nullptr;
        // execute shell program
        exec((void*)shell, argc, argv);
    }
    // fork with lower priority
    pid = forkWithPriority(1);
    if(pid == 0) {
        while(1) {
            while(!checkMouseClick());
            printf("Mouse clicked\n");
        }
    }
    wait(WAIT_ALL_CHILDREN);
    while(1);
}