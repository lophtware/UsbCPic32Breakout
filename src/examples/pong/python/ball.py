import math

from box import Box

class Ball:
	def set_position(self, x, y = None):
		self._position = x if y is None else [x, y]

	def set_direction(self, angle_rads):
		self._velocity = [self._speed * math.cos(angle_rads), self._speed * -math.sin(angle_rads)]

	def __init__(self, position, speed_angle, bounding_box, tile):
		self._position = position
		self._speed = speed_angle[0]
		self._bounding_box = bounding_box
		self._tile = tile
		self._radius = tile.width / 2

		self._display_position = [int(position[0]), int(position[1])]
		self._previous_position = None

		self.set_direction(speed_angle[1])

	def as_box(self):
		return Box(self._position[0] - self._radius, self._position[1] - self._radius, self._radius * 2, self._radius * 2)

	def _bounce(self, position):
		if self._bounding_box.contains(Box(position[0] - self._radius, position[1] - self._radius, self._radius * 2, self._radius * 2)):
			return [position, self._velocity, [0, 0]]

		x = position[0]
		x_velocity = self._velocity[0]
		x_collide = 0
		if x - self._radius < self._bounding_box.x:
			x = self._bounding_box.x + self._radius
			x_velocity = -x_velocity
			x_collide = -1
		elif x + self._radius >= self._bounding_box.max_x - 1:
			x = self._bounding_box.max_x - self._radius - 1
			x_velocity = -x_velocity
			x_collide = 1

		y = position[1]
		y_velocity = self._velocity[1]
		y_collide = 0
		if y - self._radius < self._bounding_box.y:
			y = self._bounding_box.y + self._radius
			y_velocity = -y_velocity
			y_collide = -1
		elif y + self._radius >= self._bounding_box.max_y - 1:
			y = self._bounding_box.max_y - self._radius - 1
			y_velocity = -y_velocity
			y_collide = 1

		return [[x, y], [x_velocity, y_velocity], [x_collide, y_collide]]

	def advance(self):
		[self._position, self._velocity, collision] = self._bounce([
			self._position[0] + self._velocity[0],
			self._position[1] + self._velocity[1]])

		return collision

	def draw(self, display):
		if self._previous_position is not None:
			display.xor_tile(self._previous_position[0], self._previous_position[1], self._tile)

		[x, y] = [int(self._position[0] - self._radius), int(self._position[1] - self._radius)]
		display.xor_tile(x, y, self._tile)
		self._previous_position = [x, y]

	@property
	def x(self):
		return self._position[0]

	@property
	def y(self):
		return self._position[1]

	@property
	def radius(self):
		return self._radius
