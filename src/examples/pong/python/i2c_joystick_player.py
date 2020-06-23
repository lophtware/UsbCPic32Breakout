class I2cJoystickPlayer:
	def __init__(self, paddle, i2c, axis, axis_translate, axis_scale):
		self._paddle = paddle
		self._i2c = i2c
		self._axis = axis
		self._axis_translate = axis_translate
		self._axis_scale = axis_scale

	def advance(self, ball):
		axes = self._i2c.read(3)
		value = (axes[self._axis] + self._axis_translate) * self._axis_scale
		self._paddle.move(value)
		self._paddle.advance(ball)

	def reset(self, position):
		self._paddle.move(0)
		self._paddle.set_position(position)

	def draw(self, display):
		self._paddle.draw(display)

	@property
	def x(self):
		return self._paddle.x

	@property
	def y(self):
		return self._paddle.y
