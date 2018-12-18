/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "driver/slc_register.h"
#include "driver/sdio_slv.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
//#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

#define SDIO_TOKEN_SIZE		0//4
#define RX_BUFFER_SIZE      2048
#define RX_BUFFER_NUM       5

#define TX_BUFFER_SIZE		2048
#define SLC_INTEREST_EVENT (SLC_TX_EOF_INT_ENA | SLC_RX_EOF_INT_ENA | SLC_RX_UDF_INT_ENA | SLC_TX_DSCR_ERR_INT_ENA)
#define TRIG_TOHOST_INT()	SET_PERI_REG_MASK(SLC_INTVEC_TOHOST , BIT0);\
							CLEAR_PERI_REG_MASK(SLC_INTVEC_TOHOST , BIT0)
struct sdio_queue
{
	uint32	blocksize:12;
	uint32	datalen:12;
	uint32	unused:5;
	uint32	sub_sof:1;
	uint32 	eof:1;
	uint32	owner:1;

	uint32	buf_ptr;
	uint32	next_link_ptr;
};

struct sdio_slave_status_element
{
	uint32 wr_busy:1;
	uint32 rd_empty :1;
	uint32 comm_cnt :3;
	uint32 intr_no :3;
	uint32 rx_length:16;
	uint32 res:8;
};

union sdio_slave_status
{
	struct sdio_slave_status_element elm_value;
	uint32 word_value;
};

//uint8 rx_buffer[RX_BUFFER_NUM][RX_BUFFER_SIZE],tx_buffer[1024];
uint8 tx_buffer[TX_BUFFER_SIZE];

uint32 data_len = 0;

struct sdio_list {
	uint8 buffer[RX_BUFFER_SIZE + SDIO_TOKEN_SIZE];
	uint8* tail;
	struct sdio_list* next;
};

static sdio_recv_data_callback_t sdio_recv_data_callback_ptr = NULL;
struct sdio_list* pHead_ToSend;
struct sdio_list* pTail_ToSend;
struct sdio_list* pHead_Sended;
struct sdio_list* pTail_Sended;



os_event_t * sdioQueue;
struct sdio_queue rx_que,tx_que;

static bool has_read = 0;

static void sdio_slave_isr(void *para);
static void tx_buff_handle_done(void);
static void rx_buff_read_done(void);
static void tx_buff_write_done(void);

static void sdio_try_to_load(void);
static void sdio_read_done_process(void);
#ifdef SDIO_DEBUG
extern uint32 sum_len;
#endif
void sdio_slave_init(void)
{
    uint32 regval = 0;
    union sdio_slave_status sdio_sta;
    ETS_SDIO_INTR_DISABLE();
	///
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U,FUNC_SDCLK);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U,FUNC_SDDATA0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U,FUNC_SDDATA1);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U,FUNC_SDDATA2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U,FUNC_SDDATA3);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U,FUNC_SDCMD);
	
	
    ////reset orginal link
    SET_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);
    CLEAR_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);

    os_printf("RX&TX link reset!\n");

    //set sdio mode
    SET_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_RX_EOF_MODE | SLC_RX_FILL_MODE);
    //clear to host interrupt io signal for preventing from random initial signal.
    WRITE_PERI_REG(SLC_HOST_INTR_CLR, 0xffffffff);
    //enable 2 events to trigger the to host intr io
    SET_PERI_REG_MASK(SLC_HOST_INTR_ENA , SLC_HOST_TOHOST_BIT0_INT_ENA);
    ////initialize rx queue information

    has_read = TRUE;
	pHead_ToSend = NULL;

	int loop = RX_BUFFER_NUM;
	struct sdio_list* p = NULL;
	while(loop--) {
		if(pHead_Sended == NULL) {
			pHead_Sended = (struct sdio_list*)os_malloc(sizeof(struct sdio_list));
			p = pHead_Sended;
		} else {
			p->next = (struct sdio_list*)os_malloc(sizeof(struct sdio_list));
			p = p->next;
		}
		//os_printf("p:0x%08x\r\n",p);
		p->tail = p->buffer + SDIO_TOKEN_SIZE;
		p->next = NULL;
	}
	pTail_Sended = p;
	
    rx_que.blocksize = RX_BUFFER_SIZE;
    rx_que.datalen=0;
    rx_que.eof=1;
    rx_que.owner=1;
    rx_que.sub_sof=0;
    rx_que.unused=0;
    rx_que.buf_ptr=(uint32)pHead_Sended->buffer;
    rx_que.next_link_ptr=0;


    ////initialize tx queue information
    tx_que.blocksize=TX_BUFFER_SIZE;
    tx_que.datalen=0;
    tx_que.eof=0;
    tx_que.owner=1;
    tx_que.sub_sof=0;
    tx_que.unused=0;
    tx_que.buf_ptr=(uint32)tx_buffer;
    tx_que.next_link_ptr=0;

    ///////link tx&rx queue information address to sdio hardware
    CLEAR_PERI_REG_MASK(SLC_RX_LINK,SLC_RXLINK_DESCADDR_MASK);
    regval= ((uint32)&rx_que);
    SET_PERI_REG_MASK(SLC_RX_LINK, regval&SLC_RXLINK_DESCADDR_MASK);
    CLEAR_PERI_REG_MASK(SLC_TX_LINK,SLC_TXLINK_DESCADDR_MASK);
    regval= ((uint32)&tx_que);
    SET_PERI_REG_MASK(SLC_TX_LINK, regval&SLC_TXLINK_DESCADDR_MASK);

#if (SDIO_TOKEN_SIZE == 0)
    SET_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_TOKEN_NO_REPLACE);
#endif

    /////config sdio_status reg
    sdio_sta.elm_value.comm_cnt=7;
    sdio_sta.elm_value.intr_no=INIT_STAGE;
    sdio_sta.elm_value.wr_busy=0;
    sdio_sta.elm_value.rd_empty=1;
    sdio_sta.elm_value.rx_length=0;
    sdio_sta.elm_value.res=0;
    SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START);
    WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);


    /////attach isr func to sdio interrupt
    ETS_SDIO_INTR_ATTACH(sdio_slave_isr, NULL);
    /////enable sdio operation intr
    WRITE_PERI_REG(SLC_INT_ENA,  SLC_INTEREST_EVENT);
    /////clear sdio initial random active intr signal
    WRITE_PERI_REG(SLC_INT_CLR, 0xffffffff);
    /////enable sdio intr in cpu
    ETS_SDIO_INTR_ENABLE();
}

static void sdio_slave_isr(void *para)
{
    uint32 slc_intr_status,postval;
    static uint8 state =0;
    uint16 rx_len,i;
    uint32* pword;
    union sdio_slave_status sdio_sta;

    slc_intr_status = READ_PERI_REG(SLC_INT_STATUS);

    if (slc_intr_status == 0)
    {
        /* No interested interrupts pending */
        return;
    }
    //clear all intrs
    WRITE_PERI_REG(SLC_INT_CLR, slc_intr_status);
    //os_printf("slc_intr_status:0x%08x\r\n",slc_intr_status);
    //process every intr

    //TO HOST DONE
    if (slc_intr_status & SLC_RX_EOF_INT_ENA)
    {
        //following code must be called after a data pack has been read
        rx_buff_read_done();
		sdio_read_done_process();
    }

    //FROM HOST DONE
    if (slc_intr_status & SLC_TX_EOF_INT_ENA)
    {
        //call the following function after host cpu data transmission finished
        tx_buff_write_done();
		if(sdio_recv_data_callback_ptr) {
			if (sdio_recv_data_callback_ptr((uint8*)tx_que.buf_ptr,tx_que.datalen) == tx_que.datalen) {
				tx_buff_handle_done();
				TRIG_TOHOST_INT();
			}
		} else {
			tx_buff_handle_done();
			TRIG_TOHOST_INT();
		}
    }
#if 0
    //TO HOST underflow
    if(slc_intr_status & SLC_RX_UDF_INT_ENA)
    {
    }

    //FROM HOST overflow
    if(slc_intr_status & SLC_TX_DSCR_ERR_INT_ENA)
    {
    }

    slc_intr_status = READ_PERI_REG(SLC_INT_STATUS);
    if(slc_intr_status)
    {
        WRITE_PERI_REG(SLC_INT_CLR, slc_intr_status);
    }
#endif
}

static void rx_buff_read_done(void)
{
    union sdio_slave_status sdio_sta;
    /////modify sdio status reg
    sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
    sdio_sta.elm_value.comm_cnt++;
    sdio_sta.elm_value.rd_empty=1;
    sdio_sta.elm_value.rx_length=0;
    sdio_sta.elm_value.intr_no &= (~RX_AVAILIBLE);
    WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);  //update sdio status register
    //os_printf("rx_buff_read_done\r\n");
}

static void tx_buff_write_done(void)
{
    union sdio_slave_status sdio_sta;
    /////modify sdio status reg
    sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
    sdio_sta.elm_value.comm_cnt++;
    sdio_sta.elm_value.wr_busy=1;
	sdio_sta.elm_value.intr_no &= (~TX_AVAILIBLE);
    WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);  //update sdio status register
}

static void tx_buff_handle_done(void)
{
    union sdio_slave_status sdio_sta;

    /////config tx queue information
    tx_que.blocksize=TX_BUFFER_SIZE;
    tx_que.datalen=0;
    tx_que.eof=0;
    tx_que.owner=1;

    /////modify sdio status reg
    sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
    sdio_sta.elm_value.wr_busy=0;
    sdio_sta.elm_value.intr_no |= TX_AVAILIBLE;

    SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START);       //tx buffer is ready for being written
    WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);  //update sdio status register
    //*******************************************************************//

}
static int32 rx_buff_load_done(uint16 rx_len)
{
    union sdio_slave_status sdio_sta;

	if(rx_len == 0) {
		return 0;
	}
    if(rx_len > rx_que.blocksize)
    {
        rx_len = rx_que.blocksize;
    }

    //os_memcpy(rx_que.buf_ptr,data,rx_len);
    /////config rx queue information
    rx_que.blocksize=RX_BUFFER_SIZE;
    rx_que.datalen=rx_len + SDIO_TOKEN_SIZE;
    rx_que.eof=1;
    rx_que.owner=1;
#ifdef SDIO_DEBUG
	sum_len += rx_que.datalen;
#endif
    //ETS_SDIO_INTR_DISABLE();
    //available_buffer_amount--;

    /////modify sdio status reg
    sdio_sta.word_value=READ_PERI_REG(SLC_HOST_CONF_W2);
    sdio_sta.elm_value.rd_empty=0;
    sdio_sta.elm_value.intr_no |= RX_AVAILIBLE;
    sdio_sta.elm_value.rx_length=rx_len;

    SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START);       //rx buffer is ready for being read
    WRITE_PERI_REG(SLC_HOST_CONF_W2, sdio_sta.word_value);  //update sdio status register
    //ETS_SDIO_INTR_ENABLE();
    return rx_len;
}

int32 ICACHE_FLASH_ATTR sdio_load_data(const uint8* data,uint32 len)
{
    int32 data_len = 0;
	struct sdio_list* temp_list;
	if (pHead_Sended == NULL) {
		//os_printf("no buf\r\n");
		return 0;
	}
    int32 left_len = 0;
    while(len)
    {
		ETS_SDIO_INTR_DISABLE();
		if(pHead_Sended == NULL)
        {
            //os_printf("buf full\r\n");
			ETS_SDIO_INTR_ENABLE();
            break;
        }
		temp_list = pHead_Sended;
		pHead_Sended = pHead_Sended->next;
		ETS_SDIO_INTR_ENABLE();
		left_len = RX_BUFFER_SIZE + SDIO_TOKEN_SIZE - (uint32)(temp_list->tail - temp_list->buffer);
        if(len < left_len)
        {
            os_memcpy(temp_list->tail,data,len);
			temp_list->tail += len;
			
			data_len += len;
			len = 0;
			ETS_SDIO_INTR_DISABLE();
			temp_list->next = pHead_Sended;
			pHead_Sended = temp_list;
			//ETS_SDIO_INTR_ENABLE();
        }
        else
        {
            os_memcpy(temp_list->tail,data,left_len);
			temp_list->tail += left_len;
			len -= left_len;
			data += left_len;
			data_len += left_len;
			ETS_SDIO_INTR_DISABLE();
			if(pHead_ToSend == NULL) {
				pTail_ToSend = temp_list;
				pHead_ToSend = pTail_ToSend;
				sdio_try_to_load();
			} else {
				pTail_ToSend->next = temp_list;
				pTail_ToSend = pTail_ToSend->next;
			}
			pTail_ToSend->next = NULL;
			//ETS_SDIO_INTR_ENABLE();
        }
    }
	//ETS_SDIO_INTR_DISABLE();
	if(pHead_ToSend == NULL) {
		pTail_ToSend = pHead_Sended;
		pHead_ToSend = pTail_ToSend;

		pHead_Sended = pHead_Sended->next;
		pTail_ToSend->next = NULL;
		sdio_try_to_load();
	}
	ETS_SDIO_INTR_ENABLE();
    return data_len;
}

static void sdio_try_to_load(void)
{
	if((has_read == TRUE) && (pHead_ToSend != NULL))
    {
        rx_que.buf_ptr = (uint32)pHead_ToSend->buffer;
        rx_buff_load_done(pHead_ToSend->tail- pHead_ToSend->buffer - SDIO_TOKEN_SIZE);
		//pHead_ToSend = pHead_ToSend->next;
        has_read = FALSE;
        //os_printf("SLC_INT_STATUS:0x%08x\r\n",READ_PERI_REG(SLC_INT_STATUS));
        TRIG_TOHOST_INT();
    }
}

static void sdio_read_done_process(void)
{
	has_read = TRUE;
		
	pHead_ToSend->tail = pHead_ToSend->buffer + SDIO_TOKEN_SIZE;
	if(pHead_Sended) {
       	pTail_Sended->next = pHead_ToSend;
		pTail_Sended = pTail_Sended->next;
	}else {
		pTail_Sended = pHead_ToSend;
		pHead_Sended = pTail_Sended;
	}
	pHead_ToSend = pHead_ToSend->next;
	pTail_Sended->next = NULL;
	//os_printf(">>pHead_ToSend:0x%08x,pHead_Sended:0x%08x,0x%08x,0x%08x\r\n",pHead_ToSend,pHead_Sended,pHead_Sended->buffer,pHead_Sended->tail);
	if(pHead_ToSend) {
		rx_que.buf_ptr = (uint32)pHead_ToSend->buffer;
           rx_buff_load_done(pHead_ToSend->tail - pHead_ToSend->buffer - SDIO_TOKEN_SIZE);
		has_read = FALSE;
		//os_printf("intr trig\r\n");
		//TRIG_TOHOST_INT();
	} else if ((pHead_Sended != NULL) && (pHead_Sended->buffer != (pHead_Sended->tail- SDIO_TOKEN_SIZE))) {
		pHead_ToSend = pHead_Sended;
		pTail_ToSend = pHead_ToSend;
		pHead_Sended = pHead_Sended->next;
		pTail_ToSend->next = NULL;
			
		rx_que.buf_ptr = (uint32)pHead_ToSend->buffer;			
        rx_buff_load_done(pHead_ToSend->tail- pHead_ToSend->buffer - SDIO_TOKEN_SIZE);
		has_read = FALSE;
		//os_printf("intr trig\r\n");
		//TRIG_TOHOST_INT();
	}

    if (has_read == FALSE) {
	    TRIG_TOHOST_INT();
    }
}

bool sdio_register_recv_cb(sdio_recv_data_callback_t cb)
{
	sdio_recv_data_callback_ptr = cb;
	
	return TRUE;
}

void ICACHE_FLASH_ATTR at_custom_uart_rx_buffer_fetch_cb(void)
{
	if(sdio_recv_data_callback_ptr && tx_que.datalen) {
		if (sdio_recv_data_callback_ptr((uint8*)tx_que.buf_ptr,tx_que.datalen) == tx_que.datalen) {
			tx_buff_handle_done();
			TRIG_TOHOST_INT();
		}
	}
}


#ifdef SDIO_DEBUG
void ICACHE_FLASH_ATTR at_spi_check_cb(void *arg)
{
	//TRIG_TOHOST_INT();
	os_printf("sum_len:%d\r\n",sum_len);
	//sdio_load_data("esp8266\r\n",os_strlen("esp8266\r\n"));

	sum_len = 0;
}
#endif
