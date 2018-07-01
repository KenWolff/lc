#include "StdAfx.h"

void CqwwRttyContest::AcquireLogs()
{
	CqwwContest::AcquireLogs();
	foreach(Log * log, _logsByCall)		// assign zones after loading all the logs becasue we use the log's zone
		static_cast<CqwwRttyLog*>(log)->AssignQthsExpected();
}

void CqwwRttyContest::WriteSbxFile()
{
	QFile dxQrmFile(_resDir.absoluteFilePath(Name().toLower() + "_dx_qrm.mag"));
	dxQrmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream dxQrmStream(& dxQrmFile);

	QFile usaQrmFile(_resDir.absoluteFilePath(Name().toLower() + "_usa_qrm.mag"));
	usaQrmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream usaQrmStream(& usaQrmFile);

	foreach(Log * log, _logsByCall)
	{
		if (!log->SoapBox().isEmpty())
		{
			if (log->CtyPtr() == W_id)
				usaQrmStream << QString("%1<. . .>%2.\n").arg(log->SoapBox()).arg(log->Call());
			else
				dxQrmStream << QString("%1<. . .>%2.\n").arg(log->SoapBox()).arg(log->Call());
		}
	}
	usaQrmFile.close();
	dxQrmFile.close();
}

void CqwwRttyContest::WriteTabHeader(QTextStream & strm)
{
	strm <<  "Call"				<<	'\t'
		 <<  "Cty Px"			<<	'\t'
		 <<  "Cont"				<<	'\t'
		 <<  "Zone"				<<	'\t'
		 <<  "Loc"				<<	'\t'
		 <<  "Dist"				<<	'\t'
		 <<  "Cat"				<<	'\t'
		 <<  "Cert"				<<	'\t'
		 <<  "OverlayCert"		<<	'\t'
		 <<  "Mail Cert"		<<	'\t'
		 <<  "Late"				<<	'\t'
		 <<  "Claimed"			<<	'\t'
		 <<  "Raw Scr"			<<	'\t'
		 <<  "Final Scr"		<<	'\t'
		 <<	"Raw OverlayScr"	<<	'\t'
		 <<	"OverlayScr"		<<	'\t'
		 <<  "Reduct"			<<	'\t'
		 <<  "Final Q"			<<	'\t'
		 <<  "Final S"			<<	'\t'
		 <<  "Final Z"			<<	'\t'
		 <<  "Final C"			<<	'\t'
		 <<  "Dupes"			<<	'\t'
		 <<  "B"				<<	'\t'
		 <<  "NIL"				<<	'\t'
		 <<  "X"				<<	'\t'
		 <<  "ARM"				<<	'\t'
		 <<  "BCV"				<<	'\t'
		 <<  "Uni"				<<	'\t'
		 <<  "C B"				<<	'\t'
		 <<  "C NIL"			<<	'\t'
		 <<  "C X"				<<	'\t'
		 <<  "Qso"				<<	'\t'
		 <<  "Mult"				<<	'\t'
		 <<  "80 Q"				<<	'\t'
		 <<  "80 S"				<<	'\t'
		 <<  "80 Z"				<<	'\t'
		 <<  "80 C"				<<	'\t'
		 <<  "40 Q"				<<	'\t'
		 <<  "40 S"				<<	'\t'
		 <<  "40 Z"				<<	'\t'
		 <<  "40 C"				<<	'\t'
		 <<  "20 Q"				<<	'\t'
		 <<  "20 S"				<<	'\t'
		 <<  "20 Z"				<<	'\t'
		 <<  "20 C"				<<	'\t'
		 <<  "15 Q"				<<	'\t'
		 <<  "15 S"				<<	'\t'
		 <<  "15 Z"				<<	'\t'
		 <<  "15 C"				<<	'\t'
		 <<  "10 Q"				<<	'\t'
		 <<  "10 S"				<<	'\t'
		 <<  "10 Z"				<<	'\t'
		 <<  "10 C"				<<	'\t'
		 <<  "Qso Overlay"		<<	'\t'
		 <<  "States Overlay"	<<	'\t'
		 <<  "Zones Overlay"	<<	'\t'
		 <<  "Countries Overlay"<<	'\t'
		 <<  "80 Q Overlay"		<<	'\t'
		 <<  "80 S Overlay"		<<	'\t'
		 <<  "80 Z Overlay"		<<	'\t'
		 <<  "80 C Overlay"		<<	'\t'
		 <<  "40 Q Overlay"		<<	'\t'
		 <<  "40 S Overlay"		<<	'\t'
		 <<  "40 Z Overlay"		<<	'\t'
		 <<  "40 C Overlay"		<<	'\t'
		 <<  "20 Q Overlay"		<<	'\t'
		 <<  "20 S Overlay"		<<	'\t'
		 <<  "20 Z Overlay"		<<	'\t'
		 <<  "20 C Overlay"		<<	'\t'
		 <<  "15 Q Overlay"		<<	'\t'
		 <<  "15 S Overlay"		<<	'\t'
		 <<  "15 Z Overlay"		<<	'\t'
		 <<  "15 C Overlay"		<<	'\t'
		 <<  "10 Q Overlay"		<<	'\t'
		 <<  "10 S Overlay"		<<	'\t'
		 <<  "10 Z Overlay"		<<	'\t'
		 <<  "10 C Overlay"		<<	'\t'
		 <<  "CatOp"			<<	'\t'
		 <<  "CatTx"			<<	'\t'
		 <<  "CatTime"			<<	'\t'
		 <<  "CatOverlay"		<<	'\t'
		 <<  "CatBand"			<<	'\t'
		 <<  "CatPwr"			<<	'\t'
		 <<  "CatMode"			<<	'\t'
		 <<  "CatAssisted"		<<	'\t'
		 <<  "CatStn"			<<	'\t'
		 <<  "Club"				<<	'\t'
		 <<  "Operators"		<<	'\t'
		 <<  "Name"				<<	'\t'
		 <<  "Email"			<<	'\t'
		 <<  "HQ-FROM"			<<	'\t'
		 <<  "HQ-DATE-RECEIVED" <<	'\t'
		 <<  "Soap Box"			<<	'\t'
		 <<  "Created By"		<<	'\t'
		 <<  "Op Hrs"			<<	'\t'
		 <<  "City"				<<	'\t'
		 <<  "State"			<<	'\t'
		 <<  "Postcode"			<<	'\t'
		 <<  "Country"			<<	'\t'
		 <<  "Addr 1"			<<	'\t'
		 <<  "Addr 2"			<<	'\t'
		 <<  "Addr 3"			<<	'\t'
		 <<  "Addr 4"			<<	'\t'
		 <<  "Addr 5"			<<	'\t'
		 <<  "Addr 6"			<<	'\t'
		 << endl;
}

void CqwwRttyContest::Init()
{
	IsRtty(true);
	CqwwContest::Init();

	QFile statesOverrideFile("AliasStates.lis");
	if (statesOverrideFile.exists() && statesOverrideFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream(&statesOverrideFile);
		while (!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper().simplified();
			line = line.left(line.indexOf('#')).trimmed();
			QStringList parts = line.split(' ');
			if (parts.size() < 2)
				continue;

			if(gStateAbbreviationMap.contains(parts[1]))
				_aliasStateMap.insert(parts[0], parts[1]);
		}
		statesOverrideFile.close();
	}

	_mode = "RTTY";

	_sentRstFldNum  = 5;
	_sentXchFldNum  = 6;
	_sentXchFldNum2 = 7;
	_rcvdCallFldNum = 8;
	_rcvdRstFldNum  = 9;
	_rcvdXchFldNum  = 10;
	_rcvdXchFldNum2 = 11;
	_multStnFldNum  = 12;
	_numberOfFields = 12;

	// over write CQWW bands

	_bandNameList.clear();
	_bandNameList << B80.Name() << B40.Name() << B20.Name() << B15.Name() << B10.Name();
}

void CqwwRttyContest::WriteTxtHeader(QTextStream & strm)
{
	QString s = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 ")
			.arg("Call"		,-12)
			.arg("PX"		,5)
			.arg("CONT"		,4)
			.arg("Z"		,3)
			.arg("Raw Q"	,5)
			.arg("D"		,3)
			.arg("N"		,3)
			.arg("B"		,3)
			.arg("BCV"		,4);

	foreach(QString band, _bandNameList)
	{
		s += QString("%1 %2 %3 %4 %5 ")
			.arg(band		,4)
			.arg("Q"		,4)
			.arg("S"		,3)
			.arg("Z"		,3)
			.arg("C"		,3);
	}

	s += QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10\n")
			.arg("Q"		,5)
			.arg("S"		,3)
			.arg("Z"		,3)
			.arg("C"		,4)
			.arg("Final"	,8)
			.arg("Raw"		,8)
			.arg("Claimed"	,8)
			.arg("Delta"	,7)
			.arg("Time"		,4)
			.arg("Cat"		,3);
	strm << s << endl;
}

void CqwwRttyContest::MarkCertificateWinners()
{
	QMap <QString, QMap <Cty const *,  QMap <QString, Log *> > > logsByCatCtyDist;	// mark certificate winners cat, cty, district
	foreach(Log * log, _logsByCall)
	{
		if (!log->IsCertEligible())
			continue;

		QString cat = log->Category();
		Cty const * cty = log->CtyPtr();
		QString dist = log->District();
		int score = log->Score();

		if( !_ctysByDistrictSet.contains(cty))
			dist = '1';

		if(!logsByCatCtyDist[cat][cty][dist] || score > logsByCatCtyDist[cat][cty][dist]->Score())
			logsByCatCtyDist[cat][cty][dist] = log;
	}

	foreach(QString cat, logsByCatCtyDist.keys())
		foreach(Cty const * cty, logsByCatCtyDist[cat].keys())
			foreach(QString dist, logsByCatCtyDist[cat][cty].keys())
				logsByCatCtyDist[cat][cty][dist]->Certificate(true);
}
