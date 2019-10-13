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
#include <iostream>
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
    Machine machine;
    int32_t breakpoint = -1;

    options.show_positional_help();
    options.add_options()
        ("b,break", "Set a breakpoint at HEX address", cxxopts::value<std::string>())
        ("trace", "Enable 6809 trace/debugger", cxxopts::value<bool>(debug))
        ("d,disk", "Add a .DSK image as a drive", cxxopts::value<std::vector<std::string>>())
        ("help", "Print help")
        ("hex", "Hex file", cxxopts::value<std::vector<std::string>>())
    ;

    try
    {
        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help({"", "Group"}) << std::endl;
            return 0;
        }

        if (result.count("break"))
        {
            std::string hexnum = result["break"].as<std::string>();
            breakpoint = (int)strtol(hexnum.c_str(), NULL, 16);
            printf("Setting breakpoint address: %04X\n", breakpoint);
            machine.setBreakpoint(breakpoint);
        }    

        if (result.count("hex") > 0)
        {
            auto &v = result["hex"].as<std::vector<std::string> >();

            for(auto hexfile : v)
            {
                if (!machine.loadHex(hexfile.c_str()))
                {
                    printf("Failed to load %s\n", hexfile.c_str());
                    return 1;
                }
            }
        }
        else
        {
            printf("No bootrom .hex files specified!\n");
            options.help();
            exit(1);
        }

        // load the DSK images
        if (result.count("disk") > 0)
        {
            auto &v = result["disk"].as<std::vector<std::string> >();

            uint8_t drive = 0;
            for(auto dskfile : v)
            {
                if (!machine.mountDisk(drive, dskfile))
                {
                    printf("Failed to load %s in drive %d\n", dskfile.c_str(), drive);
                    return 1;
                }
                else
                {
                    printf("Mounted %s in drive %d\n", dskfile.c_str(), drive);
                }
                drive++;
            }
        }
    }
    catch(...)
    {
        return -1;
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
        //case 'Q':
        //    quit = true;
        //    break;
        //case '!':
        //    printf("PC = %04X\n", machine.getPC());
        //    break;
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
