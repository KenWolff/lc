#include "StdAfx.h"

IaruQso::IaruQso(Log * log) : Qso(log), _zoneRcvd(0) {}

bool IaruQso::CompareExchange(QsoPtr p)
{
	IaruQso * rq = static_cast<IaruQso *> (p);

	if (!_isHq)
	{
		if (rq->XchSent().toInt() != _zoneRcvd)
		{
			CorrectXch(rq->XchSent());
			return false;
		}
		return true;
	}

	if (rq->XchSent() != _xchRcvd)
	{
		CorrectXch(rq->XchSent());
		return false;
	}
	return true;
}

void IaruQso::Parse()
{
	IaruContest * contest = static_cast<IaruContest *> (gContest);

	if (contest->IsHqAbrv(_xchRcvd))
	{
		if (contest->IsHqFromCall(_callRcvd))
		{
			_isHq = true;
			_correctXch = contest->HqAbrvFromCall(_callRcvd);
		}
	}
	else
	{
		_xchRcvd = gFromCutNumber(_xchRcvd);
		_zoneRcvd = _xchRcvd.toInt();

		int lz = _pxRcvd->IaruZone();
		if (!_zoneRcvd && _zoneRcvd != lz && !BadInfoSentSet().contains(CallSent()))
			AppendErrMsgList("ZONE Rcvd:", QString("%1 (%2)").arg(_zoneRcvd, 3).arg(QString::number(lz)));
	}

	_qrg = _band.toInt();
	if(_qrg >= B10.LowerEdge()  && _qrg <= B10.UpperEdge())  _bandEdge = B10;	else
	if(_qrg >= B15.LowerEdge()  && _qrg <= B15.UpperEdge())  _bandEdge = B15;	else
	if(_qrg >= B20.LowerEdge()  && _qrg <= B20.UpperEdge())  _bandEdge = B20;	else
	if(_qrg >= B40.LowerEdge()  && _qrg <= B40.UpperEdge())  _bandEdge = B40;	else
	if(_qrg >= B80.LowerEdge()  && _qrg <= B80.UpperEdge())  _bandEdge = B80;	else
	if(_qrg >= B160.LowerEdge() && _qrg <= B160.UpperEdge()) _bandEdge = B160;	else
	{
		_status = DOES_NOT_COUNT;
		AppendErrMsgList("BAND:", _band);
		return;
	}

	_band = _bandEdge.Name() + " " + _mode;
	if (_qrg == _bandEdge.LowerEdge() || _qrg == _bandEdge.UpperEdge())
		_isBandEdge = true;

	if(_isOutOfBand)
		_log->AppendOutOfBandList(this);
}

bool	IaruQso::IsHq()		const { return _isHq; }
int		IaruQso::ZoneSent() const { return static_cast<IaruLog*>(_log)->Zone(); }
int		IaruQso::ZoneRcvd() const { return _zoneRcvd; }

