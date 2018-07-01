#include "StdAfx.h"

WpxQso::WpxQso(Log * log) : Qso(log) {}

bool WpxQso::CompareExchange(QsoPtr rq) 
{
	int sent = rq->XchSent().toInt();
	if(sent == 0)
		return true;
	int rcvd = _xchRcvd.toInt();
	CorrectXch(rq->XchSent());
	return abs(sent - rcvd) < 2;
}

void WpxQso::Parse()
{
		_xchSent = gFromCutNumber(_xchSent);
		_xchRcvd = gFromCutNumber(_xchRcvd);

	if(!_xchRcvd.toInt() && !BadInfoRcvdSet().contains(CallSent()))
		_status = BUSTED_EXCHANGE;

	_qrg = _band.toInt();
	BandEdges band;

	if(_qrg >= B10.LowerEdge()  && _qrg <= B10.UpperEdge())  band = B10;  else
	if(_qrg >= B10.LowerEdge()  && _qrg <= B10.UpperEdge())  band = B10;  else
	if(_qrg >= B15.LowerEdge()  && _qrg <= B15.UpperEdge())  band = B15;  else
	if(_qrg >= B20.LowerEdge()  && _qrg <= B20.UpperEdge())  band = B20;  else
	if(_qrg >= B40.LowerEdge()  && _qrg <= B40.UpperEdge())  band = B40;  else
	if(_qrg >= B80.LowerEdge()  && _qrg <= B80.UpperEdge())  band = B80;  else
	if(_qrg >= B160.LowerEdge() && _qrg <= B160.UpperEdge()) band = B160; else
	{
		if(_qrg == 160){ _band =  B160.Name(); _qrg =  B160.LowerEdge();   } else
		if(_qrg == 80) { _band =  B80.Name();  _qrg =  B80.LowerEdge();	} else
		if(_qrg == 40) { _band =  B40.Name();  _qrg =  B40.LowerEdge();	} else
		if(_qrg == 20) { _band =  B20.Name();  _qrg =  B20.LowerEdge();	} else
		if(_qrg == 15) { _band =  B15.Name();  _qrg =  B15.LowerEdge();	} else
		if(_qrg == 10) { _band =  B10.Name();  _qrg =  B10.LowerEdge();	} else
		{
			_status = DOES_NOT_COUNT;
			AppendErrMsgList("BAND:", _band);
			return;
		}
	}
	_band = band.Name();
	if(_qrg == band.LowerEdge() || _qrg == band.UpperEdge())
		_isBandEdge = true;

	_wpxStr = ComputeWpxStr(_callRcvd, _portDigit);
}
