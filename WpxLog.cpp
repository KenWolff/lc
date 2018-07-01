#include "StdAfx.h"
typedef QMultiMap <int, QsoPtr> QsoByGap;

QMap  <QString, QsoList>	_wpxMultMap;
QMap  <QString, QsoList>	_rawWpxMultMap;

WpxLog::WpxLog() : Log() {}

void WpxLog::CheckBandChanges()
{
	if (IsMultiTwo())
		CheckM2BandChanges(8);

	if (IsMultiSingle())
		CheckMSBandChanges(10);
}

void WpxLog::ClearMultMaps()
{
	_wpxMultMap.clear();
	_rawWpxMultMap.clear();
	_lostMultList.clear();
}

void WpxLog::ComputeCategory()
{
	QString assisted = (_categoryAssisted == "NON-ASSISTED") ? "U" : "A";

	if (!_categoryBand.contains(QRegularExpression("160M|80M|40M|20M|15M|10M|ALL")))
	{
		_errMsgList.append(QString("%1 missing or incorrect CATEGORY-BAND (%2) setting to ALL").arg(Call(), -12).arg(_categoryOperator));
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
	}

	if (_categoryOperator == "SINGLE-OP")
	{
		_offtimeThreshold = 3600;
		_maxOperatingTime = 36 * 3600;

		if (_categoryBand != "ALL" && !ContestPtr()->BandNameList().contains(_categoryBand))
		{
			_errMsgList.append(QString("%1 missing or incorrect Band Category (%2) setting to ALL")
				.arg(Call())
				.arg(_categoryBand));
			_categoryBand = "ALL";
		}

		if (!_categoryAssisted.contains(QRegularExpression("^ASSISTED$|^NON-ASSISTED$")))
		{
			_errMsgList.append(QString("%1 missing or incorrect Assisted Category (%2) setting to NON-ASSISTED").arg(Call()).arg(_categoryAssisted));
			_categoryAssisted = "NON-ASSISTED";
		}

		QString pwr("H");
		if (_categoryPower == "HIGH")
			pwr = "H";

		if (_categoryPower == "LOW")
			pwr = "L";

		if (_categoryPower == "QRP")
			pwr = "Q";

		_category = QString("%1_%2_%3")
			.arg(_categoryBand)
			.arg(pwr)
			.arg(assisted);
	}
	else
		_categoryOverlay.clear();


	if (_categoryOperator == "MULTI-OP")
	{
		_categoryBand = "ALL";

		if (_categoryTransmitter == "ONE")
		{
			_isMultiSingle = true;
			_category = "MSL";
			if (_categoryPower == "HIGH")
				_category = "MSH";
		}
		else
			if (_categoryTransmitter == "TWO")
			{
				_isMultiTwo = true;
				_category = "M2";
			}
			else
				if (_categoryTransmitter == "UNLIMITED")
				{
					_isMultiMulti = true;
					_category = "MM";
				}
				else
					_errMsgList.append(QString("Incorrect or missing CATEGORY-TRANSMITTER in multop: %1 %2 %3")
						.arg(_call, -12)
						.arg(_categoryTransmitter, -12)
						.arg(_categoryOperator, -12));
	}
	_categoryNum = static_cast <CqContest *> (gContest)->CatNumFromInternalDesc(_category);
}

void WpxLog::ComputeMults()
{
	ClearMultMaps();

	foreach(QsoPtr q, _allQso)
	{
		q->IsMult(false);
		q->IsRawMult(false);

		if (!q->CountsForRawCredit())
			continue;

		if (q->IsOutOfTime() && IsSingleOp())
			continue;

		if (CategoryBand() != "ALL" && CategoryBand() != q->Band())	// single band logs on wrong band
			continue;

		QString multStr = q->WpxStr();								// mults are prefixes

		_rawWpxMultMap[multStr].append(q);
		if (_rawWpxMultMap[multStr].size() == 1)
			_bandInfoMap["ALL"].IncrRawMults();

		if (q->Status() == GOOD_QSO || q->Status() == NO_LOG || _doNotCheck)
		{
			_wpxMultMap[multStr].append(q);
			if (_wpxMultMap[multStr].size() == 1)
			{
				_bandInfoMap["ALL"].IncrNetMults();
				q->IsMult(true);
			}
		}
	}

	foreach(QString multStr, _rawWpxMultMap.keys())
	{
		if (!_wpxMultMap.contains(multStr))
		{
			QsoPtr q = _rawWpxMultMap.value(multStr).first();
			q->IsLostMult(true);
			_lostMultList.append(LostMult(q, ""));
		}
	}
}

void WpxLog::ComputePts()
{
	foreach(QsoPtr q, _allQso)
	{
		if (q->IsOutOfTime() && IsSingleOp())
			continue;

		if (q->IsDupe())
			continue;

		if (q->Status() == DOES_NOT_COUNT)
			continue;

		if (CategoryBand() != "ALL" && CategoryBand() != q->Band())
			continue;

		int pts = 0;

		bool lowBand = (q->Band() == B160.Name() || q->Band() == B80.Name() || q->Band() == B40.Name());

		if (_cty == q->CtyRcvd())
			pts = 1;
		else
			if (_px->Cont() != q->CtyRcvd()->Cont())
				pts = lowBand ? 6 : 3;
			else
			{
				pts = lowBand ? 2 : 1;
				if (_px->Cont() == "NA")
					pts *= 2;
			}

		if (q->IsExcluded())
			pts = 0;

		_bandInfoMap[q->Band()].IncrRawQso();
		_bandInfoMap["ALL"].IncrRawQso();
		_bandInfoMap[q->Band()].AddRawPts(pts);
		_bandInfoMap["ALL"].AddRawPts(pts);

		switch (q->Status())
		{
		case NO_LOG:
		case GOOD_QSO:
			_bandInfoMap[q->Band()].IncrNetQso();
			_bandInfoMap["ALL"].IncrNetQso();
			_bandInfoMap[q->Band()].AddNetPts(pts);
			_bandInfoMap["ALL"].AddNetPts(pts);
			q->Pts(pts);
			break;

		case NOT_IN_LOG:
		case BUSTED_CALL:
		case BUSTED_REVLOG_CALL:
		case BUSTED_LIST_CALL:
			_bandInfoMap[q->Band()].AddNetPts(-pts);
			_bandInfoMap["ALL"].AddNetPts(-pts);
			q->Pts(-pts);
			break;

		default:
			q->Pts(0);
			break;
		}
	}
}

void WpxLog::WriteLg(QTextStream & strm)
{
	static
	QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15")
		.arg("Freq"			)
		.arg("Mode"			)
		.arg("Date"			)
		.arg("Time"			)
		.arg("Call Sent"	)
		.arg("Ser Sent"		)
		.arg("Call Rcvd"	)
		.arg("Ser Rcvd"		)
		.arg("Cty Px"		)
		.arg("Stn"			)
		.arg("Pts"			)
		.arg("Status"		)
		.arg("Run/S/P"		)
		.arg("Uni"			)
		.arg("Mult"			);

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
			.arg(q->IsUnique() ? "U"			: "")
			.arg(q->IsMult() ? q->StdPx()	: ((q->IsLostMult()) ? "L" : ""));
		strm << outStr;
	}
}

void WpxLog::WriteTabLine(QTextStream & strm)
{
	strm <<  _rawCall							<<	'\t'   //	"Call"
		<<  _cty->StdPx()						<<	'\t'   //	"Cty Px"
		<<	_cont								<<	'\t'   //	"Cont"
		<<  _location							<<	'\t'   //	"Loc"
		<<  _district							<<	'\t'   //	"Dist"
		<<  _category							<<	'\t'   //	"Cat"
		<<  _certificate						<<	'\t'   //	"Cert"
		<<  _overlayCertificate					<<	'\t'   //	"Overlay Cert"
		<<  _submittedLate						<<	'\t'   //	"Late"
		<<  _claimedScore						<<	'\t'   //	"Claimed"
		<<  _rawScore							<<	'\t'   //	"Raw Scr"
		<<  _score								<<	'\t'   //	"Final Scr"
		<<  Reduction()							<<	'\t'   //	"Reduct"
		<<  LogSize()							<<	'\t'   //	"Raw Q"
		<<  DupeList().size()					<<	'\t'   //	"Dupes"
		<<  BustedCallCount()					<<	'\t'   //	"B"
		<<  NilList().size()					<<	'\t'   //	"NIL"
		<<  BustedXchList().size()				<<	'\t'   //	"X"
		<<  BandChangeViolationList().size()	<<	'\t'   //	"BCV"
		<<  OutOfTimeList().size()				<<	'\t'   //	"TV"
		<<  UniList().size()					<<	'\t'   //	"Uni"
		<<  _bustedCallCausedByTimeMap.size()	<<	'\t'   //	"C B"
		<<  _nilCausedByTimeMap.size()			<<	'\t'   //	"C NIL"
		<<  _bustedXchCausedByTimeMap.size()	<<	'\t'   //	"C X"
		<<  NetMults()							<<	'\t'   //	"Mult"
		<<  NetQso("ALL")						<<	'\t'   //	"Qso"
		<<  NetQso("160M")						<<	'\t'   //	"160M"
		<<  NetQso("80M")						<<	'\t'   //	"80M"
		<<  NetQso("40M")						<<	'\t'   //	"40M"
		<<  NetQso("20M")						<<	'\t'   //	"20M"
		<<  NetQso("15M")						<<	'\t'   //	"15M"
		<<  NetQso("10M")						<<	'\t'   //	"10M"
		<<  _categoryOperator					<<	'\t'   //	"CatOp"
		<<  _categoryTransmitter				<<	'\t'   //	"CatTx"
		<<  _categoryTime						<<	'\t'   //	"CatTime"
		<<  _categoryOverlay					<<	'\t'   //	"CatOverlay"
		<<  _categoryBand						<<	'\t'   //	"CatBand"
		<<  _categoryPower						<<	'\t'   //	"CatPwr"
		<<  _categoryMode						<<	'\t'   //	"CatMode"
		<<  _categoryAssisted					<<	'\t'   //	"CatAssisted"
		<<  _categoryStation					<<	'\t'   //	"CatStn"
		<<  _club								<<	'\t'   //	"Club"
		<<  _operators							<<	'\t'   //	"Operators"
		<<  _name								<<	'\t'   //	"Name"
		<<  _email								<<	'\t'   //	"email"
		<<  _hqFrom								<<	'\t'   //	"HQ From
		<<  _soapBox							<<	'\t'   //	"Soap Box"
		<<  _createdBy							<<	'\t'   //	"Created By"
		<<  OperatingTime()						<<	'\t'   //	"Op Hrs"
		<<  _addressCity						<<	'\t'   //	"city"
		<<  _addressStateProvince				<<	'\t'   //	"state province"
		<<  _addressPostalCode					<<	'\t'   //	postal code
		<<  _addressCountry	                    <<	'\t'   //	country
		<<  _addressStreet						<<	endl;
}

void WpxLog::WriteRptSummary(QTextStream & strm)
{
	WpxContest * contest = static_cast <WpxContest *> (gContest);
	QString title = ContestPtr()->Title();
	if(title.isEmpty())
		title = ContestPtr()->Name().toUpper();

	strm << QString("%1%2\n%3%4\n%5%6\n")
		.arg("Contest:",	-12)	.arg(title)
		.arg("Call:",		-12)	.arg(Call())
		.arg("Category:",	-12)	.arg(contest->CatDescrFromNum(_categoryNum).Full());

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

	strm << '\n';
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Summary ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("%1 Claimed QSO before checking (does not include duplicates)\n").arg(RawQso(),		8);
	strm << QString("%1 Final   QSO after  checking reductions\n\n")				.arg(NetQso(),		8);
	strm << QString("%1 Claimed QSO points\n")										.arg(RawPts(),		8);
	strm << QString("%1 Final   QSO points\n\n")									.arg(NetPts(),		8);
	strm << QString("%1 Claimed mults\n")											.arg(RawMults(),	8);
	strm << QString("%1 Final   mults\n\n")											.arg(NetMults(),	8);
	strm << QString("%1 Claimed score\n")											.arg(RawScore(),	8);
	strm << QString("%1 Final   score\n")											.arg(Score(),		8);
	if(Reduction() < 0.15)
		strm << QString("%1% Score reduction\n\n")									.arg(100.0 * double(Score() - RawScore()) / double(RawScore()) , 7, 'f', 1);
	strm << QString("%1% Error Rate based on claimed and final qso counts\n")		.arg(100.0 * (double(RawQso() - NetQso()) / RawQso()), 7, 'f', 1);
	strm << QString("%1 (%2%) duplicates (without penalty)\n")						.arg(DupeCount(),		8)	.arg((100.0 * DupeCount())			/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls copied incorrectly\n")							.arg(BustedCallCount(), 8)	.arg((100.0 * BustedCallCount())	/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) exchanges copied incorrectly\n")						.arg(BadXchCount(),		8)	.arg((100.0 * BadXchCount())		/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) not in log\n")										.arg(NilCount(),		8)	.arg((100.0 * NilCount())			/ LogSize(), 3, 'f', 1);
	if(IsMultiTwo() || IsMultiSingle())
		strm << QString("%1 (%2%) band change violations\n").arg(BandChangeViolationCount(),8).arg((100.0 * BandChangeViolationCount())/ LogSize(), 3, 'f', 1);
	strm << QString("%1 (%2%) calls unique to this log only (not removed)\n").arg(UniCount(),   8).arg((100.0 * UniCount())	  / LogSize(), 3, 'f', 1);
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Results By Band ";

	strm.setFieldWidth(0);
	strm << "\n\n";

	strm << QString("            %1 %2 %3 %4\n\n")
			.arg("Band"		,4)
			.arg("QSO"		,5)
			.arg("QPts"		,6)
			.arg("Mult"		,5);

	foreach(QString band, BandNameList())
	{
		strm << QString("   Claimed  %1 %2 %3 %4\n")
			.arg(band			,4)
			.arg(RawQso (band)	,5)
			.arg(RawPts (band)	,6)
			.arg(""				,5);

		strm << QString("   Final    %1 %2 %3 %4\n\n")
			.arg(band			,4)
			.arg(NetQso (band)	,5)
			.arg(NetPts (band)	,6)
			.arg(""				,5);
	}

	strm << QString("  Claimed   %1 %2 %3 %4  Score %5\n")
		.arg("All"		,4)
		.arg(RawQso()	,5)
		.arg(RawPts()	,6)
		.arg(RawMults()	,5)
		.arg(RawScore()	,8);

	strm << QString("  Final     %1 %2 %3 %4  Score %5\n")
		.arg("All"		,4)
		.arg(NetQso()	,5)
		.arg(NetPts()	,6)
		.arg(NetMults()	,5)
		.arg(Score()	,8);
}

QString WpxLog::ResultLine()
{
	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8")
		.arg(_rawCall							,-12)
		.arg(LogSize()							,5)
		.arg(DupeList().size()					,3)
		.arg(NilList().size()					,3)
		.arg(BustedCallCount()					,3)
		.arg(BustedXchList().size()				,3)
		.arg(BandChangeViolationList().size()	,3)
		.arg(UniList().size()					,4);

	s += QString("%1 %2 %3 %4 %5 %6% %7 %8 %9")
		.arg(_bandInfoMap["ALL"].NetQso()		,5)
		.arg(_wpxMultMap.size()					,4)
		.arg(Score()							,8)
		.arg(RawScore()							,8)
		.arg(ClaimedScore()						,8)
		.arg(100.0 * (Score() - RawScore())/RawScore(), 6, 'f', 1)
		.arg(OperatingTime()					,4, 'f', 1)
		.arg(Category()							,2)
		.arg(Call()								,-12);
	return s;
}

void WpxLog::WriteRptMults(QTextStream & strm)
{
	int count(0);
	strm << "\n";
	strm.setFieldWidth(62);
	strm.setPadChar('*');
	strm.setFieldAlignment(QTextStream::AlignCenter);
	strm << " Multipliers ";
	strm.setFieldWidth(0);
	strm << "\n\n";

	foreach(QString mult, _wpxMultMap.keys())
	{
		count++;
		strm << QString("%1").arg(mult, 8);
		if(count % 5 == 0)
			strm << endl;
	}
	strm << endl;
}

void WpxLog::WriteMultFile(QTextStream & mulStream)
{
	foreach(QString mult, _wpxMultMap.keys())
	{
		QsoPtr q = _wpxMultMap.value(mult).first();
		mulStream << QString("%1   %2").arg(q->ToString()).arg(mult) << endl;
	}
}