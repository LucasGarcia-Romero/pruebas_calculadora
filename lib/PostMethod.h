#pragma once
#include "common.h"


class PostMethod
{
public:
	string name;
	PostMethod() {};
	PostMethod(std::string name):name(name) {};

	virtual string exec(string params)=0;
	static string getPostParam(string paramList,string paramName) {
		size_t paramIdx = paramList.find_first_of(paramName+'=', 0);

		if (paramIdx == std::string::npos)return "";

		size_t paramEndIdx = paramList.find_first_of('&', paramIdx);
		if (paramEndIdx == string::npos)paramEndIdx = paramList.length();

		string paramPair = paramList.substr(paramIdx, paramEndIdx - paramIdx);
		std::string value = paramPair.substr(paramPair.find_first_of('=', 0) + 1, paramPair.length());

		return value;
	};
};

class Login : public PostMethod
{
public:
	virtual string exec(string params) override;

};

class ListWavFiles : public PostMethod
{
public:
	virtual string exec(string params) override;

};

class ListFiles : public PostMethod
{
public:
	ListFiles() :PostMethod("/listFiles") { };
	virtual string exec(string params) override;

};

class RecordData : public PostMethod
{
public:
	RecordData() :PostMethod("/recordData") { };
	virtual string exec(string params) override;

};