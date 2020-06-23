#
# Hardware:
# Breakout board connected to SPI Flash; CLK=A3, MOSI=A2, MISO=B1, SS=B4.
#
# Purpose:
# Uses the SPI functionality of the device to provide a basic way to
# communicate with an SPI Flash.  The Flash IC that this has been written
# to interface with is a Winbond W25Q128FV - other vendors may have
# different protocols.  No attempt is made to process the SFDP registers.
#

import sys
import hid

from usb_breakout_board import UsbBreakoutBoard
from spi import *

VID = 0x1209
PID = 0xcb0b

PIN_A2 = 0x02
PIN_A3 = 0x03
PIN_B1 = 0x11
PIN_B4 = 0x14

FLASH_SIZE_BYTES = 128 * 1024 * 1024 // 8
MAX_SPI_READ_BYTES = 65535

class SpiFlash:
	def __init__(self, bus, slave, size_in_bytes):
		self._bus = bus
		self._slave = slave
		self._size_in_bytes = size_in_bytes

	def get_device_id(self):
		device_id = self._send_transaction([0x90, 0x00, 0x00, 0x00], 6)
		return (device_id[4] << 8) | device_id[5]

	def _send_transaction(self, out, in_length, show_progress=False):
		return self._slave.send_transaction(self._bus, out, in_length, show_progress)

	def read(self, address, length):
		if (address < 0 or address >= self._size_in_bytes):
			raise Exception("Address 0x{:08x} is out of range".format(address))

		if (length < 1 or length > self._size_in_bytes):
			raise Exception("Length {} is out of range".format(length))

		bytes = []
		while length > 0:
			to_read = 4 + length
			if to_read > MAX_SPI_READ_BYTES:
				to_read = MAX_SPI_READ_BYTES

			bytes += self._send_transaction(
				out=[0x03, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff],
				in_length=to_read,
				show_progress=True)[4:]

			address += to_read
			length -= to_read

		return bytes

	def write(self, address, data):
		if (address < 0 or address >= self._size_in_bytes):
			raise Exception("Address 0x{:08x} is out of range".format(address))

		i = 0
		length = len(data)
		self._unlock_all_blocks()
		while length > 0:
			if (address & 0x0000ff) != 0:
				to_write = 256 - (address & 0x0000ff)
				if to_write > length:
					to_write = length
			else:
				to_write = length if length < 256 else 256

			self._enable_write()
			self._send_transaction(
				out=[0x02, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff] + data[i:i + to_write],
				in_length=0)

			i += to_write
			address += to_write
			length -= to_write

	def _unlock_all_blocks(self):
		self._enable_write()
		self._send_transaction(out=[0x98], in_length=0)

	def _enable_write(self):
		self._wait_until_flash_is_not_busy()
		self._send_transaction(out=[0x06], in_length=0)

	def _wait_until_flash_is_not_busy(self):
		status = 0x01
		while status & 0x01:
			status = self._send_transaction(out=[0x05], in_length=2)[1]

	def erase_sectors(self, address, count):
		if (address < 0 or address >= self._size_in_bytes):
			raise Exception("Address 0x{:08x} is out of range".format(address))

		if (address & 4095):
			raise Exception("Address 0x{:08x} does not begin on a 4KiB sector boundary".format(address))

		if (count < 1 or count > self._size_in_bytes / 4096):
			raise Exception("Number of sectors, {}, is out of range".format(count))

		self._unlock_all_blocks()
		for i in range(0, count):
			self._enable_write()
			self._send_transaction(
				out=[0x20, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff],
				in_length=0)

			address += 4096

def str_to_int(s):
	if s.startswith("0x"):
		return int(s[2:], base=16)
	elif s.startswith("0b"):
		return int(s[2:], base=2)
	else:
		return int(s, base=10)

def read_flash(flash, address, length, out_file):
	with open(out_file, "wb") as fd:
		fd.write(bytearray(flash.read(address, length)))

def write_flash(flash, address, length, in_file, in_file_offset):
	if (in_file_offset < 0):
		raise Exception("File offset {} is out of range".format(in_file_offset))

	with (sys.stdin if in_file == '-' else open(in_file, "rb")) as fd:
		fd.seek(in_file_offset)
		flash.write(address, list(fd.read(length)))

def erase_flash(flash, address, num_sectors):
	flash.erase_sectors(address, num_sectors)

if __name__ == "__main__":
	with UsbBreakoutBoard(VID, PID) as usb_device:
		usb_device.core.print_details()
		print()

		flash_slave = SpiSlave(address=0, ss_pin=PIN_B4, baud_rate_mhz=12)
		spi_bus = SpiBus(usb_device.core, usb_device.spi, clk_pin=PIN_A3, mosi_pin=PIN_A2, miso_pin=PIN_B1, slaves=[flash_slave])
		flash = SpiFlash(spi_bus, flash_slave, FLASH_SIZE_BYTES)

		print("Flash Manufacturer / Device ID: 0x{:04x}".format(flash.get_device_id()))
		if len(sys.argv) < 2 or \
			(sys.argv[1] == 'r' and len(sys.argv) != 5) or \
			(sys.argv[1] == 'w' and len(sys.argv) != 6) or \
			(sys.argv[1] == 'e' and len(sys.argv) != 4):

			print("Usage: flash-op.py r address length out_file")
			print("   OR: flash-op.py w address length in_file file_offset")
			print("   OR: flash-op.py e address num_4k_sectors")
			print()
			print("Pages for writes are assumed to be 256B.")
			print("Sectors for erases are assumed to be 4KiB.")

		elif sys.argv[1] == 'r':
			read_flash(flash, str_to_int(sys.argv[2]), str_to_int(sys.argv[3]), sys.argv[4])
		elif sys.argv[1] == 'w':
			write_flash(flash, str_to_int(sys.argv[2]), str_to_int(sys.argv[3]), sys.argv[4], str_to_int(sys.argv[5]))
		elif sys.argv[1] == 'e':
			erase_flash(flash, str_to_int(sys.argv[2]), str_to_int(sys.argv[3]))
