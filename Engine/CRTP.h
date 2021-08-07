#pragma once

template <typename T>
class CRTPBase
{
private:
	CRTPBase() {};
	friend T;	// This prevents oopsies where you do something like "class A : publice CRTPBase<B>" but also exposes internals... I'm not sold on the idea.

public:
	void doTheThing()
	{
		T& derived = static_cast<T>(*this);
		derived->
	}
};


class Derived final : public CRTPBase<Derived>
{
	// Careful not to have methods with the same name in Derived and CRTPBase class! 
	// Since CRTP does NOT use virtual functions it would be just hidden unless one used CRTPBase::whatever(), which is error prone and easy to forget
};