#include "ftdi.h"

using namespace FTDI;

void FtdiHandler::notifyAll(const EventCode& event,
    const Data& data)
{
    for (const auto& func : m_callBacks)
        func(event, data);
}

void FtdiHandler::startScan()
{
    m_stopScan.store(false);
    auto work = [&]() mutable
    {
        while (!m_stopScan.load())
        {
            findFtdiDevices();
            ::std::this_thread::sleep_for(::std::chrono::milliseconds(SCAN_PERIOD_MS));
        }
    };
    m_future = ::std::async(::std::launch::async, work);
}
void FtdiHandler::stopScan()
{
    m_stopScan.store(true);
}


void FtdiHandler::startReadDev(Node& node)
{
    DevDescription dev_description = makeDevDescription(node);
    m_loggerMap.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(dev_description),
        std::forward_as_tuple(node));

    auto& logger = m_loggerMap.at(dev_description);
    logger.setFileName(utf8ToUtf16({ dev_description + ".uio" }).c_str());
    if (logger.startReading() != 0)
    {
        ::std::cerr << "Can't start logging of "
            << dev_description << ::std::endl;
    }
}

void FtdiHandler::stopReadDev(Node& node)
{
    DevDescription dev_description = makeDevDescription(node);
    try
    {
        auto& logger = m_loggerMap.at(dev_description);
        logger.stop();
        if (m_loggerMap.erase(dev_description) != 1)
        {
            ::std::cerr << "More than one element was erased" << ::std::endl;
        }
    }
    catch (::std::out_of_range& oor)
    {
        ::std::cout << "No device named : " << dev_description << ::std::endl;
    }
}

void FtdiHandler::stopLogging()
{
    for (auto& pair : m_loggerMap)
    {
        pair.second.stopLogging();
    }
}

void FtdiHandler::abort()
{
    stopScan();
    for (auto& pair : m_loggerMap)
    {
        pair.second.stopLogging();
        pair.second.stop();
    }
    m_loggerMap.clear();
}

INT FtdiHandler::findFtdiDevices()
{
    ////////////////////////////////////////////////
    // Form array of devices presented in the system
    ////////////////////////////////////////////////
    //::std::cout << "Looking for the FTDI devices" << ::std::endl;
    DWORD dev_ammnt;
    /* This call builds a device information list
    and returns the number of D2XX devices connected to the system. */
    FT_STATUS ftdi_stat = FT_CreateDeviceInfoList(&dev_ammnt);
    if (ftdi_stat != FT_OK)
    {
        ::std::cerr << "Device enumeration failed. "
            << "Error code : " << int(ftdi_stat) << ::std::endl;
        m_devDescriptions.clear();
        return -1;
    }
    if (dev_ammnt == 0)
    {
        ::std::cout << "No FTDI devices in the system" << ::std::endl;
        m_devDescriptions.clear();
        return -1;
    }

    DevNodes nodes_tmp(dev_ammnt);
    // This call fills the allocated list of devices
    ftdi_stat = FT_GetDeviceInfoList(nodes_tmp.data(), &dev_ammnt);
    if (ftdi_stat != FT_OK)
    {
        ::std::cerr << "Gathering info about devices failed. "
            << "Error code : " << ftdi_stat << ::std::endl;
        m_devDescriptions.clear();
        return -1;
    }

    if (mergeDevsList(nodes_tmp) == true)
    {
        //Indirect access to the resources of other objects
        notifyAll(EventCode::SCAN_DATA, m_devDescriptions);
    }

    return 0;
}

//Uniform rule to create description
DevDescription FtdiHandler::makeDevDescription(Node& node)
{
    ::std::stringstream dev_description;
    dev_description << node.Description
                    << " SN: " << node.SerialNumber;
    return dev_description.str();
}

BOOL FtdiHandler::mergeDevsList(DevNodes& devs)
{
    BOOL changes = false;
    m_devDescriptions.clear();
    //Add new devices
    for (auto& new_dev : devs)
    {
        DevDescription dev_description = makeDevDescription(new_dev);
        m_devDescriptions.emplace_back(dev_description); //form readable devs list

        auto search = m_devDescriptionMap.find(dev_description);
        if (search == m_devDescriptionMap.end()) //new device
        {
            ::std::cout << "Adding new device '" << dev_description
                << "' to the map" << ::std::endl;
            m_devDescriptionMap.insert({dev_description, new_dev });
            changes = true;
            //after all
            startReadDev(m_devDescriptionMap.at(dev_description));
        }
        else //device present
        {
            //::std::cout << "Device '" << dev_description
            //    << "' already added" << ::std::endl;
        }
    } //end for

    //Remove absent devices
    //Look new formed descriptions for absent old one
    for (auto dev_map_it = m_devDescriptionMap.begin();\
        dev_map_it != m_devDescriptionMap.end(); )
    {
        DevDescriptions::iterator search_res = ::std::find(\
            m_devDescriptions.begin(), m_devDescriptions.end(), dev_map_it->first);
        if (search_res == m_devDescriptions.end())
        {
            ::std::cout << "Device '" << dev_map_it->first
                << "' was removed from the system" << ::std::endl;
            //before all
            stopReadDev(dev_map_it->second);
            if (dev_map_it == m_selDev)
            {
                //m_devRefCntr = 0;
                //closeSelDevice();
                m_selDev = m_devDescriptionMap.end();
            }
            //Returns tterator following the last removed element
            dev_map_it = m_devDescriptionMap.erase(dev_map_it);
            changes = true;
        }
        else
        {
            //::std::cout << "Device '" << dev_map_it->first
            //    << "' present" << ::std::endl;
            ++dev_map_it;
        }
    }
    return changes;
}

void FtdiHandler::printFtdiDevices()
{
/*
typedef struct _ft_device_list_info_node {
    ULONG Flags;
    ULONG Type;
    ULONG ID;
    DWORD LocId;
    char SerialNumber[16];
    char Description[64];
    FT_HANDLE ftHandle;
} FT_DEVICE_LIST_INFO_NODE;
*/
    if (m_devDescriptionMap.empty())
    {
        ::std::cout << "No devices in the system" << ::std::endl;
        return;
    }

    for (auto& dev_pair : m_devDescriptionMap)
    {
        ::std::cout
            << "Flags        : " << dev_pair.second.Flags << '\n'
            << "Type         : " << dev_pair.second.Type << '\n'
            << "ID           : " << dev_pair.second.ID << '\n'
            << "LocId        : " << dev_pair.second.LocId << '\n'
            << "SerialNumber : " << dev_pair.second.SerialNumber << '\n'
            << "Description  : " << dev_pair.second.Description << '\n'
            << "Type         : " << dev_pair.second.Type << '\n'
            << ::std::endl;
    } //end for
}

void FtdiHandler::setSelDev(::std::string dev_description)
{
    DevDescriptionMap::iterator new_sel_dev = m_devDescriptionMap.find(dev_description);
    if (new_sel_dev == m_devDescriptionMap.end())
    {
        ::std::cerr << "Device '" << dev_description << "' isn't found" << ::std::endl;
        return;
    }
    notifyAll(EventCode::NEW_DEV_SELECTED, Data{});
    stopLogging();
    m_selDev = new_sel_dev;
    ::std::cout << "Selected device : " << dev_description << ::std::endl;
    try
    {
        auto& logger = m_loggerMap.at(dev_description);
        logger.startLoging();
    }
    catch (::std::out_of_range& oor)
    {
        ::std::cout << "No device named : " << dev_description << ::std::endl;
    }
}

int32_t FtdiHandler::sendData(::std::vector<char>& data)
{
    FT_STATUS ftdi_stat = FT_OTHER_ERROR;
    DWORD BytesWritten = 0;

    if (m_selDev == m_devDescriptionMap.end())
    {
        ::std::cerr << "Device isn't selected" << ::std::endl;
        return -1;
    }
    if (m_selDev->second.ftHandle == nullptr)
    {
        ::std::cerr << "Device isn't opened" << ::std::endl;
        return -1;
    }
    if (data.empty())
    {
        ::std::cerr << "Send buffer is empty" << ::std::endl;
        return -1;
    }

    {
        ::std::unique_lock<::std::mutex> u_lock(m_sendMtx);
        ftdi_stat = FT_Write(m_selDev->second.ftHandle,\
            data.data(), data.size(), &BytesWritten);
    }
    if (ftdi_stat != FT_OK)
    {
        ::std::cerr << "Can't send data to the device " << m_selDev->first << '\n'
                    << "Error : " << ftdi_stat <<::std::endl;
        return -1;
    }
    if (BytesWritten != data.size())
    {
        ::std::cerr << "Error : " << BytesWritten
                    << " bytes out of required " << data.size()
                    << "is written" << ::std::endl;
        return -1;
    }
    return 0;

}


