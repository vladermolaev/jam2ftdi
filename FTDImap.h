#pragma once
// From FTDI application note "AN_373 Bit-Bang Modes for the FT-X Series.pdf",
// page 5, table 2.1 Asynchronous Bit-Bang Mode I/O Configurations.
enum ft231x_bitbang_pin_address {
  TXD = 0x1,
  RXD = 0x2,
  RTS = 0x4,
  CTS = 0x8,
  DTR = 0x10,
  DSR = 0x20,
  DCD = 0x40,
  RI = 0x80
};

// From hardware design - which FTDI pins are connected to which JTAG pins.
enum Evo3_FTDI_bitbang_pins_to_JTAG_map {
  TCK = DTR,
  TDO = DSR,
  TMS = DCD,
  TDI = RI
};
