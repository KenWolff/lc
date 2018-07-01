#include "StdAfx.h"
Cty			gEmptyCty(0,"Empty Cty", "NO CTY", 0, 0, "NIL", 0.0,0.0,0.0, false);
Px const	gEmptyPx("No Cty", 0, 0, "", "", 0.0, 0.0, & gEmptyCty, false, false, false, false);
PxMap		gPxMap;

Px const * PxMap::FindExactMatchPx(QString call)
{
	if(_exactPxMap.contains(call))
		return _exactPxMap.value(call);

	return 0;
}

Px const * PxMap::FindPx(QString pxStr)
{
	Px const * px = &gEmptyPx;

	if(_exactPxMap.contains(pxStr))
		return _exactPxMap.value(pxStr);

	while (pxStr.size())
	{
		if (_pxMap.contains(pxStr))
		{
			px = _pxMap.value(pxStr);

			if(px->GetCty()->IsKG4() && pxStr.length() != 5)
				px = W_id->PxList().first();

			return px;
		}
		else
			pxStr.chop(1);
	}
	return &gEmptyPx;
}

void PxMap::InsertPx(QString pxStr, Px * px, bool exact)
{
	if (exact && !_exactPxMap.contains(pxStr))
		_exactPxMap.insert(pxStr, px);

	if (!exact && !_pxMap.contains(pxStr))
			_pxMap.insert(pxStr, px);
}

PxMap::~PxMap() 
{
	/*foreach(PxPtr p, _pxMap)
		delete p;

	foreach(PxPtr p, _exactPxMap)
		delete p;*/
}
