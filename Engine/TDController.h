#pragma once
#include <list>
#include "IMGUI/imgui.h"
#include "Observer.h"

enum class InputEventTD { BUILD, STOP_BUILDING, CUR_TEST };

class TDController : public Observer
{
	std::list<InputEventTD> _inputEventList;

public:
	//enum class InputMode { PLAYING, BUILDING } mode;	//mode = InputMode::PLAYING;

	TDController()
	{}

	void Observe(const KeyPressMessage& msg) override
	{
		if (msg.key == 'C')
			_inputEventList.push_back(InputEventTD::STOP_BUILDING);
	}
	
	void Observe(const MouseClickMessage& msg) override
	{

		if (!ImGui::GetIO().WantCaptureMouse)
		{
			if (msg.mButtonType == MBT::LEFT && msg.pressed == true)
			{
				_inputEventList.push_back(InputEventTD::BUILD);
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