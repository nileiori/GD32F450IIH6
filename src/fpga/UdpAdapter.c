
#include "UDPAdapter.h"
#include "SPIAdapter.h"
#include "DelayAdapter.h"

#include "w5500.h"
#include "stdbool.h"
#include "string.h"

TNetPara g_Udp_Para;

void Udp_Cs_Sel()
{
    Spi_CS(SPI_W5500, 0);
}

void Udp_Cs_Desel() 
{
    Spi_CS(SPI_W5500, 1);
}

u8 Udp_Spi_Read_Byte()
{
    return Spi_ReadWriteData(SPI_W5500, 0x00);
}

void Udp_Spi_Write_Byte(u8 wb)
{
    Spi_ReadWriteData(SPI_W5500, wb);
}

void Udp_Spi_Read_Burst(u8 *pBuf, u16 len)
{
    u16 i = 0;
    for(i=0; i<len; i++)
    {
        pBuf[i] = Spi_ReadWriteData(SPI_W5500, 0x00);
    }
}

void Udp_Spi_Write_Burst(u8 *pBuf, u16 len)
{
    u16 i = 0;
    for(i=0; i<len; i++)
    {
        Spi_ReadWriteData(SPI_W5500, pBuf[i]);
    }
}

void Udp_Para_Initial()
{
    g_Udp_Para.macAddress[0] = 0x0c;
    g_Udp_Para.macAddress[1] = 0x29;
    g_Udp_Para.macAddress[2] = 0xab;
    g_Udp_Para.macAddress[3] = 0x7c;
    g_Udp_Para.macAddress[4] = 0x04;
    g_Udp_Para.macAddress[5] = 0x02;

    g_Udp_Para.ipAddress[0] = 192;
    g_Udp_Para.ipAddress[1] = 168;
    g_Udp_Para.ipAddress[2] = 4;
    g_Udp_Para.ipAddress[3] = 41;

    g_Udp_Para.subMask[0] = 255;
    g_Udp_Para.subMask[1] = 255;
    g_Udp_Para.subMask[2] = 255;
    g_Udp_Para.subMask[3] = 0;

    g_Udp_Para.gateWay[0] = 192;
    g_Udp_Para.gateWay[1] = 168;
    g_Udp_Para.gateWay[2] = 4;
    g_Udp_Para.gateWay[3] = 1;

    g_Udp_Para.dnsAddress[0] = 0;
    g_Udp_Para.dnsAddress[1] = 0;
    g_Udp_Para.dnsAddress[2] = 0;
    g_Udp_Para.dnsAddress[3] = 0;

    g_Udp_Para.portNum = 5000;
    g_Udp_Para.socketNum = 0;

}

bool w5500_OpenSocket()
{
    // Open socket
    setSn_MR(g_Udp_Para.socketNum, Sn_MR_UDP);
    setSn_TOS(g_Udp_Para.socketNum, 1);
    setSn_CR(g_Udp_Para.socketNum, Sn_CR_OPEN);
    
    return true;
}

bool W5500_Init()
{
    bool retVal = true;
    u8 ar[16] = {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
    char chipId[6];
    wiz_NetInfo netConf;    
    wiz_NetTimeout to;
    to.retry_cnt = 8;
    to.time_100us = 2000;
    // Reset w5500
    ctlwizchip(CW_RESET_WIZCHIP,(void *)0);
    Delay_ms(10);
    // Initialize the buffer
    ctlwizchip(CW_INIT_WIZCHIP,ar);
    // Get chip id
    ctlwizchip(CW_GET_ID,(void *)chipId);
    // Config Para
    memcpy(netConf.mac, g_Udp_Para.macAddress, 6);
    memcpy(netConf.ip, g_Udp_Para.ipAddress, 4);
    memcpy(netConf.sn, g_Udp_Para.subMask, 4);
    memcpy(netConf.gw, g_Udp_Para.gateWay, 4);
    memcpy(netConf.dns, g_Udp_Para.dnsAddress, 4);
    netConf.dhcp = NETINFO_STATIC;
    ctlnetwork(CN_SET_NETINFO, (void *)&netConf);
    // Set timeout
    ctlnetwork(CN_SET_TIMEOUT, (void *)&to);
    // Set socket & port number
    setSn_MSSR(g_Udp_Para.socketNum, 1460);
    setSn_PORT(g_Udp_Para.socketNum, g_Udp_Para.portNum);
    // Open socket
    w5500_OpenSocket();
    Delay_ms(10);
    
    return retVal;

}


void Udp_Init()
{
    // Initialize Udp para
    Udp_Para_Initial();
    // Initialize W5500 SPI
    Spi_Init(SPI_W5500);
    // Register SPI function
    reg_wizchip_spi_cbfunc(Udp_Spi_Read_Byte,Udp_Spi_Write_Byte);
    reg_wizchip_spiburst_cbfunc(Udp_Spi_Read_Burst,Udp_Spi_Write_Burst);
    reg_wizchip_cs_cbfunc(Udp_Cs_Sel,Udp_Cs_Desel);
    // Initialize W5500 
    W5500_Init();
}


void Udp_WriteData(TUdpData data)
{  
    static u8 g_last_ip[8][4] = {0x00};
    static u16 g_last_port[8] = {0x00};
    
    u16 offset, offset1, offset2;
    u16 tx_buffer_size = 0x00;
    u32 addrsel = 0x00;
    
    if(getSn_SR(g_Udp_Para.socketNum) == SOCK_CLOSED)
    {
        w5500_OpenSocket();
        return;
    }
    
    if(getSn_SR(g_Udp_Para.socketNum) != SOCK_UDP) return;
    
    if(memcmp(g_last_ip[g_Udp_Para.socketNum], data.ipAddress, 4) != 0)
    {
        setSn_DIPR(g_Udp_Para.socketNum, data.ipAddress);
        memcpy(g_last_ip[g_Udp_Para.socketNum], data.ipAddress, 4);
    }
    if(g_last_port[g_Udp_Para.socketNum] != data.portNum)
    {
        setSn_DPORT(g_Udp_Para.socketNum, data.portNum);
        g_last_port[g_Udp_Para.socketNum] = data.portNum;
    }
      
    tx_buffer_size = getSn_TXBUF_SIZE(g_Udp_Para.socketNum) * 1024;
    offset = getSn_TX_WR(g_Udp_Para.socketNum);
	offset1 = offset;
	offset &= (tx_buffer_size - 1);
    
    if((offset + data.len) < tx_buffer_size)
    {
        addrsel = ((uint32_t)offset << 8) + (WIZCHIP_TXBUF_BLOCK(g_Udp_Para.socketNum) << 3);
        WIZCHIP_WRITE_BUF(addrsel, data.data, data.len);
    }
    else
    {
        offset2 = tx_buffer_size - offset;
        addrsel = ((uint32_t)offset1 << 8) + (WIZCHIP_TXBUF_BLOCK(g_Udp_Para.socketNum) << 3);
        WIZCHIP_WRITE_BUF(addrsel, data.data, offset2);
        
        addrsel = ((uint32_t)0 << 8) + (WIZCHIP_TXBUF_BLOCK(g_Udp_Para.socketNum) << 3);
        WIZCHIP_WRITE_BUF(addrsel, data.data + offset2, data.len - offset2);       
    }
    
    offset1 += data.len;
    setSn_TX_WR(g_Udp_Para.socketNum, offset1);
    setSn_CR(g_Udp_Para.socketNum, Sn_CR_SEND);    
}

u32 Udp_ReadData(TUdpData *data)
{
    u16 offset, offset1, offset2, rx_size;
    u16 rx_buffer_size;
    u32 addrsel = 0x00;
    u32 i = 0;
    
    if(getSn_SR(g_Udp_Para.socketNum) == SOCK_CLOSED)
    {
        w5500_OpenSocket();
        return 0;
    }
    
    if(getSn_SR(g_Udp_Para.socketNum) != SOCK_UDP) return 0;
        
    rx_size = getSn_RX_RSR(g_Udp_Para.socketNum);
    if(rx_size == 0) return 0;
    if(rx_size > (data->len + 6)) rx_size = data->len + 6;
    rx_buffer_size = getSn_RXBUF_SIZE(g_Udp_Para.socketNum) * 1024;
    
    offset = getSn_RX_RD(g_Udp_Para.socketNum);
    offset1 = offset;
    offset &= (rx_buffer_size - 1); 
    
    if(offset + rx_size < rx_buffer_size)
    {
        addrsel = ((uint32_t)offset << 8) + (WIZCHIP_RXBUF_BLOCK(g_Udp_Para.socketNum) << 3);
        WIZCHIP_READ_BUF(addrsel, data->data, rx_size);   
    }
    else
    {
        offset2 = rx_buffer_size - offset;
        addrsel = ((uint32_t)offset1 << 8) + (WIZCHIP_RXBUF_BLOCK(g_Udp_Para.socketNum) << 3);
        WIZCHIP_READ_BUF(addrsel, data->data, offset2);      

        addrsel = ((uint32_t)0 << 8) + (WIZCHIP_RXBUF_BLOCK(g_Udp_Para.socketNum) << 3);
        WIZCHIP_READ_BUF(addrsel, data->data + offset2, rx_size - offset2);             
    }
    
    offset1 += rx_size;
    
    setSn_RX_RD(g_Udp_Para.socketNum, offset1);
    setSn_CR(g_Udp_Para.socketNum, Sn_CR_RECV); 
    
    // Parse ip & port number 
    memcpy(data->ipAddress, data->data, 4);
    data->portNum = data->data[5] + data->data[4]*256;
    // store data
    for(i = 0; i < data->len; i++)
    {
        data->data[i] = data->data[i+8];
    }
    
    return rx_size > 8? rx_size - 8 : 0;
}

