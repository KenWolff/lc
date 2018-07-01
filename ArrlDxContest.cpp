#include "StdAfx.h"

ArrlDxContest::ArrlDxContest() : Contest() {}

//void ArrlDxContest::MarkCertificateWinners() 
//{
//	QMap <QString, QMap <QString, Log *>  >		logsByCatSec;		// W and VE single op
//	QMap <QString, QMap <QString, Log *>  >		logsByCatDist;		// W multiop by district
//	QMap <QString, Log *>  						logsByCatVe;		// VE multiop
//	QMap <QString, QMap <Cty const *, Log *> >	logsByCatCty;		// DX single op
//	QMap <QString, QMap <Cty const *, Log *> >	logsByCatCtyMulti;	// DX multi-op by country
//	QSet <Log *>								bigDxScoresSet;		// DX with more than 500 qso
//
//	foreach(Log * log, _logsByCall)
//	{
//		if(!log->IsCertEligible())
//			continue;
//
//		QString cat		= log->Category();
//		Cty const * cty	= log->CtyPtr();
//		QString dist	= log->District();
//		QString loc		= log->Location();
//		int score		= log->Score();
//
//		if(log->DoNotCheck())
//			continue;
//
//		if( cty == W_id || cty == VE_id)
//		{
//			if(cat == "A" || cat == "B" || cat == "C" || cat == "10" || cat == "15" || cat == "20" || 
//				cat == "40" || cat == "80" || cat == "160" || cat == "SAQ" || cat == "SAL" || cat == "SAH")
//			{
//				if(logsByCatSec[cat][loc] == 0 || score > logsByCatSec[cat][loc]->Score())
//					logsByCatSec[cat][loc] = log;
//			}
//
//			if(cat == "MSL" || cat == "MSH" || cat == "M2" || cat == "MM") 
//			{
//				if((cty == W_id) && (logsByCatDist[cat][dist] == 0 || score > logsByCatDist[cat][dist]->Score()))
//					logsByCatDist[cat][dist] = log;
//
//				if((cty == VE_id) && (!logsByCatVe.contains(cat) || score > logsByCatVe[cat]->Score()))
//					logsByCatVe[cat] = log;
//			}
//		}
//		else
//		{
//			if(cat == "A" || cat == "B" || cat == "C" || cat == "10" || cat == "15" || cat == "20" || 
//				cat == "40" || cat == "80" || cat == "160" || cat == "SAQ" || cat == "SAL" || cat == "SAH")
//			{
//				if(logsByCatCty[cat][cty] == 0 ||score > logsByCatCty[cat][cty]->Score())
//					logsByCatCty[cat][cty] = log;
//			}
//
//			if(cat == "MSL" || cat == "MSH" || cat == "M2" || cat == "MM") 
//			{
//				if(logsByCatCtyMulti[cat][cty] == 0 || score > logsByCatCtyMulti[cat][cty]->Score())
//					logsByCatCtyMulti[cat][cty] = log;
//			}
//
//			if(log->QsoCount() >= 500)
//				bigDxScoresSet.insert(log);
//		}
//	}
//
//	foreach(QString cat, logsByCatSec.keys())					// W and VE single op
//		foreach(QString sec, logsByCatSec[cat].keys())
//		{
//			if(!logsByCatSec.value(cat).value(sec))
//				gStdErrStrm << QString("Bad log  cat %1  sec %2").arg(cat).arg(sec) << endl;
//			else
//				logsByCatSec[cat][sec]->Certificate(true);
//		}
//
//	foreach(QString cat, logsByCatDist.keys())					// multi-op by district
//		foreach(QString dist, logsByCatDist[cat].keys())
//		{
//			if(!logsByCatDist.value(cat).value(dist))
//				gStdErrStrm << QString("Bad W multiop log  cat %1  dist %2").arg(cat).arg(dist) << endl;
//			else
//				logsByCatDist[cat][dist]->Certificate(true);
//		}
//
//	foreach(QString cat, logsByCatVe.keys())					// muti-op for all of Canada
//	{
//		if(!logsByCatVe.value(cat))
//			gStdErrStrm << QString("Bad VE multi op log cat %1").arg(cat) << endl;
//		else
//			logsByCatVe[cat]->Certificate(true);
//	}
//
//	foreach(QString cat, logsByCatCty.keys())					// DX single-op
//		foreach(Cty const * cty, logsByCatCty[cat].keys())
//		{
//			if(!logsByCatCty.value(cat).value(cty))
//				gStdErrStrm << QString("Bad DX single op log  cat %1  dist %2").arg(cat).arg(cty->StdPx()) << endl;
//			else
//				logsByCatCty[cat][cty]->Certificate(true);
//		}
//
//	foreach(Log * log, bigDxScoresSet)
//	{
//		if(!log)
//				gStdErrStrm << QString("Bad big DX log") << endl;
//		else
//			log->Certificate(true);
//	}
//
//}

void ArrlDxContest::WriteTabFile()
{
	QFile wVeTabFile(_resDir.absoluteFilePath(Name().toLower() + "-WVE.tab"));
	wVeTabFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream wVeTabStream(& wVeTabFile);

	QFile dxTabFile(_resDir.absoluteFilePath(Name().toLower() + "-DX.tab"));
	dxTabFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream dxTabStream(&dxTabFile);

	WriteTabHeader(wVeTabStream);
	WriteTabHeader(dxTabStream);

	foreach(Log * log, _logsByCall)
	{
		if (log->IsWVe())
			log->WriteTabLine(wVeTabStream);
		else
			log->WriteTabLine(dxTabStream);
	}

	wVeTabFile.close();
	dxTabFile.close();
}

void ArrlDxContest::WriteTabHeader(QTextStream & strm)
{ 
	strm <<  "CALL"			<<	'\t'
		 <<  "SCORE"		<<	'\t'
		 <<  "QSO"			<<	'\t'
		 <<  "MULTS"		<<	'\t'
		 <<  "SM"			<<	'\t'
		 <<  "POWER"		<<	'\t'
		 <<  "BAND"			<<	'\t'
		 <<  "SEC"			<<	'\t'
		 <<  "CLUB"			<<	'\t'
		 <<  "MO_CALLS"		<<	'\t'
		 <<  "160 Q"		<<	'\t'
		 <<  "160 M"		<<	'\t'
		 <<  "80 Q"			<<	'\t'
		 <<  "80 M"			<<	'\t'
		 <<  "40 Q"			<<	'\t'
		 <<  "40 M"			<<	'\t'
		 <<  "20 Q"			<<	'\t'
		 <<  "20 M"			<<	'\t'
		 <<  "15 Q"			<<	'\t'
		 <<  "15 M"			<<	'\t'
		 <<  "10 Q"			<<	'\t'
		 <<  "10 M"			<<	'\t'
		 <<  "CONT"			<<	'\t'
		 <<  "Cty Px"		<<	'\t'
		 <<  "Divison"		<<	'\t'
		 <<  "Region"		<<	'\t'
		 <<  "Dist"			<<	'\t'
		 <<  "Cert"			<<	'\t'
		 <<  "Claimed"		<<	'\t'
		 <<  "Raw Q"		<<	'\t'
		 <<  "Raw M"		<<	'\t'
		 <<  "Raw Scr"		<<	'\t'
		 <<  "Reduct"		<<	'\t'
		 <<  "Dupes"		<<	'\t'
		 <<  "B"			<<	'\t'
		 <<  "NIL"			<<	'\t'
		 <<  "X"			<<	'\t'
		 <<  "BC"			<<	'\t'
		 <<  "Uni"			<<	'\t'
		 <<  "% error"		<<	'\t'
		 <<  "C B"			<<	'\t'
		 <<  "C NIL"		<<	'\t'
		 <<  "C X"			<<	'\t'
		 <<  "Category"		<<	'\t'
		 <<  "CatOp"		<<	'\t'
		 <<  "CatTx"		<<	'\t'
		 <<  "CatTime"		<<	'\t'
		 <<  "CatOverlay"	<<	'\t'
		 <<  "CatBand"		<<	'\t'
		 <<  "CatPwr"		<<	'\t'
		 <<  "CatMode"		<<	'\t'
		 <<  "CatAssisted"	<<	'\t'
		 <<  "CatStn"		<<	'\t'
		 <<  "Name"			<<	'\t'
		 <<  "Email"		<<	'\t'
		 <<  "Soap Box"		<<	'\t'
		 <<  "Created By"	<<	'\t'
		 <<  "Op Hrs"		<<	endl;
}

void ArrlDxContest::WriteTsvHeader(QTextStream & ) {}

void ArrlDxContest::DeriveExchInfo()
{
	return; // FIX THIS
	InfoCountsMappedByCallAndInfo infoCountsMappedByCallAndInfo;

	foreach(Log * log, _logsByCall)
		foreach (QsoPtr q, log->AllQso())
			infoCountsMappedByCallAndInfo[q->CallRcvd()][q->XchRcvd()]++;

	QFile infoFile(_resDir.absoluteFilePath(Name().toLower() + ".info"));
	infoFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream infoStream(& infoFile);

	QFile serFile(_resDir.absoluteFilePath(Name().toLower() + ".ser"));
	serFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream serStream(& serFile);

	foreach(QString call, infoCountsMappedByCallAndInfo.keys())
	{
		InfoCountMap const & infoCountMap = infoCountsMappedByCallAndInfo.value(call);
		Log * hisLog = GetLog(call);

		// find the info most often received for each call

		QMap< int, QString>   infoByCount;							  // reverse map so values are keys and sorted
		foreach(QString key, infoCountMap.keys())
			infoByCount[infoCountMap.value(key)] =  key;

		QMapIterator<int, QString> infoByCountIt(infoByCount);
		infoByCountIt.toBack();
		infoByCountIt.previous();
		InfoCountMap::const_iterator max = infoCountMap.find(infoByCountIt.value()); // last entry has biggest count

		int total = 0;												  // total number of times call worked
		foreach(int val, infoCountMap)
			total += val;

		QString testStr = "123456789";	   // test for single digit powers to see if serial numbers were sent
		int matchCount = 0;
		foreach(QChar c, testStr)
			if(infoCountMap.contains(QString(c)))
				matchCount++;

		// if three or more numbers from 1 to 9 appear, he sent serial numbers, so give a free pass
		// the most common value must appear at least 90% of the time, or give a free pass

		if(matchCount > 2 || (double(max.value()) / double(total) < 0.90))
		{
			serStream << qSetFieldWidth(10) << call << " " << qSetFieldWidth(4) << double(max.value()) << " " << qSetFieldWidth(4) << total << " " << double(max.value()) / double(total) << endl;
			_badInfoSentCallSet.insert(call);
		}
//else
		{
			infoStream <<  QString("%1 most common exchange is %2 %3 times").arg(call, -14).arg(max.key(), 4).arg(max.value(),4) << endl;
			foreach(QString info, infoCountMap.keys())
				infoStream << QString("%1 %2").arg(info, 15).arg(infoCountMap.value(info)) << endl;
		}

		if(hisLog)
			foreach (QsoPtr q, hisLog->AllQso())
				q->XchSent(max.key());

		if(_revLogMap.find(call) != _revLogMap.end())
		{
	        RevLog & revLog = _revLogMap[call];
		    revLog.CorrectXch(max.key());
		}
	}
	serFile.close();
	infoFile.close();
}

void ArrlDxContest::Init()
{
	if(_mode == "PH")
		_mode = "SSB";

	if(_mode != "CW" && _mode != "SSB")
		AppendErrMsgList("MODE:", _mode + "must be CW or SSB");

	_bandNameList << B160.Name() << B80.Name() << B40.Name() << B20.Name() << B15.Name() << B10.Name();
}

void ArrlDxContest::WriteTxtHeader(QTextStream & strm) { strm << "Call            Raw Q    D   N   B   X  BCV   QSO  Mults     Final  Unchecked  Claimed   Delta   Hours  Cat\n"; }

