class Box:
	def __init__(self, x, y, width, height):
		self._min_x = x
		self._min_y = y
		self._max_x = x + width
		self._max_y = y + height
		self._width = width
		self._height = height

	@property
	def x(self):
		return self._min_x

	@property
	def y(self):
		return self._min_y

	@property
	def max_x(self):
		return self._max_x

	@property
	def max_y(self):
		return self._max_y

	@property
	def width(self):
		return self._width

	@property
	def height(self):
		return self._height

	def _contains_point(self, x, y):
		return x >= self._min_x and x < self._max_x and y >= self._min_y and y < self._max_y

	def _contains_box(self, box):
		return self._contains_point(box._min_x, box._min_y) and self._contains_point(box._max_x - 1, box._max_y - 1)

	def contains(self, x, y = None):
		if y is None:
			return self._contains_box(x)

		return self._contains_point(x, y)

	def _expand_to_contain_point(self, x, y):
		if self.contains(x, y):
			return self

		if self._min_x == 0 and self._min_y == 0 and self._max_x == 0 and self._max_y == 0:
			return Box(x, y, 1, 1)

		return Box(
			min(x, self._min_x),
			min(y, self._min_y),
			max(x + 1, self._max_x) - min(x, self._min_x),
			max(y + 1, self._max_y) - min(y, self._min_y))

	def _expand_to_contain_box(self, box):
		if self._min_x == 0 and self._min_y == 0 and self._max_x == 0 and self._max_y == 0:
			return box

		return Box(
			min(box._min_x, self._min_x),
			min(box._min_y, self._min_y),
			max(box._max_x, self._max_x) - min(box._min_x, self._min_x),
			max(box._max_y, self._max_y) - min(box._min_y, self._min_y))

	def expand_to_contain(self, x, y = None):
		if y is None:
			return self._expand_to_contain_box(x)

		return self._expand_to_contain_point(x, y)

Box.NONE = Box(0, 0, 0, 0)
