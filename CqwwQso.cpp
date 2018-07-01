#include "StdAfx.h"

CqwwQso::CqwwQso(Log * log) : Qso(log), _zoneRcvd(0), _zoneSent(0), _zoneExpected(0) { }

bool CqwwQso::CompareExchange(QsoPtr p)
{
	CqwwQso * rq = static_cast<CqwwQso *> (p);

	int zoneExpected = gUseExpected ? _zoneExpected : rq->XchSent().toInt();

	if (zoneExpected != _zoneRcvd)
	{
		CorrectXch(QString::number(zoneExpected));
		return false;
	}
	return true;
}

void CqwwQso::Parse()
{
	_zoneRcvd = _rawXchRcvd.toInt();
	if(!_zoneRcvd)
		AppendErrMsgList("ZONE RCVD:", _rawXchRcvd);
	_xchRcvd.setNum(_zoneRcvd);				//  shorthand to remove leading zeros -- "01" becomes "1"

	_zoneSent = _xchSent.toInt();
	if(_zoneSent)
		_xchSent.setNum(_zoneSent);
	else
		AppendErrMsgList("ZONE SENT:", _xchSent);

	int lz = static_cast<CqwwLog*>(_log)->Zone();
	if (_zoneSent != lz && !BadInfoSentSet().contains(CallSent()))
		AppendErrMsgList("ZONE SENT:", QString("%1, (%2)").arg(_xchSent, 3).arg(QString::number(lz)));

	_zoneExpected = gDeriveZone(_callRcvd, _pxRcvd);

	_qrg = _band.toInt();
	if(_qrg >= B10.LowerEdge()  && _qrg <= B10.UpperEdge())  _bandEdge = B10;	else
	if(_qrg >= B15.LowerEdge()  && _qrg <= B15.UpperEdge())  _bandEdge = B15;	else
	if(_qrg >= B20.LowerEdge()  && _qrg <= B20.UpperEdge())  _bandEdge = B20;	else
	if(_qrg >= B40.LowerEdge()  && _qrg <= B40.UpperEdge())  _bandEdge = B40;	else
	if(_qrg >= B80.LowerEdge()  && _qrg <= B80.UpperEdge())  _bandEdge = B80;	else
	if(_qrg >= B160.LowerEdge() && _qrg <= B160.UpperEdge()) _bandEdge = B160;   else
	{
		_status = DOES_NOT_COUNT;
		AppendErrMsgList("BAND:", _band);
		return;
	}

	_band = _bandEdge.Name();
	if (_qrg == _bandEdge.LowerEdge() || _qrg == _bandEdge.UpperEdge())
		_isBandEdge = true;

	if(_isOutOfBand)
		_log->AppendOutOfBandList(this);

	if (_isRubberClocked)
	{
		_status = RUBBER_CLOCK_VIOLATION;
		_log->AppendRubberClockedList(this);
	}
}

int	CqwwQso::ZoneRcvd()		const { return _zoneRcvd;		}
int	CqwwQso::ZoneExpected()	const { return _zoneExpected;	}
void CqwwQso::AssignZoneExpected() { _zoneExpected = gDeriveZone(_callRcvd, _pxRcvd); }
