/**
 * @file NexCrop.h
 *
 * The definition of class NexCrop. 
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

#ifndef __NEXCROP_H__
#define __NEXCROP_H__

#include "NexTouch.h"
#include "NexHardware.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexCrop component. 
 */
class NexCrop: public NexTouch
{
public: /* methods */

    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexCrop(uint8_t pid, uint8_t cid, const char *name);

    /**
     * Get the number of picture. 
     *
     * @param number - an output parameter to save the number of picture. 
     * 
     * @retval true - success. 
     * @retval false - failed. 
     */
    bool Get_background_crop_picc(uint32_t *number);
    
    /**
     * Set the number of picture. 
     *
     * @param number - the number of picture. 
     * 
     * @retval true - success. 
     * @retval false - failed. 
     */
    bool Set_background_crop_picc(uint32_t number);
	
	/**
     * Get the number of picture. 
     *
     * @param number - an output parameter to save the number of picture. 
     * 
     * @retval true - success. 
     * @retval false - failed. 
     */
    bool getPic(uint32_t *number);
    
    /**
     * Set the number of picture. 
     *
     * @param number - the number of picture. 
     * 
     * @retval true - success. 
     * @retval false - failed. 
     */
    bool setPic(uint32_t number);
};

/**
 * @}
 */

#endif /* #ifndef __NEXCROP_H__ */
