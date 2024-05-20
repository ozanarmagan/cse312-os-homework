#ifndef __MYOS__MICROKERNEL_H
#define __MYOS__MICROKERNEL_H




// The init process will initialize the Process Table and ready queue
// and will start collatz conjecture program and long running task program 3 times
void initProcessPartA();


// The first strategy is randomly choosing one of the programs and
// loading it into memory 10 times (Same program 10 different processes),
// start them and will enter an infinite loop until all the processes terminate.
void initProcessFirstStrategy();


// Second Strategy is choosing 2 out 4 programs randomly and loading
// each program 3 times start them and will enter an infinite loop until all
// the processes terminate.
void initProcessSecondStrategy();


// Third Strategy, init process will initialize Process Table and ready
// queue, let the Collatz program is in the ready queue with the lowest
// priority, and after the 5th interrupt (we expect that the collatz lasts very
// longer than this of course), remaining programs will arrive as their
// priorities are the same.
void initProcessThirdStrategy();

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
void initProcessDynamicPriority();

// In this strategy, the operating system randomly chooses one of the programs and loads
// it into memory multiple times, each time creating a new process. These processes then
// enter an infinite loop, awaiting interactive input events such as mouse clicks or
// keyboard presses. Upon receiving an input event, the process reacts accordingly before
// returning to its idle state.
void initProcessRandomProcessSpawn();

void initProcessInputPriority();

#endif