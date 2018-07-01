#include "StdAfx.h"
VhfQso::VhfQso(Log * log) : Qso(log) {}

bool VhfQso::CompareExchange(QsoPtr rq)
{
	CorrectXch(rq->XchSent());
	return rq->XchSent() == _xchRcvd;
}

void VhfQso::Parse()
{
	_xchSent.truncate(4);
	_xchRcvd.truncate(4);
	
	QRegularExpression re;
	re.setPattern("/[RPM]/");  // K9JK/R/EN51
	_callRcvd.replace(re, "/");

	QRegularExpression roverRe
	(
		"("
		"^R/|"
		"^ROVER/|"
		"^R[0-9]/|"
		"^[A-Z][A-Z][0-9][0-9]/|"
		"^[A-Z][A-Z][0-9][0-9][A-Z][A-Z]/|"
		"/R$|"
		"/ROVER$|"
		"/R[0-9]$|"
		"/[A-Z][A-Z][0-9][0-9]$|"
		"/[A-Z][A-Z][0-9][0-9][A-Z][A-Z]$"
		")"
	);  // detect rovers

	std::map <QString, QString> bandByRawBandMap = 
	{
		{"A",		B6.Name()},		{"50",		B6.Name()},		{"52",		B6.Name()},		{"54",		B6.Name()},		{"6",		B6.Name()},
		{"B",		B2.Name()},		{"144",		B2.Name()},		{"145",		B2.Name()},		{"146",		B2.Name()},		{"147",		B2.Name()},		{"2", B2.Name()},
		{"C",		B222.Name()},	{"220",		B222.Name()},	{"222",		B222.Name()},	{"223",		B222.Name()},	{"224",		B222.Name()},
		{"D",		B432.Name()},	{"420",		B432.Name()},	{"432",		B432.Name()},	{"439",		B432.Name()},	{"440",		B432.Name()},	{"446", B432.Name()},
		{"9",		B903.Name()},	{"900",		B903.Name()},	{"902",		B903.Name()},	{"903",		B903.Name()},
		{"E",		B1296.Name()},	{"1240",	B1296.Name()},	{"1294",	B1296.Name()},	{"1296",	B1296.Name()},	{"1.2",		B1296.Name()},	{"1.3", B1296.Name()},	{"1.2G", B1296.Name()},
		{"F",		B2304.Name()},	{"2304",	B2304.Name()},	{"2.3",		B2304.Name()},	{"2.3G",	B2304.Name()},	{"2G",		B2304.Name()},	
		{"G",		B3456.Name()},	{"3456",	B3456.Name()},	{"3.5",		B3456.Name()},	{"3.G",		B3456.Name()},	{"3.4",		B3456.Name()},	{"3.4G", B3456.Name()},	{"H", B5760.Name()},
		{"H",		B5760.Name()},	{"5.7",		B5760.Name()},	{"5.7G",	B5760.Name()},	{"5G",		B5760.Name()},
		{"I",		B10G.Name()},	{"10G",		B10G.Name()},	{"10",		B10G.Name()},
		{"J",		B24G.Name()},	{"24G",		B24G.Name()},	{"24",		B24G.Name()},
		{"K",		B47G.Name()},	{"47G",		B47G.Name()},	{"47",		B47G.Name()},
		{"L",		B75G.Name()},	{"75G",		B75G.Name()},	{"75",		B75G.Name()},	{"76", B75G.Name()},	{"76G", B75G.Name()},
		{"M",		B119G.Name()},	{"119",		B119G.Name()},	{"119G",	B119G.Name()},
		{"N",		B142G.Name()},	{"142",		B142G.Name()},	{"142G",	B142G.Name()},
		{"O",		B241G.Name()},	{"241",		B241G.Name()},	{"241G",	B241G.Name()},
		{ "P",		BLight.Name()},	{"300",		BLight.Name()},	{"300G",	BLight.Name()},
		{"LHT",		BLight.Name()},	{"LIGHT",	BLight.Name()},	{"LGT",		BLight.Name()},	{"LASER", BLight.Name()},	{"LAS", BLight.Name()},
	};

	QRegularExpressionMatch m = roverRe.match(_callRcvd);
	if(m.hasMatch())
	{
		int start = m.capturedStart(1);
		int len = m.capturedLength();
		_callRcvd.remove(start, len);
		_callRcvd.append("-R");
	}

	static std::set <Cty const *> wTerritories = { KH0_id, KH1_id, KH2_id, KH3_id, KH4_id, KH5_id, KH6_id, KH8_id, KH9_id,KP1_id, KP2_id, KP4_id, KP5_id, KL7_id };
	static std::set <Cty const *> veTerritories = { CY9_id, CY0_id };

	if (wTerritories.count(_ctyRcvdPtr))
	{
		_ctyRcvdPtr = W_id;
		_pxRcvd  = W_id->PxList().first();
	}

	if (veTerritories.count(_ctyRcvdPtr))
	{
		_ctyRcvdPtr = VE_id;
		_pxRcvd  = VE_id->PxList().first();
	}

	_callRcvd = CallFormer().RootCall(_callRcvd);

	if(!MyCty()->IsWVe() && !_ctyRcvdPtr->IsWVe())	 // appears to be two dx stations
	{
		_status = DOES_NOT_COUNT;
		return;
	}

	if(bandByRawBandMap.count(_band))
		_band = bandByRawBandMap[_band];
	else
	{
		_status = DOES_NOT_COUNT;
		AppendErrMsgList("BAND:", _band);
	}

	if(_log->Category() == "S3" && !_band.contains(QRegularExpression("A|B|D")))
		_isExcluded = true;
}

QString VhfQso::ToString() const
{
	return QString("%1 %2 %3 %4 %5 %6 %7 %8")
		.arg(Band()			, 4)
		.arg(Mode()			, 2)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(CallSent()		, -12)
		.arg(_xchSent		, 4)
		.arg(_rawCallRcvd	, -12)
		.arg(_xchRcvd		, 4);
}

QString VhfQso::ToTabString() const
{
	return QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8")
		.arg(Band())
		.arg(Mode())
		.arg(DateStr())
		.arg(TimeStr())
		.arg(CallSent())
		.arg(_xchSent)
		.arg(_rawCallRcvd)
		.arg(_xchRcvd);
}

QString VhfQso::ToFullReverseString() const
{
	return QString("%1 %2 %3 %4 %5 %6 %7 %8")
		.arg(Band()			, 4)
		.arg(Mode()			,2)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(_rawCallRcvd	,-12)
		.arg(_xchRcvd		,8)
		.arg(CallSent()		,-12)
		.arg(_xchSent);
}
