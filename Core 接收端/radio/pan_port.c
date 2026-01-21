/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan_port.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "pan_port.h"
#include "mySensor.h"
// 声明外部函数，用于SPI的发送和接收
extern uint8_t spi_tx_rx(uint8_t tx_data);

// 定义射频端口结构体变量，并初始化其成员函数指针
rf_port_t rf_port =
{
    // 天线初始化函数
    .antenna_init = rf_antenna_init,
    // TCXO（温度补偿晶体振荡器）初始化函数
    .tcxo_init = rf_tcxo_init,
    // 设置为发射模式函数
    .set_tx = rf_antenna_tx,
    // 设置为接收模式函数
    .set_rx = rf_antenna_rx,
    // 关闭天线函数
    .antenna_close = rf_antenna_close,
    // 关闭TCXO函数
    .tcxo_close = rf_tcxo_close,
//    // SPI读写字节函数
//    .spi_readwrite = spi_readwritebyte,
//    // 设置SPI片选信号为高电平函数
//    .spi_cs_high = spi_cs_set_high,
//    // 设置SPI片选信号为低电平函数
//    .spi_cs_low = spi_cs_set_low,
    // 毫秒级延时函数
    .delayms = rf_delay_ms,
    // 微秒级延时函数
    .delayus = rf_delay_us,
};


/**
 * @brief 进行SPI读写字节操作
 * @param[in] <tx_data> 要发送的SPI字节数据
 * @return 接收到的SPI字节数据
 */
//uint8_t spi_readwritebyte(uint8_t tx_data)
//{
//    // 等待发送缓冲区为空
//    while (Reset == SPI_GetFlag(M4_SPI1, SpiFlagSendBufferEmpty))
//    {
//    }
//    // 发送数据
//    SPI_SendData8(M4_SPI1, tx_data);
//    // 等待接收缓冲区满
//    while (Reset == SPI_GetFlag(M4_SPI1, SpiFlagReceiveBufferFull))
//    {
//    }
//    // 返回接收到的数据
//    return SPI_ReceiveData8(M4_SPI1);
//}

/**
 * @brief spi-cs设置高
 * @param[in] <none>
 * @return none
 */
//void spi_cs_set_high(void)
//{
//    PORT_SetBits(PortA, Pin04);
//}

///**
// * @brief spi_cs_set_low
// * @param[in] <none>
// * @return none
// */
//void spi_cs_set_low(void)
//{
//    PORT_ResetBits(PortA, Pin04);
//}

/**
 * @brief rf_delay_ms
 * @param[in] <time> ms
 * @return none
 */
void rf_delay_ms(uint32_t time)
{
//    SysTick_Delay_10us(time*100);
	HAL_Delay(time);
}

/**
 * @brief rf_delay_us
 * @param[in] <time> us
 * @return none
 */
void rf_delay_us(uint32_t time)
{
//    SysTick_Delay_10us(time/10);
	delay_us(time);
}

/**
 * @brief 进行射频TX/RX IO初始化
 * @param[in] <none>
 * @return none
 */
void rf_antenna_init(void)
{
//    rf_set_gpio_output(MODULE_GPIO_RX);//将RF GPIO 10设置为输出 
//    rf_set_gpio_output(MODULE_GPIO_TX);//将RF GPIO 0设置为输出 
//    rf_set_gpio_input(MODULE_GPIO_CAD_IRQ);//将RF GPIO11设置为输入
//	rf_set_gpio_output(11);

//    rf_set_gpio_state(MODULE_GPIO_RX, 0);//将RF GPIO 10设置为0
//    rf_set_gpio_state(MODULE_GPIO_TX, 0);//将RF GPIO 0设置为0
//    rf_set_gpio_state(MODULE_GPIO_CAD_IRQ, 0);//将RF GPIO11设置为0
}

/**
 * @brief 进行rf XTAL IO初始化
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_init(void)
{
//	rf_set_gpio_output(MODULE_GPIO_TCXO);
//	rf_set_gpio_state(MODULE_GPIO_TCXO, 1);
}

/**
 * @brief 关闭射频XTAL IO
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_close(void)
{
//	rf_set_gpio_output(MODULE_GPIO_TCXO);
//	rf_set_gpio_state(MODULE_GPIO_TCXO, 0);
}
/**
 * @brief 将rf IO更改为rx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_rx(void)
{
//    rf_set_gpio_state(MODULE_GPIO_TX, 0);
//    rf_set_gpio_state(MODULE_GPIO_RX, 1);
}

/**
 * @brief 将rf IO更改为tx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_tx(void)
{
//    rf_set_gpio_state(MODULE_GPIO_RX, 0);
//    rf_set_gpio_state(MODULE_GPIO_TX, 1);
}

/**
 * @brief change rf IO to close
 * @param[in] <none>
 * @return none
 */
void rf_antenna_close(void)
{
//    rf_set_gpio_state(MODULE_GPIO_TX, 0);
//    rf_set_gpio_state(MODULE_GPIO_RX, 0);
}

