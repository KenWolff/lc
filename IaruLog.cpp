#include "StdAfx.h"

typedef QPair<QString, int> BandCount;

IaruLog::IaruLog() : Log(), _isHq(false), _isWrtc(false) {}
void IaruLog::WriteTsvLine(QTextStream & ) {}

void IaruLog::CheckBandChanges() 
{
	if (!_isMultiSingle || _isHq || _doNotCheck || _allQso.empty())
		return;

	// The IARU rule is 10 minutes until you change bands 
	// somtimes a logh has multiple qsos in the same minute and band chages can get out of order
	//  A, A, B, A, B, B where the B, A is in the same minute
	// step through the minutes and check fot band changes and BCV

	QMap <QDateTime, QMap<QString, QsoList> > qsoListsByTimeBand;
	foreach(auto q, _allQso)
		qsoListsByTimeBand[q->DateTime()][q->Band()].append(q);

	QDateTime currentStart = _allQso.front()->DateTime();
	QString currentBand = _allQso.front()->Band();

	foreach(auto now, qsoListsByTimeBand.keys())
	{
		QMap < QString, QsoList>  qsoListsByBand = qsoListsByTimeBand[now];

		// all qsos in this minute are on the current band
		if (qsoListsByBand.contains(currentBand) && qsoListsByBand.size() == 1)
			continue;	
		else
		{
			// too early, every qso except the ones on the current band get a BCV
			if (currentStart.secsTo(now) < 600)
			{
				foreach(auto band, qsoListsByBand.keys())
					if (band != currentBand)
						foreach(auto q, qsoListsByBand[band])
							q->Status(BAND_CHANGE_VIOLATION);
			}			
			else
			{
				currentBand = qsoListsByBand.first().first()->Band();
				currentStart = now;
			}
			currentBand = qsoListsByBand.first().first()->Band();
			currentStart = now;
		}
	}
}

void IaruLog::ClearMultMaps()
{
	_ctyMultMap.clear();
	_rawCtyMultMap.clear();
	_zoneMultMap.clear();
	_rawZoneMultMap.clear();
	_hqMultMap.clear();
	_rawHqMultMap.clear();
	_lostMultList.clear();
}

void IaruLog::ComputeCategory()
{
	IaruContest const * contest = static_cast<IaruContest const *>(_contest);
	_category = _hqCategory;

	// calls in iaru.tab or labeled IARU by robot are HQ

	if (_hqCat == "HQ")
	{
		if(!contest->IsHqFromCall(_call))
			AppendErrMsgList("HQ:", "claims HQ, but not in IaruHqStns.lis");
		else
		{
			_category = "HQ";
			_isHq = true;
		}
	}

	// calls on the Wrtc list
	if (contest->IsWrtcStn(_call))
	{ 
		_isWrtc = true;
		_category = "WRTC";
	}

	if (_category.startsWith('M'))
		_isMultiSingle = true;

	_offtimeThreshold = 3600;
	_maxOperatingTime = INT_MAX;
}

void IaruLog::ComputeMults()
{
	IaruContest * contest = static_cast <IaruContest *> (gContest);

	ComputeOperatingTime();
	ClearMultMaps();

	// run through the log in order, only counting good and unchecked q's
	// compute log exchange sent after all the qsos are loaded

	foreach(QsoPtr p, _allQso)
	{
		IaruQso * q = static_cast<IaruQso *> (p);

		q->IsMult2(false);		//zone
		q->IsRawMult2(false);
		q->IsMult3(false);		//hq
		q->IsRawMult3(false);

		if (!q->CountsForRawCredit())
			continue;

		QString hisCall = q->CallRcvd();
		QString hisBand = q->Band();
		QString hisExch = q->XchRcvd();

		QString alternateBand = hisBand;
		if (hisBand.endsWith("CW"))
			alternateBand.replace("CW", "PH");
		else
			alternateBand.replace("PH", "CW");

		int		hisZone	= hisExch.toInt();
		QString hisHq	= contest->HqAbrvFromCall(hisCall);

		if (CategoryBand() != "ALL" && CategoryBand() != q->Band())		// single band logs on wrong band
			continue;

		QString allName = "ALL " + q->Mode();
		if (!hisHq.isEmpty())
		{
			_rawHqMultMap[hisBand][hisHq].append(q);
			if (_rawHqMultMap[hisBand][hisHq].size() == 1 && !_rawHqMultMap.value(alternateBand).value(hisHq).size())
			{
				_bandInfoMap[hisBand].IncrRawMults3();
				_bandInfoMap[allName].IncrRawMults3();
				_bandInfoMap["ALL"].IncrRawMults3();
				q->IsRawMult3(true);
			}
		}
		else
		{
			if (hisZone >= 1 && hisZone <= 78)
			{
				_rawZoneMultMap[hisBand][hisZone].append(q);
				if (_rawZoneMultMap[hisBand][hisZone].size() == 1 && !_rawZoneMultMap.value(alternateBand).value(hisZone).size())
				{
					_bandInfoMap[hisBand].IncrRawMults2();
					_bandInfoMap[allName].IncrRawMults2();
					_bandInfoMap["ALL"].IncrRawMults2();
					q->IsRawMult2(true);
				}
			}
		}

		// compute final mults
		if (!q->CountsForCredit())
			continue;

		if (!hisHq.isEmpty())
		{
			_hqMultMap[hisBand][hisHq].append(q);
			if (_hqMultMap[hisBand][hisHq].size() == 1 && !_hqMultMap.value(alternateBand).value(hisHq).size())
			{
				_bandInfoMap[hisBand].IncrNetMults3();
				_bandInfoMap[allName].IncrNetMults3();
				_bandInfoMap["ALL"].IncrNetMults3();
				q->IsMult3(true);
			}
		}
		else
		{
			// HQ stns can't be a zone too
			if (hisZone >= 1 && hisZone <= 78)
			{
				_zoneMultMap[hisBand][hisZone].append(q);
				if (_zoneMultMap[hisBand][hisZone].size() == 1 && !_zoneMultMap.value(alternateBand).value(hisZone).size())
				{
					_bandInfoMap[hisBand].IncrNetMults2();
					_bandInfoMap[allName].IncrNetMults2();
					_bandInfoMap["ALL"].IncrNetMults2();
					q->IsMult2(true);
				}
			}
		}
	}

	foreach(QString band, _rawZoneMultMap.keys())
	{
		foreach(int zone, _rawZoneMultMap.value(band).keys())			// make list of lost zone mults
		{
			if (!_zoneMultMap.value(band).contains(zone))
			{
				QsoPtr q = _rawZoneMultMap.value(band).value(zone).first();
				q->IsLostMult(true);
				_lostMultList.append(LostMult(q, "-Z"));
			}
		}
	}

	foreach(QString band, _rawHqMultMap.keys())
	{
		foreach(QString Hq, _rawHqMultMap.value(band).keys())			// make list of lost HQ mults
		{
			if (!_hqMultMap.value(band).contains(Hq))
			{
				QsoPtr q = _rawHqMultMap.value(band).value(Hq).first();
				q->IsLostMult(true);
				_lostMultList.append(LostMult(q, "-H"));
			}
		}
	}
}

void IaruLog::ComputePts()
{
	ComputeOperatingTime();

	int const nilPenalty(0);
	int const bustPenalty(3);

	foreach(QsoPtr p, AllQso())
	{
		IaruQso * q = static_cast<IaruQso *> (p);
		int pts = 0;				// initialize point value of QSO to 0

		if (q->IsDupe() || q->IsExcluded())
			continue;

		if (q->IsOutOfBand())
			q->Status(OUT_OF_BAND);

		bool crossBand = CategoryBand() != "ALL" && CategoryBand() != q->Band();

		if (_zone == q->ZoneRcvd())
			pts = 1;
		else
			if (_px->Cont() == q->CtyRcvd()->Cont())
				pts = 3;
			else
				pts = 5;

		QString allName = "ALL " + q->Mode();

		if (!crossBand)
		{
			_bandInfoMap[q->Band()].IncrRawQso();
			_bandInfoMap[allName].IncrRawQso();
			_bandInfoMap["ALL"].IncrRawQso();
			_bandInfoMap[q->Band()].AddRawPts(pts);
			_bandInfoMap[allName].AddRawPts(pts);
			_bandInfoMap["ALL"].AddRawPts(pts);
		}

		switch (q->Status())
		{
		case NO_LOG:
		case GOOD_QSO:
			if (!crossBand)
			{
				q->Pts(pts);
				_bandInfoMap[q->Band()].IncrNetQso();
				_bandInfoMap[allName].IncrNetQso();
				_bandInfoMap["ALL"].IncrNetQso();
				_bandInfoMap[q->Band()].AddNetPts(pts);
				_bandInfoMap[allName].AddNetPts(pts);
				_bandInfoMap["ALL"].AddNetPts(pts);
			}
			break;

		case NOT_IN_LOG:
			if (!crossBand)
			{
				q->Pts(-nilPenalty*pts);
				_bandInfoMap[q->Band()].AddNetPts(-nilPenalty*pts);
				_bandInfoMap[allName].AddNetPts(-nilPenalty * pts);
				_bandInfoMap["ALL"].AddNetPts(-nilPenalty * pts);
			}
			break;

		case BUSTED_CALL:
		case BUSTED_FCC_CALL:
		case BUSTED_REVLOG_CALL:
		case BUSTED_LIST_CALL:
			if (!crossBand)
			{
				_bandInfoMap[q->Band()].AddNetPts(-bustPenalty*pts);
				_bandInfoMap[allName].AddNetPts(-bustPenalty * pts);
				_bandInfoMap["ALL"].AddNetPts(-bustPenalty * pts);
				q->Pts(-bustPenalty*pts);
			}
			break;

		default:
			q->Pts(0);
			break;
		}

	}
}

void IaruLog::ComputeScore()
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
}

void IaruLog::InitByContestType()
{
	_zone = _px->IaruZone();
}

QString IaruLog::ResultLine()
{
	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ")
		.arg(_rawCall, -12)
		.arg(CtyPtr()->StdPx(), 5)
		.arg(CtyPtr()->Cont(), 4)
		.arg(_zone, 3)
		.arg(LogSize(), 5)
		.arg(DupeList().size(), 3)
		.arg(NilList().size(), 3)
		.arg(BustedCallList().size(), 3)
		.arg(BandChangeViolationCount(), 4)
		.arg(UniList().size(), 3);

	s += QString("%1 %2 %3 %4 %5 %6 %7% %8 %9")
		.arg(_bandInfoMap["ALL"].NetQso(), 5)
		.arg(_bandInfoMap["ALL"].NetMults3(), 3)
		.arg(_bandInfoMap["ALL"].NetMults2(), 3)
		.arg(ClaimedScore(), 8)
		.arg(RawScore(), 8)
		.arg(Score(), 8)
		.arg(100.0 * (Score() - RawScore()) / RawScore(), 6, 'f', 1)
		.arg(OperatingTime(), 4, 'f', 1)
		.arg(Category(), 3);
	return s;
}

void IaruLog::WriteLg(QTextStream & strm)
{
	static
		QString iaruHdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16")
		.arg("Freq")
		.arg("Mode")
		.arg("Date")
		.arg("Time")
		.arg("Call Sent")
		.arg("Exch Sent")
		.arg("Call Rcvd")
		.arg("Exch Rcvd")
		.arg("Cty Px")
		.arg("Stn")
		.arg("Pts")
		.arg("Status")
		.arg("Run/S/P")
		.arg("Uni")
		.arg("Mult Z")
		.arg("Mult H");

	static
		QString wrtcHdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16")
		.arg("Freq")
		.arg("Mode")
		.arg("Date")
		.arg("Time")
		.arg("Call Sent")
		.arg("Exch Sent")
		.arg("Call Rcvd")
		.arg("Exch Rcvd")
		.arg("Cty Px")
		.arg("Stn")
		.arg("Pts")
		.arg("Status")
		.arg("Run/S/P")
		.arg("Uni")
		.arg("Mult C")
		.arg("Mult H");

	strm << iaruHdr << endl;

	foreach(QsoPtr p, _allQso)
	{
		IaruQso * q = static_cast<IaruQso *> (p);
		QString multStnStr;
		if (IsMultiSingle())
			multStnStr = q->IsMultStn() ? "Mult" : "Run";

		if (_isWrtc) // WRTC output
		{
			QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\n")
				.arg(q->ToTabString())
				.arg(q->CtyRcvd()->StdPx())
				.arg("Run")
				.arg(q->Pts())
				.arg(q->StatusChar())
				.arg(q->Running())
				.arg(q->IsUnique() ? "U" : "")
				.arg(q->IsMult() ? q->StdPx() : ((q->IsLostMult()) ? "L" : ""))
				.arg(q->IsMult3() ? q->XchRcvd() : ((q->IsLostMult3()) ? "L" : ""));
			strm << outStr;
		}
		else // IARU output
		{
			QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\n")
				.arg(q->ToTabString())
				.arg(q->CtyRcvd()->StdPx())
				.arg(multStnStr)
				.arg(q->Pts())
				.arg(q->StatusChar())
				.arg(q->Running())
				.arg(q->IsUnique() ? "U" : "")
				.arg(q->IsMult2() ? q->XchRcvd() : ((q->IsLostMult()) ? "L" : ""))
				.arg(q->IsMult3() ? q->XchRcvd() : ((q->IsLostMult3()) ? "L" : ""));
			strm << outStr;
		}
	}
}

void IaruLog::WriteMultFile(QTextStream & strm)
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

		strm << bandName << " multiplier total " << _hqMultMap.value(bandName).size() + _zoneMultMap[bandName].size() << "\n\n";

		foreach(QString hq, _hqMultMap[bandName].keys())
		{
			QsoPtr q = _hqMultMap[bandName][hq].first();
			strm << QString("%1 %2").arg(hq, -5).arg(q->ToString()) << '\n';
		}
		strm << "\n";

		for (int zone = 1; zone <= 78; zone++)
		{
			if (_zoneMultMap[bandName].contains(zone))
			{
				QsoPtr q = _zoneMultMap[bandName][zone].first();
				strm << QString("%1 %2").arg(zone, -5).arg(q->ToString()) << '\n';
			}
		}
		strm << "\n\n";
	}
}

void IaruLog::WriteRptMults(QTextStream & strm)
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
		int multTotal = _hqMultMap.value(bandName).size() + _zoneMultMap.value(bandName).size();

		if (!_bandInfoMap[bandName].NetMults3() && !_bandInfoMap[bandName].NetMults2())
			continue;

		strm << bandName << " multiplier total " << multTotal << "\n\n";

		if (_bandInfoMap[bandName].NetMults3())
		{
			int cnt(0);
			strm << bandName << "HQ Multipliers" << endl;
			foreach(QString hq, _hqMultMap[bandName].keys())
			{
				if (_hqMultMap[bandName].contains(hq) && _hqMultMap[bandName][hq].first()->IsMult3())
				{
					strm << QString("%1 ").arg(hq, -5);
					if ((++cnt % 10) == 0 && _hqMultMap[bandName][hq] != _hqMultMap[bandName].last())
						strm << '\n';
				}
			}
			strm << "\n\n";
		}

		if (_bandInfoMap[bandName].NetMults2())
		{
			int cnt(0);
			strm << bandName << " Zone Multipliers" << endl;
			for (int zone = 1; zone <= 78; zone++)
			{
				if (_zoneMultMap[bandName].contains(zone) && _zoneMultMap[bandName][zone].first()->IsMult2())
				{
					strm << QString("%1").arg((_zoneMultMap[bandName].contains(zone) ? QString::number(zone) : ""), 3);
					if ((++cnt % 20) == 0 && _zoneMultMap[bandName][zone] != _zoneMultMap[bandName].last())
						strm << '\n';
				}
			}
			strm << "\n\n";
		}
	}
}

void IaruLog::WriteRptSummary(QTextStream & strm)
{
	QString title = ContestPtr()->Title();
	if (title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n")
		.arg("Contest:", -12).arg(title)
		.arg("Call:", -12).arg(Call())
		.arg("Category:", -12).arg(_category);

	QString operators = Operators();
	while (!operators.isEmpty() && Operators() != Call())
	{
		int idx = operators.lastIndexOf(",", 60);
		if (idx != -1)
		{
			strm << QString("%1%2\n").arg("Operator(s):", -12).arg(operators.left(idx + 1));
			operators.remove(0, idx + 1);
			operators = operators.trimmed();
		}
		else
		{
			strm << QString("%1%2\n").arg("Operator(s):", -12).arg(operators);
			break;
		}
	}

	// summary

	if (!AdminText().isEmpty())
		strm << '\n' << AdminText() << '\n';

	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Summary ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("%1 Claimed QSO before checking (does not include duplicates)\n").arg(RawQso(), 8);
	strm << QString("%1 Final   QSO after  checking reductions\n\n").arg(NetQso(), 8);

	strm << QString("%1 Claimed QSO points\n").arg(RawPts(), 8);
	strm << QString("%1 Final   QSO points\n\n").arg(NetPts(), 8);

	//strm << QString("%1 Claimed countries\n").arg(RawMults(), 8);
	//strm << QString("%1 Final   countries\n\n").arg(NetMults(), 8);

	strm << QString("%1 Claimed HQ\n").arg(RawMults3(), 8);
	strm << QString("%1 Final   HQ\n\n").arg(NetMults3(), 8);

	strm << QString("%1 Claimed zones\n").arg(RawMults2(), 8);
	strm << QString("%1 Final   zones\n\n").arg(NetMults2(), 8);

	strm << QString("%1 Claimed mults total\n").arg(RawMults() + RawMults2() + RawMults3(), 8);
	strm << QString("%1 Final   mults total\n\n").arg(NetMults() + NetMults2() + NetMults3(), 8);

	strm << QString("%1 Claimed score\n").arg(QLocale().toString(RawScore()));
	strm << QString("%1 Final   score\n\n").arg(QLocale().toString(Score()));
	if (Reduction() < 0.15)
		strm << QString("%1% Score reduction\n").arg(100.0 * Reduction(), 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final qso counts\n").arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n").arg(BustedCallCount(), 8).arg((100.0 * BustedCallCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) exchanges copied incorrectly\n").arg(BadXchCount(), 8).arg((100.0 * BadXchCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n").arg(NilCount(), 8).arg((100.0 * NilCount()) / LogSize(), 3, 'f', 1);

	if (_rubberClockedByBandMap.size())
		strm << QString("%1 (%2%) removed for rubber clocking\n").arg(_rubberClockedByBandMap.size(), 8).arg((100.0 * _rubberClockedByBandMap.size()) / LogSize(), 3, 'f', 1);

	if (_outOfBandByTimeMap.size())
		strm << QString("%1 (%2%) out of band qsos\n").arg(_outOfBandByTimeMap.size(), 8).arg((100.0 * _outOfBandByTimeMap.size()) / LogSize(), 3, 'f', 1);
	if (IsMultiSingle() && BandChangeViolationCount())
		strm << QString("%1 (%2%) band change violations\n").arg(BandChangeViolationCount(), 8).arg((100.0 * BandChangeViolationCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) duplicates (Removed without penalty)\n").arg(DupeCount(), 8).arg((100.0 * DupeCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls unique to this log receiving credit (not removed)\n").arg(UniCount(), 8).arg((100.0 * UniCount()) / LogSize(), 3, 'f', 1);
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("%1 %2 %3 %4 %5 %6 %7\n\n")
		.arg("", 12)
		.arg("Band", 7)
		.arg("QSO", 5)
		.arg("QPts", 5)
		.arg("HQ", 3)
		.arg("Z", 3)
		.arg("Total", 4);

	foreach(QString band, BandNameList())
	{
		strm << QString("   Claimed   %1 %2 %3 %4 %5 %6\n")
			.arg(band, 7)
			.arg(RawQso(band), 5)
			.arg(RawPts(band), 5)
			.arg(RawMults3(band), 3)
			.arg(RawMults2(band), 3)
			.arg(RawMults(band) + RawMults2(band) + RawMults3(band), 4);

		strm << QString("   Final     %1 %2 %3 %4 %5 %6\n\n")
			.arg(band, 7)
			.arg(NetQso(band), 5)
			.arg(NetPts(band), 5)
			.arg(NetMults3(band), 3)
			.arg(NetMults2(band), 3)
			.arg(NetMults(band) + NetMults2(band) + RawMults3(band), 4);
	}

	strm << QString("   Claimed   %1 %2 %3 %4 %5 %6  Score: %7\n")
		.arg("All", 7)
		.arg(RawQso(), 5)
		.arg(RawPts(), 5)
		.arg(RawMults3(), 3)
		.arg(RawMults2(), 3)
		.arg(RawMults3() + RawMults2() + RawMults(), 4)
		.arg(QLocale().toString(RawScore()));

	strm << QString("   Final     %1 %2 %3 %4 %5 %6  Score: %7\n")
		.arg("All", 7)
		.arg(NetQso(), 5)
		.arg(NetPts(), 5)
		.arg(NetMults3(), 3)
		.arg(NetMults2(), 3)
		.arg(NetMults3() + NetMults2() + NetMults(), 4)
		.arg(QLocale().toString(Score()));
}

void IaruLog::WriteTabLine(QTextStream & strm)
{
	QString category = _hqCat.isEmpty() ? _hqCategory : _hqCat;

	bool lateFlag = _submittedLate || _isNoAward;
	double uniPercent = double(UniList().size()) / double(NetQso());
	QString uniPercentStr = QString("%1").arg(100 * uniPercent, 4, 'f', 1, '0');

	strm << _rawCall						<< '\t' //	 "Call"
		<< _cty->StdPx()					<< '\t'	//	 "Cty Px"
		<< _cont							<< '\t'	//	 "Cont"
		<< _zone							<< '\t'	//	 "Zone"
		<< _location						<< '\t'	//	 "Loc"
		<< _district						<< '\t'	//	 "Dist"
		<< _category						<< '\t'	//	 "Cat"
		<< _certificate						<< '\t'	//	 "Cert"
		<< _mailCert						<< '\t'	//	 "Mail Cert"
		<< lateFlag							<< '\t'	//	 "Late" 
		<< _claimedScore					<< '\t'	//	 "Claimed"
		<< _rawScore						<< '\t'	//	 "Raw Scr"
		<< _score							<< '\t'	//	 "Final Score"
		<< Reduction()						<< '\t'	//	 "Reduct"
		<< RawQso()							<< '\t'	//	 "Raw Qso"
		<< NetQso()							<< '\t'	//	 "Net Qso"
		<< RawMults3()						<< '\t'	//	 "Raw HQ"
		<< NetMults3()						<< '\t'	//	 "Net HQ"
		<< RawMults2()						<< '\t'	//	 "Raw Zones"
		<< NetMults2()						<< '\t'	//	 "Net Zones"
		<< RawMults()						<< '\t'	//	 "Raw Cty"
		<< NetMults()						<< '\t'	//	 "Net Cty"
		<< DupeList().size()				<< '\t'	//	 "Dupes"
		<< BustedCallCount()				<< '\t'	//	 "B"
		<< NilList().size()					<< '\t'	//	 "NIL"
		<< BustedXchList().size()			<< '\t'	//	 "X"
		<< (_fillRunMult ? "1" : "0")		<< '\t'	//	 "AUTO_RUN_MULT"
		<< BandChangeViolationList().size() << '\t'	//	 "BCV"
		<< UniList().size()					<< '\t'	//	 "Uni"
		<< uniPercentStr					<< '\t'	//	 "% Uni"
		<< _bustedCallCausedByTimeMap.size()<< '\t'	//	 "C B"
		<< _nilCausedByTimeMap.size()		<< '\t'	//	 "C NIL"
		<< _bustedXchCausedByTimeMap.size() << '\t'	//	 "C X"
		<< NetQso("ALL")					<< '\t'	//	 "Qso"
		<< NetMults("ALL") + NetMults2("ALL") + NetMults3("ALL") << '\t'	//	 "Mult"

		<< NetQso("160M CW")	+ NetQso("160M PH")		<< '\t'
		<< NetMults3("160M CW") + NetMults3("160M PH")	<< '\t'
		<< NetMults2("160M CW") + NetMults2("160M PH")	<< '\t'
		<< NetMults("160M CW")	+ NetMults("160M PH")	<< '\t'

		<< NetQso("80M CW")		+ NetQso("80M PH")		<< '\t'
		<< NetMults3("80M CW")	+ NetMults3("80M PH")	<< '\t'
		<< NetMults2("80M CW")	+ NetMults2("80M PH")	<< '\t'
		<< NetMults("80M CW")	+ NetMults("80M PH")	<< '\t'

		<< NetQso("40M CW")		+ NetQso("40M PH")		<< '\t'
		<< NetMults3("40M CW")	+ NetMults3("40M PH")	<< '\t'
		<< NetMults2("40M CW")	+ NetMults2("40M PH")	<< '\t'
		<< NetMults("40M CW")	+ NetMults("40M PH")	<< '\t'

		<< NetQso("20M CW")		+ NetQso("20M PH")		<< '\t'
		<< NetMults3("20M CW")	+ NetMults3("20M PH")	<< '\t'
		<< NetMults2("20M CW")	+ NetMults2("20M PH")	<< '\t'
		<< NetMults("20M CW")	+ NetMults("20M PH")	<< '\t'

		<< NetQso("15M CW")		+ NetQso("15M PH")		<< '\t'
		<< NetMults3("15M CW")	+ NetMults3("15M PH")	<< '\t'
		<< NetMults2("15M CW")	+ NetMults2("15M PH")	<< '\t'
		<< NetMults("15M CW")	+ NetMults("15M PH")	<< '\t'

		<< NetQso("10M CW")		+ NetQso("10M PH")		<< '\t'
		<< NetMults3("10M CW")	+ NetMults3("10M PH")	<< '\t'
		<< NetMults2("10M CW")	+ NetMults2("10M PH")	<< '\t'
		<< NetMults("10M CW")	+ NetMults("10M PH")	<< '\t'

		<< OperatingTime()			<< '\t'	//	 "Op Hrs"
		<< LastQsoTime().toString()	<< '\t'	//	 "Final QTR"
		<< _categoryOperator		<< '\t'	//	 "CatOp"
		<< _categoryTransmitter		<< '\t'	//	 "CatTx"
		<< _categoryOverlay			<< '\t'	//	 "CatOverlay"
		<< _categoryBand			<< '\t'	//	 "CatBand"
		<< _categoryPower			<< '\t'	//	 "CatPwr"
		<< _categoryMode			<< '\t'	//	 "CatMode"
		<< _categoryAssisted		<< '\t'	//	 "CatAssisted"
		<< _categoryStation			<< '\t'	//	 "CatStn"
		<< _club					<< '\t'	//	 "Club"
		<< _operators				<< '\t'	//	 "Operators"
		<< _name					<< '\t'	//	 "Name"
		<< _email					<< '\t'	//	 "email"
		<< _hqFrom					<< '\t'	//	 "HQ From"
		<< _hqDateRcvd				<< '\t'	//	 "HQ Date"
		<< _soapBox					<< '\t'	//	 "Soap Box"
		<< _createdBy				<< '\t'	//	 "Created By"
		<< _addressCity				<< '\t'	//	 "City"
		<< _addressStateProvince	<< '\t'	//	 "State-Province"
		<< _addressPostalCode		<< '\t'	//	 "Postal Code"
		<< _addressCountry			<< '\t'	//   "Country"
		<< _addressStreet			<< '\n';//	 "Addr"
}

void IaruLog::WriteMultUniqueToMe(QTextStream & strm)
{
	foreach(auto bandMap, _hqMultMap)
		foreach(auto qsoList, bandMap)
			if (_contest->LogSetByCallRcvd()[qsoList.first()->CallRcvd()].size() < 5 && qsoList.first()->StdPx() == _cty->StdPx())
				strm << QString("%1 claimed %2 for %3").arg(_call, -10).arg(qsoList.first()->CallRcvd(), -10).arg(qsoList.first()->StdPx()) << endl;

	foreach(auto bandMap, _zoneMultMap)
		foreach(auto zone, bandMap.keys())
		{
			IaruQso * q = static_cast<IaruQso *> (bandMap[zone].first());

			if (_contest->LogSetByCallRcvd()[q->CallRcvd()].size() < 5 && q->ZoneRcvd() == _zone)
				strm << QString("%1 claimed %2 for %3").arg(_call, -10).arg(q->CallRcvd(), -10).arg(_zone) << endl;
		}
}

int	IaruLog::Zone() const { return _zone; }
int	IaruLog::MostRcvdZone() const {	return _mostRcvdZone; }
