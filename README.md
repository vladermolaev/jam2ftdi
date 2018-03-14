# jam2ftdi
Customization of Altera Jam STAPL Byte-Code Player Version 2.5 to work over USB for devices built on FTDI chips.
Customization points are FTDIid.h and FTDImap.h.
FTDIid.h - defines USB VID of your FTDI based device.
FTDImap.h - defines pinout of your particular model of FTDI chip (from its data sheet) as well as which pins of that chip are connected to which JTAG signals in your hardware design.
FTDI_Open() in FTDI_API.c may also need to be customized to properly initialize specific FTDI chip, namely baud rate selection, putting the chip into bitbang mode, etc.
Beware of using low baud rates. Anything below FT_BAUD_57600 failed in the design using FT231XQ driving Intel(Altera) 10M08 FPGA. Selected baud rate affects delay between calling FT_Write() and change in the signal on the pin. The lower the baud rate the longer that delay can be.
