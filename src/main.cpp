/*

    Simulator for the HD6309 computer
    Copyright N.A. Moseley 2019
    
    www.moseleyinstruments.com
    
    namoseley.wordpress.com

    Note: This will only compile on Linux

*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <thread>

#include "cxxopts.hpp"

#include "machine.h"

termios g_oldTerminal;

void rawMode()
{
    tcgetattr( STDIN_FILENO, &g_oldTerminal);
    termios newTerminal = g_oldTerminal;

    newTerminal.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal);
}

void restoreMode()
{
    tcsetattr( STDIN_FILENO, TCSANOW, &g_oldTerminal);
}

int main(int argc, char *argv[])
{
    printf("######################################################################\n");
    printf("  HD6309 Sim by Niels A. Moseley\n");
    printf("  based on usim 6809 simulator library by Ray Bellis\n");
    printf("######################################################################\n");

    cxxopts::Options options("hd6309sim", "A simulator for the HD6309 computer");

    bool debug = false;
    options.add_options()
        ("b,break", "Set a breakpoint at address", cxxopts::value<int32_t>())
        ("debug", "Enable 6809 debugger", cxxopts::value<bool>(debug))
        ("hex", "Hex file", cxxopts::value<std::vector<std::string>>())
    ;

    Machine machine;

    auto result = options.parse(argc, argv);

    int32_t breakpoint = -1;
    if (result.count("break"))
    {
        machine.setBreakpoint(breakpoint);
    }    

    auto &v = result["hex"].as<std::vector<std::string> >();

    for(auto hexfile : v)
    {
        if (!machine.loadHex(hexfile.c_str()))
        {
            printf("Failed to load %s\n", hexfile.c_str());
            return 1;
        }
    }

    if (debug)
    {
        printf("Debugging enabled\n");
    }

    machine.debug(debug);
    machine.reset();

    std::thread t1(&Machine::run, &machine);

    rawMode();

    bool quit = false;
    while(!quit)
    {
        uint8_t c = getchar();
        switch(c)
        {
        case 'Q':
            quit = true;
            break;
        case '!':
            printf("PC = %04X\n", machine.getPC());
            break;
        case 127: // backspace ?!?
            while(!machine.clearToSend()) 
            {
                usleep(1000);  // 1ms sleep
            }
            machine.submitSerialChar(8);
            break;
        case 27:    //escape
            while(!machine.clearToSend()) 
            {
                usleep(1000);  // 1ms sleep
            }
            machine.submitSerialChar(27);
            break;
        case 10:
            while(!machine.clearToSend()) 
            {
                usleep(1000);  // 1ms sleep
            }
            machine.submitSerialChar(13);
            break;
        default:
            while(!machine.clearToSend()) 
            {
                usleep(1000);  // 1ms sleep
            }
            machine.submitSerialChar(c);
            break;
        }
    }
    
    machine.halt();
    t1.join();

    restoreMode();

    return 0;
}
