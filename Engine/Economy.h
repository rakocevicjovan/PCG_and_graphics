#pragma once
#include "pch.h"

//not really easy to modify and make data driven, strings would be more flexible for marginal perf loss...
//enum class TD_RES_TYPE { COIN, WOOD, STONE, IRON };

struct Transaction
{
	std::string _type;
	UINT _qty;

	Transaction() {}
	Transaction(const std::string& type, UINT qty) : _type(type), _qty(qty) {}
};


struct Income
{
	float _tickDuration;
	Transaction _transaction;

	Income() {}
	Income(float tickDur, const std::string& type, UINT perTick)
		: _tickDuration(tickDur), _transaction(type, perTick) {}
};


struct TDResource
{
	std::string _type;
	UINT _qty;
	UINT _capacity;
	Income _income;	//used to support some custom income 

	TDResource(const std::string& type, UINT qty, UINT cap)
		: _type(type), _qty(qty), _capacity(cap) {}
};



class Economy
{
	std::map<std::string, TDResource> _vault;

	inline bool gotEnough(const std::string& type, const UINT quantity) const
	{
		if (_vault.at(type)._qty >= quantity)
			return true;
		return false;
	}


public:

	bool createResource(const std::string& name, UINT cap = 100u, UINT qty = 0u, float tickDuration = 10.f, UINT perTick = 0u)
	{
		return _vault.emplace(name, TDResource(name, qty, cap)).second;
	}



	void deposit(const std::string& type, const UINT quantity)
	{
		_vault.at(type)._qty = std::min(_vault.at(type)._qty + quantity, _vault.at(type)._capacity);
	}

	
	bool withdraw(const std::string& type, const UINT quantity)
	{
		if (gotEnough(type, quantity))
		{
			_vault.at(type)._qty -= quantity;
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
			_vault.at(transaction._type)._qty -= transaction._qty;
		}

		return true;
	}


	//This could prove useless in case of various buildings at different/variable frequencies etc... could be better 
	//to simply let the game deposit resources if it gets too complicated... or have a full blown system for it
	bool setResourceIncome(const std::string& type, float tickDuration = 10.f, UINT amount = 0u)
	{
		_vault.at(type)._income = Income(tickDuration, type, amount);
	}


	inline bool canAfford(const std::vector<Transaction>& transactions) const
	{
		for (const Transaction& transaction : transactions)
			if (!gotEnough(transaction._type, transaction._qty))
				return false;

		return true;
	}


	const std::map<std::string, TDResource>& getVault()
	{
		return _vault;
	}


	void renderEconomyWidget()
	{
		ImGui::Begin("Resources");

		for (const auto& r : _vault)
		{
			ImGui::Text(r.first.c_str());
			ImGui::SameLine();
			ImGui::Text( (std::to_string(r.second._qty) + "/" + std::to_string(r.second._capacity)).c_str() );
			//sum icon too
		}

		ImGui::End();
	}
};