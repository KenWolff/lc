#include "StdAfx.h"

typedef QPair<QString, int> BandCount;
CqwwRttyLog::CqwwRttyLog() : CqwwLog() {}

void CqwwRttyLog::AssignQthsExpected()
{
	foreach(QsoPtr p, _allQso)
		p->AssignLocationExpected();
}

void CqwwRttyLog::ClearMultMaps()
{
	CqwwLog::ClearMultMaps();
	_stateMultMap.clear();
	_rawStateMultMap.clear();
	_overlayStateMultMap.clear();
	_rawOverlayStateMultMap.clear();
}

void CqwwRttyLog::WriteLg(QTextStream & strm)
{
	static
	QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16\t%17\t%18")
		.arg("Freq"			)
		.arg("Mode"			)
		.arg("Date"			)
		.arg("Time"			)
		.arg("Call Sent"	)
		.arg("Zone Sent"	)
		.arg("State Sent"	)
		.arg("Call Rcvd"	)
		.arg("Zone Rcvd"	)
		.arg("State Rcvd"	)
		.arg("Stn"			)
		.arg("Pts"			)
		.arg("Status"		)
		.arg("Run/S/P"		)
		.arg("Uni"			)
		.arg("Mult C"		)
		.arg("Mult Z"		)
		.arg("Mult S"		);

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
			.arg(multStnStr)
			.arg(q->Pts())
			.arg(q->StatusChar())
			.arg(q->Running())
			.arg(q->IsUnique()  ? "U" : "")
			.arg(q->IsMult()  ? q->StdPx()						: ((q->IsLostMult())	? "L" : ""))
			.arg(q->IsMult2() ? QString::number(q->ZoneRcvd())	: ((q->IsLostMult2())	? "L" : ""))
			.arg(q->IsMult3() ? q->XchRcvd2()					: ((q->IsLostMult3())	? "L" : ""));
		strm << outStr;
	}
}

void CqwwRttyLog::InitByContestType()
{
	CqwwLog::InitByContestType();
	if (IsWVe())
	{
		if(gStateAbbreviationMap.contains(_location))
		{
			_location = gStateAbbreviationMap.value(_location);
		}
		else
		{
			if (IsW())
			{
				QString newLoc = gStateByFccCallMap.value(_call);
				AppendErrMsgList("LOCATIION:", QString("%1 NOT FOUND %2").arg(_location).arg("using state from FCC data:" + newLoc));
				_location = newLoc;
			}
			else
			{
				QString newLoc = _px->Province();
				AppendErrMsgList("LOCATIION:", QString("%1 NOT FOUND %2").arg(_location).arg("using province from cqww.dat:" + newLoc));
				_location = newLoc;
			}
		}
	}
}

void CqwwRttyLog::WriteTabLine(QTextStream & strm)
{
	QString tmpCat = _category;
	if(_disqualified)
		tmpCat = "DQ";

	strm <<  (IsMarMobile() ? _rawCall + "/MM" : _rawCall)			<<	'\t'	//	 "Call"
		<<  _cty->StdPx()											<<	'\t'	//	 "Cty Px"
		<<  _cont													<<	'\t'	//	 "Cont"
		<<  _zone													<<	'\t'	//	 "Zone"
		<<  _location												<<	'\t'	//	 "Loc"
		<<  _district												<<	'\t'	//	 "Dist"
		<<  tmpCat													<<	'\t'	//	 "Cat"
		<<  _certificate											<<	'\t'	//	 "Cert"
		<<  _overlayCertificate										<<	'\t'	//	 "Overlay Cert"
		<<  _mailCert												<<	'\t'	//	 "mail Cert"
		<<  _submittedLate											<<	'\t'	//	 "Late"
		<<  _claimedScore											<<	'\t'	//	 "Claimed"
		<<  _rawScore												<<	'\t'	//	 "Raw Scr"
		<<  _score													<<	'\t'	//	 "Final Score"
		<<	_rawOverlayScore										<< '\t'		//	 "Raw Overlay Score"
		<<	_overlayScore											<< '\t'		//	 "Overlay Score"
		<<  Reduction()												<<	'\t'	//	 "Reduct"
		<<  NetQso()												<<	'\t'	//	 "Net Qso"
		<<  NetMults3()												<<	'\t'	//	 "Net States"
		<<  NetMults2()												<<	'\t'	//	 "Net Zones"
		<<  NetMults()												<<	'\t'	//	 "Net Cty"
		<<  DupeList().size()										<<	'\t'	//	 "Dupes"
		<<  BustedCallCount()										<<	'\t'	//	 "B"
		<<  NilList().size()										<<	'\t'	//	 "NIL"
		<<  BustedXchList().size()									<<	'\t'	//	 "X"
		<<  (_fillRunMult ? "1" : "0")								<<	'\t'	//	 "AUTO_RUN_MULT"
		<<  BandChangeViolationList().size()						<<	'\t'	//	 "BCV"
		<<  UniList().size()										<<  '\t'	//	 "Uni"
		<<  _bustedCallCausedByTimeMap.size()						<<	'\t'	//	 "C B"
		<<  _nilCausedByTimeMap.size()								<<	'\t'	//	 "C NIL"
		<<  _bustedXchCausedByTimeMap.size()						<<	'\t'	//	 "C X"
		<<  NetQso("ALL")											<<	'\t'	//	 "Qso"
		<<  NetMults("ALL") + NetMults2("ALL")	+  + NetMults3("ALL")<<	'\t'	//	 "Mult"
		<<  NetQso("80M")											<<  '\t'	//	 "80M"
		<<  NetMults3("80M")										<<  '\t'	//	 "80M"
		<<  NetMults2("80M")										<<  '\t'	//	 "80M"
		<<  NetMults("80M")											<<  '\t'	//	 "80M"
		<<  NetQso("40M")											<<  '\t'	//	 "40M"
		<<  NetMults3("40M")										<<  '\t'	//	 "40M"
		<<  NetMults2("40M")										<<  '\t'	//	 "40M"
		<<  NetMults("40M")											<<  '\t'	//	 "40M"
		<<  NetQso("20M")											<<  '\t'	//	 "20M"
		<<  NetMults3("20M")										<<  '\t'	//	 "20M"
		<<  NetMults2("20M")										<<  '\t'	//	 "20M"
		<<  NetMults("20M")											<<  '\t'	//	 "20M"
		<<  NetQso("15M")											<<  '\t'	//	 "15M"
		<<  NetMults3("15M")										<<  '\t'	//	 "15M"
		<<  NetMults2("15M")										<<  '\t'	//	 "15M"
		<<  NetMults("15M")											<<  '\t'	//	 "15M"
		<<  NetQso("10M")											<<  '\t'	//	 "10M"
		<<  NetMults3("10M")										<<  '\t'	//	 "10M"
		<<  NetMults2("10M")										<<  '\t'	//	 "10M"
		<<  NetMults("10M")											<<  '\t'	//	 "10M"
		<<  OverlayNetQso("ALL")									<<  '\t'	//	 "Qso Overlay"
		<<  OverlayNetMults3("ALL")									<<  '\t'	//	 "Mult S Overlay"
		<<  OverlayNetMults2("ALL")									<<  '\t'	//	 "Mult Z Overlay"
		<<  OverlayNetMults("ALL")									<<  '\t'	//	 "Mult C Overlay"
		<<  OverlayNetQso("80M")									<<  '\t'	//	 "80M Overlay"
		<<  OverlayNetMults3("80M")									<<  '\t'	//	 "80M Overlay"
		<<  OverlayNetMults2("80M")									<<  '\t'	//	 "80M Overlay"
		<<  OverlayNetMults("80M")									<<  '\t'	//	 "80M Overlay"
		<<  OverlayNetQso("40M")									<<  '\t'	//	 "40M Overlay"
		<<  OverlayNetMults3("40M")									<<  '\t'	//	 "40M Overlay"
		<<  OverlayNetMults2("40M")									<<  '\t'	//	 "40M Overlay"
		<<  OverlayNetMults("40M")									<<  '\t'	//	 "40M Overlay"
		<<  OverlayNetQso("20M")									<<  '\t'	//	 "20M Overlay"
		<<  OverlayNetMults3("20M")									<<  '\t'	//	 "20M Overlay"
		<<  OverlayNetMults2("20M")									<<  '\t'	//	 "20M Overlay"
		<<  OverlayNetMults("20M")									<<  '\t'	//	 "20M Overlay"
		<<  OverlayNetQso("15M")									<<  '\t'	//	 "15M Overlay"
		<<  OverlayNetMults3("15M")									<<  '\t'	//	 "15M Overlay"
		<<  OverlayNetMults2("15M")									<<  '\t'	//	 "15M Overlay"
		<<  OverlayNetMults("15M")									<<  '\t'	//	 "15M Overlay"
		<<  OverlayNetQso("10M")									<<  '\t'	//	 "10M Q Overlay"
		<<  OverlayNetMults3("10M")									<<  '\t'	//	 "10M S Overlay"
		<<  OverlayNetMults2("10M")									<<  '\t'	//	 "10M Z Overlay"
		<<  OverlayNetMults("10M")									<<  '\t'	//	 "10M C Overlay"
		<<  _categoryOperator										<<  '\t'	//	 "CatOp"
		<<  _categoryTransmitter									<<  '\t'	//	 "CatTx"
		<<  _categoryTime											<<  '\t'	//	 "CatTime"
		<<  _categoryOverlay										<<  '\t'	//	 "CatOverlay"
		<<  _categoryBand											<<  '\t'	//	 "CatBand"
		<<  _categoryPower											<<  '\t'	//	 "CatPwr"
		<<  _categoryMode											<<  '\t'	//	 "CatMode"
		<<  _categoryAssisted										<<  '\t'	//	 "CatAssisted"
		<<  _categoryStation										<<  '\t'	//	 "CatStn"
		<<  _club													<<  '\t'	//	 "Club"
		<<  _operators												<<  '\t'	//	 "Operators"
		<<  _name													<<  '\t'	//	 "Name"
		<<  _email													<<  '\t'	//	 "email"
		<<  _hqFrom													<<  '\t'	//	 "HQ From"
		<<  _hqDateRcvd												<<  '\t'	//	 "HQ Date"
		<<  _soapBox												<<  '\t'	//	 "Soap Box"
		<<  _createdBy												<<  '\t'	//	 "Created By"
		<<  OperatingTime()											<<  '\t'	//	 "Op Hrs"
		<<  _addressCity											<<  '\t'	//	 "city"
		<<  _addressStateProvince									<<  '\t'	//	 "state province"
		<<  _addressPostalCode										<<  '\t'	//	 "postal code"
		<<  _addressCountry											<<  '\t'	//   "country"
		<<  _addressStreet											<<  '\n';	//	 "Addr"
}

void CqwwRttyLog::WriteRptSummary(QTextStream & strm)
{
	CqwwRttyContest * contest = static_cast <CqwwRttyContest *> (gContest);
	QString title = ContestPtr()->Title();
	if(title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n")
		.arg("Contest:", -12).arg(title)
		.arg("Call:", -12).arg(Call())
		.arg("Category:", -12).arg(contest->CatDescrFromNum(_categoryNum).Full());

	if (_categoryOverlay.size())
	{
		QString overlayTag = "Overlay:";
		QString overLayData = _categoryOverlay.toLower();
		overLayData.replace(0, 1, overLayData[0].toUpper());
		strm << QString("%1%2\n").arg(overlayTag, -12).arg(overLayData);
	}

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

	strm << QString("%1 Claimed QSO before checking (does not include duplicates)\n")	.arg(RawQso(),	8);
	strm << QString("%1 Final   QSO after  checking reductions\n\n")					.arg(NetQso(),	8);

	strm << QString("%1 Claimed QSO points\n")											.arg(RawPts(),	8);
	strm << QString("%1 Final   QSO points\n\n")										.arg(NetPts(),	8);

	strm << QString("%1 Claimed Countries\n")											.arg(RawMults(), 8);
	strm << QString("%1 Final   Countries\n\n")											.arg(NetMults(), 8);

	strm << QString("%1 Claimed Zones\n")												.arg(RawMults2(), 8);
	strm << QString("%1 Final   Zones\n\n")												.arg(NetMults2(), 8);

	strm << QString("%1 Claimed States\n")												.arg(RawMults3(), 8);
	strm << QString("%1 Final   States\n\n")											.arg(NetMults3(), 8);

	strm << QString("%1 Claimed Mults\n")												.arg(RawMults() + RawMults2() + RawMults3(), 8);
	strm << QString("%1 Final   Mults\n\n")												.arg(NetMults() + NetMults2() + NetMults3(), 8);

	strm << QString("%1 Claimed Score\n")												.arg(RawScore(), 8);
	strm << QString("%1 Final   Score\n")												.arg(Score(),	 8);

	strm << QString("%1% Score reduction\n\n")											.arg(100.0 * double(Score() - RawScore()) / double(RawScore()) , 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final QSO counts\n")			.arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) duplicates\n")											.arg(DupeCount(),			8)	.arg((100.0 * DupeCount())				/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n")								.arg(BustedCallCount(),		8)	.arg((100.0 * BustedCallCount())		/ LogSize(), 3, 'f', 1);
	if (IsMultiSingle())
		strm << QString("%1 (%2%) band change violations\n")							.arg(BandChangeViolationCount(),8)	.arg((100.0 * BandChangeViolationCount())	/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n")											.arg(NilCount(),			8)	.arg((100.0 * NilCount())				/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls unique to this log only (not removed)\n")			.arg(UniCount(),			8)	.arg((100.0 * UniCount())				/ LogSize(), 3, 'f', 1);

	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("            %1 %2 %3 %4 %5 %6 %7\n\n")
			.arg("Band"		,4)
			.arg("QSO"		,5)
			.arg("QPts"		,5)
			.arg("State"	,4)
			.arg("Zone"		,5)
			.arg("Cty"		,5)
			.arg("Total");

	foreach(QString band, BandNameList())
	{
		strm << QString("   Claimed  %1 %2 %3 %4 %5 %6 %7\n")
			.arg(band,				4)
			.arg(RawQso(band),		5)
			.arg(RawPts(band),		5)
			.arg(RawMults3(band),	4)
			.arg(RawMults2(band),	5)
			.arg(RawMults(band),	5)
			.arg(RawMults(band) + RawMults2(band) + RawMults3(band), 4);

		strm << QString("   Final    %1 %2 %3 %4 %5 %6 %7\n\n")
			.arg(band				,4)
			.arg(NetQso (band)		,5)
			.arg(NetPts (band)		,5)
			.arg(NetMults3(band)	,4)
			.arg(NetMults2(band)	,5)
			.arg(NetMults(band)		,5)
			.arg(NetMults(band) + NetMults2(band) + NetMults3(band), 4);
	}

	strm << QString("   Claimed  %1 %2 %3 %4 %5 %6 %7  Score %8\n")
		.arg("All"			,4)
		.arg(RawQso()		,5)
		.arg(RawPts()		,5)
		.arg(RawMults3()	,4)
		.arg(RawMults2()	,5)
		.arg(RawMults()		,3)
		.arg(RawMults() + RawMults2() + RawMults3()  ,4)
		.arg(QLocale().toString(RawScore()), 10);

	strm << QString("   Final    %1 %2 %3 %4 %5 %6 %7  Score %8\n\n")
		.arg("All"			,4)
		.arg(NetQso()		,5)
		.arg(NetPts()		,5)
		.arg(NetMults3()	,4)
		.arg(NetMults2()	,5)
		.arg(NetMults()		,3)
		.arg(NetMults() + NetMults2() + NetMults3(), 4)
		.arg(QLocale().toString(Score())		,10);
}

QString CqwwRttyLog::ResultLine()
{
	int zoneTotal = 0;
	int ctyTotal = 0;
	int stateTotal = 0;

	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 ")
			.arg(_rawCall						,-12)
			.arg(CtyPtr()->StdPx()				,5)
			.arg(CtyPtr()->Cont()				,4)
			.arg(Zone()							,3)
			.arg(LogSize()						,5)
			.arg(DupeList().size()				,3)
			.arg(NilList().size()				,3)
			.arg(BustedCallList().size()		,3)
			.arg(BandChangeViolationCount()		,4);

	foreach(QString band, BandNameList())
	{
		s += QString("%1 %2 %3 %4 %5 ")
			.arg(band							,4)
			.arg(_bandInfoMap[band].NetQso()	,4)
			.arg(_bandInfoMap[band].NetMults3() ,3)
			.arg(_bandInfoMap[band].NetMults2() ,3)
			.arg(_bandInfoMap[band].NetMults()	,3);

			stateTotal	+= _stateMultMap[band].size();
			zoneTotal	+= _zoneMultMap	[band].size();
			ctyTotal	+= _ctyMultMap	[band].size();
	}

	s += QString("%1 %2 %3 %4 %5 %6 %7 %8% %9 %10")
			.arg(_bandInfoMap["ALL"].NetQso()	,5)
			.arg(NetMults3()					,3)
			.arg(NetMults2()					,3)
			.arg(NetMults()						,4)
			.arg(Score()						,8)
			.arg(RawScore()						,8)
			.arg(ClaimedScore()					,8)
			.arg(100.0 * (Score() - RawScore())/ RawScore(), 6, 'f', 1)
			.arg(OperatingTime()				,4, 'f', 1)
			.arg(Category()						,3);
	return s;
}

void CqwwRttyLog::ComputePts()
{
	static int nilPenalty = 2;
	static int bustPenalty = 2;

	foreach(QsoPtr q, _allQso)
	{
		int pts = 0;

		if(q->IsDupe() || q->IsExcluded())
			continue;

		bool crossBand = CategoryBand() != "ALL" && CategoryBand() != q->Band();

		if(_cty == q->CtyRcvd())
			pts = 1;
		else
			if(_px->Cont() == q->CtyRcvd()->Cont())
				pts = 2;
			else
				pts = 3;

		if (!crossBand)
		{
			_bandInfoMap[q->Band()]	.IncrRawQso();
			_bandInfoMap["ALL"]		.IncrRawQso();
			_bandInfoMap[q->Band()]	.AddRawPts(pts);
			_bandInfoMap["ALL"]		.AddRawPts(pts);
		}

		if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
		{
			_overlayBandInfoMap	[q->Band()]	.IncrRawQso();
			_overlayBandInfoMap	["ALL"]		.IncrRawQso();
			_overlayBandInfoMap	[q->Band()]	.AddRawPts(pts);
			_overlayBandInfoMap	["ALL"]		.AddRawPts(pts);
		}

		switch (q->Status())
		{
		case NO_LOG:
		case GOOD_QSO:
			if (!crossBand)
			{
				_bandInfoMap[q->Band()	].IncrNetQso();
				_bandInfoMap["ALL"		].IncrNetQso();
				_bandInfoMap[q->Band()	].AddNetPts(pts);
				_bandInfoMap["ALL"		].AddNetPts(pts);
				q->Pts(pts);
			}

			if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
			{
				_overlayBandInfoMap[q->Band()	].IncrNetQso();
				_overlayBandInfoMap["ALL"		].IncrNetQso();
				_overlayBandInfoMap[q->Band()	].AddNetPts(pts);
				_overlayBandInfoMap["ALL"		].AddNetPts(pts);
			}
			break;

		case NOT_IN_LOG:
			if (!crossBand)
			{
				_bandInfoMap[q->Band()].AddNetPts(-nilPenalty*pts);
				_bandInfoMap["ALL"].AddNetPts(-nilPenalty*pts);
				q->Pts(-nilPenalty*pts);
			}

			if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
			{
				_overlayBandInfoMap[q->Band()	].AddNetPts(-nilPenalty*pts);
				_overlayBandInfoMap["ALL"		].AddNetPts(-nilPenalty*pts);
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
				_overlayBandInfoMap[q->Band()	].AddNetPts(-bustPenalty*pts);
				_overlayBandInfoMap["ALL"		].AddNetPts(-bustPenalty*pts);
			}
			break;

		default:
			q->Pts(0);
			break;
		}
	}
}

void CqwwRttyLog::ComputeMults()
{
	CqwwLog::ComputeMults();	// CQWW has the same country and zone mults

	foreach(QsoPtr ptr, _allQso)
	{
		CqwwRttyQso * q = static_cast<CqwwRttyQso *> (ptr);

		q->IsMult3(false);							//state
		q->IsRawMult3(false);

		if (!q->CountsForRawCredit())
			continue;

		if (CategoryBand() != "ALL" && CategoryBand() != q->Band())
			continue;

		// only W/VE are state/prov mults
		if (!q->IsWVeRcvd())
			continue;

		if (q->IsMM())
			continue;

		QString hisCall  = q->CallRcvd();
		QString hisState = q->LocationExpected();
		QString hisBand  = q->Band();

		_rawStateMultMap[hisBand][hisState].append(q);
		if (_rawStateMultMap[hisBand][hisState].size() == 1)
		{
			_bandInfoMap[hisBand].IncrRawMults3();
			_bandInfoMap["ALL"].IncrRawMults3();
			q->IsRawMult3(true);		
		}

		if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
		{
			_rawOverlayStateMultMap[hisBand][hisState].append(q);
			if (_rawOverlayStateMultMap[hisBand][hisState].size() == 1)
			{
				_overlayBandInfoMap[hisBand].IncrRawMults3();
				_overlayBandInfoMap["ALL"].IncrRawMults3();
			}
		}
		
		if (!q->CountsForCredit())
			continue;

		_stateMultMap[hisBand][hisState].append(q);
		if (_stateMultMap[hisBand][hisState].size() == 1)
		{
			_bandInfoMap[hisBand].IncrNetMults3();
			_bandInfoMap["ALL"].IncrNetMults3();
			q->IsMult3(true);
		}

		if (IsOverlayRookie() || (IsOverlayClassic() && !q->IsOutOfTime()))
		{
			_overlayStateMultMap[hisBand][hisState].append(q);
			if (_overlayStateMultMap[hisBand][hisState].size() == 1)
			{
				_overlayBandInfoMap[hisBand].IncrNetMults3();
				_overlayBandInfoMap["ALL"].IncrNetMults3();
			}
		}
	}

	foreach(QString band, _rawStateMultMap.keys())
	{
		foreach(QString state, _rawStateMultMap.value(band).keys())
		{
			if (!_stateMultMap.value(band).contains(state))
			{
				QsoPtr q = _rawStateMultMap.value(band).value(state).first();
				q->IsLostMult(true);
				_lostMultList.append(LostMult(q, "-S"));
			}
		}
	}
}

void CqwwRttyLog::WriteRptMults(QTextStream & strm)
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
		int totalMults = _ctyMultMap.value(bandName).size() + _zoneMultMap.value(bandName).size() + _stateMultMap.value(bandName).size();
		if (!totalMults)
			continue;

		strm << bandName << " multiplier total " << totalMults << "\n\n";

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

		i = 0;

		foreach(QString state, _stateMultMap[bandName].keys())
		{
			i++;
			strm << QString("%1 ").arg(state, -5);
			if((i % 10) == 0)
				strm << '\n';
		}
		strm << "\n\n";

		if(gListMultiplierQsos)
		{
			foreach(QString cty, _ctyMultMap[bandName].keys())
			{
				QsoPtr q = *_ctyMultMap[bandName][cty].begin();
				strm << QString("%1 %2\n").arg(q->ToString()).arg(cty, -5);
			}
			strm << "\n\n";
		}
	}
}

void CqwwRttyLog::CheckBandChanges()
{
	if(IsMultiTwo())
		CheckM2BandChanges(8);

	if (IsMultiSingle())
		CheckMSBandChanges(8);
}

void CqwwRttyLog::CheckMSBandChanges(int maxQsy)
{
	int count = 0;
	QString band;
	int hour = INT_MAX;
	QList <QsoPtr> runStnList;
	QList <QsoPtr> multStnList;

	foreach(QsoPtr q, _allQso)
	{
		if(!q->IsLegalCall() || !q->IsValidCty() || q->IsExcluded())
			continue;

		if(q->IsMultStn())
			multStnList.append(q);
		else
			runStnList.append(q);
	}

	foreach(QsoPtr q, runStnList)
	{
		int deltaSecs = ContestPtr()->StartTime().secsTo(q->DateTime());
		int currentHour = ((abs(deltaSecs % 60) >= 30) ? deltaSecs += deltaSecs % 60 : deltaSecs) /3600;

		if(currentHour != hour)
		{
			hour  = currentHour;
			count = 0;
			band  = q->Band();
			continue;
		}

		if(band != q->Band())
		{
			if(count == maxQsy)
			{
				if (q->CountsForCredit())
					q->Status(BAND_CHANGE_VIOLATION);
			}
			else
			{
				count++;
				band = q->Band();
				q->IsBandChange(true);
			}
		}
	}

	foreach(QsoPtr q, multStnList)
	{
		int deltaSecs = ContestPtr()->StartTime().secsTo(q->DateTime());
		int currentHour = ((abs(deltaSecs % 60) >= 30) ? deltaSecs += deltaSecs % 60 : deltaSecs) /3600;

		if(currentHour != hour)
		{
			hour  = currentHour;
			count = 0;
			band  = q->Band();
			continue;
		}

		if (q->CountsForCredit() && !(q->IsMult() || q->IsMult2() || q->IsMult3()))		// mult station works ony mults when the qso is good
			q->Status(BAND_CHANGE_VIOLATION);

		if (band != q->Band())
		{
			if (count == maxQsy)
			{
				q->Status(BAND_CHANGE_VIOLATION);
			}
			else
			{
				count++;
				band = q->Band();
				q->IsBandChange(true);
			}
		}
	}
}


void CqwwRttyLog::AssignMSBandChanges()
{
	if (_dontFillRunMult || (ContestPtr()->FillRunMultMap().value(_call) == "NO"))
		return;

	if (ContestPtr()->FillRunMultMap().value(_call) == "YES" || gRunAutoFill)
		_fillRunMult = true;

	QString runBand;
	QString multBand;
	QString lastBand;
	QString newRunBand;

	QList < QList < QsoPtr> * > logSegmentList;
	QList < QsoPtr> * logSegment(0);
//	QList < QsoPtr> * lastLogSegment(0);

	foreach(QsoPtr q, _allQso)
	{
		if (q->Band() != lastBand)
		{
			if (logSegment)
				logSegmentList.append(logSegment);
			logSegment = new QList < QsoPtr >;
			lastBand = q->Band();
		}
		logSegment->append(q);
	}

	gStdOutStrm << QString("%1").arg(logSegmentList.size()) << '\n';

	foreach(QList < QsoPtr> * segment, logSegmentList)
	{
		bool multFlag;
		if (segment->size() > 2)
			multFlag = false;
		else
			multFlag = true;

		foreach(QsoPtr q, *segment)
		{
			q->IsMultStn(multFlag);
			gStdOutStrm << QString("%1 %2").arg(q->ToString()). arg(multFlag ? "Mult stn" : "Run Stn")  << '\n';
		}
		gStdOutStrm << endl;
	}
}

void CqwwRttyLog::WriteMultFile(QTextStream & strm)
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
		if (!_ctyMultMap.contains(bandName) && !_zoneMultMap.contains(bandName))
			continue;

		strm << bandName << " multiplier total " << _ctyMultMap[bandName].size() + _zoneMultMap[bandName].size() + _stateMultMap[bandName].size() << "\n\n";

		foreach(QString cty, _ctyMultMap[bandName].keys())
		{
			QsoPtr q = _ctyMultMap[bandName][cty].first();
			if (!q)
				continue;
			strm << QString("%1 %2").arg(cty, -5).arg(q->ToString()) << '\n';
		}
		strm << "\n";

		for (int zone = 1; zone <= 40; zone++)
		{
			if (_zoneMultMap[bandName].contains(zone))
			{
				QsoPtr q = _zoneMultMap[bandName][zone].first();
				if (!q)
					continue;
				strm << QString("%1 %2").arg(zone, -5).arg(q->ToString()) << '\n';
			}
		}
		strm << "\n";

		foreach(QString state, _stateMultMap[bandName].keys())
		{
			QsoPtr q = _stateMultMap[bandName][state].first();
			if (!q)
				continue;
			strm << QString("%1 %2").arg(state, -5).arg(q->ToString()) << '\n';
		}
		strm << "\n\n";
	}
}

