#pragma once
#include <list>
#include "IMGUI/imgui.h"
#include "Observer.h"

enum class InputEventTD { SELECT, STOP_BUILDING, CUR_TEST, RESET_CREEPS };

class TDController : public Observer
{
	std::list<InputEventTD> _inputEventList;

public:
	//enum class InputMode { PLAYING, BUILDING } mode;	//mode = InputMode::PLAYING;

	TDController()
	{}

	void Observe(const KeyPressMessage& msg) override
	{
		switch (msg.key)
		{
		case 'C':
			_inputEventList.push_back(InputEventTD::STOP_BUILDING);
			break;

		case 'R':
			_inputEventList.push_back(InputEventTD::RESET_CREEPS);
			break;

		case 'B':
			_inputEventList.push_back(InputEventTD::SELECT);
			break;
		}
			
	}
	
	void Observe(const MouseClickMessage& msg) override
	{
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			if (msg.mButtonType == MBT::LEFT && msg.pressed == true)
			{
				_inputEventList.push_back(InputEventTD::SELECT);
			}

			if (msg.mButtonType == MBT::RIGHT && msg.pressed == true)
			{
				_inputEventList.push_back(InputEventTD::STOP_BUILDING);
			}
		}
	}



	bool consumeNextAction(InputEventTD& inEvent)
	{
		if (_inputEventList.empty())
			return false;

		inEvent = _inputEventList.front();
		_inputEventList.pop_front();
		return true;
	}
};