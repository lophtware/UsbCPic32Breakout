from box import Box

class Font:
	NUMBER_OF_CHARS = 128

	def __init__(self, tile, char_width, char_height):
		self._width = char_width
		self._height = char_height

		chars_per_row = tile.width // char_width
		chars_per_col = tile.height // char_height

		self._tiles = [None] * (chars_per_row * chars_per_col)
		for y in range(0, chars_per_col):
			for x in range(0, chars_per_row):
				self._tiles[y * chars_per_row + x] = tile.sub_tile(Box(x * char_width, y * char_height, char_width, char_height))

	def write(self, display, x, y, str, op = "set"):
		line_x = x
		for ch in str:
			if ch == '\n':
				x = line_x
				y = y + self._height
				continue

			tile = self._tiles[ord(ch) & 0x7f]
			if op == "and":
				display.and_tile(x, y, tile)
			elif op == "or":
				display.or_tile(x, y, tile)
			elif op == "xor":
				display.xor_tile(x, y, tile)
			else:
				display.set_tile(x, y, tile)

			x = x + self._width

	@property
	def width(self):
		return self._width

	@property
	def height(self):
		return self._height
