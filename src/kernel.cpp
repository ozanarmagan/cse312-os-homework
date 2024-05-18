
#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/ata.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>

#include <drivers/amd_am79c973.h>


// #define GRAPHICSMODE


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;


TaskManager taskManager;

void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}
void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}
void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}

void printDec(uint8_t key)
{
    char* foo = "00";
    foo[0] = key / 10 + '0';
    foo[1] = key % 10 + '0';
    printf(foo);
}

void printDec32(uint32_t key)
{
    char* foo = "0000000000";
    for(int i = 9; i >= 0; i--)
    {
        foo[i] = key % 10 + '0';
        key /= 10;
    }
    printf(foo);
}





class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};




void sysprintf(char* str)
{
    asm("int $0x80" : : "a" (4), "b" (str));
}

static inline uint32_t getpid()
{
    uint32_t pid;
    asm("int $0x80" : "=c" (pid) : "a" (6));
    return pid;
}

static inline uint32_t getppid()
{
    uint32_t ppid;
    asm("int $0x80" : "=c" (ppid) : "a" (10));
    return ppid;
}



static inline uint32_t fork()
{
    uint32_t pid;
    asm("int $0x80" : "=c" (pid) : "a" (5));
    return pid;
}

static inline void exec(void (entrypoint)())
{
    asm("int $0x80" : : "a" (7), "b" (entrypoint));
}

static inline void exec(void (entrypoint)(uint32_t), uint32_t arg)
{
    asm("int $0x80" : : "a" (11), "b" (entrypoint), "c" (arg));
}

static inline void exit()
{
    asm("int $0x80" : : "a" (8));
}

static inline void wait(uint32_t pid)
{
    asm("int $0x80" : : "a" (9), "b" (pid));
}


void collatz(uint32_t start) {
    printf("Collatz conjecture\n");
    // run collatz conjecture for each number from start to 0
    while(start != 1) {
        uint32_t start_ = start;
        uint32_t counter = 0, vals[1000];
            printf("Collatz conjecture for ");
        printDec(start);
        printf(": ");
        vals[counter++] = start_;
        while(start_ != 1) {
            printDec(start_);
            printf(", ");
            if(start_ % 2 == 0) {
                start_ = start_ / 2;
            } else {
                start_ = 3 * start_ + 1;
            }
            vals[counter++] = start_;
        }
        printDec(start_);
        printf("\n");

        start--;
    }
    exit();
}

void long_running_task(uint32_t n) {
    uint32_t res = 0;
    for(uint32_t i = 0; i < n; i++) {
        for(uint32_t j = 0; j < n; j++) {
            res += i * j;
        }
    }

    printf("Long running task finished, result: ");
    printDec32(res);
    printf("\n");
    exit();
}


void taskA()
{
    printf("Task A is running\n");
    printf("Task A finished\n");
    exit();
}


void taskB()
{
    printf("Task B is running\n");
    printf("Task B finished\n");
    exit();
}


void initProcess() {
    uint32_t pid = fork();
    if(pid == 0)
    {
        printf("Starting collatz conjecture\n");
        exec(collatz, (uint32_t)100);
    }
    else
    {
        uint32_t pid2 = fork();
        if(pid2 == 0)
        {
            printf("Starting long running task\n");
            exec(long_running_task, (uint32_t)100);
        }
        else
        {
            printf("Waiting for collatz conjecture to finish\n");
            wait(pid);
            printf("Collatz conjecture finished\n");
            printf("Waiting for long running task to finish\n");
            wait(pid2);
            printf("Long running task finished\n");
        }
    }
    printf("all children finished\n");
    while(1);
}





typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{

    GlobalDescriptorTable gdt;
    
    
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);
    
    // printf("heap: 0x");
    // printfHex((heap >> 24) & 0xFF);
    // printfHex((heap >> 16) & 0xFF);
    // printfHex((heap >> 8 ) & 0xFF);
    // printfHex((heap      ) & 0xFF);
    
    void* allocated = memoryManager.malloc(1024);
    // printf("\nallocated: 0x");
    // printfHex(((size_t)allocated >> 24) & 0xFF);
    // printfHex(((size_t)allocated >> 16) & 0xFF);
    // printfHex(((size_t)allocated >> 8 ) & 0xFF);
    // printfHex(((size_t)allocated      ) & 0xFF);
    // printf("\n");

    Task::gdt = &gdt;

    Task task1(initProcess);
    taskManager.AddTask(task1);

    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80, &taskManager);
    
    //printf("Initializing Hardware, Stage 1\n");
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);
        
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        #ifdef GRAPHICSMODE
            VideoGraphicsArray vga;
        #endif
        
    //printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();
        
    //printf("Initializing Hardware, Stage 3\n");

    #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif


    /*
    printf("\nS-ATA primary master: ");
    AdvancedTechnologyAttachment ata0m(true, 0x1F0);
    ata0m.Identify();
    
    printf("\nS-ATA primary slave: ");
    AdvancedTechnologyAttachment ata0s(false, 0x1F0);
    ata0s.Identify();
    ata0s.Write28(0, (uint8_t*)"http://www.AlgorithMan.de", 25);
    ata0s.Flush();
    ata0s.Read28(0, 25);
    
    printf("\nS-ATA secondary master: ");
    AdvancedTechnologyAttachment ata1m(true, 0x170);
    ata1m.Identify();
    
    printf("\nS-ATA secondary slave: ");
    AdvancedTechnologyAttachment ata1s(false, 0x170);
    ata1s.Identify();
    // third: 0x1E8
    // fourth: 0x168
    */
    
    
    //amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);
    //eth0->Send((uint8_t*)"Hello Network", 13);
        

    interrupts.Activate();


    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
