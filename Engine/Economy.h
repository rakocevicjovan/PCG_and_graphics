#pragma once
#include "Math.h"
#include <string>
#include <map>
#include <vector>

//not really easy to modify and make data driven, strings would be more flexible for marginal perf loss...
//enum class TD_RES_TYPE { COIN, WOOD, STONE, IRON };

struct Transaction
{
	std::string _type;
	UINT _qty;
};


struct Income
{
	float _tickDuration;
	UINT _amount;

	Income(float tickDur, UINT perTick) : _tickDuration(tickDur), _amount(perTick) {}
};


struct TDResource
{
	std::string _type;
	UINT _qty;
	UINT _capacity;
	Income _income;

	TDResource(const std::string& type, UINT qty, UINT cap, float tick, UINT perTick)
		: _type(type), _qty(qty), _capacity(cap), _income(tick, perTick) {}
};


class Economy
{

	std::map<std::string, TDResource> _storage;

	inline bool gotEnough(const std::string& type, const UINT quantity) const
	{
		if (_storage.at(type)._qty >= quantity)
			return true;
		return false;
	}


public:

	bool createResource(const std::string& name, UINT cap = 100u, UINT qty = 0u, float tickDuration = 10.f, UINT perTick = 0u)
	{
		return _storage.emplace(name, TDResource(name, qty, cap, tickDuration, perTick)).second;
	}



	void deposit(const std::string& type, const UINT quantity)
	{
		_storage.at(type)._qty = min(_storage.at(type)._qty + quantity, _storage.at(type)._capacity);
	}

	
	bool withdraw(const std::string& type, const UINT quantity)
	{
		if (gotEnough(type, quantity))
		{
			_storage.at(type)._qty -= quantity;
			return true;
		}
		return false;
	}


	//only a utility function to have less code outside of the class really
	void depositMultiple(const std::vector<Transaction>& transactions)
	{
		for (const Transaction& transaction : transactions)
		{
			deposit(transaction._type, transaction._qty);
		}
	}


	bool withdrawMultiple(const std::vector<Transaction>& transactions)
	{
		if (!canAfford(transactions))
			return false;

		for (const Transaction& transaction : transactions)
		{
			_storage.at(transaction._type)._qty -= transaction._qty;
		}

		return true;
	}


	//This could prove useless in case of various buildings at different/variable frequencies etc... could be better 
	//to simply let the game deposit resources if it gets too complicated... or have a full blown system for it
	bool setResourceIncome(const std::string& type, float tickDuration = 10.f, UINT amount = 0u)
	{
		_storage.at(type)._income = { tickDuration, amount };
	}


	inline bool canAfford(const std::vector<Transaction>& transactions) const
	{
		for (const Transaction& transaction : transactions)
			if (!gotEnough(transaction._type, transaction._qty))
				return false;

		return true;
	}
};