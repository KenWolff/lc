#include "StdAfx.h"
typedef QMultiMap <int, QsoPtr> QsoByGap;
OcDxLog::OcDxLog() :Log() {}

void OcDxLog::ClearMultMaps()
{
	_multByBandPxMap.clear();
	_rawMultByBandPxMap.clear();
	_lostMultList.clear();
}

void OcDxLog::ComputeScore()
{
	ComputeOperatingTime();
	ComputeMults();
	ComputePts();
	SortLostMultList();

	_rawScore	= _bandInfoMap["ALL"].RawPts() * _bandInfoMap["ALL"].RawMults();
	_score		= _bandInfoMap["ALL"].NetPts() * _bandInfoMap["ALL"].NetMults();
}

void OcDxLog::FillPaperLogFromParts(QStringList parts)
{
	_call = parts[0];
	_category = parts[2];
	_claimedScore = parts[3];
	_score = parts[4].toInt();
	NetMults(parts[5].toInt());
	NetQso(parts[6].toInt());
	int i = 7;
	foreach(QString band, BandNameList())
		_bandInfoMap[band].NetQso(parts[i++].toInt());
	_mode = parts[13].toUpper();
	_club = parts[14];
	_operators = parts[15];
	_name = parts[16];
	_soapBox = parts[18];
	for(i = 19; i < 25; i++)
	{
		if( i < parts.size())
			_addressStreet += parts[i];
		if( i != 24)
			_addressStreet += '\t';
	}
}

void OcDxLog::WriteTabLine(QTextStream & strm)
{
	strm <<  _rawCall							<<   '\t'   //	 "Call"
		<<  _cty->StdPx()						<<   '\t'   //	 "Cty Px"
		<<  _cty->Name()						<<   '\t'   //	 "Cty name
		<<	_cont								<<   '\t'   //	 "Cont"
		<<  _location							<<   '\t'   //	 "Loc"
		<<  _district							<<   '\t'   //	 "Dist"
		<<  _category							<<   '\t'   //	 "Cat"
		<<  ContestPtr()->Mode().toUpper()		<<   '\t'   //     "mode"
		<<  _certificate						<<   '\t'   //	 "Cert"
		<<  _claimedScore						<<   '\t'   //	 "Claimed"
		<<  _rawScore							<<   '\t'   //	 "Raw Scr"
		<<  _score								<<   '\t'   //	 "Final Scr"
		<<  Reduction()							<<   '\t'   //	 "Reduct"
		<<  LogSize()							<<   '\t'   //	 "Raw Q"
		<<  DupeList().size()					<<   '\t'   //	 "Dupes"
		<<  BustedCallCount()					<<   '\t'   //	 "B"
		<<  NilList().size()					<<   '\t'   //	 "NIL"
		<<  BustedXchList().size()				<<   '\t'   //	 "X"
		<<  BandChangeViolationList().size()	<<   '\t'   //	 "BCV"
		<<  OutOfTimeList().size()				<<   '\t'   //	 "TV"
		<<  UniList().size()					<<   '\t'   //	 "Uni"
		<<  _bustedCallCausedByTimeMap.size()	<<   '\t'   //	 "C B"
		<<  _nilCausedByTimeMap.size()			<<   '\t'   //	 "C NIL"
		<<  _bustedXchCausedByTimeMap.size()	<<   '\t'   //	 "C X"
		<<  NetMults()							<<   '\t'   //	 "Mult"
		<<  NetQso("ALL")						<<   '\t'   //	 "Qso"
		<<  NetQso("160M")						<<   '\t'   //	 "160M"
		<<  NetQso("80M")						<<   '\t'   //	 "80M"
		<<  NetQso("40M")						<<   '\t'   //	 "40M"
		<<  NetQso("20M")						<<   '\t'   //	 "20M"
		<<  NetQso("15M")						<<   '\t'   //	 "15M"
		<<  NetQso("10M")						<<   '\t'   //	 "10M"
		<<  _categoryOperator					<<   '\t'   //	 "CatOp"
		<<  _categoryTransmitter				<<   '\t'   //	 "CatTx"
		<<  _categoryTime						<<   '\t'   //	 "CatTime"
		<<  _categoryOverlay					<<   '\t'   //	 "CatOverlay"
		<<  _categoryBand						<<   '\t'   //	 "CatBand"
		<<  _categoryPower						<<   '\t'   //	 "CatPwr"
		<<  _categoryMode						<<   '\t'   //	 "CatMode"
		<<  _categoryAssisted					<<   '\t'   //	 "CatAssisted"
		<<  _categoryStation					<<   '\t'   //	 "CatStn"
		<<  _club								<<   '\t'   //	 "Club"
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
		<<  _addressStreet						<<   endl;  //	 "Addr"
}

void OcDxLog::CheckBandChanges()
{
	if(IsMultiTwo())
		CheckM2BandChanges(10);

	if(IsMultiSingle())
		CheckMSBandChanges(10);
}

void OcDxLog::WriteRptSummary(QTextStream & strm)
{
	OcDxContest * contest = static_cast <OcDxContest *> (gContest);

	strm << QString("  Contest:   %1\n  %2      %3\n  %4  %5 \n  %6 %7\n")
		.arg(ContestPtr()->Name().toUpper())
		.arg("Call:")		.arg(_call)
		.arg("Category:")	.arg(contest->CatDescrFromNum(_categoryNum).Full())
		.arg("Location: ")	.arg(_location);

	QString operators = Operators();
	while(!operators.isEmpty() && Operators() != Call())
	{
		int idx = operators.lastIndexOf(",", 60);
		if(idx != -1)
		{
			strm << QString("%1%2\n").arg("  Operators: ").arg(operators.left(idx + 1));
			operators.remove(0, idx + 1);
			operators = operators.trimmed();
		}
		else
		{
			strm << QString("%1%2\n").arg("  Operators: ",  -12).arg(operators);
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

	strm << QString("%1 Claimed QSO before checking (does not include duplicates)\n").arg(RawQso(), 8);
	strm << QString("%1 Final   QSO after  checking reductions\n\n")				.arg(NetQso(), 8);
	strm << QString("%1 Claimed QSO points\n")										.arg(RawPts(), 8);
	strm << QString("%1 Final   QSO points\n\n")									.arg(NetPts(), 8);
	strm << QString("%1 Claimed mults\n")											.arg(RawMults(), 8);
	strm << QString("%1 Final   mults\n\n")											.arg(NetMults(), 8);
	strm << QString("%1 Claimed score\n")											.arg(RawScore(), 8);
	strm << QString("%1 Final   score\n")											.arg(Score(), 8);
	strm << QString("%1% Score reduction\n\n")										.arg(100.0 * double(Score() - RawScore()) / double(RawScore()) , 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final qso counts\n")		.arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) duplicates (without penalty)\n")						.arg(DupeCount(),		8)	.arg((100.0 * DupeCount())			/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n")							.arg(BustedCallCount(),	8)	.arg((100.0 * BustedCallCount())	/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) exchanges copied incorrectly\n")						.arg(BadXchCount(),		8)	.arg((100.0 * BadXchCount())		/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n")										.arg(NilCount(),		8)	.arg((100.0 * NilCount())			/ LogSize(), 3, 'f', 1);
	if(IsMultiTwo() || IsMultiSingle())	
		strm << QString("%1 (%2%) band change violations\n")						.arg(BandChangeViolationCount(),8).arg((100.0 * BandChangeViolationCount())/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls unique to this log only (not removed)\n")		.arg(UniCount(),   8)			.arg((100.0 * UniCount())			/ LogSize(), 3, 'f', 1);
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("             %1 %2 %3 %4\n\n")
			.arg("Band"			,4)
			.arg("QSO"			,5)
			.arg("QPts"			,5)
			.arg("Mult"			,5);

	foreach(QString band, BandNameList())
	{
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

	strm << QString("   Claimed   %1 %2 %3 %4  Score %5\n")
		.arg("All"			,4)
		.arg(RawQso()		,5)
		.arg(RawPts()		,5)
		.arg(RawMults()		,5)
		.arg(RawScore()		,8);

	strm << QString("   Final     %1 %2 %3 %4  Score %5\n")
		.arg("All"			,4)
		.arg(NetQso()		,5)
		.arg(NetPts()		,5)
		.arg(NetMults()		,5)
		.arg(Score()		,8);
}

QString OcDxLog::ResultLine()
{
	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8")
		.arg(_rawCall							,-12)
		.arg(LogSize()							,5)
		.arg(DupeList().size()					,3)
		.arg(NilList().size()					,3)
		.arg(BustedCallCount()					,3)
		.arg(BustedXchList().size()					,3)
		.arg(BandChangeViolationList().size()	,3)
		.arg(UniList().size()					,4);

	s += QString("%1 %2 %3 %4 %5 %6% %7 %8")
		.arg(_bandInfoMap["ALL"].NetQso()		,5)
		.arg(_bandInfoMap["ALL"].NetMults()		,4)
		.arg(Score()							,8)
		.arg(RawScore()							,8)
		.arg(ClaimedScore().left(8)				,8)
		.arg(100.0 * (Score() - RawScore())/RawScore(), 6, 'f', 1)
		.arg(OperatingTime()					,4, 'f', 1)
		.arg(Category()							,2);
	return s;
}

void OcDxLog::ComputeCategory()
{
	if (!_categoryBand.contains(QRegularExpression("160M|80M|40M|20M|15M|10M|ALL")))
	{
		_errMsgList.append(QString("%1 missing or incorrect CATEGORY-BAND (%2) setting to ALL").arg(Call(), -12).arg(_categoryBand));
		_categoryBand = "ALL";
	}

	QRegularExpression re("CHECKLOG|SINGLE-OP|MULTI-OP");
	if (!_categoryOperator.contains(re))
	{
		_errMsgList.append(QString("%1 missing or incorrect CATEGORY-OPERATOR (%2) setting to CHECKLOG").arg(Call(), -12).arg(_categoryOperator));
		_categoryOperator = "CHECKLOG";
	}

	if (_categoryOperator == "CHECKLOG")
	{
		_category = "CK";
		_categoryNum = 0;
		_categoryOverlay.clear();
		_isChecklog = true;
	}

	if (_categoryOperator == "SINGLE-OP")
	{
		if (_categoryBand != "ALL" && !ContestPtr()->BandNameList().contains(_categoryBand))
		{
			_errMsgList.append(QString("%1 missing or incorrect CATEGORY-BAND (%2) setting to ALL")
				.arg(Call())
				.arg(_categoryBand));
			_categoryBand = "ALL";
		}

		QString pwr;

		if (_categoryPower == "HIGH")
			pwr = "H";

		if (_categoryPower == "LOW")
			pwr = "L";

		if (_categoryPower == "QRP")
			pwr = "Q";

		if (pwr.isEmpty())
		{
			_errMsgList.append(QString("%1 missing or incorrect CATEGORY-POWER (%2) setting to HIGH").arg(Call()).arg(_categoryPower));
			_categoryPower = "HIGH";
			pwr = "H";
		}

		_category = QString("%1_%2_U")
			.arg(_categoryBand)
			.arg(pwr);
	}

	if (_categoryOperator == "MULTI-OP")
	{
		_categoryBand = "ALL";

		if (_categoryTransmitter == "ONE")
		{
			_isMultiSingle = true;
			_category = "MS";
		}
		else
		{
			_categoryPower = "HIGH";										// M2 and MM are both high power only
			if (_categoryTransmitter == "TWO")
			{
				_isMultiTwo = true;
				_category = "M2";
			}
			else
			{
				if (_categoryTransmitter == "UNLIMITED")
				{
					_category = "MM";
				}
				else
				{
					_errMsgList.append(QString("%1 missing or incorrect multi-op CATEGORY-TRANSMITTER (%2) setting to MM")
						.arg(_call, -12)
						.arg(_categoryTransmitter));
					_category = "MM";
				}
				_isMultiMulti = true;
			}
		}
	}

	_offtimeThreshold = 3600;		// one hour, in seconds
	_maxOperatingTime = INT_MAX;

	_categoryNum = static_cast <CqContest *> (gContest)->CatNumFromInternalDesc(_category);
}

void OcDxLog::ComputePts()
{
	foreach(QsoPtr q, _allQso)
	{
		if (Cont() != "OC" &&  q->Cont() != "OC")
			continue;

		if(q->IsDupe())
			continue;

		if(q->Status() == DOES_NOT_COUNT)
			continue;

		if(CategoryBand() != "ALL" && CategoryBand() != q->Band())
			continue;

		int pts = 0;

		if(q->Band() == "10M")
			pts = 3;
		else if (q->Band() == "15M")
			pts = 2;
		else if (q->Band() == "20M")
			pts = 1;
		else if (q->Band() == "40M")
			pts = 5;
		else if (q->Band() == "80M")
			pts = 10;
		else if (q->Band() == "160M")
			pts = 20;

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

		default:
			q->Pts(0);
			break;
		}
	}
}

void OcDxLog::WriteRptMults(QTextStream & stream)
{
	int count(0);
	stream << "\n";
	stream.setFieldWidth(62);
	stream.setPadChar('*');
	stream.setFieldAlignment(QTextStream::AlignCenter);
	stream << " Multipliers ";
	stream.setFieldWidth(0);
	stream << "\n\n";

	foreach(QString band, BandNameList())
	{
		if (!_multByBandPxMap.contains(band))
			continue;

		count = 0;
		stream << band << "\n";

		QStringList list = _multByBandPxMap.value(band).keys();
		QStringListIterator it(list);
		while(it.hasNext())
		{
			QString mult = it.next();
			count++;
			stream << QString("%1").arg(mult, 8);
			if (count % 5 == 0 && it.hasNext())
				stream << '\n';
		}
		stream << "\n";
	}
}

void OcDxLog::ComputeMults()
{
	ClearMultMaps();

	foreach(QsoPtr q, _allQso)
	{
		q->IsMult(false);
		q->IsRawMult(false);

		if (!q->CountsForRawCredit())
			continue;

		if (Cont() != "OC" &&  q->Cont() != "OC")					// at least one station must be in OC
			continue;

		if (q->IsOutOfTime() && IsSingleOp())
			continue;

		if (CategoryBand() != "ALL" && CategoryBand() != q->Band())	// single band logs on wrong band
			continue;

		QString multStr = q->WpxStr();								// mults are prefixes
		QString band = q->Band();

		_rawMultByBandPxMap[band][multStr].append(q);

		if (_rawMultByBandPxMap[band][multStr].size() == 1)
		{
			_bandInfoMap[band].IncrRawMults();
			_bandInfoMap["ALL"].IncrRawMults();
		}

		if (!q->CountsForCredit())
			continue;

		_multByBandPxMap[band][multStr].append(q);
		if (_multByBandPxMap[band][multStr].size() == 1)
		{
			_bandInfoMap["ALL"].IncrNetMults();
			_bandInfoMap[band].IncrNetMults();
			q->IsMult(true);
		}
	}

	foreach(QString band, _rawMultByBandPxMap.keys())
	{
		foreach(QString multStr, _rawMultByBandPxMap.value(band).keys())
		{
			if (!_multByBandPxMap.value(band).contains(multStr))
			{
				QsoPtr q = _rawMultByBandPxMap.value(band).value(multStr).first();
				q->IsLostMult(true);
				_lostMultList.append(LostMult(q, ""));
			}
		}
	}
}

void OcDxLog::WriteMultFile(QTextStream & strm)
{
	foreach(QString band, _multByBandPxMap.keys())
	{
		if (!_multByBandPxMap.value(band).size())
			continue;

		strm << QString("Multipliers on %1\n\n").arg(band);

		foreach(QString mult, _multByBandPxMap.value(band).keys())
		{
			QsoPtr q = _multByBandPxMap.value(band).value(mult).first();
			strm << QString("%1   %2").arg(q->ToString()).arg(mult) << endl;
		}
		strm << '\n';
	}
}

void OcDxLog::WriteLg(QTextStream & strm)
{
	static
		QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14")
		.arg("Freq")
		.arg("Mode")
		.arg("Date")
		.arg("Time")
		.arg("Call Sent")
		.arg("Ser Sent")
		.arg("Call Rcvd")
		.arg("Ser Rcvd")
		.arg("Cty Px")
		.arg("Stn")
		.arg("Pts")
		.arg("Status")
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
			.arg(q->StdPx())
			.arg(multStnStr)
			.arg(q->Pts())
			.arg(q->StatusChar())
			.arg(q->IsUnique() ? "U" : "")
			.arg(q->IsMult() ? q->WpxStr() : ((q->IsLostMult()) ? "L" : ""));
		strm << outStr;
	}
}
