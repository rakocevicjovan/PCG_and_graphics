#pragma once
#include <list>
#include "Observer.h"

enum class InputEventTD { BUILD, NONE };

class TDController : public Observer
{
	std::list<InputEventTD> _inputEventList;

public:
	enum class InputMode { PLAYING, BUILDING } mode;

	TDController()
	{
		mode = InputMode::PLAYING;
	}

	void Observe(const KeyPressMessage& msg) override
	{

	}
	
	void Observe(const MouseClickMessage& msg) override
	{
		if (msg.mButtonType == MBT::LEFT && msg.pressed == true)
		{
			_inputEventList.push_back(InputEventTD::BUILD);
		}
	}

	InputEventTD getNextAction()
	{
		InputEventTD result;

		if (!_inputEventList.empty())
		{
			result = _inputEventList.front();
			_inputEventList.pop_front();
		}
		else
		{
			result = InputEventTD::NONE;
		}
		
		return result;
	}

};