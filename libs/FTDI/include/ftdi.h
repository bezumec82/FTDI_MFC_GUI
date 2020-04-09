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
#include <algorithm>
#include <tuple>

#include <windows.h>
#include <ftd2xx.h>

#include "Utilities.h"
#include "TimeStat.h"

#define RX_TIMEOUT_MS   100
#define SAVE_PERIOD_MS  100
#define SCAN_PERIOD_MS  1000

namespace FTDI
{
    class Logger;

    using Node = FT_DEVICE_LIST_INFO_NODE;
    using DevNodes = ::std::vector<Node>;

    using DevDescription = ::std::string;
    using DevDescriptions = ::std::vector < ::std::string >;

    //UseCase :
    //Call 'findFtdiDevices' to observe all FTDI devices in the system.
    //Set variable 'm_selDevDescr' to the selected one.
    //Now it is possible to open selected device and start reading/writing
    class FtdiHandler
    {
    public: /*--- Aliaces ---*/
        using Data = ::std::variant<DevDescriptions>;
        using DevDescriptionMap = ::std::unordered_map< DevDescription, Node >;
        using LoggerMap = ::std::unordered_map< DevDescription, Logger >;

    public: /*--- Enumerators ---*/
        enum class EventCode : int8_t
        {
            ALL_GOOD = 0,
            SCAN_DATA = 1,
        };
        using CallBack =
            void(const EventCode&, const Data&);
        using CallBacks = ::std::list< ::std::function<CallBack> >;

    public: /*--- Constructor ---*/
        FtdiHandler()
            //: m_selDev{ m_devDescriptionMap.end() }
        {
            m_selDev = m_devDescriptionMap.end();
        }

    private: /*--- Implementation ----*/

        BOOL mergeDevsList(DevNodes& devs);
        void notifyAll(const EventCode&, const Data&);
        void startReadDev(Node&);
        void stopReadDev(Node&);

    public: /*--- Methods ---*/
        static DevDescription makeDevDescription(Node&);
        void startScan();
        INT findFtdiDevices();
        void printFtdiDevices();
        int32_t openDevice(Node&);
#if(0)
        int32_t openSelDevice();
        void closeSelDevice();
#endif
        int32_t sendData(::std::vector<char>&);
        int32_t recvData(::std::vector<char>&);
        int32_t clearRxBuf();

    public: /*--- Setters/Getters ---*/
        void registerCallBack(::std::function <CallBack> call_back)
        {
            m_callBacks.emplace_back(call_back);
        }
        void stopScan()
        {
            m_stopScan.store(true);
        }
        const DevDescription& getSelDev() const
        {
            return m_selDev->first;
        }
        void setSelDev(::std::string desc);
#if(0)
        bool isLocked()
        {
            return m_deviceIsLocked.load();
        }
#endif
        const DevDescriptions& getDevDescriptions() const
        {
            return m_devDescriptions;
        }
    private: /*--- Variables ---*/
        //miscelaneous
        CallBacks m_callBacks;
        ::std::future<void> m_future;
        ::std::atomic_bool m_stopScan{false};
        //::std::mutex m_scanMtx;
        LoggerMap m_loggerMap;

        //hardware
        DevNodes m_devNodes;
        DevDescriptionMap::iterator m_selDev;

        //representation at GUI side
        DevDescriptions m_devDescriptions;
        DevDescriptionMap m_devDescriptionMap;

        //synchronization
        //::std::atomic_bool m_deviceIsLocked{false};
        ::std::mutex m_sendMtx;
        //uint32_t m_devRefCntr{ 0 };

    };

    class Logger
    {
    public: /*--- Aliaces ---*/
        using Data = ::std::variant<CString>;

    public: /*--- Enumerators ---*/
        enum class EventCode : int8_t
        {
            FOPEN_ERR = -2,
            NO_FILE_NAME_ERR = -1,
            ALL_GOOD = 0,
            STOPPED = 1,
            IMMEDIATE_RX_RATE = 2,
            MEDIUM_RX_RATE = 3,
        };
        using CallBack =
            ::std::function<void(const EventCode&, const Data&)>;

    public: /*--- Constructors ---*/
        Logger(Node& node) :
            m_node_ref{node}
        {
            m_devDescription = FtdiHandler::makeDevDescription(node);
        }

    private:/*--- Methods ---*/
        int32_t openFile();
        int32_t openDevice();
        void closeDevice();
        int32_t clearRxBuf();
        int32_t recvData(::std::vector<char>&);
        void notifyAll(const EventCode& , const Data&);
        INT doLogging();

    public: /*--- Methods ---*/
        INT start();
        void stop();

    public: /*--- Getters/Setters ---*/
        void registerCallBack(CallBack call_back)
        {
            m_callBacks.emplace_back( call_back );
        }
        void setFileName(CString fileName)
        {
            m_fileName = fileName;
            //m_fileName.Replace(L' ', L'_');
            m_fileName.Replace(L':', L'_');
        }
        const CString& getFileName()
        {
            return m_fileName;
        }
        bool isLogging()
        {
            return m_isLogging.load();
        }
    private: /*--- Variables ---*/
        Node& m_node_ref;
        DevDescription m_devDescription;
        CString m_fileName;
        CFile m_saveFile;
        ::std::atomic_bool m_startStopFlag{ false };
        ::std::atomic_bool m_isLogging{ false };
        ::std::list< CallBack > m_callBacks;
        ::std::future<void> m_future;
    };

    class Writer
    {
    public: /*--- Aliaces ---*/
        using Data = ::std::variant<CString>;

    public: /*--- Enumerators ---*/
        enum class EventCode : int8_t
        {
            FTDI_OPEN_ERR = -3,
            NO_DATA_ERR = -2,
            NO_PERIOD_ERR = -1,
            ALL_GOOD = 0,
            STOPPED = 1,
        };
        using CallBack =
            ::std::function<void(const EventCode&, const Data&)>;

    public: /*--- Constructor ---*/
        Writer(FtdiHandler& ftdi_handler)
            : m_ftdiHandler_ref{ ftdi_handler }
        {}

    private: /*--- Methods ---*/
        void notifyAll(const EventCode&, const Data&);
        void sendOnce();
        void doSend();

    public: /*--- Methods ---*/
        int32_t readFile();
        void start();
        void stop();

    public: /*--- Getters/Setters ---*/
        void registerCallBack(CallBack call_back)
        {
            m_callBacks.emplace_back(call_back);
        }
        bool isWriting()
        {
            return m_startStopFlag.load();
        }
        void setPeriod(CString& period);
        int32_t getPerios()
        {
            return m_period;
        }

        void setFileName(CString fileName)
        {
            m_fileName = fileName;
        }
        const CString& getFileName()
        {
            return m_fileName;
        }

    private: /*--- Variables ---*/
        FtdiHandler& m_ftdiHandler_ref;

        CString m_fileName;
        CFile m_sendFile;
        ::std::vector<char> m_fileDataBuf;
        int32_t m_period{ -1 };

        ::std::atomic_bool m_startStopFlag{ false };
        ::std::list< CallBack > m_callBacks;
        ::std::future<void> m_future;
    };

} //end namespace

#endif