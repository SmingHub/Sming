/**
 * @file NexRtc.h
 *
 * The definition of class NexRtc. 
 *
 * @author Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date 2015/8/13
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _NEXRTC_H
#define _NEXRTC_H
     
#include "NexTouch.h"
#include "NexHardware.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexRtc component.
 */

class NexRtc
{
    public:

    bool write_rtc_time(char *time);
    
    /**
     * write rtc times
     *
     * @param time_type - To type in time   (example:write_rtc_time("year",2016))
     * @param number - the time value
     * @return true if success, false for failure
     */
    
    bool write_rtc_time(char *time_type,uint32_t number);
    
    /**
     * write rtc times
     *
     * @param time - Time to write to the array
     * @return true if success, false for failure
     */
    
    bool write_rtc_time(uint32_t *time);
    
    
    /**
     * read rtc time
     *
     * @param time - Access data array
     * @param len - len of array
     * @return true if success, false for failure
     */
    
    uint32_t read_rtc_time(char *time,uint32_t len);
    
    /**
     * read rtc times
     *
     * @param time_type - To type in time   
     * @param number - the time value
     * @return true if success, false for failure
     */
    
    uint32_t read_rtc_time(char *time_type,uint32_t *number);
    
    /**
     * read rtc time
     *
     * @param time - Access data array
     * @param len - len of array
     * @return true if success, false for failure
     */
    
    uint32_t read_rtc_time(uint32_t *time,uint32_t len);
    
};

/**
 * @}
 */

#endif /* #ifndef __NEXRTC_H__ */