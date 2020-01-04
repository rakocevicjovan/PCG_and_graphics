#pragma once
#include "Math.h"
#include <string>
#include <map>
#include <vector>

//not really easy to modify and make data driven, strings would be more flexible for marginal perf loss...
enum class TD_RES_TYPE { COIN, WOOD, STONE, IRON };

struct Transaction
{
	TD_RES_TYPE _type;
	UINT _qty;
};


struct Income
{
	float _frequency;
	UINT _amount;
};


struct TDResource
{
	TD_RES_TYPE _type;
	std::string name;
	UINT _qty;
	UINT _capacity;
	Income _income;
};


class Economy
{

	std::map< TD_RES_TYPE, TDResource> _storage;

	inline bool gotEnough(const TD_RES_TYPE type, const UINT quantity) const
	{
		if (_storage.at(type)._qty >= quantity)
			return true;
		return false;
	}


public:

	void deposit(const TD_RES_TYPE type, const UINT quantity)
	{
		_storage.at(type)._qty = min(_storage.at(type)._qty + quantity, _storage.at(type)._capacity);
	}

	
	bool withdraw(const TD_RES_TYPE type, const UINT quantity)
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
	bool setResourceIncome(const TD_RES_TYPE type, float freq = 10.f, UINT amount = 0u)
	{
		_storage.at(type)._income = { freq, amount };
	}


	inline bool canAfford(const std::vector<Transaction>& transactions) const
	{
		for (const Transaction& transaction : transactions)
			if (!gotEnough(transaction._type, transaction._qty))
				return false;

		return true;
	}
};