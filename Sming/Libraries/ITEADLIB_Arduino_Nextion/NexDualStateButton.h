/**
 * @file NexDualStateButton.h
 *
 * The definition of class NexDSButton. 
 *
 * @author  huang xianming (email:<xianming.huang@itead.cc>)
 * @date    2015/11/11
 *   
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __NEXDSBUTTON_H__
#define __NEXDSBUTTON_H__

#include "NexTouch.h"
#include "NexHardware.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexDSButton component. 
 *
 * Commonly, you want to do something after push and pop it. It is recommanded that only
 * call @ref NexTouch::attachPop to satisfy your purpose. 
 * 
 * @warning Please do not call @ref NexTouch::attachPush on this component, even though you can. 
 */
class NexDSButton: public NexTouch
{
public: /* methods */
    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name);
     */
    NexDSButton(uint8_t pid, uint8_t cid, const char *name);
    
    /**
     * Get number attribute of component.
     *
     * @param number - buffer storing text returned. 
     * @return The real length of text returned. 
     */
    bool getValue(uint32_t *number);
    
    /**
     * Set number attribute of component.
     *
     * @param number - number buffer. 
     * @return true if success, false for failure. 
     */
    bool setValue(uint32_t number);
	
    /**
     * Get text attribute of component.
     *
     * @param buffer - buffer storing text returned. 
     * @param len - length of buffer. 
     * @return The real length of text returned. 
     */
    uint16_t getText(char *buffer, uint16_t len);
	
    /**
     * Set text attribute of component.
     *
     * @param buffer - text buffer terminated with '\0'. 
     * @return true if success, false for failure. 
     */
    bool setText(const char *buffer);
	
    /**
     * Get bco0 attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_state0_color_bco0(uint32_t *number);
	
    /**
     * Set bco0 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_state0_color_bco0(uint32_t number);
	
    /**
     * Get bco1 attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_state1_color_bco1(uint32_t *number);	

    /**
     * Set bco1 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_state1_color_bco1(uint32_t number);	

    /**
     * Get pco attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_font_color_pco(uint32_t *number);	

    /**
     * Set pco attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_font_color_pco(uint32_t number);		

    /**
     * Get xcen attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_place_xcen(uint32_t *number);		
	
    /**
     * Set xcen attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_place_xcen(uint32_t number);			
	
    /**
     * Get ycen attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_place_ycen(uint32_t *number);	

    /**
     * Set ycen attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_place_ycen(uint32_t number);		

    /**
     * Get font attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t getFont(uint32_t *number);		
	
    /**
     * Set font attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool setFont(uint32_t number);		

    /**
     * Get picc0 attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */	
    uint32_t Get_state0_crop_picc0(uint32_t *number);

    /**
     * Set picc0 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_state0_crop_picc0(uint32_t number);			
	
    /**
     * Get picc1 attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_state1_crop_picc1(uint32_t *number);	
	
    /**
     * Set picc1 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_state1_crop_picc1(uint32_t number);	

    /**
     * Get pic0 attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_state0_image_pic0(uint32_t *number);	

    /**
     * Set pic0 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_state0_image_pic0(uint32_t number);	

    /**
     * Get pic1 attribute of component
     *
     * @param number - buffer storing data retur
     * @return the length of the data 
     */
    uint32_t Get_state1_image_pic1(uint32_t *number);		
	
    /**
     * Set pic1 attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_state1_image_pic1(uint32_t number);			
};
/**
 * @}
 */



#endif /* #ifndef __NEXDSBUTTON_H__ */
