#include "driver/DeltaDoreDeviceInfo.h"



DeltaDoreDeviceInfo::DeltaDoreDeviceInfo()
{
    m_name[0] = 0;
    m_region[0] = 0;
    m_onLine = false;
    m_deviceType = DEVICE_TYPE_UNKNOWN;
    printf("*********************** Create new deviceInfo\n");
}

DeltaDoreDeviceInfo::~DeltaDoreDeviceInfo()
{
    //dtor
}
