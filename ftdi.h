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
#include <list>
#include <functional>
#include <future>
#include <variant>

#include <windows.h>
#include <ftd2xx.h>

#include "Utilities.h"
#include "TimeStat.h"

#define RX_TIMEOUT_MS   100
#define SAVE_PERIOD_MS  100

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

        int32_t findFTDIDevices();
        void printFTDIDevices();
#if(0)
        void fillComboBox(CComboBox&);
#endif
        int32_t openDevice();
        void closeDevice();
        void sendData(::std::vector<char>&);
        int32_t recvData(::std::vector<char>&);
        void clearRxBuf();
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

        uint32_t m_devRefCntr{ 0 };

    };

    class Logger
    {
    public: /*--- Enumerators ---*/

        using Data = ::std::variant<CString>;
        
        enum class EventCode : int8_t
        {
            FOPEN_ERR = -2,
            NO_FILE_NAME_ERR = -1,
            ALL_GOOD = 0,
            STOPPED = 1,
            IMMEDIATE_RX_RATE = 2,
            MEDIUM_RX_RATE = 3,
        };
        using CallBack = ::std::function<void(const EventCode&, Data&)>;
    public: /*--- Constructors ---*/
        Logger(FtdiHandler& ftdi_handler) :
            m_ftdiHandler_ref{ftdi_handler}
        { }

    private:/*--- Methods ---*/
        int32_t openFile();
        void notifyAll(const EventCode& err_code, Data& data);
        void doLogging();

    public: /*--- Methods ---*/       

        void start();
        void stop();

    public: /*--- Getters/Setters ---*/
        void registerCallBack(CallBack call_back)
        {
            m_callBacks.emplace_back( call_back );
        }
        void setFileName(CString fileName)
        {
            m_fileName = fileName;
        }
        bool isLogging()
        {
            return m_startStopFlag.load();
        }
    private: /*--- Variables ---*/
        FtdiHandler& m_ftdiHandler_ref;
        CString m_fileName;
        ::std::atomic_bool m_startStopFlag{ false };
        ::std::list< CallBack > m_callBacks;
        CFile m_saveFile;
        ::std::future<void> m_future;
    };
} //end namespace


#endif
