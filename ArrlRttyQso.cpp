#include "StdAfx.h"

ArrlRttyQso::ArrlRttyQso(Log * log) : Qso(log) {}


bool ArrlRttyQso::CompareExchange(QsoPtr rq)
{
	if(!rq->IsWVe())
		if(rq->XchSent() == "0")
			return true;

	CorrectXch(rq->XchSent());
	return rq->XchSent() == _xchRcvd; 
}

void ArrlRttyQso::Parse()
{
	if(IsWVe())												   // normalize the exchange sent
		_xchSent = gStateAbbreviationMap.value(_log->Location());
	else
		_xchSent.setNum(_xchSent.toInt());

	_qrg = _band.toInt();
	BandEdges band;

	if(_qrg >= B10.LowerEdge() && _qrg <= B10.UpperEdge())   band = B10; else
	if(_qrg >= B15.LowerEdge() && _qrg <= B15.UpperEdge())   band = B15; else
	if(_qrg >= B20.LowerEdge() && _qrg <= B20.UpperEdge())   band = B20; else
	if(_qrg >= B40.LowerEdge() && _qrg <= B40.UpperEdge())   band = B40; else
	if(_qrg >= B80.LowerEdge() && _qrg <= B80.UpperEdge())   band = B80; else
	{
		if(_qrg == 80) { _band =  B80.Name();  _qrg = B80.LowerEdge();  }   else
		if(_qrg == 40) { _band =  B40.Name();  _qrg = B40.LowerEdge();  }   else
		if(_qrg == 20) { _band =  B20.Name();  _qrg = B20.LowerEdge();  }   else
		if(_qrg == 15) { _band =  B15.Name();  _qrg = B15.LowerEdge();  }   else
		if(_qrg == 10) { _band =  B10.Name();  _qrg = B10.LowerEdge();  }   else
		{
			AppendErrMsgList("BAND:", _band);
			_status = DOES_NOT_COUNT;
			return;
		}
	}
	_band = band.Name();
	if(_qrg == band.LowerEdge() || _qrg == band.UpperEdge())
		_isBandEdge = true;

	if (_ctyRcvdPtr->IsWVe())
	{
		if (gStateAbbreviationMap.contains(_xchRcvd))
			_xchRcvd = gStateAbbreviationMap.value(_xchRcvd);
		else
		{
			_status = DOES_NOT_COUNT;
			AppendErrMsgList("EXCH:", _xchRcvd);
		}
	}
	else
		_xchRcvd.setNum(_xchRcvd.toInt());
}
