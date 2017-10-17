#include "GamepadController.h"
#include <SDL.h>
#include <string>
#include <cassert>

GamepadController::GamepadController(int controller) : joystick(nullptr)
{
	assert(SDL_NumJoysticks() > 0);
	if (controller == -1) {
		joystick = SDL_JoystickOpen(0);
	} else {
		joystick = SDL_JoystickOpen(controller);
	}
	SDL_JoystickEventState(SDL_QUERY);
}


void GamepadController::processEvent(const SDL_Event& ev) {
	if (ev.type == SDL_JOYAXISMOTION) {
		if (notifier) {
			notifier({ GamepadActionType::GAMEPAD_AXIS, ev.jaxis.axis, ev.jaxis.value });
		}
	}
	if (ev.type == SDL_JOYBUTTONUP || ev.type == SDL_JOYBUTTONDOWN) {
		if (notifier) {
			notifier({ GamepadActionType::GAMEPAD_BUTTON, ev.jbutton.button, ev.jbutton.state});
		}
	}
}

SDL_Joystick* GamepadController::getJoystick() {
	return joystick;
}

GamepadController::~GamepadController()
{
	SDL_JoystickClose(joystick);
}

std::vector<std::string> GamepadController::getAllControllers() {
	std::vector<std::string> controllers;
	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		auto name = SDL_JoystickNameForIndex(i);
		if (name) {
			controllers.emplace_back(name);
		} else {
			controllers.emplace_back("Controller " + std::to_string(i));
		}
	}
	return controllers;
}
bool GamepadController::attachedControllers() {
	return SDL_NumJoysticks() > 0;
}
