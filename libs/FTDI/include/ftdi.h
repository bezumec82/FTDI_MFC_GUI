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
#include <queue>

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
    /*--- Forward declarations ---*/
    class Logger;
    class Writer;

    /*--- Aliaces ---*/
    using Node = FT_DEVICE_LIST_INFO_NODE;
    using DevNodes = ::std::vector<Node>;
    using DevDescription = ::std::string;
    using DevDescriptions = ::std::vector < ::std::string >;

    /*--- Event system ---*/
    enum class EventCode : int8_t
    {
        ALL_GOOD            = 0,
        /*--- Writers' codes ---*/
        NO_PERIOD_ERR       = -10,
        WRITE_FOPEN_ERR     = -11,
        WRITE_STOPPED       = 10,
        MEDIUM_TX_RATE      = 11,
        /*--- Loggers' codes */
        WRITE_NO_FNAME_ERR  = -20,
        READ_FOPEN_ERR      = -21,
        MEDIUM_RX_RATE      = 20,
        READ_STOPPED        = 21,
        IMMEDIATE_RX_RATE   = 22,

        /*--- Handler codes ---*/
        DEV_LIST            = 1, //changed list of devices after successful merge
        NEW_DEV_SELECTED    = 2,
        IMMEDIATE_TX_RATE   = 5,
    };

    using Data = ::std::variant<
        DevDescriptions, /* to CBox */
        CString
    >;

    using CallBack =
        void(const EventCode&, const Data&);
    using CallBacks = ::std::list< ::std::function<CallBack> >;


    class EventBuffer
    {
    public: /*--- Aliases ---*/
        using Event = ::std::pair<EventCode, Data>;
        using Events = ::std::queue<Event>;
    private: /*--- Implementation ---*/
        void doBuffer();

    public:
        EventBuffer() {}
        void registerCallBack(::std::function <CallBack>);
        CallBack receiveEvent;
        void start();
        void stop();
        bool isWorking()
        {
            return m_isWorking.load();
        }

    private:
        UINT BUFFER_DELAY_MS = 20;
        Events m_events;
        ::std::future<void> m_future;
        CallBacks m_callBacks;
        ::std::mutex m_callBacksLock;
        ::std::mutex m_eventsLock;
        ::std::atomic_bool m_stop{ true };
        ::std::atomic_bool m_isWorking{ false };
    };


    class FtdiHandler
    {
        friend class Writer;
        friend class Logger;
    public: /*--- Aliaces ---*/
        using DevDescriptionMap = ::std::unordered_map< DevDescription, Node >;
        using LoggerMap = ::std::unordered_map< DevDescription, Logger >;
        using Buffer = ::std::vector<char>;

    private: /*--- Constructor ---*/
        FtdiHandler()
        {
            m_selDev = m_devDescriptionMap.end();
        }
    public:
        static FtdiHandler& getInstance()
        {
            static FtdiHandler instance;
            return instance;
        }

    private: /*--- Implementation ----*/
        //merging device lists
        BOOL mergeDevsList(DevNodes& devs);
        void startReadDev(Node&);
        void stopReadDev(Node&);
        void notifyAll(const EventCode&, const Data&);
        void stopScan();
        INT sendData(::std::vector<char>&);
        LONG getTxQueueSize();

    public: /*--- Methods ---*/
        static DevDescription makeDevDescription(Node&);
        void startScan();
        INT findFtdiDevices();
        void printFtdiDevices();
        LONGLONG sendFile(CFile&, ::std::atomic_bool&);
        void stopLogging();
        void stopSend();
        void abort();
        void registerCallBack(::std::function <CallBack>);

        /*--- Getters/Setters ---*/
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
        DevDescriptions m_devDescriptions;
        DevDescriptionMap m_devDescriptionMap;
        DevDescriptionMap::iterator m_selDev;
        LoggerMap m_loggerMap;
        ::std::mutex m_devMtx;

    private: /*--- Flags ---*/
        ::std::atomic_bool m_stopScan{ false };
        ::std::atomic_bool m_stopSend{ false };
        ::std::atomic_bool m_isSending{ false };
    };

    /*--------------*/
    /*--- Logger ---*/
    /*--------------*/
    //This class assigned to the device,
    //it reads device continiously and starts
    //writing to file by command
    class Logger
    {
    public: /*--- Constructors ---*/
        Logger(Node& node, //node to work on
            FtdiHandler& parent) :
            m_node_ref{ node },
            m_ftdiHandler_ref{ parent }
        {
            m_devDescription = FtdiHandler::makeDevDescription(m_node_ref);
        }
        ~Logger()
        {
            stop();
        }

    private:/*--- Implementation ---*/
        INT openFile();
        INT openDevice();
        void closeDevice();
        INT clearRxBuf();
        LONG recvData(::std::vector<char>&);
        INT doReading();
        void doLogging(::std::vector<char>&);

    public: /*--- Methods ---*/
        INT startReading();
        bool isReading()
        {
            return m_isReading.load();
        }
        void startLogging()
        {
            m_startStopLogging.store(true);
        }
        void stopLogging()
        {
            m_startStopLogging.store(false);
        }
        void stop();

    public: /*--- Getters/Setters ---*/
        void setFileName(CString fileName)
        {
            m_fileName = fileName;
            m_fileName.Replace(L':', L'_');
        }
        const CString& getFileName()
        {
            return m_fileName;
        }
        void setAsSelDev()
        {
            m_isSelDev.store(true);
        }
        void setAsUnSelDev()
        {
            m_isSelDev.store(false);
        }

    private: /*--- Variables ---*/
        Node& m_node_ref;
        FtdiHandler& m_ftdiHandler_ref;
        DevDescription m_devDescription;
        CString m_fileName;
        CFile m_saveFile;
        ::std::future<void> m_future;

    private: /*--- Flags ---*/
        ::std::atomic_bool m_startStopReading{ false };
        ::std::atomic_bool m_isReading{ false };
        ::std::atomic_bool m_fileOpenedFlag{ false };
        ::std::atomic_bool m_startStopLogging{ false };
        ::std::atomic_bool m_isSelDev{ false };
    }; //end class Logger

    /*--------------*/
    /*--- Writer ---*/
    /*--------------*/
    class Writer
    {
    public: /*--- Constructor ---*/
        Writer(FtdiHandler& ftdi_handler)
            : m_ftdiHandler_ref{ ftdi_handler }
        { }



    private: /*--- Implementation ---*/
        void sendOnce();
        void doSend();
        INT openFile(CString&, CFile&);
        void rewindFile(CFile&);

    private: /*--- Notification mechanism ---*/
        void notifyAll(const EventCode& event,
            const Data& data);
    public: /*--- Notification mechanism ---*/
        void registerCallBack(::std::function<CallBack>);

    public: /*--- Methods ---*/
        void start();
        void stop();

    public: /*--- Getters/Setters ---*/
        bool isSending()
        {
            return m_isSending.load();
        }
        void setPeriod(CString& period);
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
        int32_t m_period{ -1 };
        ::std::future<void> m_future;
        CallBacks m_callBacks;
        CString m_fileName;

    private: /*--- Flags ---*/
        ::std::atomic_bool m_sendingOnce{ false };
        ::std::atomic_bool m_stopSend{ false };
        ::std::atomic_bool m_isSending{ false };
    };

} //end namespace

#endif