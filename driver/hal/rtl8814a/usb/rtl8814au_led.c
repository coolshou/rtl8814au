/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _RTL8814AU_LED_C_

//#include <drv_types.h>
#include <rtl8814a_hal.h>

//================================================================================
// LED object.
//================================================================================


//================================================================================
//	Prototype of protected function.
//================================================================================


//================================================================================
// LED_819xUsb routines.
//================================================================================

//jimmy, for DWA-192 LED setting
/*
//0x60 input value
(GPIO15) HW LED SW2 : 0x60 BIT7
* NOTE: Press Time can not over 1 sec? => double trigger?
* NOTE: detect not sensive enough? => where to place Get_DWA192_HW_LED_SW?
*
* TODO: wake up from sleep => LED not working!! "rtw_resume" did not init LED
* 	rtw_resume -> rtw_resume_process -> rtw_resume_common
* 	in usb_intf.c
* 		rtw_drv_init ->
*   in os_intf.c
* 		rtw_drv_add_vir_if -> rtw_init_drv_sw ->rtw_hal_sw_led_init
*
(GPIO11) HW WPS SW1 : 0x60 BIT3   =>> see dm_CheckPbcGPIO in rtl8814a_dm.c
*/
static
void Get_DWA192_HW_LED_SW(PADAPTER padapter)
{
	struct led_priv *pledpriv = &(padapter->ledpriv);
	u8 HWLedSW;
	//jimmy
	HWLedSW = rtw_read8(padapter, GPIO_IN_8814A); //0x60 input value
	if (HWLedSW & (1 << 7) ) { //BIT7
		DBG_871X("Detect HW Led SW Press (%s: %d) \n", __FUNCTION__, HWLedSW);
		pledpriv->bHWLedON = !pledpriv->bHWLedON;
	}
}
/* LED layout
    12
9         3
     6
   USB
*/

//0x61 out value
//(GPIO8)  BIT0: 6 o'clock,
//(GPIO9)  BIT1: 9 o'clock, DO NOT USE THIS, must use REG_LEDCFG1_8814A first
//(GPIO13) BIT5: 12 o'clock
//(GPIO14) BIT6: 3 o'clock

//LED: GPIO13: D1, GPIO14: D2, GPIO8: D5
static
void Set_DWA192_GPIO_LED(PADAPTER padapter, BOOLEAN bON)
{
	struct led_priv *pledpriv = &(padapter->ledpriv);
    u8 LedGpioCfg;
    u8 LedCfg;

    LedGpioCfg = rtw_read8(padapter, GPIO_OUT_8814A);
    if ((bON) && pledpriv->bHWLedON) {
		LedCfg = BIT5;
        LedGpioCfg &=~ (BIT0|BIT5|BIT6);
    } else {
		LedCfg = BIT5|BIT3;
        LedGpioCfg |= (BIT0|BIT5|BIT6);
    }
	//This LED setting must be exist
	rtw_write8(padapter, REG_LEDCFG1_8814A, LedCfg); //0x4D, LED 9 o'clock
    rtw_write8(padapter, GPIO_IO_SEL_8814A, (BIT0|BIT5|BIT6)); // set as gpo
    rtw_write8(padapter, GPIO_OUT_8814A, LedGpioCfg); //set gpio out value
}

//
//	Description:
//		Turn on LED according to LedPin specified.
//
static void
SwLedOn_8814AU(
	PADAPTER		padapter,
	PLED_USB		pLed
)
{
	u32 LedGpioCfg;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	if (RTW_CANNOT_RUN(padapter))
		return;
	//jimmy
	Get_DWA192_HW_LED_SW(padapter);

	LedGpioCfg = rtw_read32(padapter , REG_GPIO_PIN_CTRL_2); /* 0x60. In 8814AU, the name should be REG_GPIO_EXT_CTRL */
	switch (pLed->LedPin) {
	case LED_PIN_LED0:
		LedGpioCfg |= (BIT16 | BIT17 | BIT21 | BIT22);	/* config as gpo */
		LedGpioCfg &= ~(BIT8 | BIT9 | BIT13 | BIT14);	/* set gpo value */
		LedGpioCfg &= ~(BIT0 | BIT1 | BIT5 | BIT6);		/* set gpi value. TBD: may not need this */
		rtw_write32(padapter , REG_GPIO_PIN_CTRL_2 , LedGpioCfg);
		break;
	default:
		break;
	}
	pLed->bLedOn = _TRUE;
}


//
//	Description:
//		Turn off LED according to LedPin specified.
//
static void
SwLedOff_8814AU(
	PADAPTER		padapter,
	PLED_USB		pLed
)
{
	u32 LedGpioCfg;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	if (RTW_CANNOT_RUN(padapter))
		return;
	//jimmy
	Get_DWA192_HW_LED_SW(padapter);

	LedGpioCfg = rtw_read32(padapter , REG_GPIO_PIN_CTRL_2); /* 0x60. In 8814AU, the name should be REG_GPIO_EXT_CTRL */
	switch (pLed->LedPin) {
	case LED_PIN_LED0:
		LedGpioCfg |= (BIT16 | BIT17 | BIT21 | BIT22);	/* config as gpo */
		LedGpioCfg |= (BIT8 | BIT9 | BIT13 | BIT14);	/* set gpo output value */
		rtw_write32(padapter , REG_GPIO_PIN_CTRL_2 , LedGpioCfg);
		break;
	default:
		break;
	}

	pLed->bLedOn = _FALSE;
}

//================================================================================
// Interface to manipulate LED objects.
//================================================================================


//================================================================================
// Default LED behavior.
//================================================================================

//
//	Description:
//		Initialize all LED_871x objects.
//
void
rtl8814au_InitSwLeds(
	_adapter	*padapter
	)
{
	struct led_priv *pledpriv = &(padapter->ledpriv);

	pledpriv->LedControlHandler = LedControlUSB;

	pledpriv->SwLedOn = SwLedOn_8814AU;
	pledpriv->SwLedOff = SwLedOff_8814AU;
	//jimmy
	pledpriv->bHWLedON = 1;

	InitLed(padapter, &(pledpriv->SwLed0), LED_PIN_LED0);

	InitLed(padapter, &(pledpriv->SwLed1), LED_PIN_LED1);

	InitLed(padapter, &(pledpriv->SwLed2), LED_PIN_LED2);
}


//
//	Description:
//		DeInitialize all LED_819xUsb objects.
//
void
rtl8814au_DeInitSwLeds(
	_adapter	*padapter
	)
{
	struct led_priv	*ledpriv = &(padapter->ledpriv);

	DeInitLed( &(ledpriv->SwLed0) );
	DeInitLed( &(ledpriv->SwLed1) );
	DeInitLed( &(ledpriv->SwLed2) );
}

