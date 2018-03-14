#pragma once
int FTDI_Open(const int device_index);
int FTDI_Close();
int FTDI_WriteTMSandTDIandReadTDO(const unsigned char tms,
                                  const unsigned char tdi, unsigned char *tdo);
