/*flash.c
* 2017-10-23
* Copyright(C) 2017
* liutao@chargerlink.com
*/

#include "includes.h"
#include "flash.h"
#include "i2c.h"
#include "server.h"
#include "FIFO.h"


#define  __HT60XX_FLASH_C



/*
*********************************************************************************************************
*                                           ���غ�/�ṹ��
*********************************************************************************************************
*/

#define M8(adr)     (*((uint8_t * ) (adr)))
#define M16(adr)    (*((uint16_t *) (adr)))
#define M32(adr)    (*((uint32_t *) (adr)))

static const uint32_t RegisterWriteProtect[]={CMU_WPREG_Protected, CMU_WPREG_UnProtected};

//uint8_t TradeRecordGunId=0;


/*
*********************************************************************************************************
*                                         FLASH BYTE WRITE
*
* ����˵��: Flash�ֽ�д
*
* ��ڲ���: pWriteByte    ָ��д���ݵ��׵�ַ
*
*           Address       ���ݽ�Ҫд��Flash�ĵ�ַ
*
*           Num           д���ݳ��ȣ����ֽ�Ϊ��λ
*
* ���ز���: ��
*
* ����˵��: �û�Ӧ��֤����ִ�й����мĴ���д����״̬�Լ�Flash����״̬����
*********************************************************************************************************
*/
void HT_Flash_ByteWrite(uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;
    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< ���浱ǰд����״̬     */

    MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< �ر�д��������         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash����              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                             /*!< Flashд                */

    for(i=0; i<Num; i++)
    {
        M8(Address+i) = pWriteByte[i];                                     /*!< ִ��Flashд            */
        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /*!< �ȴ�д���             */
        Feed_WDT();
    }
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash����              */

    HT_CMU->WPREG = writeProtect;                                          /*!< �ָ�֮ǰд��������     */
    MASTER_INT_EN();
}


/*
*********************************************************************************************************
*                                         FLASH BYTE READ
*
* ����˵��: Flash�ֽڶ�
*
* ��ڲ���: pReadByte     ָ��洢���������ݵ��׵�ַ
*
*           Address       ��Flash���׵�ַ
*
*           Num           �����ݳ��ȣ����ֽ�Ϊ��λ
*
* ���ز���: ��
*
* ����˵��: �û���ע��������ݲ�Ҫ��������ķ�Χ���Է����
*********************************************************************************************************
*/
void HT_Flash_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */

    for(i=0; i<Num; i++)
    {
        pReadByte[i] = M8(Address+i);                                      /*!< ִ��Flash��            */
    }

}


void FlashReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len)
{
    uint16_t i;

    for (i=0; i<len; i++) {
        HT_Flash_ByteRead(&pBuff[i], addr+i, 1);
    }
}


/*
*********************************************************************************************************
*                                         FLASH HALF WORD WRITE
*
* ����˵��: Flash����д
*
* ��ڲ���: pWriteHalfWord    ָ��д���ݵ��׵�ַ
*
*           Address           ���ݽ�Ҫд��Flash�ĵ�ַ
*
*           Num               д���ݳ��ȣ��԰���Ϊ��λ
*
* ���ز���: ��
*
* ����˵��: 1)�û�Ӧ��֤����ִ�й����мĴ���д����״̬�Լ�Flash����״̬����
*           2)�û�Ӧ��֤���ݸ������ĵ�ַΪ���ٰ��ֶ���
*********************************************************************************************************
*/
#if 0
void HT_Flash_HalfWordWrite(uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;
    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< ���浱ǰд����״̬     */

    MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< �ر�д��������         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash����              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                             /*!< Flashд                */

    Address &= 0xFFFFFFFE;                                                 /*!< ��֤���ֶ���           */
    for(i=0; i<Num; i++)
    {
        M16(Address+i*2) = pWriteHalfWord[i];                              /*!< ִ��Flashд            */
        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /*!< �ȴ�д���             */
        Feed_WDT();
    }
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash����              */

    HT_CMU->WPREG = writeProtect;                                          /*!< �ָ�֮ǰд��������     */
    MASTER_INT_EN();
}

/*
*********************************************************************************************************
*                                         FLASH HALF WORD READ
*
* ����˵��: Flash���ֶ�
*
* ��ڲ���: pReadHalfWord ָ��洢���������ݵ��׵�ַ
*
*           Address       ��Flash���׵�ַ
*
*           Num           �����ݳ��ȣ��԰���Ϊ��λ
*
* ���ز���: ��
*
* ����˵��: 1)�û�Ӧ��֤����ִ�й����мĴ���д����״̬�Լ�Flash����״̬����
*           2)�û�Ӧ��֤���ݸ������ĵ�ַΪ���ٰ��ֶ���
*********************************************************************************************************
*/
void HT_Flash_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */
    Address &= 0xFFFFFFFE;                                                 /*!< ��֤���ֶ���           */
    for(i=0; i<Num; i++)
    {
        pReadHalfWord[i] = M16(Address+i*2);                               /*!< ִ��Flash��            */
    }

}

/*
*********************************************************************************************************
*                                         FLASH WORD WRITE
*
* ����˵��: Flash��д
*
* ��ڲ���: pWriteWord    ָ��д���ݵ��׵�ַ
*
*           Address       ���ݽ�Ҫд��Flash�ĵ�ַ
*
*           Num           д���ݳ��ȣ�����Ϊ��λ
*
* ���ز���: ��
*
* ����˵��: 1)�û�Ӧ��֤����ִ�й����мĴ���д����״̬�Լ�Flash����״̬����
*           2)�û�Ӧ��֤���ݸ������ĵ�ַΪ�����ֶ���
*********************************************************************************************************
*/
void HT_Flash_WordWrite(uint32_t* pWriteWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;
    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< ���浱ǰд����״̬     */

    MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< �ر�д��������         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash����              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                             /*!< Flashд                */

    Address &= 0xFFFFFFFC;                                                 /*!< ��֤�ֶ���             */
    for(i=0; i<Num; i++)
    {
        M32(Address+i*4) = pWriteWord[i];                                  /*!< ִ��Flashд            */
        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /*!< �ȴ�д���             */
        Feed_WDT();
    }
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash����              */

    HT_CMU->WPREG = writeProtect;                                          /*!< �ָ�֮ǰд��������     */
    MASTER_INT_EN();
}

/*
*********************************************************************************************************
*                                         FLASH WORD READ
*
* ����˵��: Flash�ֶ�
*
* ��ڲ���: pReadWord     ָ��洢���������ݵ��׵�ַ
*
*           Address       ��Flash���׵�ַ
*
*           Num           �����ݳ��ȣ�����Ϊ��λ
*
* ���ز���: ��
*
* ����˵��: 1)�û�Ӧ��֤����ִ�й����мĴ���д����״̬�Լ�Flash����״̬����
*           2)�û�Ӧ��֤���ݸ������ĵ�ַΪ�����ֶ���
*********************************************************************************************************
*/
void HT_Flash_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */

    uint32_t i;

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */
    Address &= 0xFFFFFFFC;                                                 /*!< ��֤�ֶ���             */
    for(i=0; i<Num; i++)
    {
        pReadWord[i] = M32(Address+i*4);                                   /*!< ִ��Flash��            */
    }

}

/*
*********************************************************************************************************
*                                         FLASH CHIP ERASE
*
* ����˵��: Flashȫ����
*
* ��ڲ���: ��
*
* ���ز���: ��
*
* ����˵��: �û�Ӧ��֤����ִ�й����мĴ���д����״̬�Լ�Flash����״̬����
*********************************************************************************************************
*/
void HT_Flash_ChipErase(void)
{
    /*  assert_param  */

    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< ���浱ǰд����״̬     */

	MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< �ر�д��������         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash����              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_CHIPERASE;                         /*!< Flashҳ����            */

    M32(0x1000) = 0xFF;                                                    /*!< ִ��Flashȫ����        */
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< �ȴ�д���             */
    Feed_WDT();
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash����              */

    HT_CMU->WPREG = writeProtect;                                          /*!< �ָ�֮ǰд��������     */
	MASTER_INT_EN();
}

#endif

/*
*********************************************************************************************************
*                                         FLASH PAGE ERASE
*
* ����˵��: Flashҳ����
*
* ��ڲ���: EraseAddress    ҳ������ַ
*
* ���ز���: ��
*
* ����˵��: �û�Ӧ��֤����ִ�й����мĴ���д����״̬�Լ�Flash����״̬���䣬1K bytes/page
*********************************************************************************************************
*/
void HT_Flash_PageErase(uint32_t EraseAddress)
{
    /*  assert_param  */

    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< ���浱ǰд����״̬     */

	MASTER_INT_DIS();
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< �ر�д��������         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash����              */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_PAGEERASE;                         /*!< Flashҳ����            */

    EraseAddress &= 0xFFFFFFFC;                                            /*!< ��֤�ֶ���             */
    M32(EraseAddress) = 0xFF;                                              /*!< ִ��Flashҳ����        */
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< �ȴ�д���             */
    Feed_WDT();
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash��                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash����              */

    HT_CMU->WPREG = writeProtect;                                          /*!< �ָ�֮ǰд��������     */
	MASTER_INT_EN();
}


void FlashWriteAppBackup(uint32_t app_backup_record_addr, uint8_t* buffer, uint16_t len)
{
	HT_Flash_ByteWrite((void*)buffer, app_backup_record_addr, len);
}


void FlashEraseAppBackup(void)
{
    int i;

    for (i=0; i<(APP_FW_SIZE/1024); i++) {
        HT_Flash_PageErase(AppUpBkpAddr+i*1024);
    }
}


void WriteUpdateInfo(uint32_t fsize, uint32_t checkSum)
{
    updateInfo.updateFlag = 0xaa55;
    updateInfo.fsize = fsize;
    updateInfo.checkSum = checkSum;
    BootSetReqInfo();
}


//readBack: д���Ƿ�ض� 0���ض�  1�ض�
//����: 0д�ɹ�  ����:�ض�ʧ��
#if 0
int FlashWriteSysInfo(void *pSysInfo, uint16_t size, uint8_t readBack)
{
    int i;
    uint8_t  data;

    HT_Flash_PageErase(SysCfgInfoAddr);
    HT_Flash_PageErase(SysCfgInfoAddr+1024);
	HT_Flash_ByteWrite((void*)pSysInfo, SysCfgInfoAddr, size);

    if (readBack) {
        for (i=0; i<size; i++) {
            HT_Flash_ByteRead(&data, SysCfgInfoAddr+i, 1);
            if (data != *((uint8_t*)pSysInfo + i)) {
                CL_LOG("write falsh fail.\n");
                return CL_FAIL;
            }
        }
    }
    CL_LOG("write falsh ok.\n");
    return CL_OK;
}


//readBack: д���Ƿ�ض� 0���ض�  1�ض�
//����: 0д�ɹ�  ����:�ض�ʧ��
int FlashWriteGunInfo(void *pGunInfo, uint16_t size, uint8_t readBack)
{
	int i;
    uint8_t  data;

    for (i=0; i<4; i++) {
	    HT_Flash_PageErase(GunInfoAddr+i*1024);
    }
	HT_Flash_ByteWrite((void*)pGunInfo, GunInfoAddr, size);

    if (readBack) {
        for (i=0; i<size; i++) {
            HT_Flash_ByteRead(&data, GunInfoAddr+i, 1);
            if (data != *((uint8_t*)pGunInfo + i)) {
                CL_LOG("write falsh fail.\n");
                return CL_FAIL;
            }
        }
    }
    CL_LOG("write falsh ok.\n");
    return CL_OK;
}


void FlashReadSysInfo(void *pInfo, uint16_t size)
{
	HT_Flash_ByteRead((void*)pInfo, SysCfgInfoAddr, size);
}


void FlashReadGunInfo(void *pGunInfo, uint16_t size)
{
    HT_Flash_ByteRead((void*)pGunInfo, GunInfoAddr, size);
}

#else


//EEPROM��ȡ����
int at24c64_eepromRead(uint32_t addr, void *pInfo, uint16_t size)
{
	I2C_ReadNByte(0xA0, 2, addr, (void*)pInfo, size);
	return CL_OK;
}


//EEPROMҳ����д��
int at24c64_eepromWrite(uint32_t addr, void *pInfo, uint16_t size)
{
	uint8_t  *p = NULL;
	uint8_t	 RemainingBytes = 0;
	uint16_t page = 0;
	uint16_t pCnt = 0;
	uint16_t wCheckSum=0;
	uint16_t rCheckSum=0;
	uint8_t  readBuf[EEPROM_PAGE_SIZE+1];

	page = size / EEPROM_PAGE_SIZE;
	RemainingBytes = size % EEPROM_PAGE_SIZE;
	p = pInfo;
	for (uint16_t i=0; i<size; i++) {
		wCheckSum += *(p+i);
	}
	while(1) {
		if (0 == page) {
			if (0 != RemainingBytes) {
				I2C_WriteNByte(0xA0, 2, addr+EEPROM_PAGE_SIZE*pCnt, (void*)p, RemainingBytes);
			}
			break;
		} else {
			I2C_WriteNByte(0xA0, 2, addr+EEPROM_PAGE_SIZE*pCnt, (void*)p, EEPROM_PAGE_SIZE);
			OS_DELAY_MS(15);
			p += EEPROM_PAGE_SIZE;
			page--;
			pCnt++;
		}
	}

    OS_DELAY_MS(10);
	//У��
	page = size / EEPROM_PAGE_SIZE;
	RemainingBytes = size % EEPROM_PAGE_SIZE;
	pCnt=0;
	while(1) {
		if (0 == page) {
			if (RemainingBytes) {
				at24c64_eepromRead(addr+EEPROM_PAGE_SIZE*pCnt, (void*)&readBuf, RemainingBytes);
				for (uint8_t i=0; i<RemainingBytes; i++) {
					rCheckSum += readBuf[i];
				}
			}
			break;
		} else {
			at24c64_eepromRead(addr+EEPROM_PAGE_SIZE*pCnt, (void*)&readBuf, EEPROM_PAGE_SIZE);
			OS_DELAY_MS(15);
			page--;
			pCnt++;
			for (uint8_t i=0; i<EEPROM_PAGE_SIZE; i++) {
				rCheckSum += readBuf[i];
			}
		}
	}

	if(wCheckSum == rCheckSum) {
		CL_LOG("eeprom write ok, wCheckSum = %d, rCheckSum = %d.\n", wCheckSum, rCheckSum);
		return CL_OK;
	} else {
		CL_LOG("eeprom write err, wCheckSum = %d, rCheckSum = %d.\n", wCheckSum, rCheckSum);
		return CL_FAIL;
	}
}


int EepromWriteData(uint32_t addr, void *pInfo, uint16_t size)
{
    int i;

    for (i=0; i<4; i++) {
    	if (CL_OK == at24c64_eepromWrite(addr, pInfo, size)) {
            return CL_OK;
        }
        vTaskDelay(1000);
    }
    return CL_FAIL;
}


//дϵͳ��Ϣ
int FlashWriteSysInfo(void *pSysInfo, uint16_t size, uint8_t readBack)
{
    if (CL_OK != EepromWriteData(SysInfoEepromAddr, pSysInfo, size)) {
        SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_E2PROM, 0, EVENT_OCCUR, NULL);
        return CL_FAIL;
    }
    return CL_OK;
}


//��ϵͳ��Ϣ
int FlashReadSysInfo(uint8_t *pInfo, uint16_t size)
{
    int i;

    for (i=0; i<size; i++) {
	    at24c64_eepromRead(SysInfoEepromAddr+i, &pInfo[i], 1);
    }
	return CL_OK;
}


//дǹͷ��Ϣ
int FlashWriteGunInfo(void *pGunInfo, uint16_t size, uint8_t readBack)
{
    if (CL_OK != EepromWriteData(GunInfoEepromAddr, pGunInfo, size)) {
        SendEventNotice(0, EVENT_CHIP_FAULT, CHIP_E2PROM, 1, EVENT_OCCUR, NULL);
        return CL_FAIL;
    }
    return CL_OK;
}


//��ǹͷ��Ϣ
int FlashReadGunInfo(uint8_t *pGunInfo, uint16_t size)
{
    int i;

    for (i=0; i<size; i++) {
	    at24c64_eepromRead(GunInfoEepromAddr+i, &pGunInfo[i], 1);
    }
	return CL_OK;
}


//д�������ž���ϵ��
int FlashWritMatrix(uint8_t *data, uint16_t size)
{
    if (CL_OK != EepromWriteData(EMUCaliationAddr, data, size)) {
        return CL_FAIL;
    }
    return CL_OK;
}

//���������ž���ϵ��
int FlashReadMatrix(uint8_t *data, uint16_t size)
{
	int i;

    for (i=0; i<size; i++) {
	    at24c64_eepromRead(EMUCaliationAddr+i, &data[i], 1);
    }
	return CL_OK;
}


void E2promReadDataEx(uint8_t *pBuff, uint32_t addr, uint16_t len)
{
    uint16_t i;

    for (i=0; i<len; i++) {
        at24c64_eepromRead(addr+i, (void*)&pBuff[i], 1);
    }
}


int E2promWriteDataEx(uint32_t addr, void* pBuff, uint16_t len)
{
    return at24c64_eepromWrite(addr, pBuff, len);
}

#endif

