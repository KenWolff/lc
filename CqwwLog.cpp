#include "StdAfx.h"

typedef QPair<QString, int> BandCount;

CqwwLog::CqwwLog() : Log() {}

void CqwwLog::InitByContestType()
{
	if (!_cqZone.isEmpty())
		_zone = _cqZone.toInt();
	else
		_zone = gDeriveZone(_rawCall, _px, this);
}

void CqwwLog::AssignZones()
{
	foreach(QsoPtr p, _allQso)
	{
		CqwwQso * q = static_cast<CqwwQso *> (p);
		q->AssignZoneExpected();
	}
}

void CqwwLog::AssignMostCommonZoneRcvd()  // rcvd by others
{
	if(!gContest->RevLogMap().count(Call()))
		return;

	RevLog & revLog = gContest->RevLogMap().at(Call());

	QMap <int, int> zoneCountByZone;
	foreach(QsoPtr p, revLog.AllQso())
	{
		CqwwQso * q = static_cast<CqwwQso *> (p);
		zoneCountByZone[q->ZoneRcvd()]++;
	}

	int maxRcvdZoneCount = 0;
	foreach(int z, zoneCountByZone.keys())
	{
		if (zoneCountByZone[z] > maxRcvdZoneCount)
		{
			maxRcvdZoneCount = zoneCountByZone[z];
			_mostRcvdZone = z;
		}
	}
}

void CqwwLog::ClearMultMaps()
{
	_ctyMultMap				.clear();
	_rawCtyMultMap			.clear();
	_zoneMultMap			.clear();
	_rawZoneMultMap			.clear();
	_lostMultList			.clear();
	_overlayCtyMultMap		.clear();
	_overlayZoneMultMap		.clear();
	_overlayRawCtyMultMap	.clear();
	_overlayRawZoneMultMap	.clear();
}

void CqwwLog::WriteLg(QTextStream & strm)
{
	static
	QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16")
		.arg("Freq"		)
		.arg("Mode"		)
		.arg("Date"		)
		.arg("Time"		)
		.arg("Call Sent")
		.arg("Z Sent"	)
		.arg("Call Rcvd")
		.arg("Z Rcvd"	)
		.arg("Cty Px"	)
		.arg("Stn"		)
		.arg("Pts"		)
		.arg("Status"	)
		.arg("Run/S/P"	)
		.arg("Uni"		)
		.arg("Mult C"	)
		.arg("Mult Z"	);

	strm << hdr << endl;

	foreach(QsoPtr p, _allQso)
	{
		CqwwQso * q = static_cast<CqwwQso *> (p);
		QString multStnStr;
		if (IsMultiSingle())
			multStnStr = q->IsMultStn() ? "Mult" : "Run";

		if (IsMultiTwo())
			multStnStr = q->IsMultStn() ? "TX 2" : "TX 1";

		if (IsMultiMulti())
			multStnStr = q->Band();

		QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\n")
			.arg(q->ToTabString())
			.arg(q->CtyRcvd()->StdPx())
			.arg(multStnStr)
			.arg(q->Pts())
			.arg(q->StatusChar())
			.arg(q->Running())
			.arg(q->IsUnique()	? "U"							: "")
			.arg(q->IsMult()	? q->StdPx()					: ((q->IsLostMult())  ? "L" : ""))
			.arg(q->IsMult2()	? QString::number(q->ZoneRcvd()): ((q->IsLostMult2()) ? "L" : ""));
		strm << outStr;	
	}
}

void CqwwLog::ComputeScore()
{
	ComputePts();
	ComputeMults();
	SortLostMultList();

	_rawScore = _bandInfoMap["ALL"].RawPts() *
				(	_bandInfoMap["ALL"].RawMults() +
					_bandInfoMap["ALL"].RawMults2()	+ 
					_bandInfoMap["ALL"].RawMults3()
				);

	_score	= _bandInfoMap["ALL"].NetPts() * 
				(	_bandInfoMap["ALL"].NetMults()	+ 
					_bandInfoMap["ALL"].NetMults2()	+ 
					_bandInfoMap["ALL"].NetMults3()
				);

	if (!IsOverlayRookie() && !IsOverlayClassic())
		return;

	_rawOverlayScore = _overlayBandInfoMap["ALL"].RawPts() *
				(
					_overlayBandInfoMap["ALL"].RawMults()  +
					_overlayBandInfoMap["ALL"].RawMults2() +
					_overlayBandInfoMap["ALL"].RawMults3()
				);

	_overlayScore =
				_overlayBandInfoMap["ALL"].NetPts() *
				(
					_overlayBandInfoMap["ALL"].NetMults()  +
					_overlayBandInfoMap["ALL"].NetMults2() +
					_overlayBandInfoMap["ALL"].NetMults3()
				);
}

void CqwwLog::WriteTabLine(QTextStream & strm)
{
	QString category = _category;

	if (_isAdministrativeChecklog)
		category = "CK";

	if (_disqualified)
		category = "DQ";

	bool lateFlag = _submittedLate || _isNoAward;
	double uniPercent = UniList().size() ? double(UniList().size()) / double(NetQso()) : 0;
	QString uniPercentStr = QString("%1").arg(100 * uniPercent, 4, 'f', 1, '0');

strm 	<<  _rawCall							<<  '\t'    //	 "Call"
		<<  _cty->StdPx()						<<	'\t'	//	 "Cty Px"
		<<  _cont								<<	'\t'	//	 "Cont"
		<<  _zone								<<	'\t'	//	 "Zone"
		<<  _location							<<	'\t'	//	 "Loc"
		<<  _district							<<	'\t'	//	 "Dist"
		<<	category							<<	'\t'	//	 "Cat"
		<<  _certificate						<<	'\t'	//	 "Cert"
		<<  _overlayCertificate					<<	'\t'	//	 "Overlay Cert"
		<<  _mailCert							<<	'\t'	//	 "Overlay Cert"
		<<	lateFlag							<<	'\t'	//	 "Late" 
		<<  _claimedScore						<<	'\t'	//	 "Claimed"
		<<  _rawScore							<<	'\t'	//	 "Raw Scr"
		<<  _score								<<	'\t'	//	 "Final Score"
		<<  _rawOverlayScore					<<	'\t'	//	 "Raw Overlay Score"
		<<  _overlayScore						<<	'\t'	//	 "Overlay Score"
		<<  Reduction()							<<	'\t'	//	 "Reduct"
		<<	RawQso()							<< '\t'		//	 "Raw Qso"
		<<	NetQso()							<< '\t'		//	 "Net Qso"
		<<	RawMults2()							<< '\t'		//	 "Raw Zones"
		<<	NetMults2()							<< '\t'		//	 "Net Zones"
		<<	RawMults()							<< '\t'		//	 "Raw Cty"
		<<	NetMults()							<< '\t'		//	 "Net Cty"
		<<  DupeList().size()					<<	'\t'	//	 "Dupes"
		<<  BustedCallCount()					<<	'\t'	//	 "B"
		<<  NilList().size()					<<	'\t'	//	 "NIL"
		<<  BustedXchList().size()				<<	'\t'	//	 "X"
		<<  (_fillRunMult ? "1" : "0")			<<	'\t'	//	 "AUTO_RUN_MULT"
		<<  BandChangeViolationList().size()	<<	'\t'	//	 "BCV"
		<<  UniList().size()					<<	'\t'	//	 "Uni"
		<<  uniPercentStr						<<	'\t'	//	 "% Uni"
		<<  _bustedCallCausedByTimeMap.size()	<<	'\t'	//	 "C B"
		<<  _nilCausedByTimeMap.size()			<<	'\t'	//	 "C NIL"
		<<  _bustedXchCausedByTimeMap.size()	<<	'\t'	//	 "C X"
		<<  NetQso("ALL")						<<	'\t'	//	 "Qso"
		<<  NetMults("ALL") + NetMults2("ALL")	<<	'\t'	//	 "Mult"
		<<  NetQso("160M")						<<	'\t'	//	 "160M"
		<<  NetMults2("160M")					<<	'\t'	//	 "160M"
		<<  NetMults("160M")					<<	'\t'	//	 "160M"
		<<  NetQso("80M")						<<	'\t'	//	 "80M"
		<<  NetMults2("80M")					<<	'\t'	//	 "80M"
		<<  NetMults("80M")						<<	'\t'	//	 "80M"
		<<  NetQso("40M")						<<	'\t'	//	 "40M"
		<<  NetMults2("40M")					<<	'\t'	//	 "40M"
		<<  NetMults("40M")						<<	'\t'	//	 "40M"
		<<  NetQso("20M")						<<	'\t'	//	 "20M"
		<<  NetMults2("20M")					<<	'\t'	//	 "20M"
		<<  NetMults("20M")						<<	'\t'	//	 "20M"
		<<  NetQso("15M")						<<	'\t'	//	 "15M"
		<<  NetMults2("15M")					<<	'\t'	//	 "15M"
		<<  NetMults("15M")						<<	'\t'	//	 "15M"
		<<  NetQso("10M")						<<	'\t'	//	 "10M"
		<<  NetMults2("10M")					<<	'\t'	//	 "10M"
		<<  NetMults("10M")						<<	'\t'	//	 "10M"
		<<  OverlayNetQso("ALL")				<<	'\t'	//	 "Qso Overlay"
		<<  OverlayNetMults2("ALL")				<<	'\t'	//	 "Mult Overlay"
		<<  OverlayNetMults("ALL")				<<	'\t'	//	 "Mult Overlay"
		<<  OverlayNetQso("160M")				<<	'\t'	//	 "160M Overlay"
		<<  OverlayNetMults2("160M")			<<	'\t'	//	 "160M Overlay"
		<<  OverlayNetMults("160M")				<<	'\t'	//	 "160M Overlay"
		<<  OverlayNetQso("80M")				<<	'\t'	//	 "80M Overlay"
		<<  OverlayNetMults2("80M")				<<	'\t'	//	 "80M Overlay"
		<<  OverlayNetMults("80M")				<<	'\t'	//	 "80M Overlay"
		<<  OverlayNetQso("40M")				<<	'\t'	//	 "40M Overlay"
		<<  OverlayNetMults2("40M")				<<	'\t'	//	 "40M Overlay"
		<<  OverlayNetMults("40M")				<<	'\t'	//	 "40M Overlay"
		<<  OverlayNetQso("20M")				<<	'\t'	//	 "20M Overlay"
		<<  OverlayNetMults2("20M")				<<	'\t'	//	 "20M Overlay"
		<<  OverlayNetMults("20M")				<<	'\t'	//	 "20M Overlay"
		<<  OverlayNetQso("15M")				<<	'\t'	//	 "15M Overlay"
		<<  OverlayNetMults2("15M")				<<	'\t'	//	 "15M Overlay"
		<<  OverlayNetMults("15M")				<<	'\t'	//	 "15M Overlay"
		<<  OverlayNetQso("10M")				<<	'\t'	//	 "10M Overlay"
		<<  OverlayNetMults2("10M")				<<	'\t'	//	 "10M Overlay"
		<<  OverlayNetMults("10M")				<<	'\t'	//	 "10M Overlay"
		<<  OperatingTime()						<<	'\t'	//	 "Op Hrs"
		<<	LastQsoTime().toString()			<<	'\t'	//	 "Final QTR"
		<<  _categoryOperator					<<	'\t'	//	 "CatOp"
		<<  _categoryTransmitter				<<	'\t'	//	 "CatTx"
		<<  _categoryOverlay					<<	'\t'	//	 "CatOverlay"
		<<  _categoryBand						<<	'\t'	//	 "CatBand"
		<<  _categoryPower						<<	'\t'	//	 "CatPwr"
		<<  _categoryMode						<<	'\t'	//	 "CatMode"
		<<  _categoryAssisted					<<	'\t'	//	 "CatAssisted"
		<<  _categoryStation					<<	'\t'	//	 "CatStn"
		<<  _club								<<	'\t'	//	 "Club"
		<<  _operators							<<	'\t'	//	 "Operators"
		<<  _name								<<	'\t'	//	 "Name"
		<<  _email								<<	'\t'	//	 "email"
		<<  _hqFrom								<<	'\t'	//	 "HQ From"
		<<  _hqDateRcvd							<<	'\t'	//	 "HQ Date"
		<<  _soapBox							<<	'\t'	//	 "Soap Box"
		<<  _createdBy							<<	'\t'	//	 "Created By"
		<<  _addressCity						<<	'\t'	//	 "City"
		<<  _addressStateProvince				<<	'\t'	//	 "State-Province"
		<<  _addressPostalCode					<<	'\t'	//	 "Postal Code"
		<<  _addressCountry						<<	'\t'	//   "Country"
		<<  _addressStreet						<<	'\n';	//	 "Addr"
}

void CqwwLog::WriteRptSummary(QTextStream & strm)
{
	CqwwContest * contest = static_cast <CqwwContest *> (gContest);
	QString title = ContestPtr()->Title();
	if(title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n")
		.arg("Contest:",	-12)	.arg(title)
		.arg("Call:",		-12)	.arg(Call())
		.arg("Category:",	-12)	.arg(contest->CatDescrFromNum(_categoryNum).Full());

	QString overlayTag = "Overlay:";
	QString overLayData = _categoryOverlay.toLower();
	overLayData.replace(0, 1, overLayData[0].toUpper());
	if(_categoryOverlay.size())
		strm << QString("%1%2\n").arg(overlayTag, -12).arg(overLayData);

	QString operators = Operators();
	while(!operators.isEmpty() && Operators() != Call())
	{
		int idx = operators.lastIndexOf(",", 60);
		if(idx != -1)
		{
			strm << QString("%1%2\n").arg("Operator(s):", -12).arg(operators.left(idx + 1));
			operators.remove(0, idx + 1);
			operators = operators.trimmed();
		}
		else
		{
			strm << QString("%1%2\n").arg("Operator(s):",  -12).arg(operators);
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

	strm << QString("%1 Claimed QSO before checking (does not include duplicates)\n")	.arg(RawQso(),8);
	strm << QString("%1 Final   QSO after  checking reductions\n\n")			.arg(NetQso(),8);

	strm << QString("%1 Claimed QSO points\n")					.arg(RawPts(),8);
	strm << QString("%1 Final   QSO points\n\n")				.arg(NetPts(),8);

	strm << QString("%1 Claimed countries\n")					.arg(RawMults(),8);
	strm << QString("%1 Final   countries\n\n")					.arg(NetMults(),8);

	strm << QString("%1 Claimed zones\n")						.arg(RawMults2(),8);
	strm << QString("%1 Final   zones\n\n")						.arg(NetMults2(), 8);

	strm << QString("%1 Claimed mults\n")						.arg(RawMults() + RawMults2(), 8);
	strm << QString("%1 Final   mults\n\n")						.arg(NetMults() + NetMults2(), 8);

	strm << QString("%1 Claimed score\n")						.arg(QLocale().toString(RawScore()));
	strm << QString("%1 Final   score\n\n")						.arg(QLocale().toString(Score()));
	if(Reduction() < 0.15)
		strm << QString("%1% Score reduction\n")				.arg(100.0 * Reduction(), 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final qso counts\n").arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n")		.arg(BustedCallCount(),8).arg((100.0 * BustedCallCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) exchanges copied incorrectly\n")	.arg(BadXchCount(),8).arg((100.0 * BadXchCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n").arg(NilCount(), 8).arg((100.0 * NilCount()) / LogSize(), 3, 'f', 1);

	if (_rubberClockedByBandMap.size())
		strm << QString("%1 (%2%) removed for rubber clocking\n").arg(_rubberClockedByBandMap.size(), 8).arg((100.0 * _rubberClockedByBandMap.size()) / LogSize(), 3, 'f', 1);

	if(_outOfBandByTimeMap.size())
	   strm << QString("%1 (%2%) out of band qsos\n").arg(_outOfBandByTimeMap.size(), 8).arg((100.0 * _outOfBandByTimeMap.size()) / LogSize(), 3, 'f', 1);
	if (IsMultiSingle() && BandChangeViolationCount())
		strm << QString("%1 (%2%) band change violations\n")	.arg(BandChangeViolationCount(),8).arg((100.0 * BandChangeViolationCount())	/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) duplicates (Removed without penalty)\n").arg(DupeCount(),8).arg((100.0 * DupeCount())	/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls unique to this log receiving credit (not removed)\n").arg(UniCount(),8) .arg((100.0 * UniCount())				/ LogSize(), 3, 'f', 1);
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("%1 %2 %3 %4 %5 %6 %7\n\n")
			.arg(""			,12)
			.arg("Band"		,4)
			.arg("QSO"		,5)
			.arg("QPts"		,5)
			.arg("Zone"		,5)
			.arg("Cty"		,3)
			.arg("Mult"		,4);

	foreach(QString band, BandNameList())
	{
		strm << QString("   Claimed   %1 %2 %3 %4 %5 %6\n")
			.arg(band				,4)
			.arg(RawQso		(band)	,5)
			.arg(RawPts		(band)	,5)
			.arg(RawMults2	(band)	,5)
			.arg(RawMults	(band)	,3)
			.arg(RawMults	(band)	+ RawMults2(band) ,4);

		strm << QString("   Final     %1 %2 %3 %4 %5 %6\n\n")
			.arg(band				,4)
			.arg(NetQso		(band)	,5)
			.arg(NetPts		(band)	,5)
			.arg(NetMults2	(band)	,5)
			.arg(NetMults	(band)	,3)
			.arg(NetMults	(band) + NetMults2(band) ,4);
	}

	strm << QString("   Claimed   %1 %2 %3 %4 %5 %6  Score %7\n")
		.arg("All"					,4)
		.arg(RawQso()				,5)
		.arg(RawPts()				,5)
		.arg(RawMults2()			,5)
		.arg(RawMults()		    	,3)
		.arg(RawMults2() + RawMults()  	,4)
		.arg(RawScore()		    	,8);

	strm << QString("   Final     %1 %2 %3 %4 %5 %6  Score %7\n")
		.arg("All"					,4)
		.arg(NetQso()				,5)
		.arg(NetPts()				,5)
		.arg(NetMults2()			,5)
		.arg(NetMults()		    	,3)
		.arg(NetMults2() + NetMults()	,4)
		.arg(Score()				,8);
}

QString CqwwLog::ResultLine()
{
	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ")
			.arg(_rawCall				,-12)
			.arg(CtyPtr()->StdPx()		,5)
			.arg(CtyPtr()->Cont()		,4)
			.arg(_zone		,3)
			.arg(LogSize()				,5)
			.arg(DupeList().size()		,3)
			.arg(NilList().size()		,3)
			.arg(BustedCallList().size(),3)
			.arg(BandChangeViolationCount()	,4)
			.arg(UniList().size()		,3);

	s += QString("%1 %2 %3 %4 %5 %6 %7% %8 %9")
			.arg(_bandInfoMap["ALL"].NetQso(),5)
			.arg(_bandInfoMap["ALL"].NetMults2(), 3)
			.arg(_bandInfoMap["ALL"].NetMults(), 3)
			.arg(ClaimedScore()				,8)
			.arg(RawScore()					,8)
			.arg(Score()					,8)
			.arg(100.0 * (Score() - RawScore())/ RawScore(), 6, 'f', 1)
			.arg(OperatingTime()			,4, 'f', 1)
			.arg(Category()					,3);
	return s;
}

void CqwwLog::ComputeCategory()
{
	QChar assistedLetter	= (_categoryAssisted == "NON-ASSISTED") ? 'U' : 'A';

	if(!_categoryBand.contains(QRegularExpression("160M|80M|40M|20M|15M|10M|ALL")))
	{
		AppendErrMsgList("CATEGORY-BAND:", _categoryBand);
		_categoryBand = "ALL";
	}

	if(_categoryOverlay == "CLASSIC" && _categoryAssisted.contains(QRegularExpression("^ASSISTED")))
	{
		AppendErrMsgList("Claimed both Classic and Assisted, removing Classic", "");
		_categoryOverlay.clear();
	}

	QRegularExpression re("CHECKLOG|SINGLE-OP|MULTI-OP");
	if(!_categoryOperator.contains(re))
	{
		AppendErrMsgList("CATEGORY-OPERATOR:",  _categoryOperator + " Setting to CHECKLOG");
		_categoryOperator = "CHECKLOG";
	}

	if(_categoryOperator == "CHECKLOG")
	{
		_category = "CK";
		_categoryNum = 0;
		_categoryOverlay.clear();
		_isChecklog = true;
	}

	if(_categoryOperator == "SINGLE-OP")
	{
		if (_categoryBand != "ALL" && !ContestPtr()->BandNameList().contains(_categoryBand))
		{
			AppendErrMsgList("CATEGORY-BAND:", _categoryBand + " setting to ALL");
			_categoryBand = "ALL";
		}

		if(!_categoryAssisted.contains(QRegularExpression("^ASSISTED|^NON-ASSISTED")))
		{
			_categoryAssisted = "NON-ASSISTED";
		}

		QString pwr;

		if(_categoryPower == "HIGH")
			pwr = "H";
		
		if(_categoryPower == "LOW")
			pwr = "L";
		
		if(_categoryPower == "QRP")
			pwr = "Q";
		
		if(pwr.isEmpty())
		{
			AppendErrMsgList("CATEGORY-POWER:", _categoryPower + "Missing or incorrect, setting to HIGH");
			_categoryPower = "HIGH";
			pwr = "H";
		}
			
		_category = QString("%1_%2_%3")
			.arg(_categoryBand)
			.arg(pwr)
			.arg(assistedLetter);
	}
	else
		_categoryOverlay.clear();

	if(_categoryOperator == "MULTI-OP")
	{
		_categoryBand = "ALL";

		if(_categoryTransmitter == "ONE")
		{
	        _isMultiSingle = true;
			_category = (_categoryPower == "HIGH") ? "MSH" : "MSL";			// multi-single now supports low and high pwr
		}
		else
		{
			_categoryPower = "HIGH";						// M2 and MM are both high power only
			if (_categoryTransmitter == "TWO")
		    {
			    _isMultiTwo = true;
				_category = "M2";
			}
			else
			{
				if(_categoryTransmitter == "UNLIMITED")
				{
					_category = "MM";
				}
				else
				{
					AppendErrMsgList("CATEGORY-TRANSMITTER:", _categoryTransmitter + " Missing or incorrect multi-op, setting to MM");
					_category = "MM";
				}
				_isMultiMulti = true;
			}
		}
	}

	_offtimeThreshold = 3600;		// one hour, in seconds
	_maxOperatingTime = INT_MAX;

	if(!_categoryOverlay.isEmpty())
	{
		if(_categoryOverlay.contains(QRegularExpression("ROOKIE|CLASSIC")))
		{
			_categoryOverlayRookie = (_categoryOverlay == "ROOKIE");
			_categoryOverlayClassic = (_categoryOverlay == "CLASSIC");
		}
		else
		{
			AppendErrMsgList("CATEGORY-OVERLAY:", _categoryOverlay + " setting to no overlay");
			_categoryOverlay.clear();
		}
	}

	if(_categoryOverlayClassic)
		_maxOperatingTime = 24 * 3600;

	_categoryNum = static_cast <CqContest *> (gContest)->CatNumFromInternalDesc(_category);
}

void CqwwLog::ComputePts()
{
	ComputeOperatingTime();   // includes IsOutOfTime limit for overlay

	int const nilPenalty(3);
	int const bustPenalty(3);
	int const rubberClockPenalty(10);

	foreach(QsoPtr q, AllQso())
	{
		int pts = 0;				// initialize point value of QSO to 0

		if(q->IsDupe() || q->IsExcluded())
			continue;

		if (q->IsOutOfBand())
			q->Status(OUT_OF_BAND);

		bool crossBand = CategoryBand() != "ALL" && CategoryBand() != q->Band();

		if(_cty == q->CtyRcvd())
			pts = 0;
		else
			if(_px->Cont() != q->CtyRcvd()->Cont())
				pts = 3;
			else
				pts = (_px->Cont() == "NA") ? 2 : 1;

		if (!crossBand)
		{
			_bandInfoMap[q->Band()].IncrRawQso();
			_bandInfoMap["ALL"].IncrRawQso();
			_bandInfoMap[q->Band()].AddRawPts(pts);
			_bandInfoMap["ALL"].AddRawPts(pts);

			if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
			{
				_overlayBandInfoMap[q->Band()].IncrRawQso();
				_overlayBandInfoMap["ALL"].IncrRawQso();
				_overlayBandInfoMap[q->Band()].AddRawPts(pts);
				_overlayBandInfoMap["ALL"].AddRawPts(pts);
			}
		}

		if (q->IsRubberClocked())
		{
			q->Pts(-rubberClockPenalty*pts);
			_bandInfoMap[q->Band()].AddNetPts(-rubberClockPenalty*pts);
			_bandInfoMap["ALL"].AddNetPts(-rubberClockPenalty*pts);
		}
		else
		{
			switch (q->Status())
			{
			case NO_LOG:
			case GOOD_QSO:
				if (!crossBand)
				{
					q->Pts(pts);
					_bandInfoMap[q->Band()].IncrNetQso();
					_bandInfoMap["ALL"].IncrNetQso();
					_bandInfoMap[q->Band()].AddNetPts(pts);
					_bandInfoMap["ALL"].AddNetPts(pts);
				}

				if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
				{
					_overlayBandInfoMap[q->Band()].IncrNetQso();
					_overlayBandInfoMap["ALL"].IncrNetQso();
					_overlayBandInfoMap[q->Band()].AddNetPts(pts);
					_overlayBandInfoMap["ALL"].AddNetPts(pts);
				}
				break;

			case NOT_IN_LOG:
				if (!crossBand)
				{
					q->Pts(-nilPenalty*pts);
					_bandInfoMap[q->Band()].AddNetPts(-nilPenalty*pts);
					_bandInfoMap["ALL"].AddNetPts(-nilPenalty*pts);
				}

				if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
				{
					_overlayBandInfoMap[q->Band()].AddNetPts(-nilPenalty*pts);
					_overlayBandInfoMap["ALL"].AddNetPts(-nilPenalty*pts);
				}
				break;

			case BUSTED_CALL:
			case BUSTED_FCC_CALL:
			case BUSTED_REVLOG_CALL:
			case BUSTED_LIST_CALL:
				if (!crossBand)
				{
					_bandInfoMap[q->Band()].AddNetPts(-bustPenalty*pts);
					_bandInfoMap["ALL"].AddNetPts(-bustPenalty*pts);
					q->Pts(-bustPenalty*pts);
				}

				if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
				{
					_overlayBandInfoMap[q->Band()].AddNetPts(-bustPenalty*pts);
					_overlayBandInfoMap["ALL"].AddNetPts(-bustPenalty*pts);
				}
				break;

			default:
				q->Pts(0);
				break;
			}
		}
	}
}

void CqwwLog::ComputeMults()
{
	int const rubberClockPenalty(10);
	ComputeOperatingTime();   // includes IsOutOfTime limit for overlay
	ClearMultMaps();

	// run through the log in order, only counting good and unchecked q's
	// compute log exchange sent after all the qsos are loaded

	foreach(QsoPtr p, _allQso)
	{
		CqwwQso * q = static_cast<CqwwQso *> (p);

		q->IsMult(false);	   			 			//cty
		q->IsRawMult(false);
		q->IsMult2(false);							//zone
		q->IsRawMult2(false);
		
		if (!q->CountsForRawCredit())
			continue;

		if (CategoryBand() != "ALL" && CategoryBand() != q->Band())		// single band logs on wrong band
			continue;

		QString hisCall = q->CallRcvd();
		QString hisCty = q->CtyRcvd()->StdPx();
		QString	hisBand = q->Band();
		int		hisZone = q->ZoneExpected();

		if (q->CountsForCredit() && !q->IsMM())  												//don't give country mult for maritime mobile
		{
			_rawCtyMultMap[hisBand][hisCty].append(q);
			if (_rawCtyMultMap[hisBand][hisCty].size() == 1)			// first one, incr mult count 
			{
					_bandInfoMap[hisBand].IncrRawMults();
					_bandInfoMap["ALL"].IncrRawMults();
					q->IsRawMult(true);
			}

			if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
			{
				_overlayRawCtyMultMap[hisBand][hisCty].append(q);
				if (_overlayRawCtyMultMap[hisBand][hisCty].size() == 1)	// first one, incr mult count 
				{
					_overlayBandInfoMap[hisBand].IncrRawMults();
					_overlayBandInfoMap["ALL"].IncrRawMults();
				}
			}
		}

		if (hisZone >= 1 && hisZone <= 40)
		{
			_rawZoneMultMap[hisBand][hisZone].append(q);
			if (_rawZoneMultMap[hisBand][hisZone].size() == 1)
			{
					_bandInfoMap[hisBand].IncrRawMults2();
					_bandInfoMap["ALL"].IncrRawMults2();
					q->IsRawMult(true);
			}

			if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
			{
				_overlayRawZoneMultMap[hisBand][hisZone].append(q);
				if (_overlayRawZoneMultMap[hisBand][hisZone].size() == 1)
				{
					_overlayBandInfoMap[hisBand].IncrRawMults2();
					_overlayBandInfoMap["ALL"].IncrRawMults2();
				}
			}
		}

		if (!q->CountsForCredit())
			continue;

		if (!q->IsMM())  				//don't give country mult for maritime mobile
		{
			_ctyMultMap[hisBand][hisCty].append(q);

			if (_ctyMultMap[hisBand][hisCty].size() == 1)    // first one, incr mult
			{
				if (q->IsRubberClocked())
				{
					_bandInfoMap[hisBand].AddNetMults(-rubberClockPenalty);
					_bandInfoMap["ALL"].AddNetMults(-rubberClockPenalty);
					_ctyMultMap[hisBand][hisCty].removeLast();
					_ctyMultMap[hisBand].remove(hisCty);
				}
				else
				{
					_bandInfoMap[hisBand].IncrNetMults();
					_bandInfoMap["ALL"].IncrNetMults();
					q->IsMult(true);
				}
			}

			if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
			{
				_overlayCtyMultMap[hisBand][hisCty].append(q);
				if (_overlayCtyMultMap[hisBand][hisCty].size() == 1)
				{
					_overlayBandInfoMap[hisBand].IncrNetMults();
					_overlayBandInfoMap["ALL"].IncrNetMults();
				}
			}
		}

		if (hisZone >= 1 && hisZone <= 40)
		{
			_zoneMultMap[hisBand][hisZone].append(q);
			if (_zoneMultMap[hisBand][hisZone].size() == 1)
			{
				if (q->IsRubberClocked())
				{
					_bandInfoMap[hisBand].AddNetMults2(-rubberClockPenalty);
					_bandInfoMap["ALL"].AddNetMults2(-rubberClockPenalty);
					_zoneMultMap[hisBand][hisZone].removeLast();
					_zoneMultMap[hisBand].remove(hisZone);
				}
				else
				{
					_bandInfoMap[hisBand].IncrNetMults2();
					_bandInfoMap["ALL"].IncrNetMults2();
					q->IsMult2(true);
				}
			}

			if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
			{
				_overlayZoneMultMap[hisBand][hisZone].append(q);
				if (_overlayZoneMultMap[hisBand][hisZone].size() == 1)
				{
					_overlayBandInfoMap[hisBand].IncrNetMults2();
					_overlayBandInfoMap["ALL"].IncrNetMults2();
				}
			}
		}
	}

	foreach(QString band, _rawCtyMultMap.keys())
	{
		foreach(QString ctyPx, _rawCtyMultMap.value(band).keys())			// make list of lost country mults
		{
			if (!_ctyMultMap.value(band).contains(ctyPx))
			{
				QsoPtr q = _rawCtyMultMap.value(band).value(ctyPx).first();
				q->IsLostMult(true);
				_lostMultList.append(LostMult(q, "-C"));
			}
		}

		foreach(int zone, _rawZoneMultMap.value(band).keys())				// make list of lost zone mults
		{
			if (!_zoneMultMap.value(band).contains(zone))
			{
				QsoPtr q = _rawZoneMultMap.value(band).value(zone).first();
				q->IsLostMult2(true);
				_lostMultList.append(LostMult(q, "-Z"));
			}
		}
	}
	_multMap = _ctyMultMap;
}

void CqwwLog::WriteRptMults(QTextStream & strm)
{
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << "  Multipliers by Band  ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	foreach(QString bandName, BandNameList())
	{
		int multTotal = _ctyMultMap.value(bandName).size() + _zoneMultMap.value(bandName).size();
		if (multTotal == 0)
			continue;

		strm << bandName << " multiplier total " << multTotal << "\n\n";

		int i = 0;

		foreach(QString cty, _ctyMultMap[bandName].keys())
		{
			i++;
			strm << QString("%1 ").arg(cty, -5);
			if((i % 10) == 0)
				strm << '\n';
		}
		strm << "\n\n";

		int count = 0;
		for(int zone = 1; zone <= 40; zone++)
		{
			if(_zoneMultMap[bandName].contains(zone))
			{
				strm << QString("%1").arg((_zoneMultMap[bandName].contains(zone) ? QString::number(zone) : ""), 3);
				if((++count % 20) == 0)
					strm << '\n';
			}
		}
		strm << "\n\n";
		if(gListMultiplierQsos)
		{
			foreach(QString cty, _ctyMultMap[bandName].keys())
			{
				Qso const * q = *_ctyMultMap[bandName][cty].begin();
				strm << QString("%1 %2\n").arg(q->ToString()).arg(cty, -5);
			}
			strm << "\n\n";
		}
	}
}

void CqwwLog::WriteMultFile(QTextStream & strm)
{
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << "  Multipliers by Band  ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	foreach(QString bandName, BandNameList())
	{
		if(!_ctyMultMap.contains(bandName) && !_zoneMultMap.contains(bandName))
			continue;

		strm << bandName << " multiplier total " << _ctyMultMap[bandName].size() + _zoneMultMap[bandName].size() << "\n\n";

		foreach(QString cty, _ctyMultMap[bandName].keys())
		{
			QsoPtr q = _ctyMultMap[bandName][cty].first();
			strm << QString("%1 %2").arg(cty, -5).arg(q->ToString()) << '\n';
		}
		strm << "\n";

		for(int zone = 1; zone <= 40; zone++)
		{
			if(_zoneMultMap[bandName].contains(zone))
			{
				QsoPtr q = _zoneMultMap[bandName][zone].first();
	            strm << QString("%1 %2").arg(zone, -5).arg(q->ToString()) << '\n';
			}
		}
		strm << "\n\n";
	}
}

void CqwwLog::CheckBandChanges()
{
	if(IsMultiTwo())
		CheckM2BandChanges(8);

	if(IsMultiSingle())
		CheckMSBandChanges();
}

void CqwwLog::CheckMSBandChanges()			// CQWW style 10 minute rule style
{
	if(_doNotCheck)
		return;

	AssignMSBandChanges();

	QString currentBand;
	QDateTime qsyTime = ContestPtr()->StartTime();

	// check against actual data in the log

	QList <QsoPtr> runList;
	QList <QsoPtr> multList;
	QList <QsoPtr> tmpList;

	foreach(QsoPtr q, _allQso)					// build run/mult lists to check independantly
	{
		if(q->IsExcluded())
			continue;

	   if(!q->IsMultStn())
			runList.append(q);
		else
			multList.append(q);
	}

	if(multList.size() > runList.size())		// invert the flags they gave us
	{
		tmpList = runList;
		runList = multList;
		multList = tmpList;
	    foreach(QsoPtr q, _allQso)				// build run/mult lists to check independantly
		if(!q->IsMultStn())
		{
			q->IsMultStn(true);
			multList.append(q);
	   }
		else
		{
			q->IsMultStn(false);
			runList.append(q);
	   }
	}

	if(runList.size())
	{
		currentBand = runList.first()->Band();
		qsyTime = runList.first()->DateTime();
	}

	// current band only changes when a good qso occurs
	// this prevents fake qsos earlier to start the band change times

	foreach(QsoPtr q, runList)
	{
		if(q->Band() != currentBand)
		{
			if(qsyTime.secsTo(q->DateTime()) < 600)
			{
				if(q->Status() == NO_LOG || q->Status() == GOOD_QSO )
					q->Status(BAND_CHANGE_VIOLATION);
			}
			else
			{
				currentBand = q->Band();
				qsyTime = q->DateTime();
			}
		}
	}

	if(multList.size())
	{
		currentBand = multList.first()->Band();
		qsyTime = multList.first()->DateTime();
	}

	foreach(QsoPtr q, multList)
	{
		bool qsoCounts = q->Status() == NO_LOG || q->Status() == GOOD_QSO;

		if (qsoCounts && !q->IsMult() && !q->IsMult2())						// not a mult, don't start the band change timer
		{
			q->Status(BAND_CHANGE_VIOLATION);
			continue;
		}

		if(q->Band() != currentBand)
		{
			if (qsyTime.secsTo(q->DateTime()) < 600)
			{
				if (qsoCounts)
					q->Status(BAND_CHANGE_VIOLATION);
			}
			else
			{
				currentBand = q->Band();
				qsyTime = q->DateTime();
			}
		}
	}
}

void CqwwLog::AssignMSBandChanges()
{
	if(_dontFillRunMult)
		return;

	if(ContestPtr()->FillRunMultMap().value(_call) == "NO")
		return;

	if(ContestPtr()->FillRunMultMap().value(_call) == "YES" || gRunAutoFill)
		_fillRunMult = true;

	int runCount = 0;
	int multCount = 0;

	foreach(QsoPtr q, _allQso)
	{
		if(q->IsExcluded())
			continue;

		if(q->IsMultStn())
			multCount++;
		else
			runCount++;
	}

	if(multCount > runCount)										   // run/mult marking inverted
		foreach(QsoPtr q, _allQso)
			q->IsMultStn(!q->IsMultStn());

	if(!_fillRunMult && multCount)		// no explicit order to fill, mult count is non-zero
		return;

	QString band_0;
	QString band_1;
	QString newRunBand;
	QDateTime runTime(ContestPtr()->StartTime());
	QDateTime multTime(ContestPtr()->StartTime());

	for (size_t i = 0; i < _allQso.size(); i++)	// move forward, popping off the front
	{
		QsoPtr q = _allQso[i];

		if(q->IsExcluded())
			continue;

		if(q->Band() == band_0)					// easy, don't do anything
		{
			q->IsMultStn(false);
			continue;
		}

		if(q->Band() == band_1)
		{
			q->IsMultStn(true);
			if(!(q->IsMult() || q->IsMult2()) )		// decision time, try to find run band
			{
				QsoPtr qs = q;
				QsoPtr ql = 0;										// use this to find two non-mults
				for (size_t j = i + 1; j < _allQso.size(); j++)		// move forward, popping off the front
				{

					qs = _allQso[j];
					if(!(qs->IsMult() || qs->IsMult2()) )
					{
						if(ql && qs->Band() == ql->Band())
							break;
						ql = qs;
					}
				}

				if(qs->Band() == q->Band())							// mult band has become run band
				{
					band_0 = q->Band();
					runTime = q->DateTime();
					q->IsMultStn(false);
					band_1.clear();
					continue;
				}
			}
			continue;
		}

		QsoPtr qs = q;
		QsoPtr ql = 0;								// use this to find two non-mults
		for (size_t j = i; j < _allQso.size(); j++)	// find first two non-mult on the same band, not including this one
		{
			qs = _allQso[j];
			if(!(qs->IsMult() || qs->IsMult2()) )
			{
				if(ql && qs->Band() == ql->Band())
					break;
				ql = qs;
			}
		}

		if(qs->Band() == band_0)			// found old run band
		{
			band_1 = q->Band();
			multTime = q->DateTime();		// mult band changes
			q->IsMultStn(true);
			continue;
		}

		if(qs->Band() == band_1)			// found old mult band, mult->run and new mult band
		{
			band_1 = q->Band();
			multTime = q->DateTime();
			band_0 = qs->Band();
			runTime = qs->DateTime();
			q->IsMultStn(true);
			continue;
		}

		if(band_0.isEmpty())				// found new, third band, run band is the new band,
			band_0 = qs->Band();			// leave old mult band alone

		if(q->Band() == qs->Band())
		{
			band_0 = qs->Band();
			runTime = qs->DateTime();
			q->IsMultStn(false);
		}
		else
		{
			band_1 = q->Band();
			multTime = q->DateTime();
			q->IsMultStn(true);				// must be a fourth band, new band is mult, leave run band alone
		}
	}
}

void CqwwLog::WriteSprLine(QTextStream & sprStrm)
{
	sprStrm
		<< _call << '\t'
		<< _cty->StdPx() << '\t'
		<< _px->Cont() << '\t'
		<< Zone() << '\t'
		<< _district << '\t'
		<< _category << '\t'
		<< Score() << '\t'
		<< ((WorldFinish() != -1) ? QString::number(WorldFinish()) : "") << '\t'
		<< ((EuFinish() != -1) ? QString::number(EuFinish()) : "") << '\t'
		<< ((UsaFinish() != -1) ? QString::number(UsaFinish()) : "") << '\t'
		<< _exactQrgByBand.value("160M") << '\t'
		<< _exactQrgByBand.value("80M") << '\t'
		<< _exactQrgByBand.value("40M") << '\t'
		<< _exactQrgByBand.value("20M") << '\t'
		<< _exactQrgByBand.value("15M") << '\t'
		<< _exactQrgByBand.value("10M") << '\t'
		<< _qsoCount << '\t'
		<< _runCount << '\t'
		<< _passCount << '\t'
		<< _unknownCount << '\t'
		<< _snpCount << '\t'
		<< _snpHitCount << '\t'
		<< QString("%1").arg(double(_snpCount) / double(_qsoCount), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpCount) / double(_multCount), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpHitCount) / double(_qsoCount), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpHitCount) / double(_snpCount), 0, 'f', 3) << '\t'
		<< _multCount << '\t'
		<< _snpMultCount << '\t'
		<< _snpMultHitCount << '\t'
		<< QString("%1").arg(double(_snpMultCount) / double(_multCount), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCount) / double(_multCount), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCount) / double(_snpCount), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCount) / double(_snpMultCount), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCount) / double(_snpHitCount), 0, 'f', 3) << '\t'
		<< _qsoCountDay1 << '\t'
		<< _runCountDay1 << '\t'
		<< _passCountDay1 << '\t'
		<< _unknownCountDay1 << '\t'
		<< _snpCountDay1 << '\t'
		<< _snpHitCountDay1 << '\t'
		<< QString("%1").arg(double(_snpCountDay1) / double(_qsoCountDay1), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpCountDay1) / double(_multCountDay1), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpHitCountDay1) / double(_qsoCountDay1), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpHitCountDay1) / double(_snpCountDay1), 0, 'f', 3) << '\t'
		<< _multCountDay1 << '\t'
		<< _snpMultCountDay1 << '\t'
		<< _snpMultHitCountDay1 << '\t'
		<< QString("%1").arg(double(_snpMultCountDay1) / double(_multCountDay1), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCountDay1) / double(_multCountDay1), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCountDay1) / double(_snpCountDay1), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCountDay1) / double(_snpMultCountDay1), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCountDay1) / double(_snpHitCountDay1), 0, 'f', 3) << '\t'
		<< _qsoCountDay2 << '\t'
		<< _runCountDay2 << '\t'
		<< _passCountDay2 << '\t'
		<< _unknownCountDay2 << '\t'
		<< _snpCountDay2 << '\t'
		<< _snpHitCountDay2 << '\t'
		<< QString("%1").arg(double(_snpCountDay2) / double(_qsoCountDay2), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpCountDay2) / double(_multCountDay2), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpHitCountDay2) / double(_qsoCountDay2), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpHitCountDay2) / double(_snpCountDay2), 0, 'f', 3) << '\t'
		<< _multCountDay2 << '\t'
		<< _snpMultCountDay2 << '\t'
		<< _snpMultHitCountDay2 << '\t'
		<< QString("%1").arg(double(_snpMultCountDay2) / double(_multCountDay2), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCountDay2) / double(_multCountDay2), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCountDay2) / double(_snpCountDay2), 0, 'f', 3) << '\t'
		<< QString("%1").arg(double(_snpMultHitCountDay2) / double(_snpMultCountDay2), 0, 'f', 3) << '\t'
		<< QString("%1\t\n").arg(double(_snpMultHitCountDay2) / double(_snpHitCountDay2), 0, 'f', 3);
}

void CqwwLog::WriteMultUniqueToMe(QTextStream & strm)
{
	foreach(auto bandMap, _ctyMultMap)
		foreach(auto qsoList, bandMap)
			if (_contest->LogSetByCallRcvd()[qsoList.first()->CallRcvd()].size() < 5 && qsoList.first()->StdPx() == _cty->StdPx())
				strm << QString("%1 claimed %2 for %3").arg(_call, -10).arg(qsoList.first()->CallRcvd(), -10).arg(qsoList.first()->StdPx()) << endl;

	foreach(auto bandMap, _zoneMultMap)
	{
		foreach(auto zone, bandMap.keys())
		{
			CqwwQso * q = static_cast<CqwwQso *> (bandMap[zone].first());

			if (_contest->LogSetByCallRcvd()[q->CallRcvd()].size() < 5 && q->ZoneRcvd() == _zone)
				strm << QString("%1 claimed %2 for %3").arg(_call, -10).arg(q->CallRcvd(), -10).arg(_zone) << endl;
		}
	}
}

int	CqwwLog::Zone() const { return _zone; }
int	CqwwLog::MostRcvdZone() const {	return _mostRcvdZone; }
