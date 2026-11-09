#ifndef __YEEP144_PISPI_H
#define __YEEP144_PISPI_H

// This value define the size of the transfer buffers. 
// It needs to match what is set on the Pi side so the correct amount of data is transferred each time
#define PISPI_BUFFER_SIZE 256

// Extern the buffer arrays so they are accessible from other files
extern uint8_t PiSPIRxBuf[PISPI_BUFFER_SIZE];
extern uint8_t PiSPITxBuf[PISPI_BUFFER_SIZE];

void YEEP144_PiSPI_Init(void);
void YEEP144_PiSPI_SetReady(void);
void YEEP144_PiSPI_ResetReady(void);
void YEEP144_PiSPI_LoadTx(void);
void YEEP144_PiSPI_TxRxHandler(void);

#endif