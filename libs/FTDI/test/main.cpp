#include "ftdi.h"

int main(int, char**)
{
    ::FTDI::FtdiHandler ftdi_handler;
    ftdi_handler.findFtdiDevices();
    ftdi_handler.printFtdiDevices();
}