#pragma once
#include "StdAfx.h"

class ArrlContest : public Contest 
{
public:
	ArrlContest();
	~ArrlContest();
	void WriteTsvFile();

	virtual void	WriteTsvHeader(QTextStream &) {};

protected:

};

