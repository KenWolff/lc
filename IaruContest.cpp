#include "StdAfx.h"
#include <math.h>

void IaruContest::WriteTabFile()
{
	QFile tabFile(_resDir.absoluteFilePath(Name().toLower() + ".tab"));
	tabFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream outStream(&tabFile);
	outStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));

	WriteTabHeader(outStream);
	foreach(Log * log, _logsByCall)
		log->WriteTabLine(outStream);
	tabFile.close();
}

void IaruContest::WriteAllFile()
{
	if(!gWriteAllFile)
		return;

	QFile allQsoFile(_resDir.absoluteFilePath(Name().toLower() + "-all.txt"));
	allQsoFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream allQsoStream(& allQsoFile);
	allQsoStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));

	allQsoStream << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\n")
		.arg("Date")
		.arg("Time")
		.arg("Hour")
		.arg("Band")
		.arg("QRG")
		.arg("CallSent")
		.arg("LogZone")
		.arg("CallRcvd")
		.arg("ZoneRcvd")
		.arg("Status")
		.arg("Mult")
		.arg("StdPfx")
		.arg("Run/S&P/Pass")
		.arg("Cat");

	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr q, log->AllQso())
			allQsoStream << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\n")
			.arg(q->DateStr())
			.arg(q->TimeStr())
			.arg(StartTime().secsTo(q->DateTime()) /3600)
			.arg(q->Band())
			.arg(q->Qrg())
			.arg(q->CallSent())
			.arg(q->XchSent())
			.arg(q->CallRcvd())
			.arg(q->XchRcvd())
			.arg(q->StatusChar())
			.arg((q->IsMult() || q->IsMult2() || q->IsMult3() ? "M" : ((q->IsLostMult() || q->IsLostMult2() || q->IsLostMult3()) ? "L" : "")))
			.arg(q->CtyRcvd()->StdPx())
			.arg(q->Running())
			.arg(log->Category());
	}
	allQsoFile.close();
}

void IaruContest::WriteTsvHeader(QTextStream & ) 
{

}

void IaruContest::WriteTabHeader(QTextStream & strm)
{
	strm << "Call" 					<<	'\t'
		 <<	"Cty Px"				<<	'\t'
		 <<	"Cont"					<<	'\t'
		 <<	"Zone"					<<	'\t'
		 <<	"Loc"					<<	'\t'
		 <<	"Dist"					<<	'\t'
		 <<	"Cat"					<<	'\t'
		 <<	"Cert"					<<	'\t'
		 <<	"Mail Cert"				<<	'\t'
		 <<	"Late"					<<	'\t'
		 <<	"Claimed"				<<	'\t'
		 <<	"Raw Score"				<<	'\t'
		 <<	"Final Score"			<<	'\t'
		 <<	"Reduct"				<<	'\t'
		 << "Raw Q"					<<	'\t'
		 << "Final Q"				<<	'\t'
		 << "Raw HQ"				<<	'\t'
		 << "Final HQ"				<<	'\t'
		 << "Raw Z"					<<	'\t'
		 << "Final Z"				<<	'\t'
		 << "Raw C"					<<	'\t'
		 << "Final C"				<<	'\t'
		 <<	"Dupes"					<<	'\t'
		 <<	"B"						<<	'\t'
		 <<	"NIL"					<<	'\t'
		 <<	"X"						<<	'\t'
		 <<	"ARM"					<<	'\t'
		 <<	"BCV"					<<	'\t'
		 << "Uni"					<<  '\t'
		 << "% Uni"					<<  '\t'
		 <<	"C B"					<<	'\t'
		 <<	"C NIL"					<<	'\t'
		 <<	"C X"					<<	'\t'
		 <<	"Qso"					<<	'\t'
		 <<	"Mult"					<<	'\t'
		
		<< "160 Q" << '\t'
		<< "160 Z" << '\t'
		<< "160 H" << '\t'
		<< "160 C" << '\t'

		<< "80 Q" << '\t'
		<< "80 Z" << '\t'
		<< "80 H" << '\t'
		<< "80 C" << '\t'

		<< "40 Q" << '\t'
		<< "40 Z" << '\t'
		<< "40 H" << '\t'
		<< "40 C" << '\t'

		<< "20 Q" << '\t'
		<< "20 Z" << '\t'
		<< "20 H" << '\t'
		<< "20 C" << '\t'

		<< "15 Q" << '\t'
		<< "15 Z" << '\t'
		<< "15 H" << '\t'
		<< "15 C" << '\t'

		<< "10 Q" << '\t'
		<< "10 Z" << '\t'
		<< "10 H" << '\t'
		<< "10 C" << '\t'

		 <<	"Op Hrs"				<<	'\t'
		 << "Final QTR"				<<	'\t'
		 <<	"CatOp"					<<	'\t'
		 <<	"CatTx"					<<	'\t'
		 <<	"CatOverlay"			<<	'\t'
		 <<	"CatBand"				<<	'\t'
		 <<	"CatPwr"				<<	'\t'
		 <<	"CatMode"				<<	'\t'
		 <<	"CatAssisted"			<<	'\t'
		 <<	"CatStn"				<<	'\t'
		 <<	"Club"					<<	'\t'
		 <<	"Operators"				<<	'\t'
		 <<	"Name"					<<	'\t'
		 <<	"Email"					<<	'\t'
		 <<	"HQ-FROM"				<<	'\t'
		 <<	"HQ-DATE-RECEIVED"		<<	'\t'
		 <<	"Soap Box"				<<	'\t'
		 <<	"Created By"			<<	'\t'
		 <<	"City"					<<	'\t'
		 <<	"State"					<<	'\t'
		 <<	"Postcode"				<<	'\t'
		 <<	"Country"				<<	'\t'
		 <<	"Addr 1"				<<	'\t'
		 <<	"Addr 2"				<<	'\t'
		 <<	"Addr 3"				<<	'\t'
		 <<	"Addr 4"				<<	'\t'
		 <<	"Addr 5"				<<	'\t'
		 <<	"Addr 6"				<<	'\t'
		 << endl;
}

void IaruContest::WriteHqCountsFile()
{
	IaruContest const * contest = static_cast<IaruContest const *>(this);

	QFile hqCountsFile(_resDir.absoluteFilePath(Name().toLower() + ".PotentialHqStns"));
	hqCountsFile.open(QIODevice::WriteOnly | QIODevice::Text);

	QTextStream hqCountsStrm;
	hqCountsStrm.setDevice(&hqCountsFile);

	typedef QPair <QString, QString> StrPair;

	QMap <StrPair, int> countByCallHq;
	QMultiMap <int, StrPair> callHqByCount;

	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr p, log->AllQso())
		{
			IaruQso * q = static_cast<IaruQso *> (p);
			if (q->ZoneRcvd())
				continue;

			if (contest->IsHqFromCall(q->CallRcvd()))
				continue;

			if (contest->IsHqFromAbrv(q->RawXchRcvd()))
				continue;

			StrPair pair = StrPair(q->CallRcvd(), q->RawXchRcvd());
			countByCallHq[pair]++;
		}
	}

	foreach(auto key, countByCallHq.keys())
		callHqByCount.insert(countByCallHq[key], key);

	QMapIterator<int, StrPair> it(callHqByCount);
	it.toBack();
	while (it.hasPrevious())
	{
		it.previous();
		hqCountsStrm << QString("%1 %2 %3").arg(it.value().first, -12).arg(it.value().second, -5).arg(it.key()) << endl;
	}
}

void IaruContest::Init()
{
	_bandNameList
		<< B160.Name() + " CW"
		<< B160.Name() + " PH"
		<< B80.Name() + " CW"
		<< B80.Name() + " PH"
		<< B40.Name() + " CW"
		<< B40.Name() + " PH"
		<< B20.Name() + " CW"
		<< B20.Name() + " PH"
		<< B15.Name() + " CW"
		<< B15.Name() + " PH"
		<< B10.Name() + " CW"
		<< B10.Name() + " PH";

	// read in the list of HQ abbreviations
	QFile AbrvFile("IaruHqAbbrevs.lis");
	if (AbrvFile.exists())
	{
		AbrvFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream inStrm(&AbrvFile);
		while (!inStrm.atEnd())
		{
			QString line = inStrm.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_hqAbrvSet.insert(line);
		}
		AbrvFile.close();
	}

	// read in the list of HQ stations
	QFile HqFile("IaruHqStns.lis");
	if (HqFile.exists() && HqFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream(&HqFile);
		while (!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			QStringList parts = line.split(QRegularExpression("\\s+"));
			if (parts.size() == 2)
			{
				_hqCallByAbrvMap.insert(parts[1], parts[0]);
				_hqAbrvByCallMap.insert(parts[0], parts[1]); ;
			}
		}
		HqFile.close();
	}

	// read in the list of WRTC stations
	QFile wrtcFile("Wrtc.lis");
	if (wrtcFile.exists() && wrtcFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream(&wrtcFile);
		while (!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_wrtcStnMap.insert(line, "");
		}
		wrtcFile.close();
	}
}

void IaruContest::WriteTxtHeader(QTextStream & strm)
{
	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
			.arg("Call"		,-12)
			.arg("PX"		,5)
			.arg("CONT"		,4)
			.arg("Z"		,3)
			.arg("Raw Q"	,5)
			.arg("D"		,3)
			.arg("N"		,3)
			.arg("B"		,3)
			.arg("BCV"		,4)
			.arg("U"		,3);
	
	s += QString("%1 %2 %3 %4 %5 %6 %7% %8 %9\n")
			.arg("Q"		,5)
			.arg("HQ"		,4)
			.arg("Z"		,3)
			.arg("Claimed"	,8)
			.arg("Raw"		,8)
			.arg("Final"	,8)
			.arg("Delta"	,6)
			.arg("Time"		,4)
			.arg("Cat"		,3);

	strm << s << endl;
}

bool	IaruContest::IsHqFromAbrv		(QString abrv)	const { return !HqCallFromAbrv(abrv).isEmpty();	}
bool	IaruContest::IsHqFromCall		(QString call)	const { return !HqAbrvFromCall(call).isEmpty();	}
bool	IaruContest::IsWrtcStn			(QString call)	const { return _wrtcStnMap.contains(call);		}
bool	IaruContest::IsHqAbrv			(QString abrv)	const { return _hqAbrvSet.contains(abrv);		}
QString	IaruContest::HqAbrvFromCall		(QString call)	const { return _hqAbrvByCallMap.value(call);	}
QString	IaruContest::HqCallFromAbrv		(QString abrv)	const { return _hqCallByAbrvMap.value(abrv);	}

