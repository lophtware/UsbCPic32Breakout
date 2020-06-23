import random
import time
import math

class GameLoop:
	def __init__(self, playing_area, ball, players, score_font, message_font):
		self._state = _State(
			playing_area,
			ball,
			players,
			scores = [0, 0],
			is_game_over = False,
			score_font = score_font,
			message_font = message_font)

		self._state_action = _Uninitialised(self._state)

	def do_frame(self, display):
		self._state_action = self._state_action.do_frame(display)
		return not self._state.is_game_over

class _State:
	def __init__(self, playing_area, ball, players, scores, is_game_over, score_font, message_font):
		self.playing_area = playing_area
		self.ball = ball
		self.players = players
		self.scores = scores
		self.is_game_over = is_game_over
		self.score_font = score_font
		self.message_font = message_font

class _Uninitialised:
	def __init__(self, state):
		self._state = state

	def do_frame(self, display):
		self._state.to_serve = 0
		return _DisplayScores(
			self._state,
			_ShowMessage(
				self._state,
				"!! BRING IT PONG !!",
				seconds = 3,
				next_state = _ResetPlayingPositions(self._state)))

class _DisplayScores:
	def __init__(self, state, next_state):
		self._state = state
		self._next_state = next_state

	def do_frame(self, display):
		score_length = 3
		score_centre_x = int(self._state.playing_area.centre[0] - (score_length * self._state.score_font.width) / 2)
		self._state.score_font.write(
			display,
			score_centre_x,
			2,
			"{0}-{1}".format(self._state.scores[0], self._state.scores[1]))

		return self._next_state

class _ResetPlayingPositions:
	def __init__(self, state):
		self._state = state

	def do_frame(self, display):
		if self._state.to_serve == 0:
			ball_x = self._state.players[0].x + self._state.ball.radius + 2
			any_angle = 45 - random.random() * 90
		else:
			ball_x = self._state.players[1].x - self._state.ball.radius - 2
			any_angle = 90 + 45 + random.random() * 90

		self._state.ball.set_position(ball_x, self._state.playing_area.centre[1])
		self._state.ball.set_direction(math.radians(any_angle))
		self._state.ball.draw(display)

		for player in self._state.players:
			player.reset(self._state.playing_area.centre[1])
			player.draw(display)

		if self._state.scores[0] == 3:
			return _ShowMessage(self._state, "PLAYER 1 WINS", seconds = 3, next_state = _GameOver(self._state))
		elif self._state.scores[1] == 3:
			return _ShowMessage(self._state, "PLAYER 2 WINS", seconds = 3, next_state = _GameOver(self._state))

		return _DelayFor(self._state, seconds = 2, next_state = _Playing(self._state))

class _ShowMessage:
	def __init__(self, state, message, seconds, next_state):
		self._state = state
		self._message = message
		self._seconds = seconds
		self._next_state = next_state

	def do_frame(self, display):
		message_lines = self._message.split('\n')
		message_length = max(len(x) for x in message_lines)
		message_lines = len(message_lines)
		message_centre_x = int(self._state.playing_area.centre[0] - (message_length * self._state.message_font.width) / 2)
		message_centre_y = int(self._state.playing_area.centre[1] - (message_lines * self._state.message_font.height) / 2)
		self._state.message_font.write(
			display,
			message_centre_x,
			message_centre_y,
			self._message,
			op = "xor")

		return _DelayFor(self._state, self._seconds, _HideMessage(
			self._state,
			self._message,
			next_state = self._next_state))

class _HideMessage:
	def __init__(self, state, message, next_state):
		self._state = state
		self._message = message
		self._next_state = next_state

	def do_frame(self, display):
		message_lines = self._message.split('\n')
		message_length = max(len(x) for x in message_lines)
		message_lines = len(message_lines)
		message_centre_x = int(self._state.playing_area.centre[0] - (message_length * self._state.message_font.width) / 2)
		message_centre_y = int(self._state.playing_area.centre[1] - (message_lines * self._state.message_font.height) / 2)
		self._state.message_font.write(
			display,
			message_centre_x,
			message_centre_y,
			self._message,
			op = "xor")

		return _DelayFor(self._state, seconds = 0, next_state = self._next_state)

class _DelayFor:
	def __init__(self, state, seconds, next_state):
		self._state = state
		self._delay_until = time.perf_counter() + seconds
		self._next_state = next_state

	def do_frame(self, display):
		if time.perf_counter() >= self._delay_until:
			return self._next_state

		return self

class _Playing:
	def __init__(self, state):
		self._state = state

	def do_frame(self, display):
		frame_start_time = time.perf_counter()

		for player in self._state.players:
			player.advance(self._state.ball)
			player.draw(display)

		[is_out, _] = self._state.ball.advance()
		self._state.ball.draw(display)

		if is_out < 0:
			return _PlayerOut(self._state, 0)
		elif is_out > 0:
			return _PlayerOut(self._state, 1)

		frame_time = time.perf_counter() - frame_start_time
		return _DelayFor(self._state, 0.02 - frame_time, self)

class _PlayerOut:
	def __init__(self, state, player_number):
		self._state = state
		self._player_number = player_number

	def do_frame(self, display):
		scorer = 0 if self._player_number == 1 else 1
		self._state.scores[scorer] = self._state.scores[scorer] + 1
		self._state.to_serve = self._player_number
		return _DisplayScores(
			self._state,
			_ShowMessage(
				self._state,
				"OUT !",
				seconds = 3,
				next_state = _ResetPlayingPositions(self._state)))

class _GameOver:
	def __init__(self, state):
		self._state = state

	def do_frame(self, display):
		self._state.is_game_over = True
		return self
