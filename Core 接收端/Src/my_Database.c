/**数据交互**/

#include "my_Database.h"
#include "crc.h"
#include "usart.h"

My_433 my_433;
my_modbus_typedef my_modbus;



#include "string.h"
#define Modbus_RTU 0
#define Modbus_TCP 1
#define Send_len 255
uint8_t Modbus_TX[Send_len];	//modbus回传数据

#define Modbus_addr my_433.ModbusADDR


void RTU_Send_CRC16(uint8_t* data,uint8_t Len)
{
	uint16_t CRC_Data=HAL_CRC_Calculate(&hcrc,(uint32_t *)data,Len-2);//让出2位CRC
	data[Len-1]=CRC_Data>>8;
	data[Len-2]=CRC_Data&0xff;
}

uint8_t RTU_Get_CRC16(uint8_t* data,uint8_t Len)
{
	uint16_t CRC_Data=(data[Len-1]<<8)|data[Len-2];
	if(CRC_Data==HAL_CRC_Calculate(&hcrc,(uint32_t *)data,Len-2))//让出2位CRC
	return 1;
		else return 0;
}


void INSERT_SEND(uint8_t i,int Data)
{
	Modbus_TX[i]=Data>>8;
	Modbus_TX[i+1]=Data&0xff;
}

void get_modbus_tx(uint16_t addr, uint16_t length,uint8_t XieYi)
{
	uint8_t i;
//	if(XieYi==Modbus_RTU)
//	{
		Modbus_TX[0] = Modbus_addr;
		Modbus_TX[1] = 0x03;
		Modbus_TX[2] = length*2;
		i=3;
//	}
//	else if(XieYi==Modbus_TCP)
//	{
//		Modbus_TX[8] = length*2;
//		Modbus_TX[5] = Modbus_TX[8]+3;
//		i=9;
//	}
	while(length--)
	{
		switch(++addr)
		{
			case 1:INSERT_SEND(i,my_433.ModbusADDR);//ADC0 40001
				break;
			case 2:INSERT_SEND(i,my_433.Channel+408);//ADC1
				break;
			case 3:INSERT_SEND(i,my_433.Number);//ADC2 
				break;
			case 4:INSERT_SEND(i,0);//ADC3 
				break;
			case 5:INSERT_SEND(i,0);//ADC4
				break;
			case 6:INSERT_SEND(i,0);//ADC5
				break;
			case 7:INSERT_SEND(i,0);//ADC6
				break;
			case 8:INSERT_SEND(i,my_433.err);//ADC7
				break;
			case 9:INSERT_SEND(i,
				(my_433.Device[0].DI<<1)|(my_433.Device[1].DI<<2)|(my_433.Device[2].DI<<3)|
				(my_433.Device[3].DI<<4)|(my_433.Device[4].DI<<5)|(my_433.Device[5].DI<<6)|
				(my_433.Device[6].DI<<7)|(my_433.Device[7].DI<<8)|(my_433.Device[8].DI<<9)|
				(my_433.Device[9].DI<<10)|(my_433.Device[10].DI<<11)|(my_433.Device[11].DI<<12)|
				(my_433.Device[12].DI<<13)|(my_433.Device[13].DI<<14)|(my_433.Device[14].DI<<15)
			);
				break;
			case 10:INSERT_SEND(i,
				(my_433.Device[0].Chrg<<1)|(my_433.Device[1].Chrg<<2)|(my_433.Device[2].Chrg<<3)|
				(my_433.Device[3].Chrg<<4)|(my_433.Device[4].Chrg<<5)|(my_433.Device[5].Chrg<<6)|
				(my_433.Device[6].Chrg<<7)|(my_433.Device[7].Chrg<<8)|(my_433.Device[8].Chrg<<9)|
				(my_433.Device[9].Chrg<<10)|(my_433.Device[10].Chrg<<11)|(my_433.Device[11].Chrg<<12)|
				(my_433.Device[12].Chrg<<13)|(my_433.Device[13].Chrg<<14)|(my_433.Device[14].Chrg<<15)
			);	
			break;//0 本机 1-15低功耗
			case 11:INSERT_SEND(i,my_433.Device[0].Weight);
				break;
			case 12:INSERT_SEND(i,my_433.Device[0].AngleX_int16);
				break;
			case 13:INSERT_SEND(i,my_433.Device[0].AngleY_int16);
				break;
			case 14:INSERT_SEND(i,my_433.Device[0].AngleZ_int16);
				break;
			case 15:INSERT_SEND(i,(my_433.Device[0].LowOption<<13)|my_433.Device[0].Battery);//电压
				break;
				
			case 16:INSERT_SEND(i,my_433.Device[1].Weight);
				break;
			case 17:INSERT_SEND(i,my_433.Device[1].AngleX_int16);
				break;
			case 18:INSERT_SEND(i,my_433.Device[1].AngleY_int16);
				break;
			case 19:INSERT_SEND(i,my_433.Device[1].AngleZ_int16);
				break;
			case 20:INSERT_SEND(i,(my_433.Device[1].LowOption<<13)|my_433.Device[1].Battery);//电压
				break;	
				
			case 21:INSERT_SEND(i,my_433.Device[2].Weight);
				break;
			case 22:INSERT_SEND(i,my_433.Device[2].AngleX_int16);
				break;
			case 23:INSERT_SEND(i,my_433.Device[2].AngleY_int16);
				break;
			case 24:INSERT_SEND(i,my_433.Device[2].AngleZ_int16);
				break;
			case 25:INSERT_SEND(i,(my_433.Device[2].LowOption<<13)|my_433.Device[2].Battery);//电压
				break;
				
			case 26:INSERT_SEND(i,my_433.Device[3].Weight);
				break;
			case 27:INSERT_SEND(i,my_433.Device[3].AngleX_int16);
				break;
			case 28:INSERT_SEND(i,my_433.Device[3].AngleY_int16);
				break;
			case 29:INSERT_SEND(i,my_433.Device[3].AngleZ_int16);
				break;
			case 30:INSERT_SEND(i,(my_433.Device[3].LowOption<<13)|my_433.Device[3].Battery);//电压
				break;
				
			case 31:INSERT_SEND(i,my_433.Device[4].Weight);
				break;
			case 32:INSERT_SEND(i,my_433.Device[4].AngleX_int16);
				break;
			case 33:INSERT_SEND(i,my_433.Device[4].AngleY_int16);
				break;
			case 34:INSERT_SEND(i,my_433.Device[4].AngleZ_int16);
				break;
			case 35:INSERT_SEND(i,(my_433.Device[4].LowOption<<13)|my_433.Device[4].Battery);//电压
				break;
				
			case 36:INSERT_SEND(i,my_433.Device[5].Weight);
				break;
			case 37:INSERT_SEND(i,my_433.Device[5].AngleX_int16);
				break;
			case 38:INSERT_SEND(i,my_433.Device[5].AngleY_int16);
				break;
			case 39:INSERT_SEND(i,my_433.Device[5].AngleZ_int16);
				break;
			case 40:INSERT_SEND(i,(my_433.Device[5].LowOption<<13)|my_433.Device[5].Battery);//电压
				break;
				
			case 41:INSERT_SEND(i,my_433.Device[6].Weight);
				break;
			case 42:INSERT_SEND(i,my_433.Device[6].AngleX_int16);
				break;
			case 43:INSERT_SEND(i,my_433.Device[6].AngleY_int16);
				break;
			case 44:INSERT_SEND(i,my_433.Device[6].AngleZ_int16);
				break;
			case 45:INSERT_SEND(i,(my_433.Device[6].LowOption<<13)|my_433.Device[6].Battery);//电压
				break;
				
			case 46:INSERT_SEND(i,my_433.Device[7].Weight);
				break;
			case 47:INSERT_SEND(i,my_433.Device[7].AngleX_int16);
				break;
			case 48:INSERT_SEND(i,my_433.Device[7].AngleY_int16);
				break;
			case 49:INSERT_SEND(i,my_433.Device[7].AngleZ_int16);
				break;
			case 50:INSERT_SEND(i,(my_433.Device[7].LowOption<<13)|my_433.Device[7].Battery);//电压
				break;
				
			case 51:INSERT_SEND(i,my_433.Device[8].Weight);
				break;
			case 52:INSERT_SEND(i,my_433.Device[8].AngleX_int16);
				break;
			case 53:INSERT_SEND(i,my_433.Device[8].AngleY_int16);
				break;
			case 54:INSERT_SEND(i,my_433.Device[8].AngleZ_int16);
				break;
			case 55:INSERT_SEND(i,(my_433.Device[8].LowOption<<13)|my_433.Device[8].Battery);//电压
				break;
				
			case 56:INSERT_SEND(i,my_433.Device[9].Weight);
				break;
			case 57:INSERT_SEND(i,my_433.Device[9].AngleX_int16);
				break;
			case 58:INSERT_SEND(i,my_433.Device[9].AngleY_int16);
				break;
			case 59:INSERT_SEND(i,my_433.Device[9].AngleZ_int16);
				break;
			case 60:INSERT_SEND(i,(my_433.Device[9].LowOption<<13)|my_433.Device[9].Battery);//电压
				break;
				
			case 61:INSERT_SEND(i,my_433.Device[10].Weight);
				break;
			case 62:INSERT_SEND(i,my_433.Device[10].AngleX_int16);
				break;
			case 63:INSERT_SEND(i,my_433.Device[10].AngleY_int16);
				break;
			case 64:INSERT_SEND(i,my_433.Device[10].AngleZ_int16);
				break;
			case 65:INSERT_SEND(i,(my_433.Device[10].LowOption<<13)|my_433.Device[10].Battery);//电压
				break;
				
			case 66:INSERT_SEND(i,my_433.Device[11].Weight);
				break;
			case 67:INSERT_SEND(i,my_433.Device[11].AngleX_int16);
				break;
			case 68:INSERT_SEND(i,my_433.Device[11].AngleY_int16);
				break;
			case 69:INSERT_SEND(i,my_433.Device[11].AngleZ_int16);
				break;
			case 70:INSERT_SEND(i,(my_433.Device[11].LowOption<<13)|my_433.Device[11].Battery);//电压
				break;
				
			case 71:INSERT_SEND(i,my_433.Device[12].Weight);
				break;
			case 72:INSERT_SEND(i,my_433.Device[12].AngleX_int16);
				break;
			case 73:INSERT_SEND(i,my_433.Device[12].AngleY_int16);
				break;
			case 74:INSERT_SEND(i,my_433.Device[12].AngleZ_int16);
				break;
			case 75:INSERT_SEND(i,(my_433.Device[12].LowOption<<13)|my_433.Device[12].Battery);//电压
				break;
				
			case 76:INSERT_SEND(i,my_433.Device[13].Weight);
				break;
			case 77:INSERT_SEND(i,my_433.Device[13].AngleX_int16);
				break;
			case 78:INSERT_SEND(i,my_433.Device[13].AngleY_int16);
				break;
			case 79:INSERT_SEND(i,my_433.Device[13].AngleZ_int16);
				break;
			case 80:INSERT_SEND(i,(my_433.Device[13].LowOption<<13)|my_433.Device[13].Battery);//电压
				break;
				
			case 81:INSERT_SEND(i,my_433.Device[14].Weight);
				break;
			case 82:INSERT_SEND(i,my_433.Device[14].AngleX_int16);
				break;
			case 83:INSERT_SEND(i,my_433.Device[14].AngleY_int16);
				break;
			case 84:INSERT_SEND(i,my_433.Device[14].AngleZ_int16);
				break;
			case 85:INSERT_SEND(i,(my_433.Device[14].LowOption<<13)|my_433.Device[14].Battery);//电压
				break;
				
			case 86:INSERT_SEND(i,(my_433.Device[0].myrssi<<8)|my_433.Device[0].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 87:INSERT_SEND(i,my_433.Device[0].OnlineTime>>16);
				break;
			case 88:INSERT_SEND(i,my_433.Device[0].OnlineTime);
				break;
			case 89:INSERT_SEND(i,0);//备用
				break;
			case 90:INSERT_SEND(i,0);//备用
				break;
				
			case 91:INSERT_SEND(i,(my_433.Device[1].myrssi<<8)|my_433.Device[1].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 92:INSERT_SEND(i,my_433.Device[1].OnlineTime>>16);
				break;
			case 93:INSERT_SEND(i,my_433.Device[1].OnlineTime);
				break;
			case 94:INSERT_SEND(i,0);//备用
				break;
			case 95:INSERT_SEND(i,0);//备用
				break;
				
			case 96:INSERT_SEND(i,(my_433.Device[2].myrssi<<8)|my_433.Device[2].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 97:INSERT_SEND(i,my_433.Device[2].OnlineTime>>16);
				break;
			case 98:INSERT_SEND(i,my_433.Device[2].OnlineTime);
				break;
			case 99:INSERT_SEND(i,0);//备用
				break;
			case 100:INSERT_SEND(i,0);//备用
				break;
				
			case 101:INSERT_SEND(i,(my_433.Device[3].myrssi<<8)|my_433.Device[3].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 102:INSERT_SEND(i,my_433.Device[3].OnlineTime>>16);
				break;
			case 103:INSERT_SEND(i,my_433.Device[3].OnlineTime);
				break;
			case 104:INSERT_SEND(i,0);//备用
				break;
			case 105:INSERT_SEND(i,0);//备用
				break;
				
			case 106:INSERT_SEND(i,(my_433.Device[4].myrssi<<8)|my_433.Device[4].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 107:INSERT_SEND(i,my_433.Device[4].OnlineTime>>16);
				break;
			case 108:INSERT_SEND(i,my_433.Device[4].OnlineTime);
				break;
			case 109:INSERT_SEND(i,0);//备用
				break;
			case 110:INSERT_SEND(i,0);//备用
				break;
				
			case 111:INSERT_SEND(i,(my_433.Device[5].myrssi<<8)|my_433.Device[5].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 112:INSERT_SEND(i,my_433.Device[5].OnlineTime>>16);
				break;
			case 113:INSERT_SEND(i,my_433.Device[5].OnlineTime);
				break;
			case 114:INSERT_SEND(i,0);//备用
				break;
			case 115:INSERT_SEND(i,0);//备用
				break;
				
			case 116:INSERT_SEND(i,(my_433.Device[6].myrssi<<8)|my_433.Device[6].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 117:INSERT_SEND(i,my_433.Device[6].OnlineTime>>16);
				break;
			case 118:INSERT_SEND(i,my_433.Device[6].OnlineTime);
				break;
			case 119:INSERT_SEND(i,0);//备用
				break;
			case 120:INSERT_SEND(i,0);//备用
				break;
				
			case 121:INSERT_SEND(i,(my_433.Device[7].myrssi<<8)|my_433.Device[7].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 122:INSERT_SEND(i,my_433.Device[7].OnlineTime>>16);
				break;
			case 123:INSERT_SEND(i,my_433.Device[7].OnlineTime);
				break;
			case 124:INSERT_SEND(i,0);//备用
				break;
			case 125:INSERT_SEND(i,0);//备用
				break;
				
			case 126:INSERT_SEND(i,(my_433.Device[8].myrssi<<8)|my_433.Device[8].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 127:INSERT_SEND(i,my_433.Device[8].OnlineTime>>16);
				break;
			case 128:INSERT_SEND(i,my_433.Device[8].OnlineTime);
				break;
			case 129:INSERT_SEND(i,0);//备用
				break;
			case 130:INSERT_SEND(i,0);//备用
				break;
				
			case 131:INSERT_SEND(i,(my_433.Device[9].myrssi<<8)|my_433.Device[9].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 132:INSERT_SEND(i,my_433.Device[9].OnlineTime>>16);
				break;
			case 133:INSERT_SEND(i,my_433.Device[9].OnlineTime);
				break;
			case 134:INSERT_SEND(i,0);//备用
				break;
			case 135:INSERT_SEND(i,0);//备用
				break;
				
			case 136:INSERT_SEND(i,(my_433.Device[10].myrssi<<8)|my_433.Device[10].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 137:INSERT_SEND(i,my_433.Device[10].OnlineTime>>16);
				break;
			case 138:INSERT_SEND(i,my_433.Device[10].OnlineTime);
				break;
			case 139:INSERT_SEND(i,0);//备用
				break;
			case 140:INSERT_SEND(i,0);//备用
				break;
				
			case 141:INSERT_SEND(i,(my_433.Device[11].myrssi<<8)|my_433.Device[11].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 142:INSERT_SEND(i,my_433.Device[11].OnlineTime>>16);
				break;
			case 143:INSERT_SEND(i,my_433.Device[11].OnlineTime);
				break;
			case 144:INSERT_SEND(i,0);//备用
				break;
			case 145:INSERT_SEND(i,0);//备用
				break;
				
			case 146:INSERT_SEND(i,(my_433.Device[12].myrssi<<8)|my_433.Device[12].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 147:INSERT_SEND(i,my_433.Device[12].OnlineTime>>16);
				break;
			case 148:INSERT_SEND(i,my_433.Device[12].OnlineTime);
				break;
			case 149:INSERT_SEND(i,0);//备用
				break;
			case 150:INSERT_SEND(i,0);//备用
				break;
				
			case 151:INSERT_SEND(i,(my_433.Device[13].myrssi<<8)|my_433.Device[13].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 152:INSERT_SEND(i,my_433.Device[13].OnlineTime>>16);
				break;
			case 153:INSERT_SEND(i,my_433.Device[13].OnlineTime);
				break;
			case 154:INSERT_SEND(i,0);//备用
				break;
			case 155:INSERT_SEND(i,0);//备用
				break;
				
			case 156:INSERT_SEND(i,(my_433.Device[14].myrssi<<8)|my_433.Device[14].rssi);//低功耗发射信号强度|接收板发送信号强度
				break;
			case 157:INSERT_SEND(i,my_433.Device[14].OnlineTime>>16);
				break;
			case 158:INSERT_SEND(i,my_433.Device[14].OnlineTime);
				break;
			case 159:INSERT_SEND(i,0);//备用
				break;
			case 160:INSERT_SEND(i,0);//备用
				break;
			
			default:break;
		}
		i+=2;
	}
}
uint8_t Ceshi[30];
uint8_t modbus_ack_write(uint16_t addr,uint8_t *data,uint8_t num)
{
//	uint8_t out=1;
//	while(num--){
//		switch(++addr)
//		{
//			case 11:OUT_data=Data_u16(data);break;//40011
//			case 12:OUT_FMQ_ZT=Data_u16(data);break;//40012
//			case 101:Bmq_Reset.Data=Data_u16(data);break;/*memcpy(Ceshi,my_modbus.buf,my_modbus.RX_len);*/break;//40101
//			case 102:Bmq_Reset.Data =Data_u16(data);break;
//			case 103:Bmq_Reset.Data|=Data_u16(data)<<1;break;
//			case 104:Bmq_Reset.Data|=Data_u16(data)<<2;break;
//			default:out=0;break;
//		}
//		data+=2;
//	}
//		return out;
}


void modbus_ack_handle(my_modbus_typedef* p)
{
	if( p->RX_len ) {//有数据
			if( p->buf[0] ==  Modbus_addr || p->buf[0] ==  0x00)//从机地址
				{
					if( RTU_Get_CRC16(p->buf,p->RX_len) ) {
						p->Function				 =p->buf[1];
						p->TX_addr   = ( p->buf[2]<<8 ) + p->buf[3];
						p->TX_length = ( p->buf[4]<<8 ) + p->buf[5];
					
						switch(p->Function)
						{
							case 0x03:
							case 0x04:
								if( p->TX_length <= (Send_len - 6)/2 ) 
									{
										get_modbus_tx(p->TX_addr , p->TX_length,Modbus_RTU);
										Modbus_TX[0]=p->buf[0];
										RTU_Send_CRC16( Modbus_TX , (p->TX_length)*2 + 5 ) ;//计算CRC16
										HAL_UART_Transmit_DMA(&huart2,(uint8_t *)Modbus_TX, (p->TX_length) *2 + 5);//回传数据
									}
							break;
							case 0x06://01 06 0001(地址) 0C02(数据) 5CCB(CRC)
								if(modbus_ack_write(p->TX_addr,&p->buf[4],1))
								HAL_UART_Transmit_DMA(&huart2,(uint8_t *)p->buf,p->RX_len);//回传原始数据
							break;
							case 0x10://01 10 0001(8 9地址) 0003(10 11寄存器数量) 06(12字节数) 0101(数据1) 0202(数据2) 0303(数据3) 6BDD
								if(p->buf[6]==(p->TX_length)*2 && p->buf[6]==(p->RX_len-9))//有效数据=寄存器数量X2 且 等于总长度-9
									{
										if(modbus_ack_write(p->TX_addr,&p->buf[7],p->buf[5]))
											RTU_Send_CRC16( p->buf , 8 ) ;//计算CRC16
										HAL_UART_Transmit_DMA(&huart2,(uint8_t *)p->buf, 8);//回传数据
									}
							break;
							default:break;
						}
					}else p->RX_len = 0;
		}else p->RX_len = 0;//接收一帧数据长度清零
	p->RX_len = 0;
	}
}
