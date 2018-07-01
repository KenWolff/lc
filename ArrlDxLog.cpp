#include "StdAfx.h"

ArrlDxLog::ArrlDxLog() : ArrlLog() {}
ArrlDxWVeLog::ArrlDxWVeLog() : ArrlDxLog() {}
void ArrlDxLog::WriteTsvLine(QTextStream & ) {}

void ArrlDxWVeLog::ClearMultMaps()
{
	_multByBandCtyMap		.clear();
	_rawMultByBandCtyMap	.clear();
	_lostMultList			.clear();
}

void ArrlDxDxLog::ClearMultMaps()
{
	_multByBandStateMap		.clear();
	_rawMultByBandStateMap	.clear();
	_lostMultList			.clear();
}

void ArrlDxWVeLog::WriteLg(QTextStream & strm)
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
		.arg("Pwr Rcvd")
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
			.arg(q->IsMult() ? q->StdPx() : ((q->IsLostMult()) ? "L" : ""));
		strm << outStr;
	}
}

void ArrlDxDxLog::WriteLg(QTextStream & strm)
{
	static
		QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14")
		.arg("Freq")
		.arg("Mode")
		.arg("Date")
		.arg("Time")
		.arg("Call Sent")
		.arg("Power Sent")
		.arg("Call Rcvd")
		.arg("StateRcvd")
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

		QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
			.arg(q->ToTabString())
			.arg(multStnStr)
			.arg(q->Pts())
			.arg(q->StatusChar())
			.arg(q->Running())
			.arg(q->IsUnique() ? "U" : "")
			.arg(q->IsMult() ? q->XchRcvd() : ((q->IsLostMult()) ? "L" : ""));
		strm << outStr;
	}
}

void ArrlDxLog::ComputeScore()
{
	_bandInfoMap.clear();
	ComputeOperatingTime();
	ComputeMults();
	ComputePts();

	int rawPts = _bandInfoMap["ALL"].RawPts();
	int netPts = _bandInfoMap["ALL"].NetPts();

	_rawScore	= rawPts * _bandInfoMap["ALL"].RawMults();
	_score		= netPts * _bandInfoMap["ALL"].NetMults();
}

void ArrlDxLog::CheckBandChanges()
{
	if(IsMultiTwo())
		CheckM2BandChanges(6);

	if(IsMultiSingle())
		CheckMSBandChanges(6);
}

QString ArrlDxLog::ResultLine()
{
	QString s = QString("%1   %2 %3 %4 %5 %6 %7")
			.arg(_rawCall					,-12)
			.arg(LogSize()					,5)
			.arg(DupeList().size()			,4)
			.arg(NilList().size()			,3)
			.arg(BustedCallCount()			,3)
			.arg(BustedXchList().size()		,3)
			.arg(BandChangeViolationCount()	,3);

	s += QString("%1 %2 %3 %4 %5  %6%   %7 %8 ")
			.arg(_bandInfoMap["ALL"].NetQso()   ,7)
			.arg(MultTotal()					,6)
			.arg(Score()						,9)
			.arg(RawScore()						,10)
			.arg(ClaimedScore()					,8)
			.arg(100.0 * (Score() - RawScore())/ RawScore(), 6, 'f', 1)
			.arg(OperatingTime()				,4, 'f', 1)
			.arg(Category()						,4);
	return s;
}

void ArrlDxLog::ComputeCategory()
{
	_category = _hqCat;

	if(_category.startsWith("MS"))
		_isMultiSingle  = true;

	if (_category.startsWith("M2"))
		_isMultiTwo = true;

	if (_category.startsWith("MM"))
		_isMultiMulti = true;



}

void ArrlDxLog::ComputePts()
{
	foreach(QsoPtr q, _allQso)
	{
		if(q->IsDupe())
			continue;

		if(CategoryBand() != "ALL" && CategoryBand() !=  q->Band())
			continue;

		int pts(3);

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
		case BUSTED_FCC_CALL:
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

// FIX THIS ArrlDxWVeLog::ComputeMults() should combine with ArrlDxDxLog::ComputeMults()
// FIX THIS Are cty mults ever going to be pointers to cty?

void ArrlDxWVeLog::ComputeMults()
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

		if(CategoryBand() != "ALL" && CategoryBand() !=  q->Band())	 // single band logs on wrong band
			continue;

		_rawMultByBandCtyMap[q->Band()][q->CtyRcvd()->StdPx()].append(q);

		if(_rawMultByBandCtyMap[q->Band()][(q)->CtyRcvd()->StdPx()].size() == 1)	// first one, add a mult
		{
			_bandInfoMap[q->Band()].IncrRawMults();
			_bandInfoMap["ALL"].IncrRawMults();
			q->IsMult(true);
		}

		if (!q->CountsForCredit())
			continue;

		_multByBandCtyMap[q->Band()][q->CtyRcvd()->StdPx()].append(q);

		if(_multByBandCtyMap[q->Band()][(q)->CtyRcvd()->StdPx()].size() == 1)	  // first one, add a mult
		{
			_bandInfoMap[q->Band()].IncrNetMults();
			_bandInfoMap["ALL"].IncrNetMults();
			q->IsMult(true);
		}
	}

	foreach(QString band, _rawMultByBandCtyMap.keys())
	{
		foreach(QString ctyPx, _rawMultByBandCtyMap.value(band).keys())
		{
			if(!_multByBandCtyMap.value(band).contains(ctyPx))
			{
				QsoPtr q = _rawMultByBandCtyMap.value(band).value(ctyPx).first();
				q->IsLostMult(true);
		       _lostMultList.append(LostMult(q, ""));
			}
		}
	}
}

void ArrlDxDxLog::ComputeMults()
{
	ClearMultMaps();

	foreach (QsoPtr q, _allQso)
	{
		q->IsMult(false);
		q->IsRawMult(false);

		if (!q->CountsForRawCredit())
			continue;

		if(!q->IsWVeRcvd() || q->IsMM())
			continue;

		if(CategoryBand() != "ALL" && CategoryBand() !=  q->Band())			// single band logs on wrong band
			continue;

		_rawMultByBandStateMap[q->Band()][q->XchRcvd()].append(q);
		if (_rawMultByBandStateMap[q->Band()][q->XchRcvd()].size() == 1)	// first one, add a mult
		{
			_bandInfoMap[q->Band()].IncrRawMults();
			_bandInfoMap["ALL"].IncrRawMults();
		}

		if (!q->CountsForCredit())
			continue;

		_multByBandStateMap[q->Band()][q->XchRcvd()].append(q);
		if(_multByBandStateMap[q->Band()][q->XchRcvd()].size() == 1)	 // first one, add a mult
		{
			_bandInfoMap[q->Band()].IncrNetMults();
			_bandInfoMap["ALL"].IncrNetMults();
			q->IsMult(true);
		}
	}

	foreach(QString band, _rawMultByBandStateMap.keys())
	{
		foreach(QString state, _rawMultByBandStateMap.value(band).keys())
		{
			if(!_multByBandStateMap.value(band).contains(state))
			{
				QsoPtr q = _rawMultByBandStateMap.value(band).value(state).first();
				q->IsLostMult(true);
		       _lostMultList.append(LostMult(q, ""));
			}
		}
	}
}

int ArrlDxDxLog::MultTotal()
{
	int stateTotal = 0;

	foreach(QString band, BandNameList())
		stateTotal += _multByBandStateMap.value(band).size();

	return stateTotal;
}

int ArrlDxWVeLog::MultTotal()
{
	int ctyTotal = 0;

	foreach(QString band, BandNameList())
		ctyTotal += _multByBandCtyMap.value(band).size();

	return ctyTotal;
}

void ArrlDxLog::WriteMultFile(QTextStream & strm)
{
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << "  Multipliers by Band  ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	foreach(QString bandName, BandNameList())
	{
		if (!MultMap().contains(bandName))
			continue;

		strm << bandName << " multiplier total " << MultMap()[bandName].size() << "\n\n";

		foreach(QString mult, MultMap()[bandName].keys())
		{
			QsoPtr q = MultMap()[bandName][mult].first();
			if (!q)
				continue;
			strm << QString("%1 %2").arg(mult, -5).arg(q->ToString()) << '\n';
		}
		strm << "\n";
	}
}

void ArrlDxLog::WriteRptMults(QTextStream & strm)
{
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << "  Multipliers by Band  ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	foreach(QString  band, BandNameList())
	{
		if (!MultMap().contains(band))
			continue;

		strm << band << " multiplier total " << MultMap().value(band).size() << "\n\n";

		int i = 0;
		foreach(QString cty, MultMap().value(band).keys())
		{
			i++;
			strm << QString("%1 ").arg(cty, -5);
			if ((i % 10) == 0)
				strm << '\n';
		}
		strm << "\n\n";
	}
	//	strm << "\n\n";
}

void ArrlDxLog::WriteRptSummary(QTextStream & strm)
{
	strm.setFieldWidth(0);

	QString title = ContestPtr()->Title();
	if (title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n%7%8\n")
		.arg("Contest:", -12).arg(title)
		.arg("Call:", -12).arg(Call())
		.arg("Category: ", -12).arg(_hqCategory)
		.arg("Location: ", -12).arg(_location);

	QString operators = _operators;

	while (!operators.isEmpty() && _operators != _call)
	{
		int idx = operators.lastIndexOf(",", 60);
		if (idx != -1)
		{
			strm << QString("%1%2\n").arg("Operators:", -12).arg(operators.left(idx + 1));
			operators.remove(0, idx + 1);
			operators = operators.trimmed();
		}
		else
		{
			strm << QString("%1%2\n").arg("Operators:", -12).arg(operators);
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
	strm << QString("%1 Claimed mults\n").arg(RawMults(), 8);
	strm << QString("%1 Final   mults\n\n").arg(NetMults(), 8);
	strm << QString("%1 Claimed score\n").arg(QLocale().toString(RawScore()));
	strm << QString("%1 Final   score\n\n").arg(QLocale().toString(Score()));
	strm << QString("%1% Score reduction\n\n").arg(100.0 * double(Score() - RawScore()) / double(RawScore()), 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final qso counts\n").arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) duplicates\n").arg(DupeCount(), 8).arg((100.0 * DupeCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n").arg(BustedCallCount(), 8).arg((100.0 * BustedCallCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) exchanges copied incorrectly\n").arg(BadXchCount(), 8).arg((100.0 * BadXchCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n").arg(NilCount(), 8).arg((100.0 * NilCount()) / LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls unique to this log only (not removed)\n").arg(UniCount(), 8).arg((100.0 * UniCount()) / LogSize(), 3, 'f', 1);
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";
	strm << QString("            %1 %2 %3 %4\n\n").arg("Band", 4).arg("QSO", 5).arg("QPts", 5).arg("Mult");

	foreach(QString band, BandNameList())
	{
		strm << QString("   Claimed  %1 %2 %3 %4\n")
			.arg(band, 4)
			.arg(RawQso(band), 5)
			.arg(RawPts(band), 5)
			.arg(RawMults(band), 4);

		strm << QString("   Final    %1 %2 %3 %4\n\n")
			.arg(band, 4)
			.arg(NetQso(band), 5)
			.arg(NetPts(band), 5)
			.arg(NetMults(band), 4);
	}

	strm << QString("   Claimed  %1 %2 %3 %4    Score %5\n")
		.arg("All", 4)
		.arg(RawQso(), 5)
		.arg(RawPts(), 5)
		.arg(RawMults(), 4)
		.arg(QLocale().toString(RawScore()), 10);

	strm << QString("   Final    %1 %2 %3 %4    Score %5\n")
		.arg("All", 4)
		.arg(NetQso(), 5)
		.arg(NetPts(), 5)
		.arg(NetMults(), 4)
		.arg(QLocale().toString(Score()), 10);
}

void ArrlDxLog::WriteTabLine(QTextStream & strm)
{
	QString guestCall;
	QString displayedCall(_rawCall);
	QString displayedOperators;

	foreach(QString op, _opsList)
	{
		if (op.startsWith('@'))
		{
			_hostCall = op;
			_hostCall.remove(0, 1);
		}
	}

	if (_opsList.size() && _opsList[0].contains(QRegularExpression("[A-Z]\\d+[A-Z]")))
	{
		if (_isSingleOp)		// single op
		{
			if (_opsList[0] != _call || (!_hostCall.isEmpty() && _hostCall != _call))
			{
				displayedCall.append(" (");
				if (_opsList[0] != _call)
				{
					guestCall = QString("%1, op").arg(_opsList[0]);					// show guest op
					displayedCall.append(guestCall);
				}
				if (!_hostCall.isEmpty() && _hostCall != _call)
				{
					displayedCall.append(" @ " + _hostCall);						// show host station
				}
				displayedCall.append(")");
			}
		}
		else																		// multi op
		{
			bool opsListHasMainCall = false;

			QStringList displayOpsList;
			foreach(QString op, _opsList)
			{
				if (op == _call)
				{
					if (!opsListHasMainCall)
						displayOpsList.prepend("+ ");
					opsListHasMainCall = true;
				}
				else
					if (!op.startsWith('@'))
						displayOpsList.append(op);
			}

			int displayOpsCount = displayOpsList.size();
			if (opsListHasMainCall)
				displayOpsCount--;

			displayedOperators = "(";

			QStringListIterator it(displayOpsList);
			while (it.hasNext())
			{
				QString op = it.next();
				if (op.startsWith('+'))
					displayedOperators.append(op);
				else
				{
					if (it.hasNext())
					{
						if (displayOpsCount == 2)
							displayedOperators.append(op + " & ");
						else
							displayedOperators.append(op + ", ");
					}
					else
						displayedOperators.append(op);
				}
			}

			if (!opsListHasMainCall)
				displayedOperators += ", ops";
			if (!_hostCall.isEmpty())
				displayedOperators.append(" @ " + _hostCall);
			displayedOperators += ")";
		}
	}

	// this turns OH into OH1 
	std::map <QString, QString>	arrlAliasLocationMap =
	{
		{ "CE9", "KH4" },{ "CM", "CO1" },{ "CT", "CT1" },{ "EA", "EA3" },{ "HI", "HI8" },{ "KL", "KL7" },{ "LA", "LA1" },
		{ "OH",  "OH1" },{ "OK", "OK1" },{ "ON", "ON1" },{ "SV", "SV1" },{ "TN", "TN1" },{ "UA", "UA3" }
	};
	
	QString tmpPx = _cty->StdPx();
	if (arrlAliasLocationMap.count(_cty->StdPx()))
		tmpPx = arrlAliasLocationMap[_cty->StdPx()];

	double num = double(BustedCallCount() + NilList().size() + BustedXchList().size());
	double denom = double(QsoCount());
	double err = 100.0 * num / denom;

 	QString errorRate = QString("%1").arg(err, 0, 'f', 1);
	QString outStr;

	strm << displayedCall			<< '\t'	//	 "Call"
		<< _score					<< '\t'	//	 "Final Score"
		<< NetQso()					<< '\t'	//	 "Net Qso"
		<< NetMults()				<< '\t'	//	 "Net Mults"
		<< _categoryOperator		<< '\t'	//	 "single/multi
		<< _categoryPower			<< '\t'	//	 "Power"
		<< _categoryBand			<< '\t'	//	 "Band"
		<< _location				<< '\t'	//	 "Loc"
		<< _hqClubAbbr				<< '\t'	//	 "Club"
		<< displayedOperators		<< '\t'	//	 "Operators"
		<< NetQso	("160M")		<< '\t'	//	 "160M"
		<< NetMults	("160M")		<< '\t'	//	 "160M"
		<< NetQso	("80M")			<< '\t'	//	 "80M"
		<< NetMults	("80M")			<< '\t'	//	 "80M"
		<< NetQso	("40M")			<< '\t'	//	 "40M"
		<< NetMults	("40M")			<< '\t'	//	 "40M"
		<< NetQso	("20M")			<< '\t'	//	 "20M"
		<< NetMults	("20M")			<< '\t'	//	 "20M"
		<< NetQso	("15M")			<< '\t'	//	 "15M"
		<< NetMults	("15M")			<< '\t'	//	 "15M"
		<< NetQso	("10M")			<< '\t'	//	 "10M"
		<< NetMults	("10M")			<< '\t'	//	 "10M"
		<< _cont					<< '\t'	//	 "Cont"
		<< tmpPx					<< '\t'	//	 "Cty Px"
		<< Division()				<< '\t'	//	 "Division"
		<< Region()					<< '\t'	//	 "Region"
		<< _district				<< '\t'	//	 "Dist"
		<< _certificate				<< '\t'	//	 "Cert"
		<< _claimedScore			<< '\t'	//	 "Claimed"
		<< RawQso()					<< '\t'	//	 "Qsos minus dupes
		<< RawMults()				<< '\t'	//	 "Raw Mult"
		<< _rawScore				<< '\t'	//	 "Raw Scr"
		<< Reduction()				<< '\t'	//	 "Reduct"	   
		<< DupeList().size()		<< '\t'	//	 "Dupes"
		<< BustedCallCount()		<< '\t'	//	 "B"
		<< NilList().size()			<< '\t'	//	 "NIL"
		<< BustedXchList().size()	<< '\t'	//	 "X"
		<< BandChangeViolationList().size() << '\t'	//	 "BC"
		<< UniList().size()			<< '\t'	//	 "Uni"
		<< errorRate				<< '\t'	//	 "Error Rate"
		<< _bustedCallCausedByTimeMap.size()<< '\t'	//	 "C B"
		<< _nilCausedByTimeMap.size()		<< '\t'	//	 "C NIL"
		<< _bustedXchCausedByTimeMap.size() << '\t'	//	 "C X"
		<< _category				<< '\t'	//	 "Category"
		<< _categoryOperator		<< '\t'	//	 "CatOp"
		<< _categoryTransmitter		<< '\t'	//	 "CatTx"
		<< _categoryTime			<< '\t'	//	 "CatTime"
		<< _categoryOverlay			<< '\t'	//	 "CatOverlay"
		<< _categoryBand			<< '\t'	//	 "CatBand"
		<< _categoryPower			<< '\t'	//	 "CatPwr"
		<< _categoryMode			<< '\t'	//	 "CatMode"
		<< _categoryAssisted		<< '\t'	//	 "CatAssisted"
		<< _categoryStation			<< '\t'	//	 "CatStn"
		<< _name					<< '\t'	//	 "Name"
		<< _hqFrom					<< '\t'	//	 "HQ From
		<< _hqClubAbbr				<< '\t'	//	 "HQ Club Abbreviation"
		<< _soapBox					<< '\t'	//	 "Soap Box"
		<< _createdBy				<< '\t'	//	 "Created By"
		<< OperatingTime()			<< endl;//	 "Op Hrs"
}

