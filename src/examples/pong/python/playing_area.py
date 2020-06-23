from box import Box
from tile import Tile

class PlayingArea:
	_CORNER_TL = Tile(8, 8, [
		0b11111111,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
	])

	_CORNER_TR = Tile(8, 8, [
		0b11111111,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
	])

	_CORNER_BR = Tile(8, 8, [
		0b00000000,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b11111111,
	])

	_CORNER_BL = Tile(8, 8, [
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
		0b11111111,
	])

	_HORIZONTAL = Tile(8, 1, [0b11111111])

	_VERTICAL = Tile(1, 8, [0b10000000] * 8)

	def __init__(self, display):
		self._display = display
		self._bounding_box = Box(1, 1, display.width - 2, display.height - 2)
		self._centre = [self._bounding_box.x + self._bounding_box.width / 2, self._bounding_box.y + self._bounding_box.height / 2]

	def draw(self):
		self._display.clear()
		for x in range(0, self._display.width - PlayingArea._HORIZONTAL.width + 1, PlayingArea._HORIZONTAL.width):
			self._display.or_tile(x, 0, PlayingArea._HORIZONTAL)
			self._display.or_tile(x, self._display.height - PlayingArea._HORIZONTAL.height, PlayingArea._HORIZONTAL)

		for y in range(0, self._display.height - PlayingArea._VERTICAL.height + 1, PlayingArea._VERTICAL.height):
			self._display.or_tile(0, y, PlayingArea._VERTICAL)
			self._display.or_tile(self._display.width - PlayingArea._VERTICAL.width, y, PlayingArea._VERTICAL)

		self._display.or_tile(0, 0, PlayingArea._CORNER_TL)
		self._display.or_tile(self._display.width - PlayingArea._CORNER_TR.width, 0, PlayingArea._CORNER_TR)
		self._display.or_tile(self._display.width - PlayingArea._CORNER_BR.width, self._display.height - PlayingArea._CORNER_BR.height, PlayingArea._CORNER_BR)
		self._display.or_tile(0, self._display.height - PlayingArea._CORNER_BL.height, PlayingArea._CORNER_BL)

	@property
	def centre(self):
		return self._centre

	@property
	def bounding_box(self):
		return self._bounding_box
