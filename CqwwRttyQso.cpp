#include "StdAfx.h"

void CqwwRttyQso::AssignLocationExpected()
{
	if (!IsWVeRcvd()) 
	{
		_locationExpected = "DX";
		return;
	}

	// always believe the location field when we have his log
	if (gContest->LogsByCall().contains(_callRcvd))
	{
		_locationExpected = static_cast<CqwwRttyLog *> (gContest->LogsByCall().value(_callRcvd))->Location();
		return;
	}

	// no log, but W and VE we can figure
	if (_ctyRcvdPtr->IsW())
	{
		// found in AliasStates.lis
		if (_log->ContestPtr()->AliasStateMap().contains(_callRcvd))
			_locationExpected = _log->ContestPtr()->AliasStateMap()[_callRcvd];
		else
			// lookup W in FCC data
			if (gStateByFccCallMap.contains(_callRcvd))
				_locationExpected = gStateByFccCallMap.value(_callRcvd);
		return;
	}

	// province based on prefix from cqww.dat
	_locationExpected = _pxRcvd->Province(); 
}

bool CqwwRttyQso::CompareExchange(QsoPtr p)
{
	CqwwRttyQso * rq = static_cast<CqwwRttyQso *> (p);

	// check zone and location

	QString locationExpected = gUseExpected ? LocationExpected() : rq->XchSent2();
	int zoneExpected = gUseExpected ? _zoneExpected : rq->XchSent().toInt();

	if (CqwwQso::CompareExchange(rq) && (!IsWVeRcvd() || (locationExpected == gStateAbbreviationMap.value(_xchRcvd2))))
		return true;

	CorrectXch(QString("%1 %2").arg(zoneExpected, 2).arg(locationExpected));
	return false;
}

void CqwwRttyQso::Parse()
{
	_zoneRcvd = _rawXchRcvd.toInt();
	_xchRcvd.setNum(_zoneRcvd);				//  shorthand to remove leading zeros -- "01" becomes "1"

	_zoneSent = _xchSent.toInt();
	_xchSent.setNum(_zoneSent);

	_qrg = _band.toInt();
	BandEdges band;

	if(_qrg >= B10.LowerEdge()  && _qrg <= B10.UpperEdge())  band = B10;	else
	if(_qrg >= B15.LowerEdge()  && _qrg <= B15.UpperEdge())  band = B15;	else
	if(_qrg >= B20.LowerEdge()  && _qrg <= B20.UpperEdge())  band = B20;	else
	if(_qrg >= B40.LowerEdge()  && _qrg <= B40.UpperEdge())  band = B40;	else
	if(_qrg >= B80.LowerEdge()  && _qrg <= B80.UpperEdge())  band = B80;	else
	{
		if(_qrg ==  10) { _band =  B10.Name(); _qrg = B10.LowerEdge(); band = B10;  }   else
		if(_qrg ==  15) { _band =  B15.Name(); _qrg = B15.LowerEdge(); band = B15;  }   else
		if(_qrg ==  20) { _band =  B20.Name(); _qrg = B20.LowerEdge(); band = B20;  }   else
		if(_qrg ==  40) { _band =  B40.Name(); _qrg = B40.LowerEdge(); band = B40;  }   else
		if(_qrg ==  80) { _band =  B80.Name(); _qrg = B80.LowerEdge(); band = B80;  }   else
		{
			_status = DOES_NOT_COUNT;
			AppendErrMsgList("BAND:", _band);
			return;
		}
	}
	_band = band.Name();

	if(_qrg == band.LowerEdge() || _qrg == band.UpperEdge())
		_isBandEdge = true;
}

QString CqwwRttyQso::ToFullReverseString() const
{
	return QString("QSO: %1 %2 %3 %4 %5 %6 %7   %8 %9 %10 %11 %12")
		.arg(Qrg(),			5)
		.arg(Mode(),		2)
		.arg(DateStr()       )
		.arg(TimeStr()       )
		.arg(CallRcvd(),  -12)
		.arg(_rstRcvd,		3)
		.arg(_xchRcvd,		3)
		.arg(_xchRcvd2,		4)
		.arg(CallSent(),  -12)
		.arg(_rstSent,		3)
		.arg(_xchSent,		3)
		.arg(_xchSent2,		4);
}

QString CqwwRttyQso::ToString() const
{
	return QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
		.arg(_qrg, 5)
		.arg(_mode, 2)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(CallSent(), -10)
		.arg(_xchSent, 5)
		.arg(_xchSent2, 5)
		.arg(_rawCallRcvd, -10)
		.arg(_rawXchRcvd, 5)
		.arg(_rawXchRcvd2, 5);
}

QString CqwwRttyQso::ToTabString() const
{
	return QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10")
		.arg(_qrg)
		.arg(_mode)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(CallSent())
		.arg(_xchSent)
		.arg(_xchSent2)
		.arg(_rawCallRcvd)
		.arg(_rawXchRcvd)
		.arg(_rawXchRcvd2);
}

QString	CqwwRttyQso::LocationExpected() const { return _locationExpected; }

bool CqwwRttyQso::IsStateValid() const
{
	return false;
}

CqwwRttyQso::CqwwRttyQso(Log * log) : CqwwQso((CqwwLog *)log) {}
