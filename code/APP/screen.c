#include "oled.h"
#include "screen.h"
#include "io_define.h"
static struct
{
    struct
    {
        uint16_t        uiStartPageIndex;
        uint16_t        uiStartColumnIndex;
        uint16_t        uiEndPageIndex;
        uint16_t        uiEndColumnIndex;
    }stUpdateArea;
    uint8_t        pageCtrlInitialized;
    uint8_t        page0Ctrl[LCD_SIZE_CMD];               //0-6
    uint8_t        auiDisplayCache_PAGES0[LCD_SIZE_WIDTH];//7-134
    uint8_t        page1Ctrl[LCD_SIZE_CMD];               //135-141
    uint8_t        auiDisplayCache_PAGES1[LCD_SIZE_WIDTH];//
    uint8_t        page2Ctrl[LCD_SIZE_CMD];
    uint8_t        auiDisplayCache_PAGES2[LCD_SIZE_WIDTH];
    uint8_t        page3Ctrl[LCD_SIZE_CMD];
    uint8_t        auiDisplayCache_PAGES3[LCD_SIZE_WIDTH];
    uint8_t        page4Ctrl[LCD_SIZE_CMD];
    uint8_t        auiDisplayCache_PAGES4[LCD_SIZE_WIDTH];
    uint8_t        page5Ctrl[LCD_SIZE_CMD];
    uint8_t        auiDisplayCache_PAGES5[LCD_SIZE_WIDTH];
    uint8_t        page6Ctrl[LCD_SIZE_CMD];
    uint8_t        auiDisplayCache_PAGES6[LCD_SIZE_WIDTH];
    uint8_t        page7Ctrl[LCD_SIZE_CMD];
    uint8_t        auiDisplayCache_PAGES7[LCD_SIZE_WIDTH];
}s_stLCDBuffer;

static void SCREEN_UpdateChangedBufferAreaRecord(uint8_t uiPageIndex, uint8_t uiColumnIndex);
static void SCREEN_ClearDisplayBuffer(void);

/*************************************************************************/
/** Function Name:  SCREEN_UpdateChangedBufferAreaRecord                  **/
/** Purpose:        Check changed area recodr and update.               **/
/** Resources:      None.                                               **/
/** Params:                                                             **/
/** @ uiPageIndex:      Operated page index.                            **/
/** @ uiColumnIndex:    Operated column index.                          **/
/** Return:         None.                                               **/
/** Limitation:     None.                                               **/
/*************************************************************************/
void SCREEN_UpdateChangedBufferAreaRecord(uint8_t uiPageIndex, uint8_t uiColumnIndex)
{
    // Check and set page and column index.
    if(uiPageIndex < s_stLCDBuffer.stUpdateArea.uiStartPageIndex)
    {
        s_stLCDBuffer.stUpdateArea.uiStartPageIndex = uiPageIndex;
    }
    if(uiPageIndex > s_stLCDBuffer.stUpdateArea.uiEndPageIndex)
    {
        s_stLCDBuffer.stUpdateArea.uiEndPageIndex = uiPageIndex;
    }
    if(uiColumnIndex < s_stLCDBuffer.stUpdateArea.uiStartColumnIndex)
    {
        s_stLCDBuffer.stUpdateArea.uiStartColumnIndex = uiColumnIndex;
    }
    if(uiColumnIndex > s_stLCDBuffer.stUpdateArea.uiEndColumnIndex)
    {
        s_stLCDBuffer.stUpdateArea.uiEndColumnIndex = uiColumnIndex;
    }
}

/*************************************************************************/
/** Function Name:  LCD_ClearDisplayBuffer                              **/
/** Purpose:            Clean display buffer.                           **/
/** Resources:      None.                                               **/
/** Params:         None.                                               **/
/** Return:         None.                                               **/
/** Limitation:     None.                                               **/
/*************************************************************************/
void SCREEN_ClearDisplayBuffer(void)
{
    uint16_t uiCurrentPageIndex, uiCurrentColumnIndex;
    
    for(uiCurrentPageIndex = 0; uiCurrentPageIndex < LCD_SIZE_PAGES; uiCurrentPageIndex++)
    {
        for(uiCurrentColumnIndex = 0; uiCurrentColumnIndex < LCD_SIZE_WIDTH; uiCurrentColumnIndex++)
        {
            //s_stLCDBuffer.auiDisplayCache[uiCurrentPageIndex][uiCurrentColumnIndex] = 0x00;
          s_stLCDBuffer.page0Ctrl[uiCurrentPageIndex*(LCD_SIZE_WIDTH + LCD_SIZE_CMD) + LCD_SIZE_CMD + uiCurrentColumnIndex] = 0x00;
        }
    }
//    s_stLCDBuffer.stUpdateArea.uiStartPageIndex     = (LCD_SIZE_HEIGHT/8) - 1;
//    s_stLCDBuffer.stUpdateArea.uiEndPageIndex       = 0;
//    s_stLCDBuffer.stUpdateArea.uiStartColumnIndex   = LCD_SIZE_WIDTH - 1;
//    s_stLCDBuffer.stUpdateArea.uiEndColumnIndex     = 0;
}

/*************************************************************************/
/** Function Name:  SCREEN_SetPixel                                     **/
/** Purpose:        Set a pixel color or draw a point.                  **/
/** Resources:      None.                                               **/
/** Params:                                                             **/
/** @ iPosX:        X location of point by pixels.                      **/
/** @ iPosY:        Y location of point by pixels.                      **/
/** @ iColor:       Point color, 0 is white, Nonzero is Black.          **/
/** Return:         None.                                               **/
/** Limitation:     None.                                               **/
/*************************************************************************/
void SCREEN_SetPixel(int iPosX, int iPosY, int iColor)
{
    // Operating position check.
    if((iPosX < LCD_SIZE_WIDTH) && (iPosY < LCD_SIZE_HEIGHT))
    {
        // Check and set page and column index.
        SCREEN_UpdateChangedBufferAreaRecord(iPosY/8, iPosX);
        // Set point data.
        if(SCREEN_COLOR_FRG == iColor)
        {SET_PAGE_BIT(s_stLCDBuffer.page0Ctrl[(iPosY/8)*(LCD_SIZE_WIDTH + LCD_SIZE_CMD) + LCD_SIZE_CMD + iPosX],iPosY%8);
            //SET_PAGE_BIT(s_stLCDBuffer.auiDisplayCache[iPosY/8][iPosX], iPosY%8);
        }
        else
        {CLR_PAGE_BIT(s_stLCDBuffer.page0Ctrl[(iPosY/8)*(LCD_SIZE_WIDTH + LCD_SIZE_CMD) + LCD_SIZE_CMD + iPosX],iPosY%8);
            //CLR_PAGE_BIT(s_stLCDBuffer.auiDisplayCache[iPosY/8][iPosX], iPosY%8);
        }
    }
}

/*************************************************************************/
/** Function Name:  SCREEN_GetPixel                                     **/
/** Purpose:        Get a pixel from buffer.                            **/
/** Resources:      None.                                               **/
/** Params:                                                             **/
/** @ iPosX:            X location of point by pixels.                  **/
/** @ iPosY:            Y location of point by pixels.                  **/
/** Return:         None.                                               **/
/** Limitation:     None.                                               **/
/*************************************************************************/
int SCREEN_GetPixel(int iPosX, int iPosY)
{
    // Operating position check.
    if((iPosX < LCD_SIZE_WIDTH) && (iPosY < LCD_SIZE_HEIGHT))
    {return GET_PAGE_BIT(s_stLCDBuffer.page0Ctrl[(iPosY/8)*(LCD_SIZE_WIDTH + LCD_SIZE_CMD) + LCD_SIZE_CMD + iPosX],iPosY%8);
//        return GET_PAGE_BIT(s_stLCDBuffer.auiDisplayCache[iPosY/8][iPosX], iPosY%8);
    }
    else
    {
        return 0;
    }
}

/*************************************************************************/
/** Function Name:  SCREEN_Initialize                                       **/
/** Purpose:            Simple delay function for KS0108 controler.     **/
/** Resources:      None.                                               **/
/** Params:         None.                                               **/
/** Return:         None.                                               **/
/** Limitation:     None.                                               **/
/*************************************************************************/
int SCREEN_Initialize(void)
{
//    OLED_Init();
    SCREEN_ClearDisplayBuffer();

    return 0;
}

/*************************************************************************/
/** Function Name:  LCD_RefreshScreen                                   **/
/** Purpose:            Update Screen with cache data.                  **/
/** Resources:      None.                                               **/
/** Params:         None.                                               **/
/** Return:         None.                                               **/
/** Limitation:     None.                                               **/
/*************************************************************************/

#define I2C_TX_DMA_CH DmaCh1
#define I2C_DMA_UNIT M4_DMA2
#define OLED_ADDR (0x78>>1)
uint16_t OLED_DMA_BLOCK_LEN=128+7;
volatile int8_t OLED_DMA_RUNING = false;
uint8_t  oledDmaStartColumn = 0;
uint8_t  oledDmaStartPage = 0;
extern uint8_t OLED_DMA_BUFFER[];

static void I2C_Tx_DmaStart(uint8_t *pTxBuf, uint16_t u16Cnt)
{
    DMA_SetSrcAddress(I2C_DMA_UNIT, I2C_TX_DMA_CH, (uint32_t)pTxBuf);
    DMA_SetDesAddress(I2C_DMA_UNIT, I2C_TX_DMA_CH, (uint32_t)(&OLED_IIC->DTR));
    DMA_SetTransferCnt(I2C_DMA_UNIT, I2C_TX_DMA_CH, u16Cnt);

    /* Enable DMA CH */
    DMA_ChannelCmd(I2C_DMA_UNIT, I2C_TX_DMA_CH, Enable);
}
uint16_t oledI2CTransLength;
uint16_t oledI2CTransIndex;

static en_result_t I2C_Master_Transmit_DMA(uint16_t u16DevAddr, uint8_t *pu8TxData, uint16_t u16Size, uint32_t u32TimeOut)
{
    en_result_t enRet;
//    I2C_Cmd(OLED_IIC, Enable);

    I2C_SoftwareResetCmd(OLED_IIC, Enable);
    I2C_SoftwareResetCmd(OLED_IIC, Disable);
    enRet = I2C_Start(OLED_IIC,u32TimeOut);
    if(Ok == enRet)
    {

#ifdef I2C_10BITS_ADDRESS
        enRet = I2C_Trans10BitAddr(I2C_UNIT, u16DevAddr, I2CDirTrans, u32TimeOut);
#else
        enRet = I2C_TransAddr(OLED_IIC, (uint8_t)u16DevAddr, I2CDirTrans, u32TimeOut);
#endif

        if(Ok == enRet)
        {
          oledI2CTransIndex += u16Size;
            I2C_Tx_DmaStart(pu8TxData + 1u, (u16Size - 1u));
            I2C_WriteData(OLED_IIC, pu8TxData[0]);
            
//            /* Wait for DMA complete */
//            while(Set != DMA_GetIrqFlag(I2C_DMA_UNIT,I2C_TX_DMA_CH, TrnCpltIrq))
//            {
//                ;
//            }
//            DMA_ClearIrqFlag(I2C_DMA_UNIT, I2C_TX_DMA_CH, TrnCpltIrq);
//            enRet = I2C_WaitStatus(OLED_IIC, I2C_SR_TENDF, Set, u32TimeOut);
        }
    }

//    I2C_Stop(OLED_IIC,u32TimeOut);
//    I2C_Cmd(OLED_IIC, Disable);

    return enRet;
}
void oledDmaTransComplete(void){
  DMA_ClearIrqFlag(I2C_DMA_UNIT, I2C_TX_DMA_CH, TrnCpltIrq);
  DMA_ClearIrqFlag(I2C_DMA_UNIT, I2C_TX_DMA_CH, BlkTrnCpltIrq);
  I2C_Stop(OLED_IIC,500);
  
  if(OLED_DMA_RUNING < 8){
    I2C_Master_Transmit_DMA(OLED_ADDR,&s_stLCDBuffer.page0Ctrl[OLED_DMA_RUNING*(LCD_SIZE_CMD + LCD_SIZE_WIDTH)],LCD_SIZE_CMD + LCD_SIZE_WIDTH,500);
    OLED_DMA_RUNING++;
  }else
  {
    OLED_DMA_RUNING = false;
  }
}
extern uint8_t oled_Brightness;
uint8_t oled_BrightnessLast;
void SCREEN_RefreshScreen(void)
{
    
    uint8_t uiChangedPageIndex, uiChangedColumnIndex;
#ifdef ENABLE_WDT_SUN
  if(getTick() > 10000){
    WDT_RefreshCounter();
  }
#endif
  if(OLED_DMA_RUNING){
    return;
  }
    if(s_stLCDBuffer.stUpdateArea.uiEndColumnIndex > LCD_SIZE_WIDTH-1)
    {
        s_stLCDBuffer.stUpdateArea.uiEndColumnIndex = LCD_SIZE_WIDTH-1;
    }
    if(s_stLCDBuffer.stUpdateArea.uiEndPageIndex > (LCD_SIZE_HEIGHT/8)-1)
    {
        s_stLCDBuffer.stUpdateArea.uiEndPageIndex = (LCD_SIZE_HEIGHT/8)-1;
    }
    if(OLED_DMA_RUNING == false){
      if(oled_Brightness != oled_BrightnessLast){
        oled_BrightnessLast = oled_Brightness;
        OLED_SetBrightness(oled_Brightness);
      }
    }

#if 1
    uiChangedPageIndex = s_stLCDBuffer.stUpdateArea.uiStartPageIndex;
    
    if(s_stLCDBuffer.pageCtrlInitialized == false){
      for(uint8_t i = 0 ;i< 8;i++){
        uint16_t index = i*(LCD_SIZE_WIDTH+LCD_SIZE_CMD);
        s_stLCDBuffer.page0Ctrl[index + 0] = 0x80;
        s_stLCDBuffer.page0Ctrl[index + 1] = 0xb0+i;
        s_stLCDBuffer.page0Ctrl[index + 2] = 0x80;
        s_stLCDBuffer.page0Ctrl[index + 3] = 0x00;
        s_stLCDBuffer.page0Ctrl[index + 4] = 0x80;
        s_stLCDBuffer.page0Ctrl[index + 5] = 0x10;
        s_stLCDBuffer.page0Ctrl[index + 6] = 0x40;
      }
      s_stLCDBuffer.pageCtrlInitialized = true;
    }
    
    if(OLED_DMA_RUNING == false){
        OLED_DMA_RUNING = true;
        I2C_Master_Transmit_DMA(OLED_ADDR,s_stLCDBuffer.page0Ctrl,LCD_SIZE_CMD + LCD_SIZE_WIDTH,500);
    }
#else
    uiChangedPageIndex = s_stLCDBuffer.stUpdateArea.uiStartPageIndex;
    // Loop for each changed page.
    uint16_t oledCmdDatIndex = 0;
    while(uiChangedPageIndex <= s_stLCDBuffer.stUpdateArea.uiEndPageIndex)
    {
        uiChangedColumnIndex = s_stLCDBuffer.stUpdateArea.uiStartColumnIndex;
        OLED_Set_Pos(s_stLCDBuffer.stUpdateArea.uiStartColumnIndex, uiChangedPageIndex);
        // Loop for each changed column data in current page.
        while(uiChangedColumnIndex <= s_stLCDBuffer.stUpdateArea.uiEndColumnIndex)
        {
            // Write data to screen controler.
//            OLED_Write_Dat(s_stLCDBuffer.auiDisplayCache[uiChangedPageIndex][uiChangedColumnIndex]);
            OLED_Write_Dat(s_stLCDBuffer.page0Ctrl[(uiChangedPageIndex)*(LCD_SIZE_WIDTH + LCD_SIZE_CMD) + LCD_SIZE_CMD + uiChangedColumnIndex]);
            uiChangedColumnIndex++;
        }
        uiChangedPageIndex++;
    }
#endif
    
#if 0
    uiChangedColumnIndex = s_stLCDBuffer.stUpdateArea.uiStartColumnIndex;
    // Loop for each changed page.
    while(uiChangedColumnIndex <= s_stLCDBuffer.stUpdateArea.uiEndColumnIndex)
    {
        uiChangedPageIndex = s_stLCDBuffer.stUpdateArea.uiStartPageIndex;
        // Loop for each changed column data in current page.
        while(uiChangedPageIndex <= s_stLCDBuffer.stUpdateArea.uiEndPageIndex)
        {
            // Write data to screen controler.
            OLED_Set_Pos(uiChangedColumnIndex, uiChangedPageIndex);
              for(uint8_t i = 0;i<8;i++){
//                OLED_Write_Dat(s_stLCDBuffer.auiDisplayCache[uiChangedPageIndex][uiChangedColumnIndex+i]);
                  OLED_Write_Dat(s_stLCDBuffer.page0Ctrl[(uiChangedPageIndex)*(LCD_SIZE_WIDTH + LCD_SIZE_CMD) + LCD_SIZE_CMD + uiChangedColumnIndex]);
              }
            uiChangedPageIndex++;
        }
        uiChangedColumnIndex+=8;
    }
#endif
//    Reset global variable.
    s_stLCDBuffer.stUpdateArea.uiStartPageIndex     = (LCD_SIZE_HEIGHT/8) - 1;
    s_stLCDBuffer.stUpdateArea.uiEndPageIndex       = 0;
    s_stLCDBuffer.stUpdateArea.uiStartColumnIndex   = LCD_SIZE_WIDTH - 1;
    s_stLCDBuffer.stUpdateArea.uiEndColumnIndex     = 0;
}

/*************************************************************************/
/** Function Name:  SCREEN_ClearDisplay                                 **/
/** Purpose:        Clean display buffer.                               **/
/** Resources:      None.                                               **/
/** Params:         None.                                               **/
/** Return:         None.                                               **/
/** Limitation:     None.                                               **/
/*************************************************************************/
void SCREEN_ClearDisplay(void)
{
  
//  while(OLED_DMA_RUNING == true){
//    __NOP();
//  }
    SCREEN_ClearDisplayBuffer();
//    Fill_RAM(0x00);
}

