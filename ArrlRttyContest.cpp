#include "StdAfx.h"
#include "ArrlRttyContest.h"

void ArrlRttyContest::WriteTxtHeader(QTextStream & strm) {strm << "Call	    Raw   D   N   B   X	Q   M   Final   Raw  Claimed   Delta  Hrs Cat\n";}

void ArrlRttyContest::WriteTsvHeader(QTextStream & ) {}

void ArrlRttyContest::WriteTabHeader(QTextStream & strm)
{ 
	strm <<  "Call"			<<   '\t'
		 <<  "Cty Px"		<<   '\t'
		 <<  "Cont"			<<   '\t'
		 <<  "Loc"			<<   '\t'
		 <<  "Div"			<<   '\t'
		 <<  "Reg"			<<   '\t'
		 <<  "Dist"			<<   '\t'
		 <<  "Cat"			<<   '\t'
		 <<  "Cert"			<<   '\t'
		 <<  "Claimed"		<<   '\t'
		 <<  "Raw Q"		<<   '\t'
		 <<  "Raw M"		<<   '\t'
		 <<  "Raw Scr"		<<   '\t'
		 <<  "Final Q"		<<   '\t'
		 <<  "Final M"		<<   '\t'
		 <<  "Final Scr"	<<   '\t'
		 <<  "Reduct"		<<   '\t'
		 <<  "Raw Q"		<<   '\t'
		 <<  "Dupes"		<<   '\t'
		 <<  "B"			<<   '\t'
		 <<  "NIL"			<<   '\t'
		 <<  "X"			<<   '\t'
		 <<  "BCV"			<<   '\t'
		 <<  "Uni"			<<   '\t'
		 <<  "C B"			<<   '\t'
		 <<  "C NIL"		<<   '\t'
		 <<  "C X"			<<   '\t'
		 <<  "Qso"			<<   '\t'
		 <<  "Mult"			<<   '\t'
		 <<  "160 Q"		<<   '\t'
		 <<  "160 M"		<<   '\t'
		 <<  "80 Q"			<<   '\t'
		 <<  "80 M"			<<   '\t'
		 <<  "40 Q"			<<   '\t'
		 <<  "40 M"			<<   '\t'
		 <<  "20 Q"			<<   '\t'
		 <<  "20 M"			<<   '\t'
		 <<  "15 Q"			<<   '\t'
		 <<  "15 M"			<<   '\t'
		 <<  "10 Q"			<<   '\t'
		 <<  "10 M"			<<   '\t'
		 <<  "CatOp"		<<   '\t'
		 <<  "CatTx"		<<   '\t'
		 <<  "CatTime"		<<   '\t'
		 <<  "CatOverlay"	<<   '\t'
		 <<  "CatBand"		<<   '\t'
		 <<  "CatPwr"		<<   '\t'
		 <<  "CatMode"		<<   '\t'
		 <<  "CatAssisted"	<<   '\t'
		 <<  "CatStn"		<<   '\t'
		 <<  "Club"			<<   '\t'
		 <<  "Operators"	<<   '\t'
		 <<  "Name"			<<   '\t'
		 <<  "Email"		<<   '\t'
		 <<  "HQ-FROM"		<<   '\t'
		 <<  "Soap Box"		<<   '\t'
		 <<  "Created By"	<<   '\t'
		 <<  "Op Hrs"		<<   '\t'
		 <<  "City"			<<   '\t'
		 <<  "State"		<<   '\t'
		 <<  "Postcode"		<<   '\t'
		 <<  "Country"		<<	 endl;
}

void ArrlRttyContest::Init()
{
	_mode = "RTTY";

	_bandNameList << B80.Name() << B40.Name() << B20.Name() << B15.Name() << B10.Name();
}

void ArrlRttyContest::DeriveExchInfo()
{
	InfoCountsMappedByCallAndInfo infoCountsMappedByCallAndInfo;

	foreach(Log * log, _logsByCall)
		foreach (QsoPtr q, log->AllQso())
			if (q->IsWVeRcvd())
				infoCountsMappedByCallAndInfo[q->CallRcvd()][q->XchRcvd()]++;

	QFile infoFile(_resDir.absoluteFilePath(Name().toLower() + ".info"));
	infoFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream infoStream(& infoFile);

	foreach(QString call, infoCountsMappedByCallAndInfo.keys())
	{
		InfoCountMap const & infoCountMap = infoCountsMappedByCallAndInfo.value(call);

		// find the info most often received for each call

		QMap< int, QString>   infoByCount;							  // reverse map so values are keys and sorted
		foreach(QString key, infoCountMap.keys())
			infoByCount[infoCountMap[key]] =  key;

		QMapIterator<int, QString> infoByCountIt(infoByCount);
		infoByCountIt.toBack();
		infoByCountIt.previous();
		InfoCountMap::const_iterator max = infoCountMap.find(infoByCountIt.value());

		int total = 0;												  // total number of times call worked
		foreach(int val, infoCountMap)
			total += val;

		QString outString = QString("%1 most common exchange is %2 %3 times")
			.arg(call. leftJustified(14))
			.arg(max.key(), 4)
			.arg(max.value(),4);
		infoStream << outString << "  " << endl;

		foreach(QString info, infoCountMap.keys())
			infoStream << QString("%1 %2").arg(info, 15).arg(infoCountMap.value(info)) << endl;

		Log * hisLog = GetLog(call);
		if(hisLog)
			foreach(QsoPtr q, hisLog->AllQso())
				q->XchSent(max.key());
		RevLog & revLog = _revLogMap[call];
		revLog.CorrectXch(max.key());
	}
	infoFile.close();
}

//void ArrlRttyContest::MarkCertificateWinners() 
//{
//	QMap <QString, QMap <Cty const *, Log *> >	logsByCatCty;  // mark certificate winners cat, cty for DX
//	QMap <QString, QMap <QString, Log *> >		logsByCatSec;  // mark certificate winners cat, section for W/VE
//
//	foreach(Log * log, _logsByCall)
//	{
//		if (!log->IsCertEligible())
//			continue;
//
//		QString cat = log->Category();
//		Cty const * cty = log->CtyPtr();
//		int score = log->Score();
//
//		if(cty != W_id && cty != VE_id)
//		{
//			if(!logsByCatCty[cat][cty] || score > logsByCatCty[cat][cty]->Score())
//				logsByCatCty[cat][cty] = log;
//		}
//		else
//		{
//			QString sec = log->Location();
//			if(gDistrictBySectionMap.contains(sec))
//			{
//				if(!logsByCatSec[cat][sec] || score > logsByCatSec[cat][sec]->Score())
//					logsByCatSec[cat][sec] = log;
//			}
//			else
//			{
//				log->AppendErrMsgList("SECTION:", log->Location());
//			}
//		}
//	}
//
//	foreach(QString cat, logsByCatCty.keys())
//		foreach(Cty const * cty, logsByCatCty[cat].keys())
//				logsByCatCty[cat][cty]->Certificate(true);
//
//	foreach(QString cat, logsByCatSec.keys())
//		foreach(QString sec, logsByCatSec[cat].keys())
//				logsByCatSec[cat][sec]->Certificate(true);
//}
