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

#define RX_TIMEOUT_MS               100
#define SAVE_PERIOD_MS              100
#define SCAN_PERIOD_MS              1000
#define SEND_CHUNK                  UINT(16*1024)
#define SEND_SPEED_BYTES_PER_SEC    10000.0
#define LOCK_TIMEOUT_MS             50


#define DEBUG_MERGE     FALSE

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
        using Data = ::std::variant<
            DevDescriptions, /* to CBox */
            CString /* TX rate */
        >;
        using DevDescriptionMap = ::std::unordered_map< DevDescription, Node >;
        using LoggerMap = ::std::unordered_map< DevDescription, Logger >;
        using AsyncResult = ::std::future<void>;
        using Buffer = ::std::vector<char>;

    public: /*--- Enumerators ---*/
        enum class EventCode : int8_t
        {
            ALL_GOOD = 0,
            SCAN_DATA = 1,
            NEW_DEV_SELECTED = 2,
            MEDIUM_TX_RATE = 3,
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
        void stopScan();
        INT openFile(CString&, CFile&);
        INT sendData(::std::vector<char>&);

    public: /*--- Methods ---*/
        static DevDescription makeDevDescription(Node&);
        void startScan();
        INT findFtdiDevices();
        void printFtdiDevices();
        INT sendFile(CString&, \
            ::std::atomic_bool&);
        void stopLogging();
        void stopSending();
        void abort();

        void registerCallBack(::std::function <CallBack> call_back)
        {
            m_callBacks.emplace_back(call_back);
        }
        DevDescription getSelDev() const
        {
            if (m_selDev != m_devDescriptionMap.end())
                return m_selDev->first;
            else return { "NOPE" };
        }
        void setSelDev(::std::string desc);
        const DevDescriptions& getDevDescriptions() const
        {
            return m_devDescriptions;
        }
        bool isSending()
        {
            return m_isSending.load();
        }

    private: /*--- Variables ---*/
        CallBacks m_callBacks;
        ::std::future<void> m_scanFuture;
        ::std::atomic_bool m_stopScan{ false };
        ::std::atomic_bool m_stopSend{ false };
        ::std::atomic_bool m_isSending{ false };
        LoggerMap m_loggerMap;
        DevNodes m_devNodes;
        DevDescriptionMap::iterator m_selDev;
        DevDescriptions m_devDescriptions;
        DevDescriptionMap m_devDescriptionMap;
        //::std::mutex m_devMtx;
        ::std::recursive_mutex m_devMtx;
        UINT m_sendTaskCounter{ 0 };
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
            MEDIUM_RX_RATE = 2,
        };
        using CallBack =
            ::std::function<void(const EventCode&, const Data&)>;

    public: /*--- Constructors ---*/
        Logger(Node& node) :
            m_node_ref{node}
        {
            m_devDescription = FtdiHandler::makeDevDescription(node);
        }
        ~Logger()
        {
            stop();
        }

    private:/*--- Implementation ---*/
        int32_t openFile();
        int32_t openDevice();
        void closeDevice();
        int32_t clearRxBuf();
        int32_t recvData(::std::vector<char>&);
        void notifyAll(const EventCode& , const Data&);
        INT doReading();
        void doLogging(::std::vector<char>&);

    public: /*--- Methods ---*/
        INT startReading();
        bool isReading()
        {
            return m_isReading.load();
        }
        void startLoging()
        {
            m_startStopLogging.store(true);
        }
        void stopLogging()
        {
            m_startStopLogging.store(false);
        }
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

    private: /*--- Variables ---*/
        Node& m_node_ref;
        DevDescription m_devDescription;
        CString m_fileName;
        CFile m_saveFile;
        ::std::list< CallBack > m_callBacks;
        ::std::future<void> m_future;

    private: /*--- Flags ---*/
        ::std::atomic_bool m_startStopReading{ false };
        ::std::atomic_bool m_isReading{ false };
        ::std::atomic_bool m_fileOpenedFlag{ false };
        ::std::atomic_bool m_startStopLogging{ false };
        ::std::atomic_bool m_isSelectedDev{ false };
    }; //end class Logger

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

    private: /*--- Implementation ---*/
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
        bool isSending()
        {
            return m_isSending.load();
        }
        void setPeriod(CString& period);
        void setPeriod(int32_t period)
        {
            m_period = period;
        }
        int32_t getPeriod()
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

        
        ::std::list< CallBack > m_callBacks;
        ::std::future<void> m_future;
        ::std::atomic_bool m_sendingOnce{ false };
        ::std::atomic_bool m_stopSending{ false };
        ::std::atomic_bool m_isSending{ false };

    };

} //end namespace

#endif