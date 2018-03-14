#include <stdio.h>
#include <Windows.h>
#include "user.h"
#include "ftd2xx.h"
#include "FTDImap.h"
#include "FTDIid.h"

FT_HANDLE device_handle = INVALID_HANDLE_VALUE;

static FT_STATUS SetAsyncBitbangMode() {
  const UCHAR asynchronous_bitbang_mode = 0x01;
  return FT_SetBitMode(device_handle, (UCHAR)(TCK | TMS | TDI),
                       asynchronous_bitbang_mode);
}

int FTDI_Open(const int device_index) {
  if (FT_Open(device_index, &device_handle) != FT_OK)
    return CB_BB_OPEN_ERROR_OPEN_PORT;
  DWORD id;
  char serial[256];
  char description[256];
  FT_DEVICE type;
  if (FT_GetDeviceInfo(device_handle, &type, &id, serial, description, NULL) ==
      FT_OK) {
    if (HIWORD(id) == CUSTOM_VENDORID) {
      FT_SetChars(device_handle, 0, 0, 0, 0);  // Disable event characters
      FT_SetLatencyTimer(device_handle, 2);
      FT_SetBaudRate(device_handle, FT_BAUD_921600);
      SetAsyncBitbangMode();
      printf("Opened device %d: %s - %s\n", device_index, serial, description);
      return CB_OK;
    }
  }
  FT_Close(device_handle);
  device_handle = INVALID_HANDLE_VALUE;
  return CB_BB_OPEN_DEVICEIOCONTROL_FAIL;
}

int FTDI_Close() {
  const UCHAR reset_bitbang_mode = 0;
  FT_SetBitMode(device_handle, 0, reset_bitbang_mode);
  auto status = FT_Close(device_handle);
  device_handle = INVALID_HANDLE_VALUE;
  return status == FT_OK ? CB_OK : CB_FS_CLOSE_FILE_ERROR;
}

#define outgoingBufferSize 256
static unsigned char outgoingBuffer[outgoingBufferSize];
static int outgoingByteCount = 0;

static FT_STATUS FlushOutgoingBuffer() {
  DWORD bytesWritten = 0;
  auto sts =
      FT_Write(device_handle, outgoingBuffer, outgoingByteCount, &bytesWritten);
  if (sts != FT_OK || bytesWritten != outgoingByteCount) return FT_IO_ERROR;
  outgoingByteCount = 0;
  return FT_OK;
}

static FT_STATUS WriteByte(unsigned char data) {
  outgoingBuffer[outgoingByteCount++] = data;
  if (outgoingByteCount < outgoingBufferSize) return FT_OK;
  return FlushOutgoingBuffer();
}

static unsigned char lastTMS = 0, lastTDI = 0, TCKisHigh = 1;

static FT_STATUS ResetTCK() {
  unsigned char data = (lastTMS ? TMS : 0) | (lastTDI ? TDI : 0);
  if (WriteByte(data) != FT_OK) return FT_IO_ERROR;
  TCKisHigh = 0;
  return FT_OK;
}

static FT_STATUS WriteTMSandTDI(const unsigned char tms,
                                const unsigned char tdi) {
  unsigned char data = (tms ? TMS : 0) | (tdi ? TDI : 0);
  if (WriteByte(data) != FT_OK) return FT_IO_ERROR;
  lastTMS = tms;
  lastTDI = tdi;
  data |= TCK;
  if (WriteByte(data) != FT_OK) return FT_IO_ERROR;
  TCKisHigh = 1;
  return FT_OK;
}

static FT_STATUS ReadTDO(unsigned char *tdo) {
  static long DR = 0;
  unsigned char data = 0;
  auto status = FT_GetBitMode(device_handle, &data);
  *tdo = (data & TDO) ? 1 : 0;
  return status;
}

int FTDI_WriteTMSandTDIandReadTDO(const unsigned char tms,
                                  const unsigned char tdi, const int readTDO) {
  unsigned char tdo = 0;
  FT_STATUS status = FT_OK;
  if (TCKisHigh) {
    ResetTCK();
    if (readTDO) {
      FlushOutgoingBuffer();
      void jam_delay(long microseconds);
      jam_delay(1);
      status = ReadTDO(&tdo);
      if (status != FT_OK) fprintf(stderr, "cannot read from FTDI\n");
    }
  }
  if (status == FT_OK) status = WriteTMSandTDI(tms, tdi);
  if (status != FT_OK) fprintf(stderr, "cannot write to FTDI\n");
  return tdo;
}
