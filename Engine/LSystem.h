#pragma once
#include <vector>
#include <map>


class SRay;


namespace Procedural
{

	struct RewriteRule 
	{
		char l;
		std::string r;
	};



	struct Grammar
	{
		std::vector<RewriteRule> rules;	//or std::map<char, std::string> ruleMap;
		std::vector<std::string> dictionary;
	};


	class LSystem
	{
	private:

		std::string _current, _next;
		Grammar _grammar;
		char _axiom;

	public:
		LSystem(char axiom);
		~LSystem();

		void seed(char initial);
		void rewrite();
		std::vector<SRay> getAsLineList(float length, float decay);
		void draw();
	};

}