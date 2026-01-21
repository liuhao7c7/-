/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan_rf.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "pan_port.h"
#include "pan_param.h"
#include "mySensor.h"
/*
 * 表示是否接收到新数据包的标志。
 * 初始值为RADIO_FLAG_IDLE，表示没有新数据包接收。
*/
static int packet_received = RADIO_FLAG_IDLE;

/*
 * 表示传输是否完成的标志。
 * 初始值为RADIO_FLAG_IDLE，表示传输未完成。
*/
static int packet_transmit = RADIO_FLAG_IDLE;

struct RxDoneMsg RxDoneParams;

/**
 * @brief 获取接收标志
 * @param[in] 无
 * @return 接收状态
 */
int rf_get_recv_flag(void)
{
    return packet_received;
}

/**
 * @brief 设置接收标志
 * @param[in] status 要设置的接收标志状态
 * @return 无
 */
void rf_set_recv_flag(int status)
{
    packet_received = status;
}

/**
 * @brief 获取传输标志
 * @param[in] 无
 * @return 传输状态
 */
int rf_get_transmit_flag(void)
{
    return packet_transmit;
}

/**
 * @brief 设置传输标志
 * @param[in] status 要设置的传输标志状态
 * @return 无
 */
void rf_set_transmit_flag(int status)
{
    packet_transmit = status;
}

/**
 * @brief 计算尾随零的个数
 *
 * @param val 要计算的数值
 * @return 尾随零的个数
 */
static uint8_t __ctz(uint8_t val)
{
    int i;

    for (i = 0; i < 8; ++i)
    {
        if ((val >> i) & 1)
            return (uint8_t)i;
    }
    return 0;
}

//从当前页的寄存器中读取一个字节
uint8_t rf_read_reg(uint8_t addr)
{
//	return Read_I2C_u8(addr);
	return Read_SPI_u8(addr);
}

//在当前页写入全局寄存器并检查
uint8_t rf_write_reg(uint8_t addr, uint8_t value)
{
//	return Write_I2C_u8(addr,value);
	return Write_SPI_u8(addr,value);
}

//向射频发送数据FIFO，写入字节到寄存器

void rf_write_fifo(uint8_t addr, uint8_t *buffer, int size)
{
//	Write_I2C_DMA(addr,buffer,size);
	Write_SPI_DMA(addr,buffer,size);
}

//从射频接收数据FIFO，从寄存器中读取字节
void rf_read_fifo(uint8_t addr, uint8_t *buffer, int size)
{
//	Read_I2C(addr,buffer,size);
	Read_SPI(addr,buffer,size);
}

/**
 * @brief 切换页面
 * @param[in] page 要切换到的页面
 * @return 操作结果
 */
RF_Err_t rf_switch_page(enum PAGE_SEL page)
{
	if(my_Page!=page)
	{
		my_Page=page;
    uint8_t page_sel;
    uint8_t tmpreg;

    tmpreg = rf_read_reg(REG_SYS_CTL);
    page_sel = (tmpreg & 0xFC) | page;
    rf_write_reg(REG_SYS_CTL, page_sel);

#if SPI_WRITE_CHECK
    if (rf_read_reg(REG_SYS_CTL) != page_sel)
    {
        return FAIL;
    }
#endif
	}
    return OK;
}

/**
 * @brief 向特定页面的寄存器写入一个值
 * @param[in] page 寄存器所在的页面
 * @param[in] addr 寄存器地址
 * @param[in] value 要写入的值
 * @return 操作结果
 */
RF_Err_t rf_write_spec_page_reg(enum PAGE_SEL page, uint8_t addr, uint8_t value)
{
    RF_ASSERT(rf_switch_page(page));
    RF_ASSERT(rf_write_reg(addr, value));
    return OK;
}

/**
 * @brief 从特定页面的寄存器读取一个值
 * @param[in] page 寄存器所在的页面
 * @param[in] addr 寄存器地址
 * @return 成功时返回寄存器的值，失败时返回错误值
 */
uint8_t rf_read_spec_page_reg(enum PAGE_SEL page, uint8_t addr)
{
    RF_ASSERT(rf_switch_page(page));
    return rf_read_reg(addr);
}

/**
 * @brief 向特定页面的特定地址连续写入寄存器值（缓冲区）
 * @param[in] page 寄存器所在的页面
 * @param[in] addr 寄存器起始地址
 * @param[in] buffer 要写入的值的缓冲区
 * @param[in] len 缓冲区的长度
 * @return 操作结果
 */
RF_Err_t rf_write_spec_page_regs(enum PAGE_SEL page, uint8_t addr, uint8_t *buffer, uint8_t len)
{
    uint8_t i;
//    uint16_t addr_w;
    RF_Err_t ret = RF_OK;

    RF_ASSERT(rf_switch_page(page));
//		Write_I2C(addr,buffer,len);
	Write_SPI(addr,buffer,len);
#if SPI_WRITE_CHECK

    for (i = 0; i < len; i++)
    {
        if(buffer[i] !=Read_SPI_u8(addr+i))
					//Read_I2C_u8(addr+i))
        {
            ret = FAIL;
            break;
        }
    }
#endif

    return ret;
}

RF_Err_t rf_read_spec_page_regs(enum PAGE_SEL page, uint8_t addr, uint8_t *buffer, uint8_t len)
{
    RF_ASSERT(rf_switch_page(page));
//		Read_I2C(addr,buffer,len);
	Read_SPI(addr,buffer,len);
    return OK;
}

/**
 * @brief 将特定页面寄存器的某些位设置为1
 *
 * @param page 寄存器所在的页面
 * @param addr 寄存器地址
 * @param bit_mask 要设置的位掩码
 * @return 操作结果
 */
RF_Err_t rf_set_spec_page_reg_bits(enum PAGE_SEL page, uint8_t addr, uint8_t mask)
{
    uint8_t tmp;
    RF_Err_t ret;
    tmp = rf_read_spec_page_reg(page, addr);//切换页面，返回读取的值
    ret = rf_write_spec_page_reg(page, addr, tmp | mask);//切换页面，写值
    return ret;
}

/**
 * @brief 将特定页面寄存器的某些位设置为0
 *
 * @param page 寄存器所在的页面
 * @param addr 寄存器地址
 * @param bit_mask 要设置的位掩码
 * @return 操作结果
 */
RF_Err_t rf_reset_spec_page_reg_bits(enum PAGE_SEL page, uint8_t addr, uint8_t mask)
{
    uint8_t tmp;
    RF_Err_t ret;
    tmp = rf_read_spec_page_reg(page, addr);
    ret = rf_write_spec_page_reg(page, addr, tmp & (~mask));
    return ret;
}

RF_Err_t rf_write_spec_page_reg_bits(enum PAGE_SEL page, uint8_t addr, uint8_t val, uint8_t mask)
{
    uint8_t tmp;
    RF_Err_t ret;
    uint8_t shift = GET_SHIFT(mask);

    val <<= shift;
    val &= mask;

    tmp = rf_read_spec_page_reg(page, addr);
    ret = rf_write_spec_page_reg(page, addr, (tmp & (~mask)) | val);

    return ret;
}

/**
 * @brief 清除所有中断请求
 * @param[in] 无
 * @return 操作结果
 */
uint8_t rf_clr_irq(uint8_t flags)
{
    rf_write_spec_page_reg(PAGE0_SEL, 0x6C, flags); // clr irq
	
    return OK;
}

/**
 * @brief 获取中断请求状态
 * @param[in] 无
 * @return 中断请求状态
 */
uint8_t rf_get_irq(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE0_SEL, 0x6C);

    return (tmpreg & 0x7F);
}

/**
 * @brief RF 1.2V寄存器刷新，不会改变寄存器的值
 * @param[in] 无
 * @return 操作结果
 */
RF_Err_t rf_refresh(void)
{
    uint8_t tmpreg;

    tmpreg = rf_read_reg(REG_SYS_CTL);
    tmpreg |= 0x80;
    rf_write_reg(REG_SYS_CTL, tmpreg);

    tmpreg = rf_read_reg(REG_SYS_CTL);
    tmpreg &= 0x7F;
    rf_write_reg(REG_SYS_CTL, tmpreg);

    rf_read_reg(REG_SYS_CTL);

    return OK;
}

/**
 * @brief 读取数据包计数寄存器
 * @param[in] 无
 * @return 数据包计数
 */
uint16_t rf_read_pkt_cnt(void)
{
    uint8_t reg_low, reg_high;
    uint16_t pkt_cnt;

    reg_low = rf_read_spec_page_reg(PAGE1_SEL, 0x6c);
    reg_high = rf_read_spec_page_reg(PAGE1_SEL, 0x6d);

    pkt_cnt = (reg_high << 8) | reg_low;

    return pkt_cnt;
}

/**
 * @brief 清除数据包计数寄存器
 * @param[in] 无
 * @return 无
 */
void rf_clr_pkt_cnt(void)
{
    uint8_t tmpreg;

    tmpreg = rf_read_reg(REG_SYS_CTL);
    tmpreg |= 0x40;
    rf_write_reg(REG_SYS_CTL, tmpreg);

//    tmpreg = rf_read_reg(REG_SYS_CTL);
    tmpreg = (tmpreg & 0xbf);
    rf_write_reg(REG_SYS_CTL, tmpreg);
}

/**
 * @brief 使能自动增益控制（AGC）功能
 * @param[in] state AGC_OFF/AGC_ON，表示AGC的开关状态
 * @return 操作结果
 */
RF_Err_t rf_agc_enable(bool NewState)
{
    if(NewState == AGC_OFF)
    {
        RF_ASSERT(rf_set_spec_page_reg_bits(PAGE2_SEL, 0x06, BIT0));
    }
    else
    {
        RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE2_SEL, 0x06, BIT0));
    }

    return OK;
}

/**
 * @brief 配置自动增益控制（AGC）功能
 * @param[in] 无
 * @return 操作结果
 */
RF_Err_t rf_set_agc(bool NewState)
{
    RF_ASSERT(rf_agc_enable(NewState));//使能自动增益控制（AGC）功能 02 06
    // RF_ASSERT(rf_write_spec_page_regs(PAGE2_SEL, 0x0a, (uint8_t *)reg_agc_freq400, 40));
    RF_ASSERT(rf_write_spec_page_reg(PAGE2_SEL, 0x34, 0xef));

    return OK;
}

/**
 * @brief 进行基本配置以初始化射频
 * @param[in] 无
 * @return 操作结果
 */
RF_Err_t rf_ft_calibr(void)
{
    uint8_t i, tmpreg, cal[0x26] = {0};

    rf_efuse_on();//efuse功能启用

    for (i = 17; i < 20; i++)
    {
        cal[0x0d + i] = rf_efuse_read_encry_byte(0x3b, 0x5aa5, 0x0d + i);//读取efuse数据进行初始化
    }

    if (rf_efuse_read_encry_byte(0x3b, 0x5aa5, 0x1c) == 0x5a)
    {
        rf_write_spec_page_reg(PAGE2_SEL, 0x3d, 0xfd);
        
        if (cal[0x0d + 19] != 0)
        {
            rf_write_spec_page_reg(PAGE0_SEL, 0x45, cal[0x0d + 19]);
        }

        if (rf_efuse_read_encry_byte(0x3b, 0x5aa5, 0x0d) == MODEM_MPA)
        {
            RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x1C, cal[0x1E]&0x1F, 0x1F));
        }
        else if (rf_efuse_read_encry_byte(0x3b, 0x5aa5, 0x0d) == MODEM_MPB)
        {
            tmpreg = (0xc0 | (cal[0x1e] & 0x1f));
            rf_write_spec_page_reg(PAGE3_SEL, 0x1c, tmpreg);
        }

        rf_write_spec_page_reg(PAGE3_SEL, 0x1d, cal[0x1f]);
    }

    rf_efuse_off();

    return OK;
}

/**
 * @brief 射频寄存器配置
 * @param[in] 无
 * @return 无
 */
RF_Err_t rf_reg_cfg(void)
{
    for(int i = 0; i < sizeof(g_reg_cfg)/sizeof(pan_reg_cfg_t); i++)
    {
        RF_ASSERT(rf_write_spec_page_reg(g_reg_cfg[i].page, g_reg_cfg[i].addr, g_reg_cfg[i].value));
    }

    return OK;
}

/**
 * @brief 将射频模式从深度睡眠模式切换到待机3（STB3）模式
 * @param[in] 无
 * @return 操作结果
 */

RF_Err_t rf_init(void)
{
    uint8_t rstreg, porreg;
    porreg = rf_read_reg(0x04);//从当前页的寄存器中读取一个字节
    porreg |= 0x10;//电平转换使能
		rf_write_reg(0x04, porreg);//在当前页写入全局寄存器并检查
    rf_port.delayus(10);
    porreg &= 0xEF;//电平转换失能
    rf_write_reg(0x04, porreg);
    rstreg = rf_read_reg(REG_SYS_CTL);//读0x00数据
    rstreg &= 0x7F;//软复位
    rf_write_reg(REG_SYS_CTL, rstreg);//
    rf_port.delayus(10);
    rstreg |= 0x80;//不复位
    rf_write_reg(REG_SYS_CTL, rstreg);
    rf_port.delayus(10);
    rstreg &= 0x7F;//软复位
    rf_write_reg(REG_SYS_CTL, rstreg);
    rf_port.delayus(10);

    RF_ASSERT(rf_set_mode(RF_MODE_DEEP_SLEEP));//0x02,0 深度睡眠模式
    rf_port.delayus(10);
		
    RF_ASSERT(rf_set_mode(RF_MODE_SLEEP));//睡眠模式
    rf_port.delayus(10);
		
    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE3_SEL, 0x06, BIT5));//切换页面，读出值，切换页面写值|更改值
    rf_port.delayus(10);

    RF_ASSERT(rf_set_mode(RF_MODE_STB1));//STB1模式
    rf_port.delayus(10);
		
		RF_ASSERT(rf_set_dcdc_mode(DCDC_ON));
		rf_port.delayus(10);

    RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x26, 0x2f));//向特定页面的寄存器写入一个值 0x03 0x26 0x2f
    rf_port.delayus(10);

    RF_ASSERT(rf_write_reg(0x04, 0x36));//电平转换使能，1.2v复位
    rf_port.delayus(10);

    rf_port.tcxo_init();//进行rf XTAL IO初始化 引脚？？没用

    RF_ASSERT(rf_set_mode(RF_MODE_STB2));//STB2模式
    rf_port.delayus(150);

    RF_ASSERT(rf_set_mode(RF_MODE_STB3));//STB3模式
    rf_port.delayus(10);

    RF_ASSERT(rf_ft_calibr());//进行基本配置以初始化射频 频率校准操作

    RF_ASSERT(rf_reg_cfg());//射频寄存器配置

    RF_ASSERT(rf_set_agc(AGC_ON));//配置自动增益控制（AGC）功能

//    rf_port.antenna_init();// 进行射频GPIO10,11初始化
		rf_set_gpio_output(11);
    return OK;
}

/**
*@将射频模式从睡眠模式更改为待机模式3（STB3）
*@param[i]<none>
*@返回结果
*/
//RF_Err_t rf_sleep_wakeup(void)
//{
//    // RF_ASSERT(rf_set_mode(RF_MODE_SLEEP));

//    // rf_port.delayus(10);

//    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE3_SEL, 0x06, BIT5));
//    rf_port.delayus(10);

//    RF_ASSERT(rf_set_mode(RF_MODE_STB1));
//    rf_port.delayus(10);

//    RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x26, 0x2f));
//    rf_port.delayus(10);

//    RF_ASSERT(rf_write_reg(0x04, 0x36));
//    rf_port.delayus(10);

//    rf_port.tcxo_init();

//    RF_ASSERT(rf_set_mode(RF_MODE_STB2));
//    rf_port.delayus(150);

//    RF_ASSERT(rf_set_mode(RF_MODE_STB3));

//    rf_port.delayus(10);
//    rf_port.antenna_init();

//    return OK;
//}

/**
*@将射频模式从待机3（STB3）更改为深度睡眠，射频在进入深度睡眠之前应设置DCDC_OFF
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_deepsleep(void)
{
	rf_port.antenna_close();
    rf_port.delayus(10);
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));
    rf_port.delayus(150);

    RF_ASSERT(rf_set_mode(RF_MODE_STB2));
    rf_port.delayus(10);

    RF_ASSERT(rf_set_mode(RF_MODE_STB1));
    rf_port.delayus(10);

		RF_ASSERT(rf_set_dcdc_mode(DCDC_OFF));
		rf_port.delayus(10);
//    rf_port.tcxo_close();

    RF_ASSERT(rf_write_reg(0x04, 0x06));
    rf_port.delayus(10);

    RF_ASSERT(rf_set_mode(RF_MODE_SLEEP));
    rf_port.delayus(10);

    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE3_SEL, 0x06, BIT5));
    rf_port.delayus(10);

    RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x26, 0x0f));
    rf_port.delayus(10);

    RF_ASSERT(rf_set_mode(RF_MODE_DEEP_SLEEP));

    return OK;
}

/**
*@将射频模式从待机3（STB3）更改为睡眠，射频应在进入睡眠前设置DCDC_OFF
*@param[i]<none>
*@返回结果
 */
//RF_Err_t rf_sleep(void)
//{
//	rf_port.antenna_close();

//    RF_ASSERT(rf_set_mode(RF_MODE_STB3));
//    rf_port.delayus(150);

//    RF_ASSERT(rf_set_mode(RF_MODE_STB2));
//    rf_port.delayus(10);

//    RF_ASSERT(rf_set_mode(RF_MODE_STB1));

//    rf_port.delayus(10);
////    rf_port.tcxo_close();

//    RF_ASSERT(rf_write_reg(0x04, 0x16));
//    rf_port.delayus(10);

//    RF_ASSERT(rf_set_mode(RF_MODE_SLEEP));

//    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE3_SEL, 0x06, BIT5));
//    rf_port.delayus(10);

//    RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x26, 0x0f));

//    return OK;
//}

/**
*@设置LO频率
*@param[i]<lo>lo频率
*400M/800M
*@返回结果
 */
RF_Err_t rf_set_lo_freq(uint32_t lo)
{
    if(lo == LO_400M)
    {
        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x3D, 1, BIT6|BIT5|BIT4));
    }
    else // if(lo == LO_800M)
    {
        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x3D, 0, BIT6|BIT5|BIT4));
    }

    return OK;
}

/**
 * @brief 设定频率
*要设置的@param[i]<freq>射频频率（单位：Hz）
*@返回结果
 */
RF_Err_t rf_set_freq(uint32_t freq)
{
    uint8_t lowband_sel = 0; // 400M 800M is same value?
    float tmp_var;
    int integer_part;
    float fractional_part;
    int fb, fc;

//    if(freq < freq_405000000 || freq > freq_1080000000)
//    {
//        return FAIL;
//    }

//    if (freq < 800000000)
//    {
        RF_ASSERT(rf_write_spec_page_regs(PAGE2_SEL, 0x0A, (uint8_t *)reg_agc_freq400, 40));
//    }
//    else
//    {
//        RF_ASSERT(rf_write_spec_page_regs(PAGE2_SEL, 0x0A, (uint8_t *)reg_agc_freq800, 40));
//    }
//    for(int i = 0; i < 16; i++)
	for(int i = 0; i < 8; i++)
    {
			if(freq > freq_table[i][0] && freq <= freq_table[i][1])
        {
            RF_ASSERT(rf_write_spec_page_reg(PAGE0_SEL, 0x40, freq_param_table[i]));
            break;
        }
//        if(freq == freq_405000000 || freq == freq_810000000)
//        {
//            RF_ASSERT(rf_write_spec_page_reg(PAGE0_SEL, 0x40, 0x1A));
//            break;
//        }
//        else if(freq > freq_table[i][0] && freq <= freq_table[i][1])
//        {
//            RF_ASSERT(rf_write_spec_page_reg(PAGE0_SEL, 0x40, freq_param_table[i]));
//            break;
//        }
    }

//    if (freq < 800000000)
//    {
        tmp_var = freq * 4.0 / 32000000;
        rf_set_lo_freq(LO_400M);
//    }
//    else
//    {
//        tmp_var = freq * 2.0 / 32000000;
//        rf_set_lo_freq(LO_800M);
//    }

    integer_part = (int)tmp_var; //保留整数部分
    fractional_part = tmp_var - integer_part; //保留小数部分

    fb = integer_part - 20;
    fc = (int)(fractional_part * 1600 / (2 * (1 + lowband_sel)));

    uint8_t temp_fx[3] = {fb & 0xff, fc & 0xff, (fc >> 8) & 0x0f};
    RF_ASSERT(rf_write_spec_page_regs(PAGE3_SEL, 0x15, temp_fx, 3));

    uint8_t temp_freq[4] = {freq & 0xff, (freq >> 8) & 0xff, (freq >> 16) & 0xff, (freq >> 24) & 0xff};
    RF_ASSERT(rf_write_spec_page_regs(PAGE3_SEL, 0x09, temp_freq, 4));

    return OK;
}

/**
 * @brief 读取频率（Hz）
 * @param[in] <none>
 * @return 频率（Hz）
 */
uint32_t rf_read_freq(void)
{
    uint32_t freq;
    uint8_t temp[4];

    rf_read_spec_page_regs(PAGE3_SEL, 0x09, temp, 4);
    freq = ((uint32_t)temp[3] << 24) | ((uint32_t)temp[2] << 16) | ((uint32_t)temp[1] << 8) | (uint32_t)temp[0];

    return freq;
}

/**
 * @brief 计算tx时间
 * @param[in] <size> tx len
 * @return tx time(us)
 */
uint32_t rf_get_tx_time(uint8_t size)
{
    uint8_t sf = rf_get_sf();
    uint8_t cr = rf_get_code_rate();
    uint8_t bw = rf_get_bw();
    uint8_t ldr = rf_get_ldr();
    uint32_t preamble = rf_get_preamble();

    const float bw_table[4] = {62.5, 125, 250, 500};

    if (bw < BW_62_5K || bw > BW_500K)
    {
        return 0;
    }

    float symbol_len = (float)(1 << sf) / bw_table[bw - BW_62_5K]; // symbol length
    float preamble_time;                                // preamble time:ms
    float payload_time = 0;                             // payload time:ms
    float total_time;                                   // total time:ms

    if (sf < 7)
    {
        preamble_time = (preamble + 6.25f) * symbol_len;
        payload_time = ceil((float)(size * 8 - sf * 4 + 36) / ((sf - ldr * 2) * 4));
    }
    else
    {
        preamble_time = (preamble + 4.25f) * symbol_len;
        payload_time = ceil((float)(size * 8 - sf * 4 + 44) / ((sf - ldr * 2) * 4));
    }

    payload_time = payload_time * (cr + 4);
    payload_time = payload_time + 8;
    payload_time = payload_time * symbol_len;
    total_time = (preamble_time + payload_time) * 1000;

    return (uint32_t)total_time;
}

/**
 * @brief 检查ldr是否应该打开
 * 
 * @param sf 
 * @param bw 
 * @return true 
 * @return false 
 */
bool rf_should_turnon_ldr(uint8_t sf, uint8_t bw) 
{
    if (sf == SF_11)
    {
        if (bw == BW_62_5K || bw == BW_125K)
        {
            return true;
        }
    }
    else if (sf == SF_12)
    {
        if (bw == BW_62_5K || bw == BW_125K || bw == BW_250K)
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief 设置带宽
 * @param[in] <bw_val> 值与带宽有关
 *			  BW_62_5K / BW_125K / BW_250K / BW_500K
 * @return result
 */
RF_Err_t rf_set_bw(uint8_t bw)
{
    uint8_t sf, ldr;

    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x0D, bw, BIT7|BIT6|BIT5|BIT4));

    sf = rf_get_sf();
    
    ldr = rf_should_turnon_ldr(sf, bw);

    rf_set_ldr(ldr);

//    if (bw == BW_62_5K || bw == BW_125K || bw == BW_250K)
//    {
        RF_ASSERT(rf_set_spec_page_reg_bits(PAGE2_SEL, 0x3F, BIT1));
//    }
//    else
//    {
//        RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE2_SEL, 0x3F, BIT1));
//    }

    return OK;
}

/**
 * @brief 读取带宽
 * @param[in] <none>
 * @return bandwidth
 */
uint8_t rf_get_bw(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE3_SEL, 0x0d);

    return (tmpreg >> 4);
}

/**
 * @brief 设定传播因子
 * @param[in] <sf> 设定的扩散系数
 *			 SF_5 / SF_6 /SF_7 / SF_8 / SF_9 / SF_10 / SF_11 / SF_12
 * @return result
 */
RF_Err_t rf_set_sf(uint8_t sf)
{
    uint8_t bw, ldr;
	
//    if(sf < SF_5 || sf > SF_12)
//    {
//        return FAIL;
//    }

    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x0E, sf, BIT7|BIT6|BIT5|BIT4));

    bw = rf_get_bw();
    
    ldr = rf_should_turnon_ldr(sf, bw);

    rf_set_ldr(ldr);

    return OK;
}

/**
 * @brief 读取扩散因子
 * @param[in] <none>
 * @return Spreading Factor
 */
uint8_t rf_get_sf(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE3_SEL, 0x0E);

    return (tmpreg >> 4);
}

/**
 * @brief 设置有效载荷CRC
 * @param[in] <NewState> CRC to set
 *			  CRC_ON / CRC_OFF
 * @return result
 */
RF_Err_t rf_set_crc(bool NewState)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x0E, NewState, BIT3));

    return OK;
}

/**
 * @brief 读取有效载荷CRC
 * @param[in] <none>
 * @return CRC status
 */
uint8_t rf_get_crc(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE3_SEL, 0x0e);

    return (tmpreg & 0x08) >> 3;
}

/**
 * @brief 设定码率
 * @param[in] <code_rate> code rate to set
 *			  CODE_RATE_45 / CODE_RATE_46 / CODE_RATE_47 / CODE_RATE_48
 * @return result
 */
RF_Err_t rf_set_code_rate(uint8_t code_rate)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x0D, code_rate, BIT3|BIT2|BIT1));

    return OK;
}

/**
 * @brief 获取码率
 * @param[in] <none>
 * @return code rate
 */
uint8_t rf_get_code_rate(void)
{
    uint8_t tmpreg;
	uint8_t code_rate;

    tmpreg = rf_read_spec_page_reg(PAGE3_SEL, 0x0D);
    code_rate = ((tmpreg & 0x0E) >> 1); // BIT3|BIT2|BIT1

    return code_rate;
}

/**
 * @brief 设置射频模式
 * @param[in] <mode>
 *			  RF_MODE_DEEP_SLEEP / RF_MODE_SLEEP
 *			  RF_MODE_STB1 / RF_MODE_STB2
 *			  RF_MODE_STB3 / RF_MODE_TX / RF_MODE_RX
 * @return result
 */
RF_Err_t rf_set_mode(uint8_t mode)
{
//	if(my_Mode!=mode)
//	{
//		my_Mode=mode;
    RF_ASSERT(rf_write_reg(REG_OP_MODE, mode));
//	}
    return OK;
}

/**
 * @brief获取射频模式
 * @param[in] <none>
 * @return mode
 *		   RF_MODE_DEEP_SLEEP / RF_MODE_SLEEP
 *		 RF_MODE_STB1 / RF_MODE_STB2
 *		 RF_MODE_STB3 / RF_MODE_TX / RF_MODE_RX
 */
uint8_t rf_get_mode(void)
{
    return rf_read_reg(REG_OP_MODE);
}

/**
 * @brief 设置射频发射模式
 * @param[in] <tx_mode>
 *			  RF_TX_SINGLE/RF_TX_CONTINOUS
 * @return result
 */
RF_Err_t rf_set_tx_mode(uint8_t tx_mode)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x06, tx_mode, BIT2));

    return OK;
}

/**
 * @brief 设置射频接收模式
 * @param[in] <mode>
 *			  RF_RX_SINGLE/RF_RX_SINGLE_TIMEOUT/RF_RX_CONTINOUS
 * @return result
 */
RF_Err_t rf_set_rx_mode(uint8_t rx_mode)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x06, rx_mode, BIT1|BIT0));

    return OK;
}

/**
 * @brief 设置调制解调器模式
 * @param[in] <modem_mode>
 *			  MODEM_MODE_NORMAL / MODEM_MODE_MULTI_SECTOR
 * @return result
 */
RF_Err_t rf_set_modem_mode(uint8_t modem_mode)
{
    if (modem_mode == MODEM_MODE_NORMAL)
    {
        RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x0b, 0x08));
    }
    else if (modem_mode == MODEM_MODE_MULTI_SECTOR)
    {
        RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x0b, 0x18));
        RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x2f, 0x54));
        RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x30, 0x40));
    }

    return OK;
}

/**
 * @brief 设置Rx超时。它在RF_RX_SINGLE_TIMEOUT模式下很有用
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
RF_Err_t rf_set_rx_single_timeout(uint16_t timeout)
{
    uint8_t temp[2] = {timeout & 0xff, (timeout >> 8) & 0xff};
    RF_ASSERT(rf_write_spec_page_regs(PAGE3_SEL, 0x07, temp, 2));

    return OK;
}

/**
 * @brief 获取信噪比值
 * @param[in] <none>
 * @return snr
 */
float rf_get_snr(void)
{
    float snr_val;
    uint32_t sig_pow_val;
    uint32_t noise_pow_val;
    uint32_t sf_val;
    uint8_t temp[3];

    rf_read_spec_page_regs(PAGE2_SEL, 0x71, temp, 3);
    noise_pow_val = ((temp[2] << 16) | (temp[1] << 8) | temp[0]);

    rf_read_spec_page_regs(PAGE1_SEL, 0x74, temp, 3);
    sig_pow_val = ((temp[2] << 16) | (temp[1] << 8) | temp[0]);

    sf_val = (rf_read_spec_page_reg(PAGE1_SEL, 0x7c) & 0xf0) >> 4;

    if (noise_pow_val == 0)
    {
        noise_pow_val = 1;
    }

    snr_val = (float)(10 * log10((sig_pow_val / (2 << sf_val)) / noise_pow_val));

    return snr_val;
}

/**
 * @brief 获取rssi值
 * @param[in] <none>
 * @return rssi
 */
int8_t rf_get_rssi(void)
{
    return rf_read_spec_page_reg(PAGE1_SEL, 0x7F);
}

/**
 * @brief 当前信道能量检测
 * @param[in] <none>
 * @return rssi
 */
int8_t rf_get_channel_rssi(void)
{
    int8_t rssi_energy;

    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE2_SEL, 0x06, BIT2));
    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE2_SEL, 0x06, BIT2));

    rssi_energy = rf_read_spec_page_reg(PAGE1_SEL, 0x7E);

    return rssi_energy;
}

/**
 * @brief 设置发射功率
 * @param[in] <tx_power> open gears (range in 1--23（405MHz-565MHz）1-22(868/915MHz))
 * @return result
 */
RF_Err_t rf_set_tx_power(uint8_t tx_power)
{
    uint8_t tmp_value1, tmp_value2, pa_bias;
//    uint32_t freq;//, pwr_table;

//    if (tx_power < RF_MIN_RAMP)
//    {
//        tx_power = RF_MIN_RAMP;
//    }

//    freq = rf_read_freq();

//    if ((freq >= freq_405000000) && (freq <= freq_565000000))
//    {
//        if (tx_power > RF_MAX_RAMP + 1)
//        {
//            tx_power = RF_MAX_RAMP;
//        }

        /* 调制波斜坡模式*/
        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x1E, power_ramp_cfg[tx_power - 1].ramp, 0x3F));
        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x4B, power_ramp_cfg[tx_power - 1].pa_ldo >> 4, 0x0F));
        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x22, power_ramp_cfg[tx_power - 1].pa_ldo & 0x01, BIT0));

//        if (power_ramp_cfg[tx_power - 1].pa_duty != 0x70)
//        {
//            RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x46, BIT2));
//        }
//        else
//        {
            RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE0_SEL, 0x46, BIT2));
//        }
        rf_efuse_on();
        pa_bias = rf_efuse_read_encry_byte(0x3b, 0x5aa5, 0x0d + 19);
        rf_efuse_off();
        if (pa_bias == 0)
        {
            pa_bias = 8;
        }

        tmp_value1 = pa_bias - (power_ramp_cfg[tx_power - 1].pa_duty & 0x0f);
        tmp_value2 = (power_ramp_cfg[tx_power - 1].pa_duty & 0xf0) | tmp_value1;
        RF_ASSERT(rf_write_spec_page_reg(PAGE0_SEL, 0x45, tmp_value2));

        return OK;
//    }
//    else if ((freq >= freq_810000000) && (freq <= freq_890000000))
//    {
//        pwr_table = 2;
//    }
//    else if ((freq >= freq_890000000) && (freq <= freq_1080000000))
//    {
//        pwr_table = 3;
//    }
//    else
//    {
//        return FAIL;
//    }

//    if (tx_power > RF_MAX_RAMP)
//    {
//        tx_power = RF_MAX_RAMP;
//    }

//    // 调制波斜坡模式
//    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x1E, power_ramp[tx_power - 1][pwr_table].ramp, 0x3F));
//    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x4B, power_ramp[tx_power - 1][pwr_table].pa_trim, 0x0F));
//    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x22, power_ramp[tx_power - 1][pwr_table].pa_ldo, BIT0));

//    if (power_ramp[tx_power - 1][pwr_table].pa_duty != 0xff)
//    {
//        RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x46, BIT2));
//        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x45, power_ramp[tx_power - 1][pwr_table].pa_duty, BIT6|BIT5|BIT4));
//    }
//    else
//    {
//        RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE0_SEL, 0x46, BIT2));
//    }

//    return OK;
}

/**
 * @brief 获取tx_power
 * @param[in] <none>
 * @return 如果返回值为0，则表示获取tx_power失败
 */
//uint8_t rf_get_tx_power(void)
//{
//    uint8_t open_ramp, pa_trim, pa_ldo, pa_duty, pa_duty_en;
//    uint8_t i, pa_bias;
//    uint32_t freq, pwr_table;

//    open_ramp = rf_read_spec_page_reg(PAGE0_SEL, 0x1e) & 0x3f;
//    pa_trim = rf_read_spec_page_reg(PAGE0_SEL, 0x4b) & 0x0f;
//    pa_ldo = rf_read_spec_page_reg(PAGE3_SEL, 0x22) & 0x01;
//    pa_duty = ((rf_read_spec_page_reg(PAGE0_SEL, 0x45) & 0x70) >> 4);
//    pa_duty_en = ((rf_read_spec_page_reg(PAGE0_SEL, 0x46) & 0x04) >> 2);

//    freq = rf_read_freq();

//    if ((freq >= freq_405000000) && (freq <= freq_565000000))
//    {
//        rf_efuse_on();
//        pa_bias = rf_efuse_read_encry_byte(0x3b, 0x5aa5, 0x0d + 19);
//        rf_efuse_off();
//        if (pa_bias == 0)
//        {
//            pa_bias = 8;
//        }
//        pa_duty = rf_read_spec_page_reg(PAGE0_SEL, 0x45);
//        for (i = 0; i < RF_MAX_RAMP + 1; i++)
//        {
//            if (open_ramp == power_ramp_cfg[i].ramp)
//            {
//                if (((pa_trim << 4) | pa_ldo) == power_ramp_cfg[i].pa_ldo)
//                {
//                    if ((pa_duty_en == true) && ((pa_duty + (power_ramp_cfg[i].pa_duty & 0x0f)) == ((power_ramp_cfg[i].pa_duty & 0xf0) + pa_bias)))
//                    {
//                        return i + 1;
//                    }
//                    else if ((pa_duty_en == false) && ((pa_duty | 0x70) == ((power_ramp_cfg[i].pa_duty & 0xf0) + pa_bias)))
//                    {
//                        return i + 1;
//                    }
//                }
//            }
//        }
//        return 0;
//    }
//    else if ((freq >= freq_810000000) && (freq <= freq_890000000))
//    {
//        pwr_table = 2;
//    }
//    else if ((freq >= freq_890000000) && (freq <= freq_1080000000))
//    {
//        pwr_table = 3;
//    }
//    else
//    {
//        return FAIL;
//    }

//    for (i = 0; i < RF_MAX_RAMP; i++)
//    {
//        if (open_ramp == power_ramp[i][pwr_table].ramp)
//        {
//            if ((pa_trim == power_ramp[i][pwr_table].pa_trim) && (pa_ldo == power_ramp[i][pwr_table].pa_ldo))
//            {
//                if ((pa_duty_en == true) && (pa_duty == power_ramp[i][pwr_table].pa_duty))
//                {
//                    return i + 1;
//                }
//                else if ((pa_duty_en == false) && (0xff == power_ramp[i][pwr_table].pa_duty))
//                {
//                    return i + 1;
//                }
//            }
//        }
//    }

//    return 0;
//}

/**
 * @brief 设置前导码
 * @param[in] preamble
 * @return result
 */
RF_Err_t rf_set_preamble(uint16_t preamble)
{
    uint8_t temp[2] = {preamble & 0xff, (preamble >> 8) & 0xff};
    RF_ASSERT(rf_write_spec_page_regs(PAGE3_SEL, 0x13, temp, 2));

    return OK;
}

/**
 * @brief 获取前导码
 * @param[in] <none>
 * @return preamble
 */
uint16_t rf_get_preamble(void)
{
    uint8_t temp[2];

    rf_read_spec_page_regs(PAGE3_SEL, 0x13, temp, 2);

    return ((uint16_t)temp[1] << 8) | temp[0];
}

/**
 * @brief 将RF GPIO设置为输入
 * @param[in] <gpio_pin>  pin number of GPIO to be enable
 * @return result
 */
RF_Err_t rf_set_gpio_input(uint8_t gpio_pin)
{
    if(gpio_pin < 8)
    {
        RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x63, (1 << gpio_pin)));
    }
    else
    {
        RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x64, (1 << (gpio_pin - 8))));
    }

    return OK;
}

/**
 * @brief 将RF GPIO设置为输出
 * @param[in] <gpio_pin>  pin number of GPIO to be enable
 * @return result
 */
RF_Err_t rf_set_gpio_output(uint8_t gpio_pin)
{
    if(gpio_pin < 8)
    {
        RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x65, (1 << gpio_pin)));
    }
    else
    {
        RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x66, (1 << (gpio_pin - 8))));
    }

    return OK;
}

/**
 * @brief 设置GPIO输出状态，set或RESET
 * @param[in] <gpio_pin>  pin number of GPIO to be opearted
 *			<state>   0  -  reset,
 *					  1  -  set
 * @return result
 */
RF_Err_t rf_set_gpio_state(uint8_t gpio_pin, uint8_t state)
{
    if(gpio_pin < 8)
    {
        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x67, state, (1 << gpio_pin)));
    }
    else
    {
        RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x68, state, (1 << (gpio_pin - 8))));
    }

    return OK;
}

/**
 * @brief 获取GPIO输入状态
 * @param[in] <gpio_pin>  pin number of GPIO to be opearted
 *			<state>   0  -  low,
 *					  1  -  high
 * @return result
 */
bool rf_get_gpio_state(uint8_t gpio_pin)
{
    uint8_t tmpreg;

    if(gpio_pin < 6)
    {
        tmpreg = rf_read_spec_page_reg(PAGE0_SEL, 0x74);
    }
    else
    {
        tmpreg = rf_read_spec_page_reg(PAGE0_SEL, 0x75);
        gpio_pin -= 6;
    }

    return (bool)((tmpreg >> gpio_pin) & 0x01);
}

/**
 * @brief 启用CAD功能
 * @param[in] <none>
 * @return  result
 */
RF_Err_t rf_cad_on(uint8_t threshold, uint8_t chirps)
{
    rf_set_gpio_output(11);

    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE0_SEL, 0x5E, BIT6));
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE1_SEL, 0x25, chirps, BIT0|BIT1));
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x0f, threshold));

    return OK;
}

/* @brief CAD功能禁用
* @param[in] <none>
* @return  result
*/
RF_Err_t rf_cad_off(void)
{
    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x5E, BIT6));
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x0f, 0x0a));

    return OK;
}

/**
 * @brief set 射频同步字
 * @param[in] <sync> syncword
 * @return result
 */
RF_Err_t rf_set_syncword(uint8_t sync)
{
    RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x0f, sync));
    
    return OK;
}

/**
 * @brief 读取射频同步字
 * @param[in] <none>
 * @return syncword
 */
uint8_t rf_get_syncword(void)
{
   return rf_read_spec_page_reg(PAGE3_SEL, 0x0f);
}

/**
 * @brief发送一个数据包
 * @param[in] <buff> 缓冲区包含要发送的数据
 * @param[in] <len>要发送的数据长度
 * @return result
 */
RF_Err_t rf_send_packet(uint8_t *buff, int len)
{
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, REG_PAYLOAD_LEN, len));//向特定页面的寄存器写入一个值
    RF_ASSERT(rf_set_mode(RF_MODE_TX));//5

    rf_write_fifo(REG_FIFO_ACC_ADDR, buff, len);

    return OK;
}

/**
 * @brief 在非块方法中接收数据包，当没有数据时，它将返回0
 * @param[in] <buff> buffer provide for data to receive
 * @return length, it will return 0 when no data got
 */
uint8_t rf_recv_packet(uint8_t *buff)
{
    uint8_t len;

    len = rf_read_spec_page_reg(PAGE1_SEL, 0x7D);
    rf_read_fifo(REG_FIFO_ACC_ADDR, buff, len);

    return len;
}

/**
 * @brief 设置早期中断
 * @param[in] <earlyirq_val> PLHD IRQ to set
 *			  PLHD_IRQ_ON / PLHD_IRQ_OFF
 * @return result
 */
RF_Err_t rf_set_early_irq(bool NewState)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE1_SEL, 0x2B, NewState, BIT6));

    return OK;
}

/**
 * @brief 读取plhd irq状态
 * @param[in] <none>
 * @return plhd irq status
 */
bool rf_get_early_irq(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE1_SEL, 0x2B);

    return (bool)(!!(tmpreg&BIT6));
}

/**
 * @brief 设置plhd
 * @param[in] <addr>PLHD起始地址，范围：0..7f
 *			  <len> PLHD len
 *			  PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
RF_Err_t rf_set_plhd(uint8_t addr, uint8_t len)
{
    uint8_t tmpreg = ((addr & 0x7f) | (len << 7));

    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x2e, tmpreg));

    return OK;
}

/**
 * @brief 获取plhd读取注册表值
 * @param[in] <none>
 * @return <len> PLHD_LEN8 / PLHD_LEN16
 */
uint8_t rf_get_plhd_len(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE1_SEL, 0x2e);

    return ((tmpreg & 0x80) >> 7);
}

/**
 * @brief 设置plhd掩码
 * @param[in] <plhd_val> plhd mask to set
 *			  PLHD_ON / PLHD_OFF
 * @return result
 */
RF_Err_t rf_set_plhd_mask(uint8_t plhd_val)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x58, plhd_val, BIT4));

    return OK;
}

/**
 * @brief 读取plhd掩码
 * @param[in] <none>
 * @return plhd mask
 */
uint8_t rf_get_plhd_mask(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE0_SEL, 0x58);

    return tmpreg;
}

/**
 * @brief 在非块方法中接收数据包，当没有数据时，它将返回0
 * @param[in] <buff> buffer provide for data to receive
 *			  <len> PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
uint8_t rf_plhd_receive(uint8_t *buf, uint8_t len)
{
    if (len == PLHD_LEN8)
    {
		rf_read_spec_page_regs(PAGE2_SEL, 0x76, buf, 8);
        return 8;
    }
    else if (len == PLHD_LEN16)
    {
        rf_read_spec_page_regs(PAGE2_SEL, 0x76, buf, 10);
        rf_read_spec_page_regs(PAGE0_SEL, 0x76, &buf[10], 6);
        return 16;
    }

    return 0;
}

/**
 * @brief 打开rf plhd模式，rf将使用早期中断
 * @param[in] <addr> PLHD start addr,Range:0..7f
		      <len> PLHD len
			  PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
void rf_set_plhd_rx_on(uint8_t addr, uint8_t len)
{
    rf_set_early_irq(PLHD_IRQ_ON);
    rf_set_plhd(addr, len);
    rf_set_plhd_mask(PLHD_ON);
}

/**
 * @brief 关闭射频plhd模式
 * @param[in] <none>
 * @return result
 */
void rf_set_plhd_rx_off(void)
{
    rf_set_early_irq(PLHD_IRQ_OFF);
    rf_set_plhd_mask(PLHD_OFF);
}

/**
 * @brief 设置dcdc模式，默认配置为dcdc_OFF，rf应在进入睡眠/深度睡眠之前设置dcdc_OOFF
 * @param[in] <dcdc_val> dcdc switch
 *			  DCDC_ON / DCDC_OFF
 * @return result
 */
RF_Err_t rf_set_dcdc_mode(uint8_t dcdc_val)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x24, dcdc_val, BIT3));

    return OK;
}

/**
 * @brief 设置LDR模式
 * @param[in] <mode> LDR switch
 *			  LDR_ON / LDR_OFF
 * @return result
 */
RF_Err_t rf_set_ldr(uint32_t mode)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x12, mode, BIT3));

    return OK;
}

/**
 * @brief 获取LDR模式
 * @param[in] <none>
 * @return result LDR_ON / LDR_OFF
 */
bool rf_get_ldr(void)
{
    uint8_t tmpreg = rf_read_spec_page_reg(PAGE3_SEL, 0x12);

    return (bool)(!!(tmpreg&BIT3));
}

int calculate_chirp_count(int sf_range[], int size, int chirp_counts[])
{
    int i, j;
    
    for (i = 0; i < size; i++) {
        int sf = sf_range[i];
        int fft_length = 1<<sf;
        int chirp_points = fft_length;
        int rx_points = 0;
        
        for (j = 0; j < size; j++) {
            int rx_sf = sf_range[j];
            int rx_chirp_points = (1<<rx_sf)* 2;
            rx_points += rx_chirp_points;
        }
        
        int preamble_length = rx_points;
        int quotient = preamble_length / chirp_points;
        int remainder = preamble_length % chirp_points;
        int chirp_count;
        
        if (remainder > 0) {
            chirp_count = (quotient + 1) + 2;  // Add 2 for safety margin
        } else {
            chirp_count = quotient + 2;
        }
        
        if (chirp_count < 8) {
            chirp_count = 8;
        }
        
        chirp_counts[i] = chirp_count;
    }
    
    return size;
}

/**
 * @brief 通过扩频因子设置前导码，这在all_sf_search模式下很有用
 * @param[in] <sf> Spreading Factor
 * @return result
 */
RF_Err_t rf_set_auto_sf_tx_preamble(int sf, int sf_range[], int size, int chirp_counts[])
{
	for (int i = 0; i < size; i++) {
		if( sf == sf_range[i])
		{
			RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x13, (chirp_counts[i]& 0xff)));
			RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x14, ((chirp_counts[i] >> 8) & 0xff)));
			return OK;
		}
	}
	
    return FAIL;
}

/**
 * @brief 打开所有sf自动搜索模式
 * @param[in] <none>
 * @return result
 */
RF_Err_t rf_set_auto_sf_rx_on(int sf_range[], int size)
{
    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE3_SEL, 0x12, BIT0));
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x25, 0x04));
	
	uint8_t sf_mask = 0;
	for (int i = 0; i < size; i++) {

		sf_mask |= (1 << (sf_range[i] - 5));
		
	}

    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x2d, sf_mask));

    return OK;
}

/**
 * @brief 关闭所有sf自动搜索模式
 * @param[in] <none>
 * @return result
 */
RF_Err_t rf_set_auto_sf_rx_off(void)
{
    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE3_SEL, 0x12, BIT0));
    RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x14, 0));
    RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, 0x13, 8));

    return OK;
}

/**
 * @brief 打开carrier_wave模式，在调用此函数之前设置BW和SF
 * @param[in] <none>
 * @return result
 */
RF_Err_t rf_set_carrier_wave_on(void)
{
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));

    RF_ASSERT(rf_set_tx_mode(RF_TX_CONTINOUS));
    RF_ASSERT(rf_set_tx_power(RF_MAX_RAMP));

    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE1_SEL, 0x1E, BIT0));

    return OK;
}

/**
 * @brief 设置载波模式频率并发送载波
 * @param[in] <freq> RF frequency(in Hz) to set
 * @return result
 */
RF_Err_t rf_set_carrier_wave_freq(uint32_t freq)
{
    uint8_t buf[1];

    RF_ASSERT(rf_set_mode(RF_MODE_STB3));
    RF_ASSERT(rf_set_tx_mode(RF_TX_CONTINOUS));
    RF_ASSERT(rf_set_freq(freq));
    RF_ASSERT(rf_set_ldo_pa_on());

    rf_port.set_tx();

    RF_ASSERT(rf_send_packet(buf, 1));

    return OK;
}

/**
 * @brief 关闭载波模式
 * @param[in] <none>
 * @return result
 */
RF_Err_t rf_set_carrier_wave_off(void)
{
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));
    RF_ASSERT(rf_set_ldo_pa_off());
    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE1_SEL, 0x1E, BIT0));

    return OK;
}

/**
 * @brief 设置mapm模式启用
 * @param[in] <none>
 * @return result
 */
RF_Err_t rf_mapm_en(void)
{
    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE1_SEL, 0x38, BIT0));

    return OK;
}

/**
 * @brief 设置mapm模式禁用
 * @param[in] <none>
 * @return result
 */
RF_Err_t rf_mapm_dis(void)
{
    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE1_SEL, 0x38, BIT0));

    return OK;
}

/**
 * @brief 设置mapm掩码
 * @param[in] <mapm_val> mapm mask to set
 *			  MAPM_ON / MAPM_OFF
 * @return result
 */
RF_Err_t rf_set_mapm_mask(uint8_t mapm_val)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE0_SEL, 0x58, mapm_val, !BIT6));

    return OK;
}

/**
 * @brief 获取字段数
 * @param[in] <none>

 * @return <fn>
 */
uint8_t rf_get_mapm_field_num(void)
{
    uint8_t reg_fn, fn_h, fn_l, fn;

    reg_fn = rf_read_spec_page_reg(PAGE1_SEL, 0x3d);
    fn_h = ((reg_fn >> 4) - 1) * 15;
    fn_l = (reg_fn & 0x0f) - 1;
    fn = fn_h + fn_l;

    return fn;
}

/**
 * @brief 设置字段数（范围在0x01~0xe0之间）
 * @param[in] <fn> the number of fields you want to set

 * @return result
 */
RF_Err_t rf_set_mapm_field_num(uint8_t fn)
{
    uint8_t reg_fn, fn_h, fn_l;

    fn_h = fn / 15 + 1;
    fn_l = fn % 15 + 1;
    reg_fn = (fn_h << 4) + fn_l;
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x3d, reg_fn));

    return OK;
}

/**
 * @brief 设置字段计数器的单位码字，表示多个字段
 * @param[in] <fnm> the represents number you want to set
              0--1
              1--2
              2--4
              3--8
 * @return result
 */
RF_Err_t rf_set_mapm_field_num_mux(uint8_t fnm)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE1_SEL, 0x37, fnm, BIT7|BIT6));

    return OK;
}

/**
 * @brief 设置最后一个组功能选择
 * @param[in] <group_fun_sel> The last group in the Field, its ADDR position function selection
 *             0:ordinary address      1:Field counter
 * @return result
 */
RF_Err_t rf_set_mapm_group_fun_sel(uint8_t gfs)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE1_SEL, 0x38, gfs, BIT1));

    return OK;
}

/**
 * @brief 在字段中设置组数
 * @param[in] <gn> the number of groups

 * @return result
 */
RF_Err_t rf_set_mapm_group_num(uint8_t gn)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE1_SEL, 0x38, gn, BIT3|BIT2));

    return OK;
}

/**
 * @brief 设置第一组中的前导码数量
 * @param[in] <pgl> The numbers want set to Preambles in first groups(at least 10)

 * @return result
 */
RF_Err_t rf_set_mapm_firgroup_preamble_num(uint8_t pgl)
{
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x3b, pgl));

    return OK;
}

/**
 * @brief 为第一组以外的组设置前导码的数量
 * @param[in] <pgn>  the number of Preambles in other groups
 * @return result
 */
RF_Err_t rf_set_mapm_group_preamble_num(uint8_t pgn)
{
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x3c, pgn));

    return OK;
}

/**
 * @brief 设置mapm模式的组地址1
 * @param[in] <addr> The value of group address1 you want to set

 * @return result
 */
RF_Err_t rf_set_mapm_neces_preamble_num(uint16_t pn)
{
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE1_SEL, 0x39, (uint8_t)(pn >> 8), 0x0F));
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x3A, (uint8_t)(pn)));

    return OK;
}

/**
 * @brief 设置mapm模式的组地址4
 * @param[in] <addr> The value of group address4 you want to set

 * @return result
 */
RF_Err_t rf_set_mapm_addr(uint8_t addr_no, uint8_t addr)
{
    RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x3e + addr_no, addr));

    return OK;
}

/**
 * @brief 计算mapm前导码可以休眠的时间
 * @param[in] <none>
 * @return sleeptime(ms)
 */
uint32_t rf_calculate_mapm_preambletime(stc_mapm_cfg_t *mapm_cfg, uint32_t one_chirp_time)
{
    uint8_t fnm, gn, pgn, pg1, fn, pn;
    uint16_t one_field_chirp, chirp_num;
    uint32_t preamble_time;

    pn = mapm_cfg->pn;
    pgn = mapm_cfg->pgn;
    pg1 = mapm_cfg->pg1;
    gn = mapm_cfg->gn;
    fnm = mapm_cfg->fnm;
    fn = mapm_cfg->fn;
    one_field_chirp = pg1 + 2 + (pgn + 2) * gn;
    chirp_num = (1 << fnm) * fn * one_field_chirp + pn - one_field_chirp;
    preamble_time = one_chirp_time * chirp_num;

    return preamble_time / 1000;
}

/**
 * @brief 打开rf mapm模式，rf将使用mapm中断
 * @param[in] <none>
 * @return result
 */
void rf_set_mapm_on(void)
{
    rf_mapm_en();
    rf_set_mapm_mask(MAPM_ON);
}

/**
 * @brief 关闭mapm模式
 * @param[in] <none>
 * @return result
 */
void rf_set_mapm_off(void)
{
    rf_mapm_dis();
    rf_set_mapm_mask(MAPM_OFF);
}

/**
 * @brief 配置mapm模式中使用的相关参数
 * @param[in]<p_mapm_cfg>
<fn>设置字段数（范围0x01~0xe0）
<field_num_mux>字段计数器的单位码字表示多个字段
<group_fun_sel>字段中的最后一个组，其ADDR位置函数选择
0：普通地址1：字段计数器
<gn>注册以配置字段中的组数
0 1组\1 2组\2 3组\3 4组
<pgl>设置第一组中的前导码数量>
<pgn>其他组中的前导码数量
<pn>发送完所有字段后同步字前的鸣叫次数
 * @return result
 */
void rf_set_mapm_cfg(stc_mapm_cfg_t *p_mapm_cfg)
{
    rf_set_mapm_field_num(p_mapm_cfg->fn);
    rf_set_mapm_field_num_mux(p_mapm_cfg->fnm);
    rf_set_mapm_group_fun_sel(p_mapm_cfg->gfs);
    rf_set_mapm_group_num(p_mapm_cfg->gn);
    rf_set_mapm_firgroup_preamble_num(p_mapm_cfg->pg1);
    rf_set_mapm_group_preamble_num(p_mapm_cfg->pgn);
    rf_set_mapm_neces_preamble_num(p_mapm_cfg->pn);
}

/**
 * @brief efuse功能启用
 * @param[in] <none>
 * @return  result
 */
RF_Err_t rf_efuse_on(void)
{
    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE2_SEL, 0x3E, BIT3));

    return OK;
}

/**
 * @brief efuse功能禁用
 * @param[in] <none>
 * @return  result
 */
RF_Err_t rf_efuse_off(void)
{
    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE2_SEL, 0x3E, BIT3));

    return OK;
}

/**
 * @brief 以未加密模式读取efuse区域数据
*@param[i]<reg_addr>使用寄存器地址，客户使用0x3c的固定设置
<efuse_addr>aaddress要读取efuse中的数据，客户的使用范围为0x2d~0x7f
*@返回数据
*/
uint8_t rf_efuse_read_byte(uint8_t reg_addr, uint8_t efuse_addr)
{
    uint8_t value = 0;
    uint16_t timeout = 100;

    efuse_addr <<= 1;
    rf_switch_page(PAGE2_SEL);
    rf_write_fifo(reg_addr, &efuse_addr, 1);
    do
    {
        if (rf_read_spec_page_reg(PAGE0_SEL, 0x6c) & 0x80)
        {
            break;
        }
    } while (timeout--);

    rf_switch_page(PAGE2_SEL);
    rf_read_fifo(reg_addr, &value, 1);

    return value;
}

/**
 * @brief 以未加密模式写入efuse区域数据
*@param[i]<reg_addr>使用寄存器地址，客户使用0x3c的固定设置
<efuse_addr>地址要在efuse中写入数据，客户的使用范围为0x2d~0x7f
<value>数据要写入efuse
*@return＜none＞
 */
void rf_efuse_write_byte(uint8_t reg_addr, uint8_t efuse_addr, uint8_t value)
{
    uint8_t data_buf[2];
    uint16_t timeout = 100;

    data_buf[0] = (efuse_addr << 1) | 0x01;
    data_buf[1] = value;

    rf_switch_page(PAGE2_SEL);
    rf_write_fifo(reg_addr, data_buf, 2);
    do
    {
        if (rf_read_spec_page_reg(PAGE0_SEL, 0x6c) & 0x80)
        {
            break;
        }
    } while (timeout--);
}

/**
 * @brief读取efuse数据进行初始化
*@返回数据
 */
uint8_t rf_efuse_read_encry_byte(uint8_t reg_addr, uint16_t pattern, uint8_t efuse_addr)
{
    uint8_t data_buf[3];
    uint8_t value = 0;
    uint16_t timeout = 100;

    data_buf[0] = pattern >> 8;
    data_buf[1] = pattern & 0xff;
    data_buf[2] = efuse_addr << 1;

    rf_switch_page(PAGE2_SEL);//切换页面2
    rf_write_fifo(reg_addr, data_buf, sizeof(data_buf));//向射频发送长数据FIFO，写入字节到寄存器
    do
    {
        if (rf_read_spec_page_reg(PAGE0_SEL, 0x6C) & BIT7)//高电平说明读写完成
        {
            break;
        }
    } while (timeout--);
    rf_switch_page(PAGE2_SEL);
    rf_read_fifo(reg_addr, &value, 1);//读数据

    return value;
}

/**
 * @brief 启用DCDC校准
*@param[i]<calibr_type>校准点
1--参考校准
2--零点校准
3--imax校准
*@返回结果
 */
RF_Err_t rf_set_dcdc_calibr_on(uint8_t calibr_type)
{
    if ((calibr_type < CALIBR_REF_CMP) || (calibr_type > CALIBR_IMAX_CMP))
    {
        return FAIL;
    }

    uint8_t loop_time = 5;
    uint8_t dcdc_cal = 0;
    uint8_t rd_data, wr_data;
    uint8_t offset_reg_addr;

    if (calibr_type == CALIBR_ZERO_CMP)
    {
        offset_reg_addr = 0x1E;
    }
    else if (calibr_type == CALIBR_REF_CMP)
    {
        offset_reg_addr = 0x1D;
    }
    else if (calibr_type == CALIBR_IMAX_CMP)
    {
        offset_reg_addr = 0x1C;
    }

    /* calibration on */
    RF_ASSERT(rf_write_spec_page_reg_bits(PAGE3_SEL, 0x20, calibr_type, BIT5|BIT6));

    for (; loop_time > 0; loop_time--)
    {
        dcdc_cal |= (0x01 << (loop_time - 1));
        wr_data = 0x80 | dcdc_cal;
        RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, offset_reg_addr, wr_data));

        rd_data = rf_read_spec_page_reg(PAGE3_SEL, 0x27);
        if (rd_data & 0x01)
        {
            dcdc_cal &= ~(0x01 << (loop_time - 1));
        }
        else
        {
            dcdc_cal |= (0x01 << (loop_time - 1));
        }
        wr_data = 0x80 | dcdc_cal;
        RF_ASSERT(rf_write_spec_page_reg(PAGE3_SEL, offset_reg_addr, wr_data));
    }

    return OK;
}

/**
 * @brief 禁用DCDC校准
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_set_dcdc_calibr_off(void)
{
    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE3_SEL, 0x20, BIT5|BIT6));

    return OK;
}

/**
 * @brief 启用LDO PA
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_set_ldo_pa_on(void)
{    
    RF_ASSERT(rf_set_spec_page_reg_bits(PAGE0_SEL, 0x4F, BIT3));

    return OK;
}

/**
 * @brief 禁用LDO PA
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_set_ldo_pa_off(void)
{
    RF_ASSERT(rf_reset_spec_page_reg_bits(PAGE0_SEL, 0x4F, BIT3));

    return OK;
}

/**
 * @brief rf进入rx连续模式接收数据包
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_enter_continous_rx(void)
{
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));

    rf_port.set_rx();

    RF_ASSERT(rf_set_rx_mode(RF_RX_CONTINOUS));
    RF_ASSERT(rf_set_mode(RF_MODE_RX));

    return OK;
}

/**
 * @brief rf进入rx单超时模式接收数据包
*@param[i]<timeout>rx单次超时时间（毫秒）
*@返回结果
 */
RF_Err_t rf_enter_single_timeout_rx(uint32_t timeout)
{
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));//4

    rf_port.set_rx();

    RF_ASSERT(rf_set_rx_mode(RF_RX_SINGLE_TIMEOUT));
    RF_ASSERT(rf_set_rx_single_timeout(timeout));
    RF_ASSERT(rf_set_mode(RF_MODE_RX));//6

    return OK;
}

/**
 * @brief rf进入rx单模接收数据包
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_enter_single_rx(void)
{
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));

    rf_port.set_rx();

    RF_ASSERT(rf_set_rx_mode(RF_RX_SINGLE));
    RF_ASSERT(rf_set_mode(RF_MODE_RX));

    return OK;
}

/**
 * @brief rf进入单tx模式并发送数据包
*@param[i]<buf>缓冲区包含要发送的数据
*@param[i]<size>要发送的数据长度
*@param[i]<tx_time>数据包tx时间（us）
*@返回结果
 */
#include "my_Database.h"
RF_Err_t rf_single_tx_data(uint8_t *buf, uint8_t size, uint32_t *tx_time)
{
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));//4

    RF_ASSERT(rf_set_ldo_pa_on());

    rf_port.set_tx();

    RF_ASSERT(rf_set_tx_mode(RF_TX_SINGLE));

    *tx_time = rf_get_tx_time(size);

    RF_ASSERT(rf_send_packet(buf, size));
	
    return OK;
}

/**
 * @briefrf进入连续tx模式准备发送数据包
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_enter_continous_tx(void)
{
    RF_ASSERT(rf_set_mode(RF_MODE_STB3));
    RF_ASSERT(rf_set_tx_mode(RF_TX_CONTINOUS));

    return OK;
}

/**
 * @brief 射频连续模式发送数据包
*@param[i]<buf>缓冲区包含要发送的数据
*@param[i]<size>要发送的数据长度
*@返回结果
 */
RF_Err_t rf_continous_tx_send_data(uint8_t *buf, uint8_t size)
{
    RF_ASSERT(rf_set_ldo_pa_on());

    rf_port.set_tx();

    RF_ASSERT(rf_send_packet(buf, size));

    return OK;
}

/**
 * @brief RF IRQ服务器例程，应在IRQ引脚的ISR处调用
*@param[i]<none>
*@返回结果
*/
void rf_irq_process(void)  
{  
    // 检查是否有中断发生  
    if(CHECK_IRQ())  
    {  
        // 获取中断状态寄存器的值，IRQ中断标志位  
        uint8_t irq = rf_get_irq();  
        // 如果接收到包头完成中断  
        if(irq & REG_IRQ_RX_PLHD_DONE)  
        {  
            // 读取包头长度  
            RxDoneParams.PlhdSize = rf_get_plhd_len();  
            // 接收包头数据，返回实际接收大小  
            RxDoneParams.PlhdSize = rf_plhd_receive(RxDoneParams.PlhdPayload, RxDoneParams.PlhdSize);  
            // 清除该中断标志位  
            irq &= ~REG_IRQ_RX_PLHD_DONE;  
            rf_clr_irq(REG_IRQ_RX_PLHD_DONE);  
            // 设置包头接收完成标志，通知上层处理  
            rf_set_recv_flag(RADIO_FLAG_PLHDRXDONE);  
        }  
        // 如果MAPM解码完成中断  
        if(irq & REG_IRQ_MAPM_DONE)  
        {  
            // 读取寄存器PAGE0_SEL页0x6e的值（MAPM解码结果）  
            uint8_t addr_val = rf_read_spec_page_reg(PAGE0_SEL, 0x6e);  
            // 将读取数据放入缓冲区，并递增缓冲索引  
            RxDoneParams.mpam_recv_buf[RxDoneParams.mpam_recv_index++] = addr_val;  
            // 清除该中断标志位  
            irq &= ~REG_IRQ_MAPM_DONE;  
            rf_clr_irq(REG_IRQ_MAPM_DONE);  
            // 设置MAPM接收完成标志  
            rf_set_recv_flag(RADIO_FLAG_MAPM);  
        }  
        // 如果收到完整数据包中断  
        if(irq & REG_IRQ_RX_DONE)  
        {  
            // 获取信号噪声比  
            RxDoneParams.Snr = rf_get_snr();  
            // 获取接收信号强度指数  
            RxDoneParams.Rssi = rf_get_rssi();  
            // 从FIFO中读取完整数据包，返回包长度  
            RxDoneParams.Size = rf_recv_packet(RxDoneParams.Payload);  
            // 清除该中断标志位  
            irq &= ~REG_IRQ_RX_DONE;  
            rf_clr_irq(REG_IRQ_RX_DONE);  
            // 设置接收完成标志  
            rf_set_recv_flag(RADIO_FLAG_RXDONE);  
        }  
        // 如果出现CRC校验错误中断  
        if(irq & REG_IRQ_CRC_ERR)  
        {  
            // 读取10字节的测试模式Payload数据，可能用于调试  
            rf_read_fifo(REG_FIFO_ACC_ADDR, RxDoneParams.TestModePayload, 10);  
            // 清除CRC错误中断标志位  
            irq &= ~REG_IRQ_CRC_ERR;  
            rf_clr_irq(REG_IRQ_CRC_ERR);  
            // 设置接收错误标志  
            rf_set_recv_flag(RADIO_FLAG_RXERR);  
        }  
        // 如果接收超时中断  
        if(irq & REG_IRQ_RX_TIMEOUT)  
        {  
            // 复位/刷新RF模块，重启接收  
            rf_refresh();  
            // 清除接收超时中断标志位  
            irq &= ~REG_IRQ_RX_TIMEOUT;  
            rf_clr_irq(REG_IRQ_RX_TIMEOUT);  
            // 设置接收超时标志  
            rf_set_recv_flag(RADIO_FLAG_RXTIMEOUT);  
        }  
        // 如果发送完成中断  
        if(irq & REG_IRQ_TX_DONE)  
        {  
            // 关闭PA供电，进入低功耗模式  
            rf_set_ldo_pa_off();  
            // 清除发送完成中断标志位  
            irq &= ~REG_IRQ_TX_DONE;  
            rf_clr_irq(REG_IRQ_TX_DONE);  
            // 设置发送完成标志  
            rf_set_transmit_flag(RADIO_FLAG_TXDONE);  
        }  
    }  
}  
/**
 * @brief 得到一个唧唧声时间
*@param[i]<bw>，<sf>
*@return<time>我们
 */
uint32_t rf_get_chirp_time(uint8_t bw, uint8_t sf)
{
    const uint32_t bw_table[4] = {62500, 125000, 250000, 500000};

    if(bw < BW_62_5K || bw > BW_500K)
    {
        return 0;
    }
    return (1000000 / bw_table[bw - BW_62_5K]) * (1 << sf);
}

/**
 * @brief检查cad rx是否处于非活动状态
*@param[i]<one_chirp_te>
*@return<结果>LEVEL_ACTIVE/LEVEL_INACTIVE
*/
//bool check_cad_rx_inactive(uint32_t one_chirp_time)
//{
//    rf_delay_us(one_chirp_time * 7);
//    rf_delay_us(360); //进入rx状态后的状态机启动时间

//    if (CHECK_CAD()!= 1)
//    {
//        rf_set_mode(RF_MODE_STB3);
//        return LEVEL_INACTIVE;
//    }
//    return LEVEL_ACTIVE;
//}

/**
 * @brief 设置射频默认参数
*@param[i]<none>
*@返回结果
 */
RF_Err_t rf_set_default_para(uint32_t freq)
{
    RF_ASSERT(rf_set_freq(freq));//设定频率
	
    RF_ASSERT(rf_set_code_rate(DEFAULT_CR));//设定码率

    RF_ASSERT(rf_set_bw(DEFAULT_BW));//设置带宽

    RF_ASSERT(rf_set_sf(DEFAULT_SF));//设定传播因子

    RF_ASSERT(rf_set_crc(CRC_ON));//设置有效载荷CRC
		RF_ASSERT(rf_set_tx_power(23));//DEFAULT_PWR));//设置低功耗发射功率最小1 最大23

    return OK;
}

