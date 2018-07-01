#pragma once
#include "StdAfx.h"

class ArrlLog : public Log
{
public:
	ArrlLog();
	~ArrlLog();

	QString	Division() { return _secToDivMap.count(_location) ? _secToDivMap[_location] : ""; }
	QString	Region()   { return _secToRegMap.count(_location) ? _secToRegMap[_location] : ""; }

protected:
	static std::map <QString, QString>	_secToRegMap;
	static std::map <QString, QString>	_secToDivMap;
};

