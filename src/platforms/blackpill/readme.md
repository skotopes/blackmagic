# Pinout

	                     S S
	                     W W 3
	                   G C D .
	                   N L I 3
	                   D K O V
	             /-----| | | |-----\
	Activity LED - B12   A A   B11 -
	             - B13   1 1   B10 -
	             - B14   4 3    B1 -
	             - B15          B0 - 
	             - A8           A7 - 
	             - A9           A6 - 
	             - A10          A5 - 
	Reserved USB - A11          A4 - 
	Reserved USB - A12          A3 - SWO RX
	         TDI - A15          A2 - 
	         TDO - B3           A1 - 
	        JRST - B4           A0 - 
	             - B5          RST - 
	          TX - B6         PC13 - 
	             - B7           B9 - 
	             - 3.3V         B8 - 
	             - GND         GND - 
	             \_______USB_______/


# Building

	make PROBE_HOST=blackpill

# Flashing empty device

- You'll need serial port. 5v is ok because A9 and A10 is 5 volt tolerant.
- Connect GND, A9 to RX and A10 to TX
- Switch boot0 jumper to 1
- `stm32flash -w src/blackmagic_dfu.hex /dev/yourserialhere`
- Revert boot0 jumper to 0

Now goes magic: stm32f100c8t6b oficially got 64kb of ROM, but actually got more.
Use scripts/stm32_mem.py to flash beyond designated area: `scripts/stm32_mem.py src/blackmagic.bin`

That's it.

# Notes:

Device own SWD is DISABLED. If you need to debug blackmagic - remap ports and remove `SELF_SWD_DISABLE` in platform.h
