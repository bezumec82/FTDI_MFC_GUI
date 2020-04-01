#pragma once

#ifndef __FTDI_HPP__
#define __FTDI_HPP__

#include <afxwin.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <mutex>

#include <windows.h>
#include <ftd2xx.h>


namespace FTDI
{
    using DevListUptr = ::std::unique_ptr< FT_DEVICE_LIST_INFO_NODE[] >;
    using DevDescription = ::std::string;
    using DevDescriptions = ::std::vector < ::std::string >;
    using DevDescriptionMap = ::std::unordered_map< ::std::string, FT_DEVICE_LIST_INFO_NODE >;
    
    //UseCase :
    //Call 'findFTDIDevices' to observe all FTDI devices in the system.
    //Set variable 'm_selDevDescr' to the selected one.
    //Now it is possible to open selected device and start reading/writing
    class FtdiHandler
    {
    public: /*--- Constructor ---*/
        FtdiHandler():
            m_ft_status{ FT_OTHER_ERROR },
            m_num_devs{ 0 }
        { }

        void findFTDIDevices();
        void printFTDIDevices();
#if(0)
        void fillComboBox(CComboBox&);
#endif
        int32_t openDevice();
        void closeDevice();
        void sendData(::std::vector<char>&);
        void recvData(::std::vector<char>&);

    public: /*--- Setters/Getters ---*/
        const DevDescription& getSelDev() const
        {
            return m_selDevDescription;
        }
        void setSelDev(::std::string desc);
        bool isLocked()
        {
            return m_deviceIsLocked.load();
        }
        const DevDescriptions& getDevDescriptions() const
        {
            return m_devDescriptions;
        }


    private: /*--- Variables ---*/
        //hardware
        FT_STATUS m_ft_status;
        DevListUptr m_dev_list_uptr;
        DWORD m_num_devs;
        FT_HANDLE m_selDevHandle{ nullptr };

        //representation at GUI side
        DevDescription m_selDevDescription;
        DevDescriptions m_devDescriptions;
        DevDescriptionMap m_devDescriptionMap;

        //synchronization
        ::std::atomic_bool m_deviceIsLocked{false};
        ::std::mutex m_sendMtx;

    };
} //end namespace

#endif
