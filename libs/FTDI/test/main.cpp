#include "ftdi.h"

int main(int, char**)
{
    ::FTDI::FtdiHandler& ftdi_handler{\
        ::FTDI::FtdiHandler::getInstance()};
    ftdi_handler.findFtdiDevices();
    ftdi_handler.printFtdiDevices();
}