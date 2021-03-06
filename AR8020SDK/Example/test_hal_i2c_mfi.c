#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_i2c.h"
#include "hal_mfi.h"
#include "test_hal_i2c.h"
#include "hal.h"
#include "systicks.h"
#include "test_hal_i2c_mfi.h"


#define I2C_MFI_TIMEOUT_VALUE    (100)
#define TEST_MFI_BUFFSIZE        (150)

static uint16_t u16_i2cMfiAddr[6];


void command_TestHalI2cInitMfi(uint8_t *ch, uint8_t *i2c_addr, uint8_t *speed)
{
    uint32_t u32_ch = strtoul(ch, NULL, 0);
    uint32_t u32_addr = (uint16_t)strtoul(i2c_addr, NULL, 0);
    uint32_t u32_speed = strtoul(speed, NULL, 0);

    u16_i2cMfiAddr[u32_ch] = (uint16_t)u32_addr;

    HAL_I2C_MasterInit(u32_ch,  u16_i2cMfiAddr[u32_ch], (ENUM_HAL_I2C_SPEED)u32_speed);

    DLOG_Info("e_i2cComponent:%d i2cAddr:0x%x speed:%d",
               u32_ch,  u16_i2cMfiAddr[u32_ch], u32_speed);
}


void command_TestHalI2cWriteMfi(uint8_t *ch, uint8_t *subAddr, uint8_t *subAddrLen, uint8_t *data, uint8_t *dataLen)
{
    uint8_t u8_addr_data[TEST_MFI_BUFFSIZE];
    uint8_t *pu8_addr;
    uint8_t i = 0;

    uint32_t u32_ch = strtoul(ch, NULL, 0);
    uint32_t u32_addr = strtoul(subAddr, NULL, 0);
    uint32_t u32_addrLen = strtoul(subAddrLen, NULL, 0);
    uint32_t u32_data = strtoul(data, NULL, 0);
    uint32_t u32_dataLen = strtoul(dataLen, NULL, 0);

    while(i < u32_addrLen)
    {
        u8_addr_data[i] = (u32_addr >> ((u32_addrLen - i -1) * 8)) & 0xFF;
        i++;
    }

    i = 0;
    pu8_addr = &u8_addr_data[u32_addrLen];
    while(i < u32_dataLen)
    {
        pu8_addr[i] = (u32_data + i) & 0xFF;
        i++;
    }

    HAL_MFI_Write(u32_ch, 
                    u16_i2cMfiAddr[u32_ch],
                    u8_addr_data,
                    u32_addrLen + u32_dataLen,
                    I2C_MFI_TIMEOUT_VALUE);

    DLOG_Info("e_i2cComponent:%d i2cAddr:0x%x",u32_ch,  u16_i2cMfiAddr[u32_ch]);

    for(i = 0; i < (u32_addrLen + u32_dataLen); i++)
    {
        HAL_Delay(50);
        DLOG_Info("addr_data[%d]:0x%x", i, u8_addr_data[i]);
    }
}


void command_TestHalI2cReadMfi(uint8_t *ch, uint8_t *subAddr, uint8_t *subAddrLen, uint8_t *dataLen)
{
    uint8_t u8_addr_data[8];
    uint8_t u8_rxdata[TEST_MFI_BUFFSIZE];
    uint8_t *pu8_addr;
    uint8_t i = 0;
    uint8_t tmpCnt = 10;

    uint32_t u32_ch = strtoul(ch, NULL, 0);
    uint32_t u32_addr = strtoul(subAddr, NULL, 0);
    uint32_t u32_addrLen = strtoul(subAddrLen, NULL, 0);
    uint32_t u32_dataLen = strtoul(dataLen, NULL, 0);

    memset(u8_rxdata, 0x00, sizeof(u8_rxdata));

    while(i < u32_addrLen)
    {
        u8_addr_data[i] = (u32_addr >> ((u32_addrLen - i -1) * 8)) & 0xFF;
        i++;
    }

    MFI_MFI_Read(u32_ch, 
                   u16_i2cMfiAddr[u32_ch],
                   u8_addr_data,
                   u32_addrLen,
                   u8_rxdata,
                   u32_dataLen,
                   I2C_MFI_TIMEOUT_VALUE);
    
    for(i = 0; i < u32_addrLen; i++)
    {
        DLOG_Info("addr[%d]:0x%x", i, u8_addr_data[i]);
    }

    for(i = 0; i < u32_dataLen; i++)
    {
        DLOG_Info("rxdata[%d]:0x%x", i, u8_rxdata[i]);
        HAL_Delay(50);
    }
} 
