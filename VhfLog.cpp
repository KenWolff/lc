#include "StdAfx.h"

VhfLog::VhfLog() : ArrlLog(), _isRover(false)
{
	_offtimeThreshold = INT_MAX;

	foreach(QString band, BandNameList())
	{
		_bandInfoMap[band];
		_bandInfoMap[band].Band(band);
	}
}

void VhfLog::WriteTsvLine(QTextStream & strm) 
{
	strm << gContest->ContestType() << '\t'
		<< gContest->StartTime().toString("yyyy") << '\t'
		<< _call << '\t'
		<< _hostCall << '\t'
		<< _operators << '\t'
		<< _hqCat << '\t'
		<< _location << '\t'
		<< _px->StdPx() << '\t'
		<< _score << '\t'
		<< NetQso() << '\t'
		<< NetMults() << '\t'
		<< SubmittedLate() << '\t'
		<< _hqClubAbbr << '\t'
		<< OperatingTime() * 60 << '\t';

	QString bandString;
	foreach(QString band, BandNameList())
	{
		if (_bandInfoMap[band].NetQso())
			bandString += band + ' ';

		strm << NetQso(band) << '\t' << NetPts(band) << '\t' << NetMults(band) << '\t';
	}

	strm << bandString << '\t' << _gridsSentMap.keys().join(' ') << '\t';
	strm << endl;
}

QsoList VhfLog::GetQsoListByCall(QString call, QsoPtr qPtr)
{
	QsoList list;
	Log * logPtr = gContest->GetLog(call);

	if (!logPtr)
		return list;

	VhfLog * hisLog = (VhfLog *)logPtr;
	VhfQso * q = (VhfQso *)qPtr;

	if(!hisLog->IsRover())
		list = _qsoListByCallBandMap.value(call).value(q->Band());
	else
		list = GetQsoListByCallBandGridRcvd(call, q->Band(), q->XchRcvd());

	return list;
}

QList < QPair <QsoList, QsoList> > VhfLog::GetQsoLists()
{
	QList < QPair <QsoList, QsoList> > listOfPairsOfQsoLists;
	
	foreach(QString callRcvd, _qsoListByCallBandMap.keys())
	{
		Log * logPtr = gContest->GetLog(callRcvd);
		if (!logPtr)
			continue;

		VhfLog * hisLog = (VhfLog *) logPtr;

		foreach(QString band, _qsoListByCallBandMap.value(callRcvd).keys())
		{
			if (!hisLog->IsRover())
			{
				QsoList myList = GetQsoListByCallBandMap().value(callRcvd).value(band);
				QsoList hisList = hisLog->GetQsoListByCallBandMap().value(_call).value(band);
				QPair <QsoList, QsoList> pairOfQsoLists(myList, hisList);
				listOfPairsOfQsoLists.append(pairOfQsoLists);
			}
			else
			{
				QStringList hisGridSentList = hisLog->GetGridsSentMap().keys();
				foreach(QString hisGridsent, hisGridSentList)
				{
					QsoList myList  = GetQsoListByCallBandGridRcvd(callRcvd, band, hisGridsent);
					QsoList hisList = hisLog->GetQsoListByCallBandGridSent(_call, band, hisGridsent);

					QPair <QsoList, QsoList> pairOfQsoLists(myList, hisList);
					listOfPairsOfQsoLists.append(pairOfQsoLists);
				}
			}
		}
	}

	return listOfPairsOfQsoLists;
}

void VhfLog::ClearMultMaps()
{
	_rawMultByBandGridRcvdMap.clear();
	_multByBandGridRcvdMap.clear();
	_lostMultList.clear();
}

void VhfLog::ComputeMults()
{
	ClearMultMaps();

	foreach(QsoPtr q, _allQso)
	{
		q->IsMult(false);
		q->IsRawMult(false);

		if (!q->CountsForRawCredit())
			continue;

		_rawMultByBandGridRcvdMap[q->Band()][q->XchRcvd()].append(q);
		if (_rawMultByBandGridRcvdMap[q->Band()][(q)->XchRcvd()].size() == 1)	 // first one, add a mult
		{
			_bandInfoMap[q->Band()].IncrRawMults();
			_bandInfoMap["ALL"].IncrRawMults();
			q->IsRawMult(true);
		}

		if (q->Status() == GOOD_QSO || q->Status() == NO_LOG || _doNotCheck)
		{
			_multByBandGridRcvdMap[q->Band()][(q)->XchRcvd()].append(q);
			if (_multByBandGridRcvdMap[q->Band()][(q)->XchRcvd()].size() == 1)	// first one, add a mult
			{
				_bandInfoMap[q->Band()].IncrNetMults();
				_bandInfoMap["ALL"].IncrNetMults();
				q->IsMult(true);
			}
		}
	}

	foreach(QString band, _rawMultByBandGridRcvdMap.keys())
	{
		foreach(QString gridRcvd, _rawMultByBandGridRcvdMap.value(band).keys())
		{
			if (!_multByBandGridRcvdMap.value(band).value(gridRcvd).size())
			{
				QsoPtr q = _rawMultByBandGridRcvdMap.value(band).value(gridRcvd).first();
				q->IsLostMult(true);
				_lostMultList.append(LostMult(q, ""));
			}
		}
	}

	// rovers get an extra mult for each grid visited

	foreach(QString grid, _gridsSentMap.keys())		// grid sent map gets filled at the start of check by contest type
	{
		if (!IsRover())
			break;

		_bandInfoMap["ALL"].IncrRawMults();
		_bandInfoMap["ALL"].IncrNetMults();
	}
}

void VhfLog::WriteLg(QTextStream & strm)
{
	static QString hdrString = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13")
		.arg("Band"			)
		.arg("Mode"			)
		.arg("Date"			)
		.arg("Time"			)
		.arg("Call Sent"	)
		.arg("Grid Sent"	)
		.arg("Call Rcvd"	)
		.arg("Grid Rcvd"	)
		.arg("Pts"			)
		.arg("Status"		)
		.arg("Uni"			)
		.arg("CtyPfx"		)
		.arg("Mult"			);

	strm << hdrString << endl;

	foreach(QsoPtr q, _allQso)
	{
		QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6")
			.arg(q->ToTabString())
			.arg(q->Pts())
			.arg(q->StatusChar())
			.arg(q->IsUnique() ? "U" : "")
			.arg(q->CtyRcvd()->StdPx())
			.arg(q->IsMult() ? q->XchRcvd() : ((q->IsLostMult()) ? QString("Lost %1").arg(q->XchRcvd()) : ""));
		strm << outStr << endl;
	}
}

void VhfLog::WriteTabLine(QTextStream & strm)
{
	std::map <QString, QString> bandToCode = 
	{ 
		{"50" ,		"A"},
		{"144",		"B"},
		{"222",		"C"},
		{"432",		"D"},
		{"902",		"9"},
		{"1.2G",	"E"},
		{"2.3G",	"F"},
		{"3.4G",	"G"},
		{"5.7G",	"H"},
		{"10G",		"I"},
		{"24G",		"J"},
		{"47G",		"K"},
		{"75G",		"L"},
		{"119G",	"M"},
		{"142G",	"N"},
		{"241G",	"O"},
		{"LHT",		"P"}
	};

	QString bandString;

	strm<<  _rawCall								<<   '\t'   //	 "Call"
		<<  _location								<<   '\t'   //	 "Loc"
		<< Division()								<<   '\t'   //	 "Division"
		<< Region()									<<   '\t'   //	 "Region"
		<<  _district								<<   '\t'   //	 "Dist"
		<<  _category								<<	 '\t'	//	 "Cat"
		<<  _hqCat									<<	 '\t'	//	 "HqCat"
		<<  _certificate							<<   '\t'   //	 "Cert"
		<<  _claimedScore							<<   '\t'   //	 "Claimed"
		<<  RawQso()								<<   '\t'   //	 "Raw Qso"
		<<  RawMults()								<<   '\t'   //	 "Raw Mult"
		<<  _rawScore								<<   '\t'   //	 "Raw Scr"
		<<  NetQso()								<<   '\t'   //	 "Net Qso"
		<<  NetMults()								<<   '\t'   //	 "Net Mults"
		<<  GridsActivatedCount()					<<   '\t'   //	 "Grid Activated count
		<<  _score									<<   '\t'   //	 "Final Score"
		<<  Reduction()								<<   '\t'   //	 "Reduct"	   
		<<  DupeList().size()						<<   '\t'   //	 "Dupes"
		<<  BustedCallCount()						<<   '\t'   //	 "B"
		<<  NilList().size()						<<   '\t'   //	 "NIL"
		<<  BustedXchList().size()					<<   '\t'   //	 "X"
		<<  UniList().size()						<<   '\t'   //	 "Uni"
		<<  _bustedCallCausedByTimeMap.size()		<<   '\t'   //	 "C B"
		<<  _nilCausedByTimeMap.size()				<<   '\t'   //	 "C NIL"
		<<  _bustedXchCausedByTimeMap.size()		<<   '\t';  //	 "C X"

	foreach(QString band, BandNameList())
	{
		strm << QString("%1\t%2\t").arg(_bandInfoMap[band].NetQso()).arg(_bandInfoMap[band].NetMults());
		if (_bandInfoMap[band].NetQso())
			bandString += bandToCode[band];
	}

	strm <<  bandString				<<	'\t'	//	bands operated
		<<  _categoryOperator		<<	'\t'	//	CatOp
		<<  _categoryTransmitter	<<	'\t'	//	CatTx
		<<  _categoryTime			<<	'\t'	//	CatTime
		<<  _categoryOverlay		<<	'\t'	//	CatOverlay
		<<  _categoryBand			<<	'\t'	//	CatBand
		<<  _categoryPower			<<	'\t'	//	CatPwr
		<<  _categoryMode			<<	'\t'	//	CatMode
		<<  _categoryAssisted		<<	'\t'	//	CatAssisted
		<<  _categoryStation		<<	'\t'	//	CatStn
		<<  _hqClubAbbr				<<	'\t'	//	Club
		<<  _operators				<<	'\t'	//	Operators
		<<  _name					<<	'\t'	//	Name
		<<  _email					<<	'\t'	//	email
		<<  _hqFrom					<<  '\t'	//	HQ From
		<<  _hqClubAbbr				<<  '\t'	//	HQ Club Abbreviation
		<<  _soapBox				<<	'\t'	//	Soap Box
		<<  _createdBy				<<	'\t'	//	Created By
		<<  OperatingTime()			<<	'\t'	//	Op Hrs
		<<  _addressCity			<<	'\t'	//	city
		<<  _addressStateProvince	<<	'\t'	//	state province
		<<  _addressPostalCode		<<	'\t'	//	postal code 
		<<  _addressCountry			<<	'\t'	//	country
		<<  _addressStreet			<<	endl;
}

void VhfLog::WriteRptSummary(QTextStream & strm)
{
	QString title = ContestPtr()->Title();
	if(title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n%7%8\n")
		.arg("Contest:",	-12)	.arg(title)
		.arg("Call:",		-12)	.arg(Call())
		.arg("Category:",	-12)	.arg(_hqCategory)
		.arg("Location: ",	-12)	.arg(_location);

	QString operators = Operators();
	while(!operators.isEmpty() && Operators() != Call())
	{
		int idx = operators.lastIndexOf(",", 60);
		if(idx != -1)
		{
			strm << QString("%1%2\n").arg("Operators:", -12).arg(operators.left(idx + 1));
			operators.remove(0, idx + 1);
			operators = operators.trimmed();
		}
		else
		{
			strm << QString("%1%2\n").arg("Operators:",  -12).arg(operators);
			break;
		}
	}

	// summary

	if(!AdminText().isEmpty())
		strm << '\n' << AdminText() << '\n';

	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Summary ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("%1 Claimed QSO before checking (does not include duplicates)\n")	.arg(RawQso(), 8);
	strm << QString("%1 Final   QSO after  checking reductions\n\n")					.arg(NetQso(), 8);

	strm << QString("%1 Claimed QSO points\n")											.arg(RawPts(), 8);
	strm << QString("%1 Final   QSO points\n\n")										.arg(NetPts(), 8);

	strm << QString("%1 Claimed grids\n")												.arg(RawMults(), 8);
	strm << QString("%1 Final   grids\n\n")												.arg(NetMults(), 8);

	if(Category().startsWith("R"))
		strm << QString("%1 Grids Activated (rovers only)\n\n").arg(GridsActivatedCount(), 8);

	strm << QString("%1 Claimed score\n")												.arg(RawScore(), 8);
	strm << QString("%1 Final   score\n")												.arg(Score(), 8);
	strm << QString("%1% Score reduction\n\n")											.arg(100.0 * double(Score() - RawScore()) / double(RawScore()) , 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final qso counts\n")			.arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) duplicates\n")											.arg(DupeCount(),		8)	.arg((100.0 * DupeCount())			/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n")								.arg(BustedCallCount(),	8)	.arg((100.0 * BustedCallCount())	/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) exchanges copied incorrectly\n")							.arg(BadXchCount(),		8)	.arg((100.0 * BadXchCount())		/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n")											.arg(NilCount(),		8)	.arg((100.0 * NilCount())			/ LogSize(), 3, 'f', 1);
		strm << QString("%1 (%2%) calls unique to this log only (not removed)\n")		.arg(UniCount(),		8)	.arg((100.0 * UniCount())			/ LogSize(), 3, 'f', 1);

	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("             %1 %2 %3 %4\n\n")
			.arg("Band"		,4)
			.arg("QSO"		,5)
			.arg("QPts"		,5)
			.arg("Grid"		,5);

	foreach(QString band, BandNameList())
	{
		if(!RawQso(band))
			continue;

		strm << QString("   Claimed   %1 %2 %3 %4\n")
			.arg(band			,4)
			.arg(RawQso (band)	,5)
			.arg(RawPts (band)	,5)
			.arg(RawMults(band)	,5);

		strm << QString("   Final     %1 %2 %3 %4\n\n")
			.arg(band			,4)
			.arg(NetQso (band)	,5)
			.arg(NetPts (band)	,5)
			.arg(NetMults(band)	,5);
	}

	if(_isRover)
		strm << QString("   Grids Activated (rovers only) %1\n\n").arg(GridsActivatedCount(), 2);

	strm << QString("   Claimed   %1 %2 %3 %4  Score %5\n")
		.arg("All"				,4)
		.arg(RawQso()			,5)
		.arg(RawPts()			,5)
		.arg(RawMults()			,5)
		.arg(RawScore()			,8);

	strm << QString("   Final     %1 %2 %3 %4  Score %5\n")
		.arg("All"				,4)
		.arg(NetQso()			,5)
		.arg(NetPts()			,5)
		.arg(NetMults()			,5)
		.arg(Score()			,8);
}

QString VhfLog::ResultLine()
{
	QString s = QString("%1 %2 %3 %4 %5 %6")
			.arg(_rawCall						,-12)
			.arg(LogSize()						,5)
			.arg(DupeList().size()				,3)
			.arg(NilList().size()				,3)
			.arg(BustedCallCount()				,3)
			.arg(BustedXchList().size()				,3);

	s += QString("%1 %2 %3 %4 %5 %6% %7 %8 ")
			.arg(_bandInfoMap["ALL"].NetPts()	,6)
			.arg(_bandInfoMap["ALL"].NetMults()	,5)
			.arg(Score()						,8)
			.arg(RawScore()						,8)
			.arg(ClaimedScore()					,8)
			.arg(100.0 * (Score() - RawScore())/ RawScore(), 6, 'f', 1)
			.arg(OperatingTime()				,4, 'f', 1)
			.arg(Category()						,3);
	return s;
}

void VhfLog::ComputeCategory()
{
	VhfContest * contest = static_cast <VhfContest *> (gContest);

	if(contest->HqCategoryMap().contains(_hqCategory))
	{
		_category = contest->HqCategoryMap().value(_hqCategory);

		if(_category == "CK")
		   _isChecklog = true;

		QRegularExpression roverRe("(^R/|^R[0-9][0-9]/|/R[0-9][0-9]$|/R$|/ROVER$|/[A-Z][A-Z][0-9][0-9]$|/[A-Z][A-Z][0-9][0-9][A-Z][A-Z]$|^[A-Z][A-Z][0-9][0-9]/)");  // detect rovers
		if(_category.startsWith('R'))
		{
			_call.remove(QRegularExpression("/R$|/ROVER$|-R$"));
			_call.append("-R");
			_isRover = true;

			int portIdx = _call.indexOf('/', 4);
			if(portIdx != -1)
				_call = _call.left(portIdx);
		}
		else
			_call.remove(roverRe);
	}
	else
		AppendErrMsgList("ROVER:", _hqCategory);
}

void VhfLog::AddToQsoLists(QsoPtr q)
{
	if(q->Status() == FORMAT_ERR)
		return;

	_qsoListByCallBandMap[q->CallRcvd()][q->Band()].append(q);
	_qsoListByCallBandGridSentMap[q->CallRcvd()][q->Band()][q->XchSent()].append(q);
	_qsoListByCallBandGridRcvdMap[q->CallRcvd()][q->Band()][q->XchRcvd()].append(q);
	_qsoListByCallBandGridSentGridRcvdMap[q->CallRcvd()][q->Band()][q->XchSent()][q->XchRcvd()].append(q);
}

QsoPtr VhfLog::Find(QString call, QsoPtr q, int window, bool chkOtherBands)
{
	// this is his log
	// q is from my log
	// Call is normally my call or a test call off by one from me
	// Swap XchSent and XchRcvd in his map

	QsoList qList = _qsoListByCallBandGridSentGridRcvdMap
		.value(call)
		.value(q->Band())
		.value(q->XchRcvd())			// swapped
		.value(q->XchSent());			// swapped

	if (!chkOtherBands)									// check single band
	{
		// FIX THIS seems like a real hack

		if (IsRover() && !_gridsSentMap.contains(q->XchRcvd()))
			q->Status(BUSTED_EXCHANGE);
		return FindClosestQso(q->DateTime(), qList);
	}

	if (_isRover)										// no way to figure out rover cross band
		return 0;

	QStringList bandNames = BandNameList();				// all bands except band of claimed qso
	bandNames.removeAll(q->Band());

	QsoList otherBandQsoList;

	foreach(QString band, bandNames)					// create a list of closest qsos from other bands
	{
		QsoList qList = _qsoListByCallBandGridSentGridRcvdMap
			.value(call)
			.value(band)
			.value(q->XchRcvd())
			.value(q->XchSent());

		QsoPtr rq = FindClosestQso(q->DateTime(), qList, window);

		if (rq)
			otherBandQsoList << rq;
	}

	// FindClosestQso finds the closest qso in time from a list of qsos -- perfect!

	return FindClosestQso(q->DateTime(), otherBandQsoList, window);
}

void VhfLog::ComputePts()
{
	static
	std::map <QString, int> junPtsByBand = 
	{	
		{B6.Name(),		1},	{B2.Name(),		1},	{B222.Name(),	2},	{B432.Name(),	2},	{B903.Name(),	3},	{B1296.Name(),	3},	{B2304.Name(),	4},
		{B3456.Name(),	4},	{B5760.Name(),	4},	{B10G.Name(),	4},	{B24G.Name(),	4},	{B47G.Name(),	4},	{B75G.Name(),	4},	{B119G.Name(),	4},
		{B142G.Name(),	4},	{B241G.Name(),	4},	{BLight.Name(),	4}
	};		
	
	static
	std::map <QString, int> janPtsByBand =
	{
		{B6.Name(),		1},	{B2.Name(),		1},	{B222.Name(),	2},	{ B432.Name(),	2},	{B903.Name(),	4},	{ B1296.Name(),	4},	{ B2304.Name(),	8},
		{B3456.Name(),	8},	{B5760.Name(),	8},	{B10G.Name(),	8},	{ B24G.Name(),	8},	{B47G.Name(),	8},	{ B75G.Name(),	8},	{ B119G.Name(),	8},
		{B142G.Name(),	8},	{B241G.Name(),	8},	{BLight.Name(),	8}
	};

	static
	std::map <QString, int> augPtsByBand =
	{
		{ B6.Name(),	 1},{B2.Name(),		1},	{B222.Name(),	3},	{ B432.Name(),	3},	{ B903.Name(),	6},	{ B1296.Name(),	6},	{ B2304.Name(),	12},
		{ B3456.Name(),	12},{B5760.Name(),	12},{B10G.Name(),	12},{ B24G.Name(),	12},{ B47G.Name(),	12},{ B75G.Name(),	12},{ B119G.Name(),	12},
		{ B142G.Name(),	12},{B241G.Name(),	12},{BLight.Name(),	12}
	};

	foreach(QsoPtr q, _allQso)
	{
		if(q->IsDupe())
			continue;

		QString type = gSrcMap["ContestType"];

		int pts(0);
		if (type == "VHF-JUN" || type == "VHF-SEP")
			pts = junPtsByBand.at(q->Band());

		if (type == "VHF-JAN")
			pts = janPtsByBand.at(q->Band());

		if (type == "UHF-AUG")
			pts = augPtsByBand.at(q->Band());

		if(q->IsExcluded())
			pts = 0;

		_bandInfoMap[q->Band()	].IncrRawQso();
		_bandInfoMap["ALL"		].IncrRawQso();
		_bandInfoMap[q->Band()	].AddRawPts(pts);
		_bandInfoMap["ALL"		].AddRawPts(pts);

		switch (q->Status())
		{
		case NO_LOG:
		case GOOD_QSO:
			_bandInfoMap[q->Band()	].IncrNetQso();
			_bandInfoMap["ALL"		].IncrNetQso();
			_bandInfoMap[q->Band()	].AddNetPts(pts);
			_bandInfoMap["ALL"		].AddNetPts(pts);
			q->Pts(pts);
			break;

		case NOT_IN_LOG:
		case BUSTED_CALL:
		case BUSTED_REVLOG_CALL:
		case BUSTED_LIST_CALL:
			_bandInfoMap[q->Band()	].DecrNetQso();
			_bandInfoMap["ALL"		].DecrNetQso();
			_bandInfoMap[q->Band()	].AddNetPts(-pts);
			_bandInfoMap["ALL"		].AddNetPts(-pts);
			q->Pts(-pts);
			break;

		default:
			q->Pts(0);
			break;
		}
	}
}

void VhfLog::ComputeOperatingTime()
{
	_operatingTime = 0;
	time_t lastTime = 0;
	foreach(ConstQsoPtr q, _allQso)
	{
		int delta = q->TimeInSeconds() - lastTime;

		if(lastTime && q->TimeInSeconds()  < (_offtimeThreshold + lastTime))
			_operatingTime += delta;
		lastTime = q->TimeInSeconds();
	}
}

void VhfLog::WriteRptMults(QTextStream & strm)
{
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << "  Multipliers by Band  ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	foreach(QString band, BandNameList())
	{
		if (!RawQso(band))
			continue;

		strm << band << " multiplier total " << NetMults(band) << "\n\n";
		foreach(QString gridRcvd, _multByBandGridRcvdMap.value(band).keys())
		{
			QsoPtr q = _multByBandGridRcvdMap.value(band).value(gridRcvd).first();
			strm << QString("%1").arg(q->ToString()) << endl;
		}
		strm << endl;
	}

	if(!IsRover())
		return;

	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << "  Rover Bonus Multipliers  ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << " Total Grids Activated " << _gridsSentMap.size() << "\n\n";

	int i = 0;
	foreach(QString grid, _gridsSentMap.keys())
	{
		i++;
		strm << QString("%1 ").arg(grid, -5);
		if((i % 10) == 0)
			strm << '\n';
	}
	strm << endl;
}

void VhfLog::CheckDupes()
{
	QsoListByCallBandGridGrid map = _qsoListByCallBandGridSentGridRcvdMap;

	foreach(QString call, map.keys())								// each call
	{
		foreach(QString band, map.value(call).keys())				// each band
		{
			foreach(QString gridSent, map.value(call).value(band).keys())	// each grid sent
			{
				foreach(QString gridRcvd, map.value(call).value(band).value(gridSent).keys())	// grid rcvd
				{
					QsoList qList = map.value(call).value(band).value(gridSent).value(gridRcvd);
					QsoPtr matchedQso(0);

					foreach(QsoPtr q, qList)											// search for a matched qso
					{
						if (q->Matched() || q->Status() == GOOD_QSO)					// found, then back up marking early dupes
						{
							matchedQso = q;
							foreach(QsoPtr p, qList)									// others either dupe, either early or normal
							{
								if (p == matchedQso)
									continue;

								if (p->DateTime() < matchedQso->DateTime())
									p->IsEarlyDupe(true);

								p->IsDupe(true);
							}
							break;
						}
					}

					if (!matchedQso)
					{
						for (int i = 0; i < qList.size(); i++)		// didn't find an adjudicated qso
						{
							Qso * q = qList[i];
							{
								if (i == 0)
									q->IsDupe(false);
								else
									q->IsDupe(true);
							}
						}
					}
				}
			}
		}
	}
}

QsoList	VhfLog::GetQsoListByCallBandGridSentGridRcvd(QString call, QString band, QString gridSent, QString gridRcvd)
{
	return _qsoListByCallBandGridSentGridRcvdMap
		.value(call)
		.value(band)
		.value(gridSent)
		.value(gridRcvd);
}

QsoList	VhfLog::GetQsoListByCallBandGridSent(QString call, QString band, QString gridSent)
{
	return _qsoListByCallBandGridSentMap
		.value(call)
		.value(band)
		.value(gridSent);
}

QsoList	VhfLog::GetQsoListByCallBandGridRcvd(QString call, QString band, QString gridRcvd)
{
	return _qsoListByCallBandGridRcvdMap
		.value(call)
		.value(band)
		.value(gridRcvd);
}

QMap < QString, QsoPtr> & VhfLog::GetGridsSentMap() { return _gridsSentMap; }
