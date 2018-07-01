#include "StdAfx.h"

ArrlRttyLog::ArrlRttyLog() : ArrlLog() {}
void ArrlRttyLog::WriteTsvLine(QTextStream & ) {}

void ArrlRttyLog::ClearMultMaps()
{
	_rawArrlRttyMultMap	.clear();
	_arrlRttyMultMap	.clear();
	_lostMultList		.clear();
}

void ArrlRttyLog::WriteLg(QTextStream & strm)
{
	static
		QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15")
		.arg("Freq")
		.arg("Mode")
		.arg("Date")
		.arg("Time")
		.arg("Call Sent")
		.arg("State Sent")
		.arg("Call Rcvd")
		.arg("Xch Rcvd")
		.arg("Cty Px")
		.arg("Stn")
		.arg("Pts")
		.arg("Status")
		.arg("Run/S/P")
		.arg("Uni")
		.arg("Mult");

	strm << hdr << endl;

	foreach(QsoPtr q, _allQso)
	{
		QString multStnStr;
		if (IsMultiSingle())
			multStnStr = q->IsMultStn() ? "Mult" : "Run";

		if (IsMultiTwo())
			multStnStr = q->IsMultStn() ? "TX 2" : "TX 1";

		if (IsMultiMulti())
			multStnStr = q->Band();

		QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\n")
			.arg(q->ToTabString())
			.arg(q->CtyRcvd()->StdPx())
			.arg(multStnStr)
			.arg(q->Pts())
			.arg(q->StatusChar())
			.arg(q->Running())
			.arg(q->IsUnique() ? "U" : "")
			.arg(q->IsMult() ? (q->IsWVeRcvd() ? q->XchRcvd() : q->CtyRcvd()->StdPx()) : q->IsLostMult() ? "Lost" : "");
		strm << outStr;
	}
}

void ArrlRttyLog::WriteTabLine(QTextStream & strm)
{
	strm << _rawCall							<<   '\t'   //	 "Call"
		<<  _cty->StdPx()						<<   '\t'   //	 "Cty Px"
		<<  _cont								<<   '\t'   //	 "Cont"
		<<  _location							<<   '\t'   //	 "Loc"
		<<  Division()							<<   '\t'   //	 "Division"
		<<	Region()							<<   '\t'   //	 "Region"
		<<  _district							<<   '\t'   //	 "Dist"
		<<  _hqCat								<<   '\t'   //	 "Cat"
		<<  _certificate						<<   '\t'   //	 "Cert"
		<<  _claimedScore						<<   '\t'   //	 "Claimed"
		<<  RawQso()							<<   '\t'   //	 "Raw Qso"
		<<  RawMults()							<<   '\t'   //	 "Raw Mult"
		<<  _rawScore							<<   '\t'   //	 "Raw Scr"
		<<  NetQso()							<<   '\t'   //	 "Net Qso"
		<<  NetMults()							<<   '\t'   //	 "Net Mults"
		<<  _score								<<   '\t'   //	 "Final Score"
		<<  Reduction()							<<   '\t'   //	 "Reduct"
		<<  LogSize()							<<   '\t'   //	 "Raw Q"
		<<  DupeList().size()					<<   '\t'   //	 "Dupes"
		<<  BustedCallCount()					<<   '\t'   //	 "B"
		<<  NilList().size()					<<   '\t'   //	 "NIL"
		<<  BustedXchList().size()				<<   '\t'   //	 "X"
		<<  BandChangeViolationList().size()	<<   '\t'   //	 "BCV"
		<<  UniList().size()					<<   '\t'   //	 "Uni"
		<<  _bustedCallCausedByTimeMap.size()	<<   '\t'   //	 "C B"
		<<  _nilCausedByTimeMap.size()			<<   '\t'   //	 "C NIL"
		<<  _bustedXchCausedByTimeMap.size()	<<   '\t'   //	 "C X"
		<<  NetQso("ALL")						<<   '\t'   //	 "Qso"
		<<  NetMults("ALL")						<<   '\t'   //	 "Mult"
		<<  NetQso("160M")						<<   '\t'   //	 "160M"
		<<  NetMults("160M")					<<   '\t'   //	 "160M"
		<<  NetQso("80M")						<<   '\t'   //	 "80M"
		<<  NetMults("80M")						<<   '\t'   //	 "80M"
		<<  NetQso("40M")						<<   '\t'   //	 "40M"
		<<  NetMults("40M")						<<   '\t'   //	 "40M"
		<<  NetQso("20M")						<<   '\t'   //	 "20M"
		<<  NetMults("20M")						<<   '\t'   //	 "20M"
		<<  NetQso("15M")						<<   '\t'   //	 "15M"
		<<  NetMults("15M")						<<   '\t'   //	 "15M"
		<<  NetQso("10M")						<<   '\t'   //	 "10M"
		<<  NetMults("10M")						<<   '\t'   //	 "10M"
		<<  _categoryOperator					<<   '\t'   //	 "CatOp"
		<<  _categoryTransmitter				<<   '\t'   //	 "CatTx"
		<<  _categoryTime						<<   '\t'   //	 "CatTime"
		<<  _categoryOverlay					<<   '\t'   //	 "CatOverlay"
		<<  _categoryBand						<<   '\t'   //	 "CatBand"
		<<  _categoryPower						<<   '\t'   //	 "CatPwr"
		<<  _categoryMode						<<   '\t'   //	 "CatMode"
		<<  _categoryAssisted					<<   '\t'   //	 "CatAssisted"
		<<  _categoryStation					<<   '\t'   //	 "CatStn"
		<<  _hqClubAbbr							<<   '\t'   //	 "Club" (from HQ-CLUB-ABBR)
		<<  _operators							<<   '\t'   //	 "Operators"
		<<  _name								<<   '\t'   //	 "Name"
		<<  _email								<<   '\t'   //	 "email"
		<<  _hqFrom								<<   '\t'   //	 "HQ From
		<<  _soapBox							<<   '\t'   //	 "Soap Box"
		<<  _createdBy							<<   '\t'   //	 "Created By"
		<<  OperatingTime()						<<   '\t'   //	 "Op Hrs"
		<<  _addressCity						<<   '\t'   //	 "city"
		<<  _addressStateProvince				<<   '\t'   //	 "state province"
		<<  _addressPostalCode					<<   '\t'   //	 postal code
		<<  _addressCountry	                    <<   '\t'   //     country
		<<  _addressStreet						<<   endl;
}

QString ArrlRttyLog::ResultLine()
{
	QString s = QString("%1 %2 %3 %4 %5 %6")
			.arg(_rawCall				,-12)
			.arg(LogSize()				,5)
			.arg(DupeList().size()		,3)
			.arg(NilList().size()		,3)
			.arg(BustedCallCount()		,3)
			.arg(BustedXchList().size()	,3);

	s += QString("%1 %2 %3 %4 %5 %6% %7 %8 %9")
			.arg(_bandInfoMap["ALL"].NetQso()	,5)
			.arg(_arrlRttyMultMap.size()		,4)
			.arg(Score()						,8)
			.arg(RawScore()						,8)
			.arg(ClaimedScore()					,8)
			.arg(100.0 * (Score() - RawScore())/RawScore(), 6, 'f', 1)
			.arg(OperatingTime()				,4, 'f', 1)
			.arg(Category()						,2)
			.arg(Call()							,-12);
	return s;
}

void ArrlRttyLog::ComputeCategory()
{
	_category = _hqCategory;		// older iaru logs don't come with _hqCat

	if (_category.startsWith('M'))
		_isMultiSingle = true;
}

void ArrlRttyLog::ComputePts()
{
	foreach(QsoPtr q, _allQso)
	{
		if (q->IsOutOfTime())
			continue;

		if(q->IsDupe())
			continue;

		if(q->Status() == DOES_NOT_COUNT)
			continue;

		int pts(1);

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
			_bandInfoMap[q->Band()	] .IncrNetQso();
			_bandInfoMap["ALL"		] .IncrNetQso();
			_bandInfoMap[q->Band()	] .AddNetPts(pts);
			_bandInfoMap["ALL"		] .AddNetPts(pts);
			q->Pts(pts);
			break;

		case NOT_IN_LOG:
		case BUSTED_CALL:
		case BUSTED_REVLOG_CALL:
		case BUSTED_LIST_CALL:
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

void ArrlRttyLog::WriteRptMults(QTextStream & strm)
{
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Multipliers ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	foreach(QString mult, _arrlRttyMultMap.keys())
		strm << _arrlRttyMultMap[mult].first()->ToString() << "  " << mult << endl;
	strm << endl;
}

void ArrlRttyLog::ComputeMults()
{
	ClearMultMaps();

	foreach(QsoPtr q, _allQso)
	{
		q->IsMult(false);
		q->IsRawMult(false);

		if (!q->CountsForRawCredit())
			continue;

		if (q->IsMM())
			continue;

		QString multStr = q->IsWVeRcvd() ? q->XchRcvd() + " " : q->CtyRcvd()->StdPx(); // mults are states and countries

		_rawArrlRttyMultMap[multStr].append(q);
		if (_rawArrlRttyMultMap[multStr].size() == 1)
			_bandInfoMap["ALL"].IncrRawMults();

		if (!q->CountsForCredit())
			continue;

		_arrlRttyMultMap[multStr].append(q);
		if (_arrlRttyMultMap[multStr].size() == 1)
		{
			_bandInfoMap["ALL"].IncrNetMults();
			q->IsMult(true);
		}
	}

	foreach(QString multStr, _rawArrlRttyMultMap.keys())
	{
		if(!_arrlRttyMultMap.contains(multStr))
		{
			{
				QsoPtr q = _rawArrlRttyMultMap.value(multStr).first();
				q->IsLostMult(true);
		       _lostMultList.append(LostMult(q, ""));
			}
		}
	}
}

void ArrlRttyLog::WriteRptSummary(QTextStream & strm)
{
	QString title = ContestPtr()->Title();
	if(title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n")
		.arg("Contest:",	-12)	.arg(title)
		.arg("Call:",		-12)	.arg(Call())
		.arg("Category:",	-12)	.arg(_hqCategory);

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

	strm << QString("%1 Claimed mults\n")												.arg(RawMults(),8);
	strm << QString("%1 Final   mults\n\n")												.arg(NetMults(),8);

	strm << QString("%1 Claimed score\n")												.arg(QLocale().toString(RawScore()), 8);
	strm << QString("%1 Final   score\n")												.arg(QLocale().toString(Score()), 8);
	strm << QString("%1% Score reduction\n\n")											.arg(100.0 * double(Score() - RawScore()) / double(RawScore()) , 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final qso counts\n")			.arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) duplicates\n")											.arg(DupeCount(),		8)	.arg((100.0 * DupeCount())			/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n")								.arg(BustedCallCount(),	8)	.arg((100.0 * BustedCallCount())	/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) exchanges copied incorrectly\n")							.arg(BadXchCount(),		8)	.arg((100.0 * BadXchCount())		/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n")											.arg(NilCount(),		8)	.arg((100.0 * NilCount())			/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls unique to this log only (not removed)\n")			.arg(UniCount(),		8) 	.arg((100.0 * UniCount())			/ LogSize(), 3, 'f', 1);

	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("             %1 %2 %3 %4\n\n")
			.arg("Band"				,4)
			.arg("QSO"				,5)
			.arg("QPts"				,5)
			.arg("Mult"				,5);

	foreach(QString band, BandNameList())
	{
		strm << QString("   Claimed  %1 %2 %3\n")
			.arg(band			,4)
			.arg(RawQso (band)  ,5)
			.arg(RawPts (band)  ,5);

		strm << QString("   Final    %1 %2 %3\n\n")
			.arg(band			,4)
			.arg(NetQso (band)  ,5)
			.arg(NetPts (band)  ,5);
	}

	strm << QString("   Claimed  %1 %2 %3 %4  Score %5\n")
		.arg("All"		,4)
		.arg(RawQso()	,5)
		.arg(RawPts()	,5)
		.arg(RawMults()	,5)
		.arg(QLocale().toString(RawScore())	,8);

	strm << QString("   Final    %1 %2 %3 %4  Score %5\n\n")
		.arg("All"		 ,4)
		.arg(NetQso()	 ,5)
		.arg(NetPts()	 ,5)
		.arg(NetMults()	 ,5)
		.arg(QLocale().toString(Score()) ,8);
}

// FIX THIS

void ArrlRttyLog::ComputeOperatingTime()
{
	// if there are N qso, there are N - 1 time periods between them
	// scan the log and record all gaps >= 30 minutes
	// take the two biggest gaps and call them offtime
	// scan log again adding up all gaps to record ontime
	// when ontime - offtime >= 24 hours, the contest is over
	// this means time before the first qso is never offtime

	if (!_allQso.size())
	{
		AppendErrMsgList("EMPTY LOG:", QString::number(_allQso.size()));
		return;
	}

	QString tmDirName = "tm";
	QDir tmDir = ContestPtr()->TmDir();

	QFile  tmeFile(tmDir.absoluteFilePath(_fileInfo.baseName() + ".tm"));
	tmeFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream outStream;
	outStream.setDevice(& tmeFile);

	_operatingTime = 0;

	int largestGap(0);
	int secondLargestGap(0);
	
	for(size_t i = 0; i+1 < _allQso.size(); i++)
	{
		QsoPtr q = _allQso.at(i);
		QDateTime curTime(q->DateTime());
		if (curTime.secsTo(ContestPtr()->EndTime()) < -120) // two minute grace
		{
			outStream << "After end: " << q->ToString() << endl;
			q->IsOutOfTime(true);
			AppendOutOfTimeList(q);
			continue;
		}

		if (curTime.secsTo(ContestPtr()->StartTime()) > 120) // two minute grace
		{
			outStream << "Before start: " << q->ToString() << endl;
			q->IsOutOfTime(true);
			AppendOutOfTimeList(q);
			continue;
		}

		// find the two largest gaps >= 30 minutes

		int gap = q->DateTime().secsTo(_allQso.at(i + 1)->DateTime());		// get gap to next qso

		if (gap >= 30 * 60)
		{
			if (gap > largestGap)
			{
				secondLargestGap = largestGap;
				largestGap = gap;
			}
			else
			{
				if (gap > secondLargestGap)
					secondLargestGap = gap;
			}
		}
	}
	
	
	int offTime = largestGap + secondLargestGap;
	
	outStream << "largest time off: " << largestGap << endl;
	outStream << "second largest time off: " << secondLargestGap << endl;
	
	bool outOfTime(false);

	for (size_t i = 0; i+1 < _allQso.size(); i++)
	{
		QsoPtr q = _allQso.at(i);
		QsoPtr qNext = _allQso.at(i + 1);

		if(outOfTime)
		{
			q->IsOutOfTime(true);
			outStream << "Out of time: " << q->ToString() << endl;
			AppendOutOfTimeList(q);
			continue;
		}

		int delta = q->DateTime().secsTo(qNext->DateTime());	// get gap to next qso
		if(_operatingTime + delta - offTime > 3600 *24)
			outOfTime = true;
		else
			_operatingTime += delta;
	}

	_operatingTime = _operatingTime - offTime;

	outStream << QString("%1 %2").arg(Call() , -12).arg(_operatingTime) << endl;

	 tmeFile.close();
}

void ArrlRttyLog::CheckBandChanges()
{
	if(!_category.startsWith("M"))
		return;

	int count = 0;
	QString band;
	int hour = INT_MAX;

	foreach(QsoPtr q, _allQso)
	{
		int currentHour = (q->DateTime().toTime_t() - ContestPtr()->StartTime().toTime_t()) / 3600;
		if(currentHour != hour)
		{
			hour  = currentHour;
			count = 0;
			band  = q->Band();
			continue;
		}

		if(band != q->Band())
		{
			if(count == 6)
			{
				if(q->CountsForCredit())
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

void ArrlRttyLog::WriteMultFile(QTextStream & strm)
{
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << "  Multipliers  ";
	strm.setFieldWidth(0);
	strm << "\n\n";
	strm <<  " Multiplier total " << _arrlRttyMultMap.size() << "\n\n";

	foreach(QString mult, _arrlRttyMultMap.keys())
	{
		QsoPtr q = _arrlRttyMultMap[mult].first();
		if (!q)
			continue;
		strm << QString("%1 %2").arg(q->ToString()).arg(mult) << '\n';
	}
	strm << "\n";
}
