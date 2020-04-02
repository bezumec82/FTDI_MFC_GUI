#include "pch.h"
#include "ftdi.h"

using namespace FTDI;

void FtdiHandler::findFTDIDevices()
{
    if (isLocked()) 
    {
        ::std::cerr << "Can't start looking for another device."
                    << "Work in process." << ::std::endl;
        return;
    }
    ::std::cout << "Looking for the FTDI devices" << ::std::endl;

    /* This call builds a device information list
    and returns the number of D2XX devices connected to the system. */
    m_ft_status = FT_CreateDeviceInfoList(&m_num_devs);
    if (m_ft_status != FT_OK)
    {
        ::std::cerr << "Device enumeration failed. "
                    << "Error code : "<< int(m_ft_status) << ::std::endl;
    }
    if (m_num_devs == 0)
    {
        ::std::cout << "No FTDI devices in the system" << ::std::endl;
        return;
    }
    m_dev_list_uptr.swap( ::std::make_unique<FT_DEVICE_LIST_INFO_NODE[]>(m_num_devs) );

    /* This call fills the allocated list of devices */
    m_ft_status = FT_GetDeviceInfoList(m_dev_list_uptr.get(), &m_num_devs);
    if (m_ft_status != FT_OK)
    {
        ::std::cerr << "Gathering info about devices failed. "
                    << "Error code : " << m_ft_status << ::std::endl;
        m_num_devs = 0;
    }
    else
    {
        //fill map
        m_devDescriptionMap.clear();
        m_devDescriptions.clear();
        for (DWORD dev_idx = 0; dev_idx < m_num_devs; dev_idx++)
        {
            ::std::stringstream dev_description;
            dev_description << m_dev_list_uptr[dev_idx].Description
                            << " SN: " << m_dev_list_uptr[dev_idx].SerialNumber 
                            << ::std::endl;
            m_devDescriptions.push_back(dev_description.str());
            m_devDescriptionMap.insert( 
                { ::std::move(dev_description.str()), m_dev_list_uptr[dev_idx] } );

        }
    }
}

void FtdiHandler::printFTDIDevices()
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
    if (!m_num_devs)
    {
        ::std::cout << "No devices in the system" << ::std::endl;
        return;
    }

    for (DWORD dev_idx = 0; dev_idx < m_num_devs; dev_idx++)
    {
        ::std::cout
            << "Flags        : " << m_dev_list_uptr[dev_idx].Flags << '\n'
            << "Type         : " << m_dev_list_uptr[dev_idx].Type << '\n'
            << "ID           : " << m_dev_list_uptr[dev_idx].ID << '\n'
            << "LocId        : " << m_dev_list_uptr[dev_idx].LocId << '\n'
            << "SerialNumber : " << m_dev_list_uptr[dev_idx].SerialNumber << '\n'
            << "Description  : " << m_dev_list_uptr[dev_idx].Description << '\n'
            << "Type         : " << m_dev_list_uptr[dev_idx].Type << '\n'
            << ::std::endl;
    } //end for
}

#if(0)
void FtdiHandler::fillComboBox(CComboBox& cbox)
{
    cbox.ResetContent();
    if (!m_num_devs) 
    { 
        ::std::cout << "No devices in the system" << ::std::endl; 
        return;
    }
#if(1)
    for (::std::pair< ::std::string, FT_DEVICE_LIST_INFO_NODE > elem : m_devDescriptionMap)
    {
        cbox.AddString( CString{ elem.first.c_str() } );
    }
#else
    for (DWORD dev_idx = 0; dev_idx < m_num_devs; dev_idx++)
    {
        ::std::wstringstream devices;
        devices << m_dev_list_uptr[dev_idx].Description
                << " SN: " << m_dev_list_uptr[dev_idx].SerialNumber << ::std::end;
        cbox.AddString(devices.str().c_str());
    }
#endif
}
#endif

void FtdiHandler::setSelDev(::std::string desc)
{
    if (!isLocked())
    {
        m_selDevDescription = desc;
        ::std::cout << "Selected device : "
            << m_selDevDescription << ::std::endl;
    }
    else
        ::std::cerr << "Can't set another device."
        << "Work in process." << ::std::endl;
}

int32_t FtdiHandler::openDevice()
{
    
    if (!m_num_devs)
    {
        ::std::cerr << "No devices in the system" << ::std::endl;
        return -1;
    }
    if (m_selDevDescription == "")
    {
        ::std::cerr << "Device is not selected" << ::std::endl;
        return -1;
    }
    if (isLocked())
    {
        m_devRefCntr++;
        ::std::cout << "Device already opened" << ::std::endl;
        return 0;
    }

    try
    {
        FT_DEVICE_LIST_INFO_NODE node = m_devDescriptionMap.at(m_selDevDescription);
        m_ft_status = FT_OpenEx(
            node.SerialNumber,
            FT_OPEN_BY_SERIAL_NUMBER,
            &node.ftHandle);
        if (m_ft_status != FT_OK)
        {
            ::std::cout << "Can't open device : " << m_selDevDescription << '\n'
                        << "Error : " << m_ft_status << ::std::endl;
            return -1;
        }
        else
        {
            m_deviceIsLocked.store(true);
            m_devDescriptionMap.at(m_selDevDescription) = node; //rewrite back acquired handle
            m_selDevHandle = node.ftHandle;
            ::std::cout << "Opened device : " << m_selDevDescription << ::std::endl;
            return 0;
        }
    }
    catch (const ::std::out_of_range oor)
    {
        ::std::cout << "No device " << m_selDevDescription << ::std::endl;
        ::std::cout << oor.what() << ::std::endl;
        return -1;
    }
}

void FtdiHandler::closeDevice()
{
    if (!m_num_devs)
    {
        ::std::cout << "No devices in the system" << ::std::endl;
        return;
    }
    if (m_devRefCntr != 0)
    {
        m_devRefCntr--;
        ::std::cout << "Device stil in use" << ::std::endl;
        return;
    }

    try
    {
        FT_DEVICE_LIST_INFO_NODE node = m_devDescriptionMap.at(m_selDevDescription);
        m_ft_status = FT_Close(node.ftHandle);
        if (m_ft_status != FT_OK)
        {
            ::std::cerr << "Can't close device " << m_selDevDescription << '\n'
                        << "Error : " << m_ft_status << ::std::endl;
        }
        else
        {
            m_devDescriptionMap.at(m_selDevDescription) = node; //rewrite back changed handle
            m_selDevHandle = node.ftHandle;
            m_deviceIsLocked.store(false);
            ::std::cout << "Closed device : " << m_selDevDescription << ::std::endl;
        }
    }
    catch (const ::std::out_of_range oor)
    {
        ::std::cout << "No device " << m_selDevDescription << ::std::endl;
        ::std::cout << oor.what() << ::std::endl;
        return;
    }
}

//Device must be opened before use this function
void FtdiHandler::sendData(::std::vector<char>& data)
{
    if (!m_num_devs)
    {
        ::std::cout << "No devices in the system" << ::std::endl;
        return;
    }
    if (m_selDevHandle == nullptr)
    {
        ::std::cerr << "Device isn't opened" << ::std::endl;
        return;
    }
    if (data.empty())
    {
        ::std::cerr << "Send buffer is empty" << ::std::endl;
        return;
    }

    DWORD BytesWritten = 0;
    {
        ::std::unique_lock<::std::mutex> u_lock(m_sendMtx);
        m_ft_status = FT_Write(m_selDevHandle, data.data(), data.size(), &BytesWritten);
    }
    if (m_ft_status != FT_OK)
    {
        ::std::cerr << "Can't send data to the device " << m_selDevDescription << '\n'
                    << "Error : " << m_ft_status <<::std::endl;
    }
    if (BytesWritten != data.size())
    {
        ::std::cerr << "Error : " << BytesWritten
                    << " bytes out of required " << data.size() 
                    << "is written" << ::std::endl;
    }

}

void FtdiHandler::recvData(::std::vector<char>& buffer)
{
    DWORD EventDWord;
    DWORD TxBytes; DWORD RxBytes;
    DWORD BytesReceived;

    m_ft_status = FT_GetStatus(m_selDevHandle, &RxBytes, &TxBytes, &EventDWord);
    if (RxBytes <= 0) return;

    buffer.resize(RxBytes);
    m_ft_status = FT_Read(m_selDevHandle, buffer.data(), buffer.size(), &BytesReceived);
    if (m_ft_status != FT_OK)
    {
        ::std::cerr << "Can't read data from the device " << m_selDevDescription << '\n'
                    << "Error : " << m_ft_status << ::std::endl;
    }
    if (BytesReceived != RxBytes)
    {
        ::std::cerr << "Error : " << BytesReceived
                    << " bytes out of declared " << RxBytes 
                    << "is received" << ::std::endl;
    }
    
}

