import copy

from box import Box

class Ssd1306I2cSlaveDisplay:
	_WIDTH_PIXELS = 128
	_HEIGHT_PIXELS = 64
	_NUM_CGRAM_BYTES = int((_WIDTH_PIXELS * _HEIGHT_PIXELS) / 8)
	_SCREEN_BOX = Box(0, 0, _WIDTH_PIXELS, _HEIGHT_PIXELS)

	_INITIALISATION_SEQUENCE = [
		[0xae], # Display off
		[0xd5, 0x80], # Clock divide ratio
		[0xa8, 0x3f], # Multiplex ratio for 128x64
		[0xd3, 0x00], # Display offset
		[0x40], # Start line 0
		[0x8d, 0x14], # Charge pump is internal
		[0xa1, 0xc8], # Segment re-scan and COM scan direction
		[0xda, 0x12], # Alternative COM configuration
		[0x81, 0x7f], # Mid-contrast
		[0xd9, 0xf1], # Pre-charge period
		[0xdb, 0x20], # Default VCOMH de-select level
		[0xa4], # Display RAM contents
		[0xa6], # Display not inverted
		[0xaf] # Display on
	]

	_SET_HORIZONTAL_ADDRESSING_MODE = [0x20, 0x00]
	_SET_PAGE_ADDRESSING_MODE = [0x20, 0x02]

	_SET_PAGE_ADDRESS = lambda start, end: [0x22, start, end]
	_SET_COLUMN_ADDRESS = lambda start, end: [0x21, start, end]

	def __init__(self, i2c):
		self._i2c = i2c
		self._cgram = [0x00] * Ssd1306I2cSlaveDisplay._NUM_CGRAM_BYTES
		self._previous_cgram = None

	def _write_commands(self, *commands):
		concatenated = []
		for cmd in commands:
			concatenated = concatenated + [0x00] + cmd

		self._i2c.write(concatenated)

	def _write_data(self, data):
		for i in range(0, int((len(data) + 511) / 512)):
			self._i2c.write([0x40] + data[i * 512:min(len(data), (i + 1) * 512)])

	def cgram_address_of(self, x, y):
		return ((x + (y >> 3) * Ssd1306I2cSlaveDisplay._WIDTH_PIXELS) << 8) + (1 << (y & 0x07))

	def _box_of_changes_since_last_blit(self):
		if self._previous_cgram is None:
			self._previous_cgram = copy.copy(self._cgram)
			return Ssd1306I2cSlaveDisplay._SCREEN_BOX

		pos = 0
		[min_x, min_y] = [Ssd1306I2cSlaveDisplay._WIDTH_PIXELS, Ssd1306I2cSlaveDisplay._HEIGHT_PIXELS]
		[max_x, max_y] = [-1, -1]
		for y in range(0, Ssd1306I2cSlaveDisplay._HEIGHT_PIXELS, 8):
			for x in range(0, Ssd1306I2cSlaveDisplay._WIDTH_PIXELS):
				if self._cgram[pos] != self._previous_cgram[pos]:
					[min_x, min_y] = [min(min_x, x), min(min_y, y)]
					[max_x, max_y] = [max(max_x, x), max(max_y, y)]

				pos = pos + 1

		self._previous_cgram = copy.copy(self._cgram)
		return Box.NONE if max_x < 0 else Box(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1)

	def blit(self):
		bounding_box = self._box_of_changes_since_last_blit()
		if bounding_box.width == 0 or bounding_box.height == 0:
			return

		self._write_commands(
			Ssd1306I2cSlaveDisplay._SET_HORIZONTAL_ADDRESSING_MODE,
			Ssd1306I2cSlaveDisplay._SET_PAGE_ADDRESS(bounding_box.y >> 3, 0xff),
			Ssd1306I2cSlaveDisplay._SET_COLUMN_ADDRESS(bounding_box.x, bounding_box.max_x - 1))

		patch = []
		box_address = self.cgram_address_of(bounding_box.x, bounding_box.y) >> 8
		box_max_address = self.cgram_address_of(bounding_box.max_x, bounding_box.max_y) >> 8
		for span_address in range(box_address, box_max_address + 1, Ssd1306I2cSlaveDisplay._WIDTH_PIXELS):
			patch += self._cgram[span_address:span_address + bounding_box.width]

		self._write_data(patch)

	def _set_pixel(self, x, y, value, op):
		address = self.cgram_address_of(x, y)
		if op == "or":
			self._cgram[address >> 8] |= (address & 0x00ff) if value != 0 else 0
		elif op == "xor":
			self._cgram[address >> 8] ^= (address & 0x00ff) if value != 0 else 0
		elif op == "and":
			self._cgram[address >> 8] &= 0xff if value != 0 else ~(address & 0x00ff)
		else:
			if value != 0:
				self._cgram[address >> 8] |= (address & 0x00ff)
			else:
				self._cgram[address >> 8] &= ~(address & 0x00ff)

	def _add_tile(self, x, y, tile, tile_op):
		for tile_y in range(0, len(tile.cgram), (tile.width + 7) >> 3):
			for tile_x in range(0, tile.width):
				self._set_pixel(x + tile_x, y, tile.cgram[tile_y + (tile_x >> 3)] & (1 << (7 - (tile_x & 0x07))), tile_op)

			y = y + 1

	def or_tile(self, x, y, tile):
		self._add_tile(x, y, tile, "or")

	def xor_tile(self, x, y, tile):
		self._add_tile(x, y, tile, "xor")

	def and_tile(self, x, y, tile):
		self._add_tile(x, y, tile, "and")

	def set_tile(self, x, y, tile):
		self._add_tile(x, y, tile, "set")

	def clear(self):
		self._cgram[:] = [0x00] * Ssd1306I2cSlaveDisplay._NUM_CGRAM_BYTES

	def initialise(self):
		self.clear()
		self._write_commands(*Ssd1306I2cSlaveDisplay._INITIALISATION_SEQUENCE)

	def set_pixel(self, x, y, value):
		self._set_pixel(x, y, value, op = "set")

	def set_brightness(self, percent):
		steps = int(percent * 100 / 7)
		precharge = 0x11 * steps
		scale = 1 / 2

		self._write_commands([0xd9, precharge], [0x81, min(255, int(abs(percent * scale * 255)))])

	@property
	def width(self):
		return Ssd1306I2cSlaveDisplay._WIDTH_PIXELS

	@property
	def height(self):
		return Ssd1306I2cSlaveDisplay._HEIGHT_PIXELS
