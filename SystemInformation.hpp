#ifndef CPU_INFO_SYSTEMINFORMATION_H
#define CPU_INFO_SYSTEMINFORMATION_H
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "sys/times.h"

class ISystemInformation{
public:
    ISystemInformation() = default;
    virtual ~ISystemInformation() = default;

    /**
	* @brief Get Total Available Memory Bytes
	*
	* \return
	*/
    virtual int64_t GetTotalMemory() = 0;


    /**
    * @brief Get Total Usage Memory Bytes at System
    *
    * \return
    */
    virtual int64_t GetTotalUsageMemory() = 0;


    virtual double GetCpuTotalUsage() = 0;
};


class SystemInformation: public ISystemInformation{
    
public:
    SystemInformation();
    virtual ~SystemInformation() override;

    int64_t GetTotalMemory() override;
    int64_t GetTotalUsageMemory() override;
    double GetCpuTotalUsage() override;

private:
    struct PImpl;

    PImpl* m_impl;
};


class IProcessInfo{
public:
    IProcessInfo() = default;
    virtual ~IProcessInfo() = default;

    /*!
     * Get Current % Cpu Usage
     * @return percent
     */
   virtual double GetCpuUsage() = 0;
   /*!
    * @brief Get Current Memory Usage
    * @return bytes
    */
   virtual int64_t GetMemoryUsage() = 0;
};


class ProcessInfo : public IProcessInfo{
public:
    ProcessInfo();
    ~ProcessInfo() override;

    /*!
     * @copydoc IProcessInfo
     * @return percent;
     */
    double GetCpuUsage() override;
    /*!
     * @copydoc IProcessInfo
     * @return bytes;
     */
    int64_t GetMemoryUsage() override;

private:
    struct PImpl;

    PImpl* m_impl;
};



#endif //CPU_INFO_SYSTEMINFORMATION_H
