#include "StdAfx.h"

typedef QPair<QString, int> BandCount;

WrtcLog::WrtcLog() : IaruLog() {}

void WrtcLog::ComputeMults()
{
	_ctyMultMap.clear();
	_rawCtyMultMap.clear();
	_hqMultMap.clear();
	_rawHqMultMap.clear();

	foreach(QsoPtr p, _allQso)
	{
		QString hisCall;
		QString hisCty;
		QString	hisBand;
		QString	hisExch;
		QString	hisHq;

		IaruQso * q = static_cast<IaruQso *> (p);

		if (!q->CountsForRawCredit() || q->CallRcvd().startsWith("D1"))
			continue;

		hisCall = q->CallRcvd();
		hisCty  = q->CtyRcvd()->StdPx();
		hisBand = q->Band();
		hisExch = q->XchRcvd();

		if (static_cast <IaruContest *> (gContest)->IsHqAbrv(hisExch))
			hisHq = hisExch;
		else
			continue; // hisZone = q->PxRcvdPtr()->IaruZone();

		q->IsMult(false);		//zone
		q->IsRawMult(false);
		q->IsMult3(false);		//hq
		q->IsRawMult3(false);

		QString alternateBand = hisBand;
		if (hisBand.endsWith("CW"))
			alternateBand.replace("CW", "PH");
		else
			alternateBand.replace("PH", "CW");

		if (hisExch.contains(QRegularExpression("[A-Z]")))
			hisHq = hisExch;

		q->IsMult(false);	   	//cty
		q->IsRawMult(false);

		QString allName = "ALL " + q->Mode();

		// insert raw mult
		if (!hisHq.isEmpty())
		{
			if (!_rawHqMultMap.value(hisBand).value(hisHq).size() && !_rawHqMultMap.value(alternateBand).value(hisHq).size())
			{
				_rawHqMultMap[hisBand][hisHq].append(q);
				_bandInfoMap[hisBand].IncrRawMults3();
				_bandInfoMap[allName].IncrRawMults3();
				_bandInfoMap["ALL"].IncrRawMults3();
				q->IsRawMult3(true);
			}
		}
		else
		{
			// an HQ mult can't be a cty mult
			if (!_rawCtyMultMap.value(hisBand).value(hisCty).size() && !_rawCtyMultMap.value(alternateBand).value(hisCty).size() && !q->IsRawMult3())
			{
				_rawCtyMultMap[hisBand][hisCty].append(q);
				_bandInfoMap[hisBand].IncrRawMults();
				_bandInfoMap[allName].IncrRawMults();
				_bandInfoMap["ALL"].IncrRawMults();
				q->IsRawMult(true);
			}
		}

		// insert final mult
		if (q->CountsForCredit())
		{
			if (!hisHq.isEmpty())
			{
				if (!_hqMultMap.value(hisBand).value(hisHq).size() && !_hqMultMap.value(alternateBand).value(hisHq).size())
				{
					_hqMultMap[hisBand][hisHq].append(q);
					_bandInfoMap[hisBand].IncrNetMults3();
					_bandInfoMap[allName].IncrNetMults3();
					_bandInfoMap["ALL"].IncrNetMults3();
					q->IsMult3(true);
				}
			}
			else
			{
				// an HQ call can't be a cty mult too
				if (!_ctyMultMap.value(hisBand).value(hisCty).size() && !_ctyMultMap.value(alternateBand).value(hisCty).size() && !q->IsMM())
				{
					_ctyMultMap[hisBand][hisCty].append(q);
					_bandInfoMap[hisBand].IncrNetMults();
					_bandInfoMap[allName].IncrNetMults();
					_bandInfoMap["ALL"].IncrNetMults();
					q->IsMult(true);
				}
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

		foreach(QString cty, _rawCtyMultMap.value(band).keys())			// make list of lost country mults
		{
			if (!_ctyMultMap.value(band).contains(cty))
			{
				QsoPtr q = _rawCtyMultMap.value(band).value(cty).first();
				q->IsLostMult(true);
				_lostMultList.append(LostMult(q, "-C"));
			}
		}
	}

	_multMap = _ctyMultMap;
}

void WrtcLog::ComputePts()
{
	ComputeOperatingTime();

	int const nilPenalty(1);
	int const bustPenalty(1);

	foreach(QsoPtr p, AllQso())
	{
		IaruQso * q = static_cast<IaruQso *> (p);
		int pts = 0;

		if (q->IsDupe() || q->IsExcluded())
			continue;

		if (q->IsOutOfBand())
			q->Status(OUT_OF_BAND);

		bool crossBand = CategoryBand() != "ALL" && CategoryBand() != q->Band();

		if (gWrtcYear == "2014")
		{
			// zone 8 or HQ/IARU 
			if (_zone == q->ZoneRcvd() || !_zone)
				pts = 2;
			else
				if (_px->Cont() == q->CtyRcvd()->Cont())
					pts = 3;
				else
					pts = 5;
		}
		else
		{
			if (_px->Cont() == q->CtyRcvd()->Cont())
				pts = 2;
			else
				pts = 5;
		}

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

		default:
			q->Pts(0);
			break;
		}

	}
}

QString WrtcLog::ResultLine()
{
	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ")
			.arg(_rawCall				,-12)
			.arg(CtyPtr()->StdPx()		,5)
			.arg(CtyPtr()->Cont()		,4)
			.arg(_zone					,3)
			.arg(LogSize()				,5)
			.arg(DupeList().size()		,3)
			.arg(NilList().size()		,3)
			.arg(BustedCallList().size(),3)
			.arg(BandChangeViolationCount()	,4)
			.arg(UniList().size()		,3);

	s += QString("%1 %2 %3 %4 %5 %6 %7% %8 %9")
			.arg(_bandInfoMap["ALL"].NetQso(),5)
			.arg(_bandInfoMap["ALL"].NetMults3(), 3)
			.arg(_bandInfoMap["ALL"].NetMults(), 3)
			.arg(ClaimedScore()				,8)
			.arg(RawScore()					,8)
			.arg(Score()					,8)
			.arg(100.0 * (Score() - RawScore())/ RawScore(), 6, 'f', 1)
			.arg(OperatingTime()			,4, 'f', 1)
			.arg(Category()					,3);
	return s;
}

void WrtcLog::WriteMultFile(QTextStream & strm)
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

		foreach(QString cty, _ctyMultMap[bandName].keys())
		{
			QsoPtr q = _ctyMultMap[bandName][cty].first();
			strm << QString("%1 %2").arg(cty, -5).arg(q->ToString()) << '\n';
		}
		strm << "\n\n";
	}
}

void WrtcLog::WriteRptMults(QTextStream & strm)
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
		int multTotal = _ctyMultMap.value(bandName).size() + _hqMultMap.value(bandName).size();
		
		if (!_bandInfoMap[bandName].NetMults3() && !_bandInfoMap[bandName].NetMults())
			continue;

		strm << bandName << " Multiplier Total: " << multTotal << "\n\n";

		if (_bandInfoMap[bandName].NetMults3())
		{
			int cnt(0);
			strm << bandName << " HQ Multipliers" << endl;
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

		if (_bandInfoMap[bandName].NetMults())
		{
			int cnt(0);
			strm << bandName << "Country Multipliers" << endl;
			foreach(QString cty, _ctyMultMap[bandName].keys())
			{
				if (_ctyMultMap[bandName].contains(cty) && _ctyMultMap[bandName][cty].first()->IsMult3())
				{
					strm << QString("%1 ").arg(cty, -5);
					if ((++cnt % 10) == 0 && _ctyMultMap[bandName][cty] != _ctyMultMap[bandName].last())
						strm << '\n';
				}
			}
			strm << "\n\n";
		}
		strm << "\n";
	}
}

void WrtcLog::WriteRptSummary(QTextStream & strm)
{
	QString title = ContestPtr()->Title();
	if (title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n")
		.arg("Contest:", -12).arg(title)
		.arg("Call:", -12).arg(Call())
		.arg("Category:", -12).arg("WRTC Competitor");

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

	strm << QString("%1 Claimed Ctrys\n").arg(RawMults(), 8);
	strm << QString("%1 Final   zones\n\n").arg(NetMults(), 8);

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
		.arg("Cty", 3)
		.arg("Total", 4);

	foreach(QString band, BandNameList())
	{
		if (band.startsWith("160"))
			continue;

		strm << QString("   Claimed   %1 %2 %3 %4 %5 %6\n")
			.arg(band, 7)
			.arg(RawQso(band), 5)
			.arg(RawPts(band), 5)
			.arg(RawMults3(band), 3)
			.arg(RawMults(band), 3)
			.arg(RawMults(band) + RawMults3(band), 4);

		strm << QString("   Final     %1 %2 %3 %4 %5 %6\n\n")
			.arg(band, 7)
			.arg(NetQso(band), 5)
			.arg(NetPts(band), 5)
			.arg(NetMults3(band), 3)
			.arg(NetMults(band), 3)
			.arg(NetMults(band) + RawMults3(band), 4);
	}

	strm << QString("   Claimed   %1 %2 %3 %4 %5 %6  Score: %7\n")
		.arg("All  ", 7)
		.arg(RawQso(), 5)
		.arg(RawPts(), 5)
		.arg(RawMults3(), 3)
		.arg(RawMults(), 3)
		.arg(RawMults3() + RawMults(), 4)
		.arg(QLocale().toString(RawScore()));

	strm << QString("   Final     %1 %2 %3 %4 %5 %6  Score: %7\n")
		.arg("All  ", 7)
		.arg(NetQso(), 5)
		.arg(NetPts(), 5)
		.arg(NetMults3(), 3)
		.arg(NetMults(), 3)
		.arg(NetMults3() + NetMults(), 4)
		.arg(QLocale().toString(Score()));
}

//void WrtcLog::WriteLg(QTextStream & strm)
//{
//	static
//	QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16")
//		.arg("Freq"		)
//		.arg("Mode"		)
//		.arg("Date"		)
//		.arg("Time"		)
//		.arg("Call Sent")
//		.arg("Exch Sent")
//		.arg("Call Rcvd")
//		.arg("Exch Rcvd")
//		.arg("Cty Px"	)
//		.arg("Stn"		)
//		.arg("Pts"		)
//		.arg("Status"	)
//		.arg("Run/S/P"	)
//		.arg("Uni"		)
//		.arg("Mult C"	)
//		.arg("Mult Z"	);
//
//	strm << hdr << endl;
//
//	foreach(QsoPtr p, _allQso)
//	{
//		IaruQso * q = static_cast<IaruQso *> (p);
//		QString multStnStr;
//		if (IsMultiSingle())
//			multStnStr = q->IsMultStn() ? "Mult" : "Run";
//
//		if (IsMultiTwo())
//			multStnStr = q->IsMultStn() ? "TX 2" : "TX 1";
//
//		if (IsMultiMulti())
//			multStnStr = q->Band();
//
//		QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\n")
//			.arg(q->ToTabString())
//			.arg(q->CtyRcvd()->StdPx())
//			.arg(multStnStr)
//			.arg(q->Pts())
//			.arg(q->StatusChar())
//			.arg(q->Running())
//			.arg(q->IsUnique()	? "U"			: "")
//			.arg(q->IsMult()	? q->StdPx()	: ((q->IsLostMult())  ? "L" : ""))
//			.arg(q->IsMult2()	? q->XchRcvd()	: ((q->IsLostMult2()) ? "L" : ""));
//		strm << outStr;	
//	}
//}

