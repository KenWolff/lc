#include "StdAfx.h"

ArrlDxQso::ArrlDxQso(Log * log) : Qso(log) {}

QString ArrlDxQso::StringToPower(QString s) const
{
	if(s == "J")
		return "100";

	s.remove(QRegularExpression("W|WATTS"));
	s.replace(QRegularExpression("^K$|^1K$|^AK$|^1?KILO?$"), "1000");
	s.remove(QRegularExpression("[^\\d\\.AENTO]"));
	s.replace(QRegularExpression("A"),   "1");
	s.replace(QRegularExpression("E"),   "5");
	s.replace(QRegularExpression("N"),   "9");
	s.replace(QRegularExpression("T"),   "0");
	s.replace(QRegularExpression("O"),   "0");
	s.replace(QRegularExpression("^000$"), "1000");

	if (s.contains('.'))				// we get 1000.0 occasionally
		s.setNum(int(s.toFloat()));

	s.setNum(s.toInt());				// idiom to strip leading zeros
	return s;
}

void ArrlDxQso::Parse()
{
	_qrg = _band.toInt();
	BandEdges band;

	if(_qrg >= B10  .LowerEdge() && _qrg <= B10 .UpperEdge()) band = B10;  else
	if(_qrg >= B15  .LowerEdge() && _qrg <= B15 .UpperEdge()) band = B15;  else
	if(_qrg >= B20  .LowerEdge() && _qrg <= B20 .UpperEdge()) band = B20;  else
	if(_qrg >= B40  .LowerEdge() && _qrg <= B40 .UpperEdge()) band = B40;  else
	if(_qrg >= B80  .LowerEdge() && _qrg <= B80 .UpperEdge()) band = B80;  else
	if(_qrg >= B160 .LowerEdge() && _qrg <= B160.UpperEdge()) band = B160; else
	{
		if(_qrg ==  10)  { _band = B10 .Name(); _qrg = B10 .LowerEdge();  }  else
		if(_qrg ==  15)  { _band = B15 .Name(); _qrg = B15 .LowerEdge();  }  else
		if(_qrg ==  20)  { _band = B20 .Name(); _qrg = B20 .LowerEdge();  }  else
		if(_qrg ==  40)  { _band = B40 .Name(); _qrg = B40 .LowerEdge();  }  else
		if(_qrg ==  80)  { _band = B80 .Name(); _qrg = B80 .LowerEdge();  }  else
		if(_qrg == 160)  { _band = B160.Name(); _qrg = B160.LowerEdge();  }  else
		{
			_status  = DOES_NOT_COUNT;
			AppendErrMsgList("BAND:", _band);
			return;
		}
	}
	_band = band.Name();
	if(_qrg == band.LowerEdge() || _qrg == band.UpperEdge())
		_isBandEdge = true;

	if(!IsWVe() && !_ctyRcvdPtr->IsWVe())						// appears to be two dx stations
	{															// if rcvd info parses to a state, define him to be W/VE
		if (gStateAbbreviationMap.contains(_xchRcvd))
			AppendErrMsgList("DX/DX:", "");

		_status = DOES_NOT_COUNT;
		return;
	}

	if(IsWVe() == _ctyRcvdPtr->IsWVe())							// W can't work W, DX can't work DX
	{
		_status = DOES_NOT_COUNT;
		return;
	}

	if(!IsWVe())	// DX
	{
		if (_xchSent.contains("."))
			AppendErrMsgList("POWER:", _xchSent);
		_xchSent = StringToPower(_xchSent);

		_xchRcvd = gStateAbbreviationMap.value(_xchRcvd);
		if(_xchRcvd.isNull() && !_log->IsBadRcvdInfo())
		{
			AppendErrMsgList("STATE/PROV:", _xchRcvd);
			_status = BUSTED_EXCHANGE;
		}
	}
	else   // WVE
	{
		_xchSent = gStateAbbreviationMap.value(_xchSent);
		if (_xchSent.isNull())
			AppendErrMsgList("STATE/PROV:", _xchSent);

		if (_xchRcvd.contains("."))
			AppendErrMsgList("POWER:", _xchSent);

		_xchRcvd = StringToPower(_xchRcvd);
	}
}

QString ArrlDxQso::ToString() const
{
	return QString("%1 %2 %3 %4 %5 %6 %7 %8")
		.arg(_qrg, 5)
		.arg(_mode, 2)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(CallSent(), -10)
		.arg(_xchSent, 5)
		.arg(_rawCallRcvd, -10)
		.arg(_rawXchRcvd, 5);
}
