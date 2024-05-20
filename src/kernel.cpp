
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
#include <syscallwrapper.h>
#include <program.h>
#include <microkernel.h>

#include <drivers/amd_am79c973.h>


// #define GRAPHICSMODE


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;


TaskManager taskManager;


uint32_t rand() {
    timer = timer * 1103515245 + 12345;
    return (unsigned int) (timer / 65536) % 32768;
    return timer;
}

void printf(char* str)
{
    for(int i = 0; i<500000; i++);

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
            case '\b':
                if(x > 0) {
                    x--;
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
                }
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
            for(int i = 0; i < 25; i++)
                for(int j = 0; j < 80; j++)
                    VideoMemory[80*i+j] = (VideoMemory[80*i+j] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printDec32(uint32_t key)
{
    char foo[] = "0000000000";
    for(int i = 9; i >= 0; i--)
    {
        foo[i] = key % 10 + '0';
        key /= 10;
    }

    // count leading zeros
    int i = 0;
    while(foo[i] == '0') i++;

    if (i == 10) i--;
    
    // move to left
    for(int j = i; j < 10; j++)
        foo[j-i] = foo[j];
    
    foo[10-i] = '\0';



    printf(foo);
}





class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
private:
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
    PrintfKeyboardEventHandler(TaskManager* manager) : KeyboardEventHandler(manager)
    {
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole(TaskManager* manager) : MouseEventHandler(manager)
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

    void* allocated = memoryManager.malloc(1024);


    Task::gdt = &gdt;


// Run the following code to test the different strategies according to the build flags
#ifdef BUILD_PART_A
    Task task1(initProcessPartA);
    taskManager.AddTask(task1);
    taskManager.SetSchedulerType(TaskManager::SchedulerType::ROUNDROBIN);
#elif BUILD_PART_B_FIRST_STRATEGY
    Task task1(initProcessFirstStrategy);
    taskManager.AddTask(task1);
    taskManager.SetSchedulerType(TaskManager::SchedulerType::ROUNDROBIN);
#elif BUILD_PART_B_SECOND_STRATEGY
    Task task1(initProcessSecondStrategy);
    taskManager.AddTask(task1);
    taskManager.SetSchedulerType(TaskManager::SchedulerType::ROUNDROBIN);
#elif BUILD_PART_B_THIRD_STRATEGY
    Task task1(initProcessThirdStrategy);
    taskManager.AddTask(task1);
    taskManager.SetSchedulerType(TaskManager::SchedulerType::PREEMPTIVE);
#elif BUILD_PART_B_DYNAMIC_PRIORITY
    Task task1(initProcessDynamicPriority);
    taskManager.AddTask(task1);
    taskManager.SetSchedulerType(TaskManager::SchedulerType::PREEMPTIVE_DYNAMIC_PRIORITY);
#elif BUILD_PART_C_RANDOM_PROCESS_SPAWN
    Task task1(initProcessRandomProcessSpawn);
    taskManager.AddTask(task1);
    taskManager.SetSchedulerType(TaskManager::SchedulerType::ROUNDROBIN);
#elif BUILD_PART_C_INPUT_PRIORITY
    Task task1(initProcessInputPriority);
    taskManager.AddTask(task1);
    taskManager.SetSchedulerType(TaskManager::SchedulerType::ROUNDROBIN);
#endif

    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80, &taskManager);
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            KeyboardEventHandler kbhandler(&taskManager);
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler(&taskManager);
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

    interrupts.Activate();


    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
