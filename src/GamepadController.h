#pragma once
#include <string>
#include <vector>
#include <memory>
#include <SDL_gamecontroller.h>
#include <SDL_events.h>
#include <functional>
enum class GamepadActionType {
	GAMEPAD_AXIS,
	GAMEPAD_BUTTON
};
enum class GamepadAxisType {
	GAMEPAD_AXIS_LEFT_X,
	GAMEPAD_AXIS_LEFT_Y,
	GAMEPAD_AXIS_RIGHT_X,
	GAMEPAD_AXIS_RIGHT_Y,
	GAMEPAD_AXIS_IDK_X,
	GAMEPAD_AXIS_IDK_Y
};

struct GamepadAction {
	GamepadActionType type;
	int num;
	int val;
};

class GamepadController
{
public:
	GamepadController(int controller = -1);
	~GamepadController();
	void processEvent(const SDL_Event& ev);
	static std::vector<std::string> getAllControllers();
	std::function<void(GamepadAction)> notifier;
	static bool attachedControllers();
	SDL_Joystick* getJoystick();
private:
	SDL_Joystick* joystick;
};

