#include "StdAfx.h"
#include "CqContest.h"

CqContest::CqContest() {}
CqContest::~CqContest() {}

CatDesc const CqContest::CatDescrFromNum(int num)	const { return _catDescByNum.value(num); }
int	CqContest::CatNumFromInternalDesc(QString s) const { return _catNumByInternalDesc.value(s); }