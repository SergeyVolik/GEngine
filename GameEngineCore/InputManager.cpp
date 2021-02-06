#include "InputManager.h"
#include "Time.h"
#include "WindowManager.h"

bool* te::InputManager::_keys;
unsigned int* te::InputManager::_frames;
float te::InputManager::deltaX = 0;
float te::InputManager::deltaY = 0;
float te::InputManager::x = 0;
float te::InputManager::y = 0;
bool te::InputManager::_cursorLoked = false;
bool te::InputManager::_cursorrStarted = false;

bool te::InputManager::_initialized = false;

void te::InputManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (te::InputManager::_cursorrStarted)
	{
		te::InputManager::deltaX = xpos - te::InputManager::x;
		te::InputManager::deltaY = ypos - te::InputManager::y;
	}
	else {
		te::InputManager::_cursorrStarted = false;
	}
	te::InputManager::y = ypos;
	te::InputManager::x = xpos;
}

void te::InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

	if (action == GLFW_PRESS)
	{
		te::InputManager::_keys[_MOUSE_BUTTONS + button] = true;
		te::InputManager::_frames[_MOUSE_BUTTONS + button] = Time::currentFrame;
	}
	else if (action == GLFW_RELEASE)
	{
		te::InputManager::_keys[_MOUSE_BUTTONS + button] = false;
		te::InputManager::_frames[_MOUSE_BUTTONS + button] = Time::currentFrame;
	}
}

void te::InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		te::InputManager::_keys[key] = true;
		te::InputManager::_frames[key] = Time::currentFrame;
	}
	else if (action == GLFW_RELEASE)
	{
		te::InputManager::_keys[key] = false;
		te::InputManager::_frames[key] = Time::currentFrame;
	}
}
	

void te::InputManager::initialize(GLFWwindow* window)
{

	if (!WindowManager::isInitialized())
	{
		throw std::runtime_error("error: can't init Input Manager before Window Manager!");
	}

	if (!te::InputManager::_initialized)
	{
		_keys = new bool[1032];
		_frames = new unsigned int[1032];

		memset(_keys, false, 1032);
		memset(_frames, 0, 1032);

		glfwSetKeyCallback(window, keyCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, cursorPositionCallback);
		te::InputManager::_initialized = true;
	}
}