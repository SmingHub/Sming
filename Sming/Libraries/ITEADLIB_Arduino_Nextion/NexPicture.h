/**
 * @file NexPicture.h
 *
 * The definition of class NexPicture. 
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

#ifndef __NEXPICTURE_H__
#define __NEXPICTURE_H__

#include "NexTouch.h"
#include "NexHardware.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexPicture component. 
 */
class NexPicture: public NexTouch
{
public: /* methods */
    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexPicture(uint8_t pid, uint8_t cid, const char *name);
    
    /**
     * Get picture's number.
     * 
     * @param number - an output parameter to save picture number.  
     * 
     * @retval true - success. 
     * @retval false - failed. 
     */
    bool Get_background_image_pic(uint32_t *number);
    
    /**
     * Set picture's number.
     * 
     * @param number -the picture number.
     *
     * @retval true - success.
     * @retval false - failed. 
     */
    bool Set_background_image_pic(uint32_t number);
	
	/**
     * Get picture's number.
     * 
     * @param number - an output parameter to save picture number.  
     * 
     * @retval true - success. 
     * @retval false - failed. 
     */
    bool getPic(uint32_t *number);
    
    /**
     * Set picture's number.
     * 
     * @param number -the picture number.
     *
     * @retval true - success.
     * @retval false - failed. 
     */
    bool setPic(uint32_t number);
};

/**
 * @}
 */

#endif /* #ifndef __NEXPICTURE_H__ */
