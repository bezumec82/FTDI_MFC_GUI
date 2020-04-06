#include "ftdi.h"

int main(int, char**)
{
    ::FTDI::FtdiHandler ftdi_handler;
    ftdi_handler.findFTDIDevices();
    ftdi_handler.printFTDIDevices();
}