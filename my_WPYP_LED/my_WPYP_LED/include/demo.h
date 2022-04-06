#ifndef __DOME_H__
#define __DOME_H__
/*********************************************************************
 * 头文件 INCLUDES
 */

/********************************************************************* 
 * 宏定义  *#define 
 */
#define FLAGS_MSK1 0x00000001U

#define LED_ON GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 1)      //设置Gpio_2输出高电平点亮LED灯
#define LED_OFF GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0)     //设置Gpio_2输出高电平点亮LED灯
#define LED_FLIP(x) GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, x) //设置Gpio_2输出高电平点亮LED灯

/*******************************************************************************
 * EXTERNAL VARIABLES
 */
extern osEventFlagsId_t wifi_connect_evt_id; // event flags id

void LED_DUTY_con(unsigned int per);

#endif /* __DOME_H__ */

