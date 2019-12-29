#pragma once

struct KeyPressMessage
{
	char key;
	bool pressed;

	KeyPressMessage(char k, bool p) : key(k), pressed(p) {}
};

enum class MBT { LEFT, MIDDLE, RIGHT};

struct MouseClickMessage
{
	short x, y;
	bool pressed;
	MBT mButtonType;

	MouseClickMessage(short x, short y, MBT btn, bool pressed) : x(x), y(y), mButtonType(btn), pressed(pressed) {}
};


class Observer
{
public:
	virtual void Observe(const KeyPressMessage& msg) { };
	virtual void Observe(const MouseClickMessage& msg) { };

	virtual ~Observer() = default;
};


/* implementing crtp would mean my input manager has to be templated?? Not sure, but I can't have that!

class ObserverBase
{
public:

};

template <typename T
class Observer : ObserverBase
{
public:
	virtual void Observe(const T& attribute) = 0;

	virtual ~Observer() = default;
};
*/