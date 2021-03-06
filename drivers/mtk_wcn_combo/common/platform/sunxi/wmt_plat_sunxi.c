/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

/*******************************************************************************
* Copyright (c) 2009 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#if CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#define CFG_WMT_WAKELOCK_SUPPORT 1
#endif


#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG         "[WMT-PLAT]"


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*header files*/
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/spinlock.h>

/* MTK_WCN_COMBO header files */
#include "wmt_plat.h"
#include "wmt_exp.h"
#include "mtk_wcn_cmb_hw.h"
#include "osal.h"

#include <linux/export.h>
#include <linux/gpio.h>
#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>

#define  COMBO_CONFIG_PARA     "wifi_para"

extern void wifi_pm_power(int on);

static signed int gpio_eint_bgf = -1;
static u32 eint_bgf_handle = 0;

extern void sw_mci_rescan_card(unsigned id, unsigned insert);

/*
* mtk6620 sdio+uart wii+bt+gps+fm gpio config
* mtk_6620_ldo		= port:PC22<1><default><default><0>
* mtk_6620_pmu_en	= port:PI15<1><default><default><0>
* mtk_6620_sys_rst	= port:PH09<1><default><default><0>
*/
static signed int gpio_ldo = -1;
static signed int gpio_pmuen = -1;
static signed int gpio_rst = -1;

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

static VOID wmt_plat_func_ctrl (UINT32 type, UINT32 on);
static VOID wmt_plat_bgf_eirq_cb (VOID);
static VOID wmt_plat_wifi_eirq_cb (VOID);

static INT32 wmt_plat_ldo_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_pmu_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_rtc_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_rst_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_bgf_eint_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_wifi_eint_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_all_eint_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_uart_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_pcm_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_i2s_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_sdio_pin_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_sync_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_lna_ctrl (ENUM_PIN_STATE state);

static INT32 wmt_plat_dump_pin_conf (VOID);

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

unsigned int bgf_g_balance_flag;
spinlock_t g_balance_lock;
/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/********************** add for sunxi: begin****************************/

/**
 * aw_enable_irq - 
 *
 */
static void aw_enable_irq(__u32 gpio)
{
	sw_gpio_eint_set_enable(gpio, 1);
	return;
}

/**
 * aw_disable_irq - 
 *
 */
static void aw_disable_irq(__u32 gpio)
{
	sw_gpio_eint_set_enable(gpio, 0);
	return;
}

static void aw_free_irq_resource(__u32 gpio)
{
	WMT_INFO_FUNC("=======%s: free irq resource =========.\n", __func__);
  gpio_free(gpio);
}


/**
 * aw_set_gpio_mode - according sysconfig's subkey to config io port.
 *
 * return value: 
 *              0:      success;
 *              others: fail; 
 */
static int aw_set_gpio_mode(__u32 gpio)
{
	int ret = 0;	 
  
	//config gpio to io mode
	WMT_INFO_FUNC("%s entry\n", __func__);

	sw_gpio_setcfg(gpio, GPIO_CFG_INPUT);
	
	return ret;
}

/**
 * aw_set_eint_mode - according sysconfig's subkey to config io port.
 *
 * return value: 
 *              0:      success;
 *              others: fail; 
 */
static int aw_set_eint_mode(__u32 gpio)
{
	int ret = 0;	 
  
	WMT_INFO_FUNC("%s: config eint mode. \n", __func__);
	
	sw_gpio_setcfg(gpio, R_GPIO_CFG_EINT);
	sw_gpio_setpull(gpio, 1);
	
	return ret;
}

/********************** add for sunxi: end****************************/

#if CFG_WMT_WAKELOCK_SUPPORT
static OSAL_SLEEPABLE_LOCK gOsSLock;
static struct wake_lock wmtWakeLock;
#endif

const static fp_set_pin gfp_set_pin_table[] =
{
    [PIN_LDO] = wmt_plat_ldo_ctrl,
    [PIN_PMU] = wmt_plat_pmu_ctrl,
    [PIN_RTC] = wmt_plat_rtc_ctrl,
    [PIN_RST] = wmt_plat_rst_ctrl,
    [PIN_BGF_EINT] = wmt_plat_bgf_eint_ctrl,
    [PIN_WIFI_EINT] = wmt_plat_wifi_eint_ctrl,
    [PIN_ALL_EINT] = wmt_plat_all_eint_ctrl,
    [PIN_UART_GRP] = wmt_plat_uart_ctrl,
    [PIN_PCM_GRP] = wmt_plat_pcm_ctrl,
    [PIN_I2S_GRP] = wmt_plat_i2s_ctrl,
    [PIN_SDIO_GRP] = wmt_plat_sdio_pin_ctrl,
    [PIN_GPS_SYNC] = wmt_plat_gps_sync_ctrl,
    [PIN_GPS_LNA] = wmt_plat_gps_lna_ctrl,

};

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*!
 * \brief audio control callback function for CMB_STUB on ALPS
 *
 * A platform function required for dynamic binding with CMB_STUB on ALPS.
 *
 * \param state desired audio interface state to use
 * \param flag audio interface control options
 *
 * \retval 0 operation success
 * \retval -1 invalid parameters
 * \retval < 0 error for operation fail
 */
INT32 wmt_plat_audio_ctrl (CMB_STUB_AIF_X state, CMB_STUB_AIF_CTRL ctrl)
{
    INT32 iRet = 0;
    UINT32 pinShare;

    /* input sanity check */
    if ( (CMB_STUB_AIF_MAX <= state)
        || (CMB_STUB_AIF_CTRL_MAX <= ctrl) ) {
        iRet = -1;
        WMT_ERR_FUNC("WMT-PLAT: invalid para, state(%d), ctrl(%d),iRet(%d) \n", state, ctrl, iRet);
        return iRet;
    }
    if (0/*I2S/PCM share pin*/) {
        // TODO: [FixMe][GeorgeKuo] how about MT6575? The following is applied to MT6573E1 only!!
        pinShare = 1;
        WMT_INFO_FUNC( "PCM/I2S pin share\n");
    }
    else{ //E1 later
        pinShare = 0;
        WMT_INFO_FUNC( "PCM/I2S pin seperate\n");
    }

    iRet = 0;

    /* set host side first */
    switch (state) {
    case CMB_STUB_AIF_0:
        /* BT_PCM_OFF & FM line in/out */
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_DEINIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
        break;

    case CMB_STUB_AIF_1:
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_INIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
        break;

    case CMB_STUB_AIF_2:
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_DEINIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
        break;

    case CMB_STUB_AIF_3:
        iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_INIT);
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
        break;

    default:
        /* FIXME: move to cust folder? */
        WMT_ERR_FUNC("invalid state [%d]\n", state);
        return -1;
        break;
    }

    if (CMB_STUB_AIF_CTRL_EN == ctrl) {
        WMT_INFO_FUNC("call chip aif setting \n");
        /* need to control chip side GPIO */
        iRet += wmt_lib_set_aif(state, (pinShare) ? MTK_WCN_BOOL_TRUE : MTK_WCN_BOOL_FALSE);
    }
    else {
        WMT_INFO_FUNC("skip chip aif setting \n");
    }
    return iRet;

}

static VOID wmt_plat_func_ctrl (UINT32 type, UINT32 on)
{
    if (on) {
        mtk_wcn_wmt_func_on((ENUM_WMTDRV_TYPE_T)type);
    }
    else {
        mtk_wcn_wmt_func_off((ENUM_WMTDRV_TYPE_T)type);
    }
    return;
}

#if CFG_WMT_PS_SUPPORT
irqreturn_t bgf_irq_handler(void *arg)
{
  int pin_state = 0;
  
  pin_state = __gpio_get_value(gpio_eint_bgf);
  printk("========%s, pin_state:%d =========\n",__func__,pin_state);
  if (likely(pin_state == 0)) {		
    wmt_plat_bgf_eirq_cb();
	} else {			   
    printk(KERN_INFO "%s bgf_irq false alarm:pin_state(%d)!\n", __FUNCTION__, pin_state);
  }
  
	return 0;
}

#endif

static VOID
wmt_plat_bgf_eirq_cb (VOID)
{
#if CFG_WMT_PS_SUPPORT
//#error "need to disable EINT here"
    WMT_INFO_FUNC("WMT-PLAT:BGFInt (++) \n");
    wmt_lib_ps_irq_cb();
    WMT_INFO_FUNC("WMT-PLAT:BGFInt (++) \n");

#else
    return;
#endif

}


INT32
wmt_plat_init (P_PWR_SEQ_TIME pPwrSeqTime)
{
    CMB_STUB_CB stub_cb;
    /*PWR_SEQ_TIME pwr_seq_time;*/
    INT32 iret;

    stub_cb.aif_ctrl_cb = wmt_plat_audio_ctrl;
    stub_cb.func_ctrl_cb = wmt_plat_func_ctrl;
    stub_cb.size = sizeof(stub_cb);

    /* register to cmb_stub */
    iret = mtk_wcn_cmb_stub_reg(&stub_cb);

    /* init cmb_hw */
    iret += mtk_wcn_cmb_hw_init(pPwrSeqTime);

    /*init wmt function ctrl wakelock if wake lock is supported by host platform*/
    #ifdef CFG_WMT_WAKELOCK_SUPPORT
    wake_lock_init(&wmtWakeLock, WAKE_LOCK_SUSPEND, "wmtFuncCtrl");
    osal_sleepable_lock_init(&gOsSLock);
    #endif

    spin_lock_init(&g_balance_lock);

    WMT_DBG_FUNC("WMT-PLAT: ALPS platform init (%d)\n", iret);

    return 0;
}


INT32
wmt_plat_deinit (VOID)
{
    INT32 iret;

    /* 1. de-init cmb_hw */
    iret = mtk_wcn_cmb_hw_deinit();
    /* 2. unreg to cmb_stub */
    iret += mtk_wcn_cmb_stub_unreg();
    /*3. wmt wakelock deinit*/
    #ifdef CFG_WMT_WAKELOCK_SUPPORT
    wake_lock_destroy(&wmtWakeLock);
    osal_sleepable_lock_deinit(&gOsSLock);
    WMT_DBG_FUNC("destroy wmtWakeLock\n");
    #endif
    WMT_DBG_FUNC("WMT-PLAT: ALPS platform init (%d)\n", iret);

    return 0;
}

INT32 wmt_plat_sdio_ctrl (WMT_SDIO_SLOT_NUM sdioPortType, ENUM_FUNC_STATE on)
{
    if (FUNC_OFF == on)  {
        /* add control logic here to generate SDIO CARD REMOVAL event to mmc/sd
         * controller. SDIO card removal operation and remove success messages
         * are expected.
         */
		sw_mci_rescan_card(1,0);
		printk("[MTK6620] %s: remove card \n", __FUNCTION__);
    }
    else {
        /* add control logic here to generate SDIO CARD INSERTION event to mmc/sd
         * controller. SDIO card detection operation and detect success messages
         * are expected.
         */
		sw_mci_rescan_card(1,1);
		printk("[MTK6620] %s: rescan card \n", __FUNCTION__);
    }
    return 0;
}

#if 0
INT32
wmt_plat_irq_ctrl (
    ENUM_FUNC_STATE state
    )
{
    return -1;
}
#endif

static INT32
wmt_plat_dump_pin_conf (VOID)
{
    WMT_INFO_FUNC( "[WMT-PLAT]=>dump wmt pin configuration start<=\n");
    WMT_INFO_FUNC( "[WMT-PLAT]=>dump wmt pin configuration emds<=\n");
    return 0;
}


INT32 wmt_plat_pwr_ctrl (
    ENUM_FUNC_STATE state
    )
{
    INT32 ret = -1;

    switch (state) {
    case FUNC_ON:
        // TODO:[ChangeFeature][George] always output this or by request throuth /proc or sysfs?
        wmt_plat_dump_pin_conf();
        ret = mtk_wcn_cmb_hw_pwr_on();
        break;

    case FUNC_OFF:
        ret = mtk_wcn_cmb_hw_pwr_off();
        break;

    case FUNC_RST:
        ret = mtk_wcn_cmb_hw_rst();
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) in pwr_ctrl\n", state);
        break;
    }

    return ret;
}

INT32 wmt_plat_ps_ctrl (ENUM_FUNC_STATE state)
{
    return -1;
}

INT32
wmt_plat_eirq_ctrl (
    ENUM_PIN_ID id,
    ENUM_PIN_STATE state
    )
{
    INT32 iRet;
    unsigned int flags;
    int err;
    
    // TODO: [ChangeFeature][GeorgeKuo]: use another function to handle this, as done in gpio_ctrls

    if ( (PIN_STA_INIT != state )
        && (PIN_STA_DEINIT != state )
        && (PIN_STA_EINT_EN != state )
        && (PIN_STA_EINT_DIS != state ) ) {
        iRet = -1;
        WMT_WARN_FUNC("WMT-PLAT:invalid PIN_STATE(%d) in eirq_ctrl for PIN(%d), ret(%d) \n", state, id, iRet);
        return iRet;
    }

    iRet = -2;
    switch (id) {
    case PIN_BGF_EINT:
        if (PIN_STA_INIT == state) {
          /*request irq,low level triggered*/
          bgf_g_balance_flag = 1;//do not modify this value
          WMT_INFO_FUNC("WMT-PLAT:BGFInt (init) \n");
          
          if (eint_bgf_handle){
          	 iRet = 0;
             break;
          }
          
          eint_bgf_handle = sw_gpio_irq_request(gpio_eint_bgf, TRIG_LEVL_LOW,(peint_handle)bgf_irq_handler, NULL);
          if (!eint_bgf_handle) {
            WMT_INFO_FUNC( "%s: request irq failed err:%d \n",__func__,err);
            goto exit_irq_request_failed;
          }
        } else if (PIN_STA_EINT_EN == state) {
          /*enable irq*/
          spin_lock_irqsave(&g_balance_lock,flags);
          if (bgf_g_balance_flag) {
            /*if enter this case, the bgf eint has been enabled,so skip it.*/
            WMT_INFO_FUNC("BGF_EINT has been enabled,bgf_g_balance_flag(%d)!\n",bgf_g_balance_flag);
          } else {
				    /*do real irq enable implement is this case*/
		        bgf_g_balance_flag++;
		        
		        aw_set_eint_mode(gpio_eint_bgf);
            WMT_INFO_FUNC("WMT-PLAT:BGFInt (en),bgf_g_balance_flag(%d)\n",bgf_g_balance_flag);
            aw_enable_irq(gpio_eint_bgf);
	    	  }
          spin_unlock_irqrestore(&g_balance_lock,flags);
        } else if (PIN_STA_EINT_DIS == state) {
          /*disable irq*/
          spin_lock_irqsave(&g_balance_lock,flags);
	 	      if (!bgf_g_balance_flag) {
            /*if enter this case, the bgf eint has been disabled,so skip it.*/
            WMT_INFO_FUNC("BGF_EINT has been disabled,bgf_g_balance_flag(%d)!\n",bgf_g_balance_flag);
          } else {
            /*do real irq disable implement is this case*/
            bgf_g_balance_flag--;
            WMT_INFO_FUNC("WMT-PLAT:BGFInt (dis) bgf_g_balance_flag(%d)\n",bgf_g_balance_flag);
            aw_disable_irq(gpio_eint_bgf);
            aw_set_gpio_mode(gpio_eint_bgf);
		      }
		      spin_unlock_irqrestore(&g_balance_lock,flags);
        } else {
          /* de-init: free irq*/
          WMT_INFO_FUNC("WMT-PLAT:BGFInt (deinit) \n");
          aw_set_eint_mode(gpio_eint_bgf);
          sw_gpio_irq_free(eint_bgf_handle);
          eint_bgf_handle = 0;
          gpio_eint_bgf = -1;
        }
        iRet = 0;
        break;

    case PIN_ALL_EINT:
#if 0
        if (PIN_STA_INIT == state) {

            WMT_DBG_FUNC("WMT-PLAT:ALLInt (INIT but not used yet) \n");
        }
        else if (PIN_STA_EINT_EN == state) {
             WMT_DBG_FUNC("WMT-PLAT:ALLInt (EN but not used yet) \n");
        }
        else if (PIN_STA_EINT_DIS == state) {
            WMT_DBG_FUNC("WMT-PLAT:ALLInt (DIS but not used yet) \n");
        }
        else {

            WMT_DBG_FUNC("WMT-PLAT:ALLInt (DEINIT but not used yet) \n");
            /* de-init: nothing to do in ALPS, such as un-registration... */
        }
#else
        WMT_DBG_FUNC("WMT-PLAT:ALLInt (not used yet) \n");
#endif
        iRet = 0;
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:unsupported EIRQ(PIN_ID:%d) in eirq_ctrl, ret (%d)\n", id, iRet);
        iRet = -1;
        break;
    }
exit_irq_request_failed:
	
    return iRet;
}

INT32 wmt_plat_gpio_ctrl (
    ENUM_PIN_ID id,
    ENUM_PIN_STATE state
    )
{
    if ( (PIN_ID_MAX > id)
        && (PIN_STA_MAX > state) ) {

        // TODO: [FixMe][GeorgeKuo] do sanity check to const function table when init and skip checking here
        if (gfp_set_pin_table[id]) {
            return (*(gfp_set_pin_table[id]))(state); /* .handler */
        }
        else {
            WMT_WARN_FUNC("WMT-PLAT: null fp for gpio_ctrl(%d)\n", id);
            return -2;
        }
    }
	WMT_ERR_FUNC("WMT-PLAT:[out of range] id(%d), state (%d)\n", id, state);
    return -1;
}

INT32
wmt_plat_ldo_ctrl (
    ENUM_PIN_STATE state
    )
{
    int ret = 0;
    script_item_u val;
    script_item_value_type_e type;
	 
    switch(state)
    {
    case PIN_STA_INIT:
        /*set to gpio output low, disable pull*/
        WMT_INFO_FUNC("WMT-PLAT:LDO init (out 0) \n");
        
        wifi_pm_power(1);
        
        type = script_get_item(COMBO_CONFIG_PARA,"mtk_6620_ldo",&val);
        if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type){ 
		      WMT_INFO_FUNC("has no mtk mtk_6620_ldo gpio\n");
		      return 0;
	      }else{
		      gpio_ldo = val.gpio.gpio;
        }
        
        ret = gpio_request(gpio_ldo, NULL);
        if (ret) {
	        	WMT_ERR_FUNC("failed to request gpio %d!\n", gpio_ldo);
	        	return -1;
	      }
        
        //set the gpio to output mode, low val
        ret = gpio_direction_output(gpio_ldo, 0);
        if (ret) {
        	WMT_ERR_FUNC("failed to set gpio %d to output low val!\n", gpio_ldo);
        	gpio_free(gpio_ldo);
        	gpio_ldo = -1;
        	return -1;
        }
        
        break;

    case PIN_STA_OUT_H:
        WMT_INFO_FUNC("WMT-PLAT:LDO (out 1) \n");
        
        if (gpio_ldo == -1)
        {
        	  return 0;
        }
        
        //set the gpio to output mode,high level
        ret = sw_gpio_setcfg(gpio_ldo, GPIO_CFG_OUTPUT);
        if (ret) {
        	WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	return -1;
        }
        
        //set val to high
        __gpio_set_value(gpio_ldo, 1);
        
        break;

    case PIN_STA_OUT_L:
        WMT_INFO_FUNC("WMT-PLAT:LDO (out 0) \n");
		    
		    if (gpio_ldo == -1)
        {
        	  return 0;
        }
		    
        //set the gpio to output mode
        ret = sw_gpio_setcfg(gpio_ldo, GPIO_CFG_OUTPUT);
        if (ret) {
        	WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	return -1;
        }
        
        //set val to low
        __gpio_set_value(gpio_ldo, 0);
        
        break;

    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
    	  /*set to gpio input low, pull down enable*/
    	  WMT_DBG_FUNC("WMT-PLAT:LDO deinit (in pd) \n");
    	  
    	  if (gpio_ldo != -1)
        {
        	  ret = sw_gpio_setcfg(gpio_ldo, GPIO_CFG_INPUT);
            if (ret) 
            {
        	      WMT_ERR_FUNC("failed to set gpio %d to input!\n", gpio_ldo);
        	      return -1;
            }

            gpio_free(gpio_ldo);
            gpio_ldo = -1;
        }
        
        wifi_pm_power(0);
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on LDO\n", state);
        break;
    }
    return 0;
}

INT32
wmt_plat_pmu_ctrl (
    ENUM_PIN_STATE state
    )
{
    int ret = 0;
	  script_item_u val ;
	  script_item_value_type_e type;
	
    switch(state)
    {
    case PIN_STA_INIT:
        /*set to gpio output low, disable pull*/
        WMT_INFO_FUNC("WMT-PLAT:PMU init (out 0) \n");
        
        type = script_get_item(COMBO_CONFIG_PARA,"mtk_6620_pmu_en",&val);
        if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type){ 
		      WMT_INFO_FUNC("get mtk mtk_6620_pmu_en gpio failed\n");
	      }else{
		      gpio_pmuen = val.gpio.gpio;
        }
        
        ret = gpio_request(gpio_pmuen, NULL);
        if (ret) {
          WMT_ERR_FUNC("failed to request gpio %d!\n", gpio_pmuen);
          return -1;
	      }
 
        //set the gpio to output mode, low val
        ret = gpio_direction_output(gpio_pmuen, 0);
        if (ret) {
        	WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	return -1;
        }
        
        break;

    case PIN_STA_OUT_H:
        WMT_INFO_FUNC("WMT-PLAT:PMU (out 1) \n");
		    
        //set the gpio to output mode
        ret = sw_gpio_setcfg(gpio_pmuen, GPIO_CFG_OUTPUT);
        if (ret) {
        	WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	return -1;
        }
        
        //set val to high
        __gpio_set_value(gpio_pmuen, 1);
        break;

    case PIN_STA_OUT_L:
        WMT_INFO_FUNC("WMT-PLAT:PMU (out 0) \n");
		
		    //set the gpio to output mode
        ret = sw_gpio_setcfg(gpio_pmuen, GPIO_CFG_OUTPUT);
        if (ret) {
        	WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	return -1;
        }
        
        //set val to low
        __gpio_set_value(gpio_pmuen, 0);
        break;

    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
        /*set to gpio input low, pull down enable*/
        WMT_DBG_FUNC("WMT-PLAT:PMU deinit (in pd) \n");
        ret = sw_gpio_setcfg(gpio_pmuen, GPIO_CFG_INPUT);
        if (ret) {
        	WMT_ERR_FUNC("failed to set gpio %d to input!\n", gpio_ldo);
        	return -1;
        }
        
       gpio_free(gpio_pmuen);
       gpio_pmuen = -1;
        
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on PMU\n", state);
        break;
    }

    return 0;
}

INT32
wmt_plat_rtc_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch(state)
    {
    case PIN_STA_INIT:
        WMT_DBG_FUNC("WMT-PLAT:RTC init \n");
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on RTC\n", state);
        break;
    }
    return 0;
}


INT32
wmt_plat_rst_ctrl (
    ENUM_PIN_STATE state
    )
{
    int ret = 0;
	  script_item_u val ;
	  script_item_value_type_e type;	

    switch(state)
    {
        case PIN_STA_INIT:
            /*set to gpio output low, disable pull*/
            WMT_INFO_FUNC("WMT-PLAT:RST init (out 0) \n");
        
            type = script_get_item(COMBO_CONFIG_PARA,"mtk_6620_sys_rst",&val);
            if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type){ 
		          WMT_INFO_FUNC("get mtk mtk_6620_sys_rst gpio failed\n");
	          }else{
		          gpio_rst = val.gpio.gpio;
            }
        
            ret = gpio_request(gpio_rst, NULL);
            if (ret) {
              WMT_ERR_FUNC("failed to request gpio %d!\n", gpio_rst);
              return -1;
            }
	       
            //set the gpio to output mode,low value
            ret = gpio_direction_output(gpio_rst, 0);
            if (ret) {
        	    WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	    return -1;
            }
            
            break;

        case PIN_STA_OUT_H:
            WMT_INFO_FUNC("WMT-PLAT:RST (out 1) \n");
            
            //set the gpio to output mode
            ret = sw_gpio_setcfg(gpio_rst, GPIO_CFG_OUTPUT);
            if (ret) {
        	    WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	    return -1;
            }
        
            //set val to high
            __gpio_set_value(gpio_rst, 1);
            
            break;

        case PIN_STA_OUT_L:
            WMT_INFO_FUNC("WMT-PLAT:RST (out 0) \n");
            
            //set the gpio to output mode
            ret = sw_gpio_setcfg(gpio_rst, GPIO_CFG_OUTPUT);
            if (ret) {
        	    WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	    return -1;
            }
        
            //set val to low
            __gpio_set_value(gpio_rst, 0);
            
            break;

        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
            /*set to gpio input low, pull down enable*/
            WMT_DBG_FUNC("WMT-PLAT:RST deinit (in pd) \n");
            ret = sw_gpio_setcfg(gpio_rst, GPIO_CFG_INPUT);
            if (ret) {
        	    WMT_ERR_FUNC("failed to set gpio %d to input!\n", gpio_ldo);
        	    return -1;
            }
            
            gpio_free(gpio_rst);
            gpio_rst = -1;
            
            break;

        default:
            WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on RST\n", state);
            break;
    }

    return 0;
}

INT32
wmt_plat_bgf_eint_ctrl (
    ENUM_PIN_STATE state
    )
{
  int ret = 0;
  script_item_u val ;
  script_item_value_type_e type;	
	
	WMT_INFO_FUNC(" %s state:%d \n",__func__,state);
    switch(state)
    {
        case PIN_STA_INIT:
            /*set to gpio input low, pull down eanble*/
            WMT_DBG_FUNC("WMT-PLAT:BGFInt init(in pd) \n");
            
						type = script_get_item(COMBO_CONFIG_PARA,"mtk_6620_bgf_int",&val);
					  if (SCIRPT_ITEM_VALUE_TYPE_PIO!=type){ 
					    WMT_INFO_FUNC("get mtk mtk_6620_bgf_int gpio failed\n");
					    ret = -1;
					  }else{
					    gpio_eint_bgf = val.gpio.gpio;
						}
						
            break;

        case PIN_STA_MUX:
            /* first: set to EINT mode,interrupt input, pull up enable*/
            WMT_INFO_FUNC("WMT-PLAT:BGFInt mux (eint) \n");
            break;

        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
	          /* first: disable bgf irq wake up host function*/
 	          /* second: set to gpio input low, pull down enable*/
            WMT_DBG_FUNC("WMT-PLAT:BGFInt deinit(in pd) \n");
            /*do nothing on A31,because sw_gpio_irq_request and sw_gpio_irq_free */
 	          /*will request and free gpio_eint_bgf */
            

            break;

        default:
            WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on BGF EINT\n", state);
            break;
    }
exit_ioremap_failed:
    return 0;
}


INT32 wmt_plat_wifi_eint_ctrl(ENUM_PIN_STATE state)
{
#if 0
    int ret = 0;
    script_item_u val ;
    script_item_value_type_e type;

    switch(state)
    {
        case PIN_STA_INIT:
            break;
        case PIN_STA_MUX:

            break;
        case PIN_STA_EINT_EN:
            break;
        case PIN_STA_EINT_DIS:
            break;
        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
            break;
        default:
            WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on WIFI EINT\n", state);
            break;
    }
#else
    WMT_INFO_FUNC("WMT-PLAT:WIFI EINT is controlled by MSDC driver \n");
#endif

    return 0;
}


INT32
wmt_plat_all_eint_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch(state)
    {
        case PIN_STA_INIT:
        	  /*set to gpio input low, pull down eanble*/
            WMT_DBG_FUNC("WMT-PLAT:ALLInt init(in pd) \n");
            break;

        case PIN_STA_MUX:
        	  /*set to gpio EINT mode, pull down enable*/
            break;

        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
            /*set to gpio input low, pull down enable*/
            break;

        default:
            WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on ALL EINT\n", state);
            break;
    }
    return 0;
}

INT32 wmt_plat_uart_ctrl(ENUM_PIN_STATE state)
{
    switch(state)
    {
    case PIN_STA_MUX:
    case PIN_STA_INIT:
        WMT_DBG_FUNC("WMT-PLAT:UART init (mode_01, uart) \n");
        break;
    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
        WMT_DBG_FUNC("WMT-PLAT:UART deinit (out 0) \n");
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on UART Group\n", state);
        break;
    }

    return 0;
}


INT32 wmt_plat_pcm_ctrl(ENUM_PIN_STATE state)
{
    switch(state)
    {
    case PIN_STA_MUX:
    case PIN_STA_INIT:
    	  /*set to PCM function*/
        WMT_DBG_FUNC("WMT-PLAT:PCM init (pcm) \n");
        break;

    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
        WMT_DBG_FUNC("WMT-PLAT:PCM deinit (out 0) \n");
        break;

    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on PCM Group\n", state);
        break;
    }
    return 0;
}


INT32 wmt_plat_i2s_ctrl(ENUM_PIN_STATE state)
{
#ifndef FM_ANALOG_INPUT
    switch(state)
    {
    case PIN_STA_INIT:
    case PIN_STA_MUX:
    /*set to I2S function*/
        WMT_DBG_FUNC("WMT-PLAT:I2S init \n");
        break;
    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
    /*set to gpio input low, pull down enable*/
        WMT_DBG_FUNC("WMT-PLAT:I2S deinit (out 0) \n");
        break;
    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on I2S Group\n", state);
        break;
    }
#else
        WMT_INFO_FUNC( "[MT6620]warnning:FM analog mode is set, no I2S GPIO settings should be modified by combo driver\n");
#endif

    return 0;
}

INT32
wmt_plat_sdio_pin_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_MUX:
        break;
    case PIN_STA_DEINIT:
        break;
    default:
        WMT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on SDIO Group\n", state);
        break;
    }
    return 0;
}

static INT32
wmt_plat_gps_sync_ctrl (
    ENUM_PIN_STATE state
    )
{
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_DEINIT:
    /*set GPS_SYNC GPIO to GPIO mode, pull disable,output low*/
        break;

    case PIN_STA_MUX:
    /*set GPS_SYNC GPIO to GPS_SYNC function*/
        break;

    default:
        break;
    }
    return 0;
}


static INT32
wmt_plat_gps_lna_ctrl (
        ENUM_PIN_STATE state
        )
{
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_DEINIT:
    /*set GPS_LNA GPIO to GPIO mode, pull disable,output low*/
        break;
    case PIN_STA_OUT_H:
    /*set GPS_LNA GPIO to GPIO mode, pull disable,output high*/
        break;
    case PIN_STA_OUT_L:
    /*set GPS_LNA GPIO to GPIO mode, pull disable,output low*/
        break;

    default:
        WMT_WARN_FUNC("%d mode not defined for  gps lna pin !!!\n", state);
        break;
    }
    return 0;

}



INT32 wmt_plat_wake_lock_ctrl(ENUM_WL_OP opId)
{
#ifdef CFG_WMT_WAKELOCK_SUPPORT
    static INT32 counter = 0;


    osal_lock_sleepable_lock( &gOsSLock);
    if (WL_OP_GET == opId)
    {
        ++counter;
    }else if (WL_OP_PUT == opId)
    {
        --counter;
    }
    osal_unlock_sleepable_lock( &gOsSLock);
    if (WL_OP_GET == opId && counter == 1)
    {
        wake_lock(&wmtWakeLock);
        WMT_DBG_FUNC("WMT-PLAT: after wake_lock(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter);

    }
    else if (WL_OP_PUT == opId && counter == 0)
    {
        wake_unlock(&wmtWakeLock);
        WMT_DBG_FUNC("WMT-PLAT: after wake_unlock(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter);
    }
    else
    {
        WMT_WARN_FUNC("WMT-PLAT: wakelock status(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter);
    }
    return 0;
#else
    WMT_WARN_FUNC("WMT-PLAT: host awake function is not supported.");
    return 0;

#endif
}

void mt6620_pm_shutdown(void)
{
	 int ret = 0;
	 
	 printk("mt6620 shutdown!\n");
	 
	 if (gpio_ldo != -1)
	 {
	 	   //set the gpio to output mode
        ret = sw_gpio_setcfg(gpio_ldo, GPIO_CFG_OUTPUT);
        if (ret) 
        {
        	  WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	  return ;
        }
        
        //set val to low
        __gpio_set_value(gpio_ldo, 0);
   }
   
   if (gpio_pmuen != -1)
   {
       //set the gpio to output mode
        ret = sw_gpio_setcfg(gpio_pmuen, GPIO_CFG_OUTPUT);
        if (ret) 
        {
        	  WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	  return ;
        }
        
        //set val to low
        __gpio_set_value(gpio_pmuen, 0);
   }
   
   if (gpio_rst != -1)
   {
   	   //set the gpio to output mode
       ret = sw_gpio_setcfg(gpio_rst, GPIO_CFG_OUTPUT);
       if (ret)
       {
        	 WMT_ERR_FUNC("failed to set gpio %d to output!\n", gpio_ldo);
        	 return ;
       }
        
       //set val to low
       __gpio_set_value(gpio_rst, 0);
   }
	 return ;
}
EXPORT_SYMBOL(mt6620_pm_shutdown);