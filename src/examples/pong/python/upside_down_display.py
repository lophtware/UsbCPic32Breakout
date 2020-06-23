class UpsideDownDisplay:
	def __init__(self, driver):
		self._driver = driver

	def initialise(self):
		self._driver.initialise()

	@property
	def width(self):
		return self._driver.width

	@property
	def height(self):
		return self._driver.height

	def clear(self):
		self._driver.clear()

	def blit(self):
		self._driver.blit()

	def cgram_address_of(self, x, y):
		return self._driver.cgram_address_of(
			self._driver.width - 1 - x,
			self._driver.height - 1 - y)

	def set_pixel(self, x, y, value):
		return self._driver.set_pixel(
			self._driver.width - 1 - x,
			self._driver.height - 1 - y,
			value)

	def or_tile(self, x, y, tile):
		return self._driver.or_tile(
			self._driver.width - x - tile.width,
			self._driver.height - y - tile.height,
			tile.rotated_180)

	def xor_tile(self, x, y, tile):
		return self._driver.xor_tile(
			self._driver.width - x - tile.width,
			self._driver.height - y - tile.height,
			tile.rotated_180)

	def and_tile(self, x, y, tile):
		return self._driver.and_tile(
			self._driver.width - x - tile.width,
			self._driver.height - y - tile.height,
			tile.rotated_180)

	def set_tile(self, x, y, tile):
		return self._driver.set_tile(
			self._driver.width - x - tile.width,
			self._driver.height - y - tile.height,
			tile.rotated_180)

	def set_brightness(self, percent):
		self._driver.set_brightness(percent)
