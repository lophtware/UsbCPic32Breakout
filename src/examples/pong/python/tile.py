class Tile:
	def __init__(self, width, height, cgram):
		self._width = width
		self._height = height
		self._cgram = cgram
		self._inverse = None
		self._rotated_180 = None

	@classmethod
	def from_raw(cls, raw, width, height):
		if len(raw) != width * height:
			raise Exception("Expected raw bitmap to be {0}x{1}={2} bytes; actualSize={3}".format(width, height, width * height, len(raw)))

		pos = 0
		width_bytes = (width + 7) // 8
		cgram = [0x00] * (width_bytes * height)
		for y in range(0, height):
			for x in range(0, width, 8):
				for x_bit in range(0, 8):
					raw_pos = y * width + x + x_bit
					cgram[pos] |= (1 << (7 - x_bit)) if (x + x_bit) < width and raw[raw_pos] != 0 else 0

				pos = pos + 1

		return Tile(width, height, cgram)

	@property
	def width(self):
		return self._width

	@property
	def height(self):
		return self._height

	@property
	def cgram(self):
		return self._cgram

	@property
	def inverse(self):
		if self._inverse is None:
			self._inverse = Tile(self.width, self.height, list((~x & 0xff) for x in self._cgram))
		
		return self._inverse

	def _reversed_byte(self, x):
		reversed = 0
		for i in range(0, 8):
			reversed |= (1 << (7 - i)) if x & (1 << i) != 0 else 0

		return reversed

	@property
	def rotated_180(self):
		if self._rotated_180 is None:
			num_residual_bits = self._width & 0x07
			last_byte_in_row = ((self._width + 7) >> 3) - 1
			as_bits = ""
			row_as_bits = ""
			for i in range(0, len(self._cgram)):
				if num_residual_bits > 0 and i == last_byte_in_row:
					row_as_bits += format(self._cgram[i], "08b")[:num_residual_bits]
					for padding in range(0, (8 - num_residual_bits) & 0x07):
						as_bits += "0"

					as_bits += row_as_bits
					last_byte_in_row += (self._width + 7) >> 3
					row_as_bits = ""
				else:
					row_as_bits += format(self._cgram[i], "08b")

			if row_as_bits != "":
				as_bits += row_as_bits

			reversed_cgram = int("".join(reversed(as_bits)), base=2).to_bytes(len(as_bits) >> 3, byteorder="big")
			self._rotated_180 = Tile(self.width, self.height, reversed_cgram)
		
		return self._rotated_180

	def sub_tile(self, box):
		cgram_out = [0x00] * box.height * ((box.width + 7) // 8)
		pos = 0

		row_width_bytes = (self._width + 7) // 8
		start_x_byte = box.x // 8
		start_x_bit = 1 << (7 - (box.x & 0x07))
		y_byte = box.y * row_width_bytes
		for y in range(0, box.height):
			x_byte = start_x_byte
			x_bit = start_x_bit
			x_bit_out = 0x80
			for x in range(0, box.width):
				cgram_out[pos] |= x_bit_out if (self._cgram[y_byte + x_byte] & x_bit) != 0 else 0
				x_bit_out >>= 1
				if x_bit_out == 0:
					x_bit_out = 0x80
					pos = pos + 1

				x_bit >>= 1
				if x_bit == 0:
					x_bit = 0x80
					x_byte = x_byte + 1

			if x_bit_out != 0x80:
				pos = pos + 1

			y_byte += row_width_bytes

		return Tile(box.width, box.height, cgram_out)
