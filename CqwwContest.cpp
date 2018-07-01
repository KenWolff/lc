#include "StdAfx.h"
#include <math.h>

void CqwwContest::AcquireLogs()
{
	Contest::AcquireLogs();
	foreach(Log * log, _logsByCall)		// assign zones after loading all the logs becasue we use the log's zone
		static_cast<CqwwLog*>(log)->AssignZones();
}

void CqwwContest::WriteTabFile()
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

void CqwwContest::WriteWorkedBustedSpotFile()
{
	QString bustedSpotsFileName("BustedSpots.tab");
	QFile bustedSpotsFile(bustedSpotsFileName);
	bustedSpotsFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream bustedSpotstream(& bustedSpotsFile);
	bustedSpotstream.setCodec(QTextCodec::codecForName("ISO 8859-1"));

	int count(0);
	QMap <QString, QMap <QString, QMap <QDateTime, Spot * > > > bustedSpotbyBandCallTime;
	QList <Spot *> bustedSpotList;

	while(!bustedSpotstream.atEnd())		// read in busted spots 
	{
		QString line = bustedSpotstream.readLine();
		QStringList parts = line.split('\t');
		if(parts.size() < 4)
			continue;

		double qrg		= parts[0].trimmed().toDouble();
		QString dtStr	= parts[1] + " " + parts[2];
		QDateTime dt = QDateTime::fromString(dtStr, gLcDateTimeFormat);
		dt.setTimeSpec(Qt::UTC);

		QString call	= parts[3].simplified();
		QString correct	= parts[4];
		
		Spot * bustedSpot = new Spot("", qrg, dt, call);
		bustedSpot->CorrectCall(correct);
		bustedSpotList.append(bustedSpot);
		bustedSpotbyBandCallTime[bustedSpot->Band()][bustedSpot->Call()][bustedSpot->DateTime()] = bustedSpot;
		count++;
	}
	gStdOutStrm << QString("%1 busted spots read").arg(count) << endl;
	bustedSpotsFile.close();

	QString bustFileName(_resDir.absoluteFilePath(Name().toLower() + "-worked-spot-bust.tab"));
	QFile bustFile(bustFileName);

	bustFile.open(QIODevice::WriteOnly | QIODevice::Text);

	QTextStream bustStream(& bustFile);
	bustStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));

	QString outLineDefinition = "%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t\t%10\t%11\t%12\t%13\t%14\n";
	bustStream << QString(outLineDefinition)
		.arg("World")
		.arg("EU")
		.arg("US")
		.arg("Cat")
		.arg("QRG")
		.arg("Date")
		.arg("Time")
		.arg("Call Tx")
		.arg("Call Rx")
		.arg("Spot QRG")
		.arg("Spot Date")
		.arg("Spot Time")
		.arg("Spot Call")
		.arg("Correct Call");

	int maxReported = INT_MAX;

	foreach(Log * log, _logsByCall)
	{
		if(!log->Category().contains(QRegularExpression("U$")))
			continue;

		foreach(Spot * sp, bustedSpotList)
		{
			foreach(Qso * q, log->GetQsoListByCallBand(sp->Call(), sp->Band()))
			{
				int diff = sp->DateTime().secsTo(q->DateTime());
				if(diff < 0 || diff > 300)						// claimed within five minutes
					continue;

				if(q->IsMyRun())
					continue;

				if(fabs(float(q->Qrg()) - float(sp->Qrg())) > 0.2)
					continue;

				if(log->WorldFinish() <= maxReported)
				{
					bustStream << QString(outLineDefinition)
					.arg(log->WorldFinish())
					.arg(log->Cont() == "EU" ? QString("%1").arg(log->EuFinish()) : "")
					.arg(log->CtyPtr() == W_id ? QString("%1").arg(log->UsaFinish()) : "")
					.arg(q->GetLogPtr()->Category())
					.arg(q->Qrg())
					.arg(q->DateStr())
					.arg(q->TimeStr())
					.arg(q->CallSent())
					.arg(q->CallRcvd())
					.arg(sp->Qrg())
					.arg(sp->DateStr())
					.arg(sp->TimeStr())
					.arg(sp->Call())
					.arg(sp->CorrectCall());
				}
			}
		}
	}
}

void CqwwContest::WriteAllFile()
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

void CqwwContest::MarkCertificateWinners() 
{
	QMap <QString, QMap <Cty const *,  QMap <int, Log *> > > logsByCatCtyDist;			// mark certificate winners cat, cty, district
	QMap <Cty const *, QMap <int,  QMap <int, CqwwLog *> > > bestRkLogByCtyDistPwr;
	QMap <Cty const *, QMap <int,  QMap <int, CqwwLog *> > > bestClassicLogByCtyDistPwr;

	foreach(Log * l, _logsByCall)
	{
		CqwwLog * log = static_cast<CqwwLog*>(l);

		if (!log->IsCertEligible())
			continue;

		QString cat = log->Category();
		if(cat == "DQ" || cat == "CK" || log->SubmittedLate())
			continue;

		int magOrderNum = log->CategoryNum();
		if(magOrderNum == 10000)
			continue;

		Cty const * cty		= log->CtyPtr();
		int dist			= log->District().toInt();
		int pwr				= log->PwrNum();
		int score			= log->Score();
		int overlayScore	= log->OverlayScore();

		if(!_ctysByDistrictSet.contains(cty))
			dist = 1;

		if(!logsByCatCtyDist[cat][cty][dist] || score > logsByCatCtyDist[cat][cty][dist]->Score())
			logsByCatCtyDist[cat][cty][dist] = log;

		// overlay cats are by cty,  district, power and band

		if(log->IsOverlay())
		{
			if(pwr == Q)
				pwr = L;

			// compare current overlay log score to the the one left in overlayLogsByCatCtyDist[cat][cty][dist]->OverlayScore()
			if(log->IsOverlayRookie())
				if( !bestRkLogByCtyDistPwr[cty][dist][pwr] || overlayScore > bestRkLogByCtyDistPwr[cty][dist][pwr]->OverlayScore())
					bestRkLogByCtyDistPwr[cty][dist][pwr] = log;

			if(log->IsOverlayClassic())
				if( !bestClassicLogByCtyDistPwr[cty][dist][pwr] || overlayScore > bestClassicLogByCtyDistPwr[cty][dist][pwr]->OverlayScore())
					bestClassicLogByCtyDistPwr[cty][dist][pwr] = log;
		}
	}

	foreach(Cty const * cty, bestRkLogByCtyDistPwr.keys())
		foreach(int dist, bestRkLogByCtyDistPwr[cty].keys())
			foreach(int pwr, bestRkLogByCtyDistPwr[cty][dist].keys())
			        bestRkLogByCtyDistPwr[cty][dist][pwr]->OverlayCertificate(true);

	foreach(Cty const * cty, bestClassicLogByCtyDistPwr.keys())
		foreach(int dist, bestClassicLogByCtyDistPwr[cty].keys())
			foreach(int pwr, bestClassicLogByCtyDistPwr[cty][dist].keys())
			       bestClassicLogByCtyDistPwr[cty][dist][pwr]->OverlayCertificate(true);

	foreach(QString cat, logsByCatCtyDist.keys())
		foreach(Cty const * cty, logsByCatCtyDist[cat].keys())
			foreach(int dist, logsByCatCtyDist[cat][cty].keys())
				logsByCatCtyDist[cat][cty][dist]->Certificate(true);
}

void CqwwContest::WriteTabHeader(QTextStream & strm)
{
	strm << "Call" 					<<	'\t'
		 <<	"Cty Px"				<<	'\t'
		 <<	"Cont"					<<	'\t'
		 <<	"Zone"					<<	'\t'
		 <<	"Loc"					<<	'\t'
		 <<	"Dist"					<<	'\t'
		 <<	"Cat"					<<	'\t'
		 <<	"Cert"					<<	'\t'
		 <<	"OverlayCert"			<<	'\t'	
		 <<	"Mail Cert"				<<	'\t'
		 <<	"Late"					<<	'\t'
		 <<	"Claimed"				<<	'\t'
		 <<	"Raw Score"				<<	'\t'
		 <<	"Final Score"			<<	'\t'
		 << "Raw Overlay Score"		<<	'\t'
		 <<	"Final Overlay Score"	<<	'\t'
		 <<	"Reduct"				<<	'\t'
		 << "Raw Q"					<<	'\t'
		 << "Final Q"				<<	'\t'
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
		 <<	"160 Q"					<<	'\t'
		 <<	"160 Z"					<<	'\t'
		 <<	"160 C"					<<	'\t'
		 <<	"80 Q"					<<	'\t'
		 <<	"80 Z"					<<	'\t'
		 <<	"80 C"					<<	'\t'
		 <<	"40 Q"					<<	'\t'
		 <<	"40 Z"					<<	'\t'
		 <<	"40 C"					<<	'\t'
		 <<	"20 Q"					<<	'\t'
		 <<	"20 Z"					<<	'\t'
		 <<	"20 C"					<<	'\t'
		 <<	"15 Q"					<<	'\t'
		 <<	"15 Z"					<<	'\t'
		 <<	"15 C"					<<	'\t'
		 <<	"10 Q"					<<	'\t'
		 <<	"10 Z"					<<	'\t'
		 <<	"10 C"					<<	'\t'
		 <<	"Qso Overlay"			<<	'\t'
		 <<	"Zones Overlay"			<<	'\t'
		 <<	"Countries Overlay"		<<	'\t'
		 <<	"160 Q Overlay"			<<	'\t'
		 <<	"160 Z Overlay"			<<	'\t'
		 <<	"160 C Overlay"			<<	'\t'
		 <<	"80 Q Overlay"			<<	'\t'
		 <<	"80 Z Overlay"			<<	'\t'
		 <<	"80 C Overlay"			<<	'\t'
		 <<	"40 Q Overlay"			<<	'\t'
		 <<	"40 Z Overlay"			<<	'\t'
		 <<	"40 C Overlay"			<<	'\t'
		 <<	"20 Q Overlay"			<<	'\t'
		 <<	"20 Z Overlay"			<<	'\t'
		 <<	"20 C Overlay"			<<	'\t'
		 <<	"15 Q Overlay"			<<	'\t'
		 <<	"15 Z Overlay"			<<	'\t'
		 <<	"15 C Overlay"			<<	'\t'
		 <<	"10 Q Overlay"			<<	'\t'
		 <<	"10 Z Overlay"			<<	'\t'
		 <<	"10 C Overlay"			<<	'\t'
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

void CqwwContest::WriteMagazineFile()
{
	QFile magFile(_resDir.absoluteFilePath(Name().toLower() + ".mag"));
	if (!magFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		gStdErrStrm << _resDir.absoluteFilePath(Name().toLower() + ".mag") << " failed to open" << endl;
		return;
	}
	QTextStream magStream(&magFile);

	QMap <int, QString> contByNum;
	contByNum.insert(0, "UNITED STATES");
	contByNum.insert(1, "NORTH AMERICA");
	contByNum.insert(2, "AFRICA");
	contByNum.insert(3, "ASIA");
	contByNum.insert(4, "EUROPE");
	contByNum.insert(5, "OCEANIA");
	contByNum.insert(6, "SOUTH AMERICA");
	contByNum.insert(7, "MAX_CONTINENT");

	QMap <QString, int> numByCont;
	numByCont.insert("USA", 0);
	numByCont.insert("NA", 1);
	numByCont.insert("AF", 2);
	numByCont.insert("AS", 3);
	numByCont.insert("EU", 4);
	numByCont.insert("OC", 5);
	numByCont.insert("SA", 6);
	numByCont.insert("MAX", 7);

	QMap <QString, int> callAreaMap;
	for (int i = 1; i <= 10; i++)
		callAreaMap.insert(QString::number(i % 10), i);
	callAreaMap.insert("11", 11);

	// countries sorted by name, except United States first

	QStringList ctyNames(gCtyNames);
	ctyNames.sort();
	ctyNames.removeAt(ctyNames.indexOf("United States"));
	ctyNames.prepend("United States");

	int ctyNum = 0;
	QMutableListIterator<QString> it(ctyNames);
	while (it.hasNext())
	{
		QString & name = it.next();
		gCtyNameByNum.insert(ctyNum, name);
		gCtyNumByName.insert(name, ctyNum);
		ctyNum++;
	}
	gCtyNameByNum.insert(ctyNum, "DX");
	gCtyNumByName.insert("DX", ctyNum);

	// order of finsh by world/eu/usa, then categroy, then score
	QMap <QString, QMap < QString, QMap <int, Log * > > > finishMap;

	// opCat, cont, cty, call area, pwr, band.  Full results, set up to create output in magazine order
	QMap <int, QMap <int, QMap <int, QMap <int, QMap <int, QMap <int, QMap < int, QList <Log *> > > > > > > > resultsMap;

	QMap <QString, QString > adminCheckLogMap;
	QMap <QString, QString > checkLogMap;
	QMap <QString, QString > mmLogMap;
	QMap <QString, QString > noAwardMap;
	QString opCat;

	// fill in finishMap for rankings within category in World, EU, USA
	// fill in resultsMap for all scores by opcat, cont, cty, callarea, pwr, band

	foreach(Log * log, _logsByCall)
	{
		if (log->Disqualified())
			continue;

		finishMap["World"][log->Category()].insertMulti(log->Score(), log);			// entire World by score
		if (log->Cont() == "EU")
			finishMap["Europe"][log->Category()].insertMulti(log->Score(), log);	// Europe by score
		if (log->CtyPtr() == W_id)
			finishMap["USA"][log->Category()].insertMulti(log->Score(), log);		// USA by score

		if (log->Category() == "CK")
		{
			checkLogMap.insert(log->Call(), log->Call());							// special checklog map
			continue;
		}

		if (log->IsMarMobile())
		{
			mmLogMap.insert(log->Call(), log->Call());
			continue;
		}

		if (log->IsNoAward())
			noAwardMap.insert(log->Call(), log->Call());							// special administrative checklog map

		if (log->IsAdministrativeChecklog())
		{
			adminCheckLogMap.insert(log->Call(), log->Call());
			continue;
		}

		int assisted = log->Category().endsWith("A");
		int magOrderNum = log->MagOrder();
		int pwr = log->PwrNum();
		int bandNum = log->BandNum();
		int contNum = numByCont.value(log->Cont());
		int ctyNum = gCtyNumByName.value(log->CtyName());
		int distNum = callAreaMap.value(log->District());

		if (pwr == Q)
		{
			magOrderNum = QRP_MAG_ORDER;							// qrp listed after all single ops
			ctyNum = -1;
			contNum = -1;
			distNum = -1;
		}

		Cty const * cty = log->CtyPtr();

		if (!_ctysByDistrictSet.contains(cty))
			distNum = -1;

		if (magOrderNum == M2_MAG_ORDER || magOrderNum == MM_MAG_ORDER)				// group M2 and MM by cont only
		{
			distNum = -1;
			if (ctyNum == 0)
				contNum = 0;		// USA is fake continent
			else
				ctyNum = -1;
		}

		if (!log->CategoryNum())
			log->AppendErrMsgList("CATEGORY:", log->Category());

		resultsMap[magOrderNum][contNum][ctyNum][distNum][pwr][bandNum][assisted].append(log);

		if (log->IsOverlay())
		{
			contNum = numByCont.value(log->Cont());
			ctyNum = gCtyNumByName.value(log->CtyName());
			distNum = callAreaMap.value(log->District());
			if (!_ctysByDistrictSet.contains(cty))
				distNum = -1;

			if (log->IsOverlayRookie())
				resultsMap[RK_MAG_ORDER][contNum][ctyNum][distNum][pwr][0][false].append(log);

			if (log->IsOverlayClassic())
				resultsMap[CL_MAG_ORDER][contNum][ctyNum][distNum][pwr][0][false].append(log);
		}
	}

	resultsMap.remove(CK_MAG_ORDER);

	// mark the log's finish position in World, EU and USA

	foreach(QString heading, finishMap.keys())
	{
		foreach(QString cat, finishMap.value(heading).keys())
		{
			int count(0);

			QMapIterator <int, Log *> it(finishMap.value(heading).value(cat));

			it.toBack();
			while (it.hasPrevious())
			{
				it.previous();
				++count;
				if (heading == "World")
					it.value()->WorldFinish(count);

				if (heading == "Europe")
					it.value()->EuFinish(count);

				if (heading == "USA")
					it.value()->UsaFinish(count);
			}
		}
	}

	//	[magOrderNum][contNum][ctyNum][distNum][pwr][bandNum][assisted].append(log);
	// Write main .mag file
	foreach(MagOpDesc magOpDesc, _magOpDescByMagOrderNumMap)				// magazine order 
	{
		int magOrder = magOpDesc.MagOrder();
		magStream << magOpDesc.Desc() << endl;								// for example, SINGLE OPERATOR#

		foreach(int cont, resultsMap.value(magOrder).keys())				// by continent
		{
			magStream << contByNum.value(cont) << "@\n";

			foreach(int ctyNum, resultsMap.value(magOrder).value(cont).keys())		// by country
			{
				QString ctyName = gCtyNameByNum.value(ctyNum);

				if (magOrder != QRP_MAG_ORDER && magOrder != M2_MAG_ORDER && magOrder != MM_MAG_ORDER)
				{
					QString ctyName = gCtyNameByNum.value(ctyNum);
					if (resultsMap.value(magOrder).value(cont).value(ctyNum).keys().first() < 0)
						magStream << ctyName << "%\n";
				}

				foreach(int districtNum, resultsMap.value(magOrder).value(cont).value(ctyNum).keys())	// by district
				{
					if (districtNum >= 0)
						magStream << QString("%1 - District %2%\n").arg(ctyName).arg(districtNum % 10);

					for (int assisted = 0; assisted < 2; assisted++)
					{
						foreach(int pwr, resultsMap.value(magOrder).value(cont).value(ctyNum).value(districtNum).keys()) // by pwr
						{
							QString lastBandName;

							foreach(int band, resultsMap
									.value(magOrder)
									.value(cont)
									.value(ctyNum)
									.value(districtNum)
									.value(pwr)
									.keys())
							{
								QMap <int, Log *> logsByScore;

								foreach(Log * log, resultsMap
									.value(magOrder)
									.value(cont)
									.value(ctyNum)
									.value(districtNum)
									.value(pwr)
									.value(band)
									.value(assisted))
								{
									if (magOrder == RK_MAG_ORDER || magOrder == CL_MAG_ORDER)
										logsByScore.insertMulti(log->OverlayScore(), log);
									else
										logsByScore.insertMulti(log->Score(), log);
								}

								QList <Log *> reverseLogList;					// stable_sort highest to lowest score
								foreach(Log * log, logsByScore)
									reverseLogList.push_front(log);

								foreach(Log * log, reverseLogList)
								{
									QString rawCall = log->RawCall();
									rawCall.replace('0', "$$");
									if (log->CategoryPower() == "LOW")
										rawCall.prepend('*');

									if (log->CategoryPower() == "QRP" && (magOrder == CL_MAG_ORDER || magOrder == RK_MAG_ORDER))
										rawCall.prepend("**");

									QString tmp = _catDescByNum.value(log->CategoryNum()).Abbreviation();
									QString bandName = (lastBandName == tmp) ? "\"" : tmp;
									lastBandName = tmp;

									if (log->IsMultiSingle() || log->Category() == "M2" || log->Category() == "MM")
										bandName.clear();

									QString outLine;

									if (_contestType == "CQWW-RTTY")							// add extra state column for RTTY
									{
										if (magOrder == CL_MAG_ORDER || magOrder == RK_MAG_ORDER)
										{
											bandName.clear();
											outLine = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7")
												.arg(rawCall)
												.arg(bandName)
												.arg(QLocale().toString(log->OverlayScore()))
												.arg(log->OverlayNetQso())
												.arg(log->OverlayNetMults2())
												.arg(log->OverlayNetMults())
												.arg(log->OverlayNetMults3());
										}
										else
										{
											outLine = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7")
												.arg(rawCall)
												.arg(bandName)
												.arg(QLocale().toString(log->Score()))
												.arg(log->NetQso())
												.arg(log->NetMults2())
												.arg(log->NetMults())
												.arg(log->NetMults3());
										}
									}
									else
									{
										if (magOrder == CL_MAG_ORDER || magOrder == RK_MAG_ORDER)
										{
											bandName.clear();
											outLine = QString("%1\t%2\t%3\t%4\t%5\t%6")
												.arg(rawCall)
												.arg(bandName)
												.arg(QLocale().toString(log->OverlayScore()))
												.arg(log->OverlayNetQso())
												.arg(log->OverlayNetMults2())
												.arg(log->OverlayNetMults());
										}
										else
										{
											outLine = QString("%1\t%2\t%3\t%4\t%5\t%6")
												.arg(rawCall)
												.arg(bandName)
												.arg(QLocale().toString(log->Score()))
												.arg(log->NetQso())
												.arg(log->NetMults2())
												.arg(log->NetMults());
										}
									}

									if (log->Certificate() && !log->IsNoAward() && magOrder != CL_MAG_ORDER && magOrder != RK_MAG_ORDER)
										outLine.append('^');

									if (log->OverlayCertificate() && (magOrder == CL_MAG_ORDER || magOrder == RK_MAG_ORDER))
										outLine.append('^');

									if (log->SubmittedLate() || log->IsNoAward())
										outLine.append('!');

									QString guestOpString = log->MakeGuestOpStr();

									if (!guestOpString.isEmpty())
									{
										guestOpString.replace('0', "$$");
										QString guestLine = QString("(OP:%1)").arg(guestOpString);
										if (log->Certificate())
											guestLine.append('^');
										if (log->SubmittedLate())
											guestLine.append('!');
										outLine += QString("\n%1").arg(guestLine);
									}
									magStream << outLine << '\n';
								}
							}
						}
						magStream << '\n';
					}
				}
			}
		}
	}

	if (mmLogMap.size())
	{
		magStream << "\n\n=== Maritime Mobile ===#\n" << endl;
		QString outStr;
		foreach(QString call, mmLogMap)
		{
			call.replace('0', "$$");
			outStr += call + ", ";
		}
		outStr.chop(2);
		magStream << outStr << endl;
	}

	if (checkLogMap.size())
	{
		magStream << "\n\n=== Check Logs ===#\n" << endl;
		QString outStr;
		foreach(QString call, checkLogMap)
		{
			call.replace('0', "$$");
			outStr += call + ", ";
		}
		outStr.chop(2);
		magStream << outStr << endl;
	}

	if (adminCheckLogMap.size())
	{
		magStream << "\n\n=== Administrative Check Logs ===#\n" << endl;
		QString outStr;
		foreach(QString call, adminCheckLogMap)
		{
			call.replace('0', "$$");
			outStr += call + ", ";
		}
		outStr.chop(2);
		magStream << outStr << endl;
	}

	WriteRedAndYellowCards(magStream);
	magFile.close();

	// create top score boxes

	QFile topScoresFile(_resDir.absoluteFilePath(Name().toLower() + "_top_scores.mag"));
	topScoresFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream topScoresStrm(&topScoresFile);

	enum { WORLD, US, EU };
	QMap <int, QString> topByNum;
	topByNum.insert(WORLD,	"World#");
	topByNum.insert(US,		"United States#");
	topByNum.insert(EU,		"Europe#");

	QMap <int, QMap <int, QMap <int, QMap <int, QMap <int, QList <Log *> > > > > > topMap;

	foreach(Log * log, _logsByCall)
	{
		if (log->Category() == "CK" || log->SubmittedLate() || log->Disqualified() || log->IsNoAward() || log->IsAdministrativeChecklog())
			continue;

		bool assisted = log->Category().endsWith("A");
		int  magOrderNum = log->MagOrder();
		int  pwr = log->PwrNum();
		int  bandNum = log->BandNum();

		topMap[WORLD][magOrderNum][assisted][pwr][bandNum].append(log);
		if (log->CtyPtr() == W_id)
			topMap[US][magOrderNum][assisted][pwr][bandNum].append(log);
		if (log->Cont() == "EU")
			topMap[EU][magOrderNum][assisted][pwr][bandNum].append(log);

		if (log->CategoryOverlay() == "ROOKIE" && log->CategoryOperator() == "SINGLE-OP")
		{
			if (pwr == Q)
				pwr = L;
			topMap[WORLD][RK_MAG_ORDER][false][pwr][1].append(log);
			if (log->CtyPtr() == W_id)
				topMap[US][RK_MAG_ORDER][false][pwr][1].append(log);
			if (log->Cont() == "EU")
				topMap[EU][RK_MAG_ORDER][false][pwr][1].append(log);
		}

		if (log->CategoryOverlay() == "CLASSIC" && log->CategoryOperator() == "SINGLE-OP")
		{
			if (pwr == Q)
				pwr = L;
			topMap[WORLD][CL_MAG_ORDER][false][pwr][1].append(log);
			if (log->CtyPtr() == W_id)
				topMap[US][CL_MAG_ORDER][false][pwr][1].append(log);
			if (log->Cont() == "EU")
				topMap[EU][CL_MAG_ORDER][false][pwr][1].append(log);
		}
	}

	foreach(int top, topMap.keys())
	{
		topScoresStrm << topByNum.value(top) << endl;

		foreach(int magOrderNum, topMap[top].keys())
		{
			if (magOrderNum == RK_MAG_ORDER)
				topScoresStrm << QString("%1").arg("\nROOKIE^\n") << endl;

			if (magOrderNum == CL_MAG_ORDER)
				topScoresStrm << QString("%1").arg("\nCLASSIC^\n") << endl;

			foreach(int assisted, topMap[top][magOrderNum].keys())
			{
				foreach(int pwr, topMap[top][magOrderNum][assisted].keys())
				{
					foreach(int bandNum, topMap[top][magOrderNum][assisted][pwr].keys())
					{
						QMap <int, Log *> logsByScore;
						foreach(Log * l, topMap[top][magOrderNum][assisted][pwr][bandNum])
						{
							CqwwLog * log = (CqwwLog *)l;
							int score = (magOrderNum == RK_MAG_ORDER || magOrderNum == CL_MAG_ORDER) ? log->OverlayScore() : log->Score();
							logsByScore.insertMulti(score, log);
						}

						QList <Log *> reverseLogList;
						foreach(Log * log, logsByScore)
							reverseLogList.push_front(log);

						int count = 0;
						foreach(Log * l, reverseLogList)
						{
							CqwwLog * log = static_cast<CqwwLog*> (l);
							if (log == reverseLogList.first())
							{
								if (magOrderNum == RK_MAG_ORDER || magOrderNum == CL_MAG_ORDER)			// change category display for overlay logs
								{
									if (pwr == H)
										topScoresStrm << "Single Op All Band High Power" << '^' << endl;
									else
										topScoresStrm << "Single Op All Band Low Power" << '^' << endl;
								}
								else
									topScoresStrm << _catDescByNum.value(log->CategoryNum()).Full() << '^' << endl;
							}

							QString rawCall = log->RawCall();

							QString guestOpString = log->MakeGuestOpStr();
							if (!guestOpString.isEmpty())
								rawCall += QString(" (%1)").arg(guestOpString);

							rawCall.replace('0', "$$");

							int score = (magOrderNum == RK_MAG_ORDER || magOrderNum == CL_MAG_ORDER) ? log->OverlayScore() : log->Score();
							topScoresStrm << QString("%1\t%2&\n")
								.arg(rawCall)
								.arg(QLocale().toString(score));

							if (++count >= 10)
								break;
						}
						topScoresStrm << endl;
					}
				}
			}
		}
	}
	topScoresFile.close();

	QFile topZoneScoresFile(_resDir.absoluteFilePath(Name().toLower() + "_top_zone_scores.mag"));
	topZoneScoresFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream topZoneScoresStrm(&topZoneScoresFile);
	topZoneScoresStrm << "TOP SCORES IN MOST ACTIVE ZONES#\n" << endl;

	QMap <int, QList <Log *> > topScoresByZoneMap;
	foreach(Log * l, _logsByCall)
	{
		CqwwLog * log = static_cast<CqwwLog*>(l);

		if (log->Category() == "CK" || log->SubmittedLate() || log->Disqualified() || log->IsNoAward() || log->IsAdministrativeChecklog())
			continue;

		if(log->Category().startsWith("AB") && !log->Category().endsWith("A"))
		{
			int z = log->Zone();
			if(z == 3 || z == 4 || z == 5 || z == 14 || z == 15 || z == 16 || z == 20 || z == 25)
				topScoresByZoneMap[z].append(log);
		}
	}

	foreach(int zone, topScoresByZoneMap.keys())
	{
		topZoneScoresStrm << QString("Zone %1#\n").arg(zone);
		QMap <int, Log *> logsByScore;
		foreach(Log * log, topScoresByZoneMap[zone])
			logsByScore.insertMulti(log->Score(), log);

		QList <Log *> reverseLogList;
		foreach(Log * log, logsByScore)
			reverseLogList.push_front(log);

		int count = 0;
		foreach(Log * log, reverseLogList)
		{
			QString rawCall = log->RawCall();

			if (log->PwrNum() == L)
				rawCall.prepend('*');

			QString guestOpString = log->MakeGuestOpStr();
			if (!guestOpString.isEmpty())
				rawCall += QString(" (%1)").arg(guestOpString);

			rawCall.replace('0', "$$");
			topZoneScoresStrm << QString("%1\t%2&\n").arg(rawCall).arg(QLocale().toString(log->Score()));

			if (++count >= 10)
				break;
		}
		topZoneScoresStrm << endl;
	}
	topZoneScoresFile.close();

	QFile breakdownFile(_resDir.absoluteFilePath(Name().toLower() + "_band_breakdowns.mag"));
	breakdownFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream breakdownStream(&breakdownFile);

	// band breakdowns box -- only count ALL_H_U, ALL_L_U, ALL_H_A, ALL_H_L. MS, M2, MM

	QMap <int, QString> BreakDownOpMap;

	BreakDownOpMap.insert(1, "SINGLE OPERATOR ALL BAND#");
	BreakDownOpMap.insert(2, "SINGLE OPERATOR ASSISTED ALL BAND#");
	BreakDownOpMap.insert(3, "MULTI-OPERATOR SINGLE-TRANSMITTER#");
	BreakDownOpMap.insert(4, "MULTI-OPERATOR TWO-TRANSMITTER#");
	BreakDownOpMap.insert(5, "MULTI-OPERATOR MULTI-TRANSMITTER#");

	QMap <int, QString> WorldUsEuByNumMap;
	WorldUsEuByNumMap.insert(1, "WORLD ");
	WorldUsEuByNumMap.insert(2, "USA ");
	WorldUsEuByNumMap.insert(3, "EUROPE ");
	QMap <int,  QMap <int, QList <Log *> > > bandByBandMap;

	foreach(Log * log, _logsByCall)
	{
		int magOrderNum(-1);

		if(log->SubmittedLate() || log->HasRedCard() || log->HasYellowCard())
			continue;

		if(log->Category().startsWith("AB"))
		{
			if(log->Category().endsWith("U"))
				magOrderNum = 1;
			if(log->Category().endsWith("A"))
				magOrderNum = 2;
		}
		
		if(log->Category().startsWith("MS"))
				magOrderNum = 3;

		if(log->Category() == "M2")
				magOrderNum = 4;

		if(log->Category() == "MM")
				magOrderNum = 5;

		if(magOrderNum == -1)
			continue;

		// only all band and multi's make the box 

		int top = 1;							// World
		if(log->CtyPtr() == W_id)					// US
			top = 2;
		if(log->Cont() == "EU")						// EU
			top = 3;

		bandByBandMap[1][magOrderNum].append(log);			// all go into world category
		if(top !=1)
			bandByBandMap[top][magOrderNum].append(log);		// others go to EU or USA
	}

	foreach(int op, WorldUsEuByNumMap.keys())
	{	
		foreach(int magOrderNum, BreakDownOpMap.keys())
		{
		   breakdownStream << WorldUsEuByNumMap.value(op) << BreakDownOpMap.value(magOrderNum) << endl;
		   QMap <int, Log *> logsByScore;
		   foreach(Log * log, bandByBandMap[op][magOrderNum])
				logsByScore.insertMulti(log->Score(), log);

			QList <Log *> reverseLogList;
			foreach(Log * log, logsByScore)
				reverseLogList.push_front(log);

		   int count = 0;
		   foreach(Log * log, reverseLogList)
		   {
			   QString rawCall = log->RawCall();
			   rawCall.replace('0', "$$");
			   if(log->PwrNum() == L)
				   rawCall.prepend('*');
			   count++;
			   QString outString = QString("%1\t").arg(rawCall, -10);
			   foreach(QString band, _bandNameList)
			   {
				   outString += QString("%1/%2/%3")
						.arg(log->NetQso(band))
						.arg(log->NetMults2(band))
						.arg(log->NetMults(band));
			   
				  if (IsRtty())
					   outString += QString("/%1").arg(log->NetMults3(band));		// extra mult for states

				  outString += '\t';
			   }

			   outString += "$";
			   breakdownStream << outString << endl;
			   if(count >= 10)
				   break;
		   }
		   breakdownStream << endl;
		}
	}
	breakdownFile.close();	
}

CqwwContest::CqwwContest() : CqContest() {}

void CqwwContest::Init()
{
	if (_mode == "PH")
		_mode = "SSB";

	if (_mode != "CW" && _mode != "SSB" && _mode != "RTTY")
		AppendErrMsgList("MODE:", _mode + "must be CW or SSB");

	_bandNameList << B160.Name() << B80.Name() << B40.Name() << B20.Name() << B15.Name() << B10.Name();

	_ctysByDistrictSet.insert(W_id);
	_ctysByDistrictSet.insert(JA_id);
	_ctysByDistrictSet.insert(VE_id);
	_ctysByDistrictSet.insert(UA_id);
	_ctysByDistrictSet.insert(UA0_id);

	// number scheme by digit column
	// CMBPA
	// C = checklog
	// M = multiop
	// B = band
	// P = power
	// A = assisted

	// magazine order 
	// magazine section labels

	MagOpDesc SOMag("SINGLE OPERATOR#"						, SO_MAG_ORDER);
	MagOpDesc QPMag("QRP#"									, QRP_MAG_ORDER);
	MagOpDesc MSMag("MULTI-OPERATOR#\nSINGLE-TRANSMITTER#"	, MS_MAG_ORDER);
	MagOpDesc M2Mag("MULTI-OPERATOR#\nTWO-TRANSMITTER#"		, M2_MAG_ORDER);
	MagOpDesc MMMag("MULTI-OPERATOR#\nMULTI-TRANSMITTER#"	, MM_MAG_ORDER);
	MagOpDesc RKMag("ROOKIE#"								, RK_MAG_ORDER);
	MagOpDesc CLMag("CLASSIC#"								, CL_MAG_ORDER);
	MagOpDesc CKMag("CHECK LOGS#"							, CK_MAG_ORDER);

	_magOpDescByMagOrderNumMap.insert(SOMag.MagOrder(), SOMag);
	_magOpDescByMagOrderNumMap.insert(QPMag.MagOrder(), QPMag);
	_magOpDescByMagOrderNumMap.insert(MSMag.MagOrder(), MSMag);
	_magOpDescByMagOrderNumMap.insert(M2Mag.MagOrder(), M2Mag);
	_magOpDescByMagOrderNumMap.insert(MMMag.MagOrder(), MMMag);
	_magOpDescByMagOrderNumMap.insert(RKMag.MagOrder(), RKMag);
	_magOpDescByMagOrderNumMap.insert(CLMag.MagOrder(), CLMag);

	CatDesc	SOABHDesc("ALL_H_U",	"A",	"Single Op All Band High Power",	H,	0, SOMag);
	CatDesc SO10HDesc("10M_H_U",	"28",	"Single Op 28 MHz High Power",		H,	1, SOMag);
	CatDesc SO15HDesc("15M_H_U",	"21",	"Single Op 21 MHz High Power",		H,	2, SOMag);
	CatDesc SO20HDesc("20M_H_U",	"14",	"Single Op 14 MHz High Power",		H,	3, SOMag);
	CatDesc SO40HDesc("40M_H_U",	 "7",	"Single Op 7 MHz High Power",		H,	4, SOMag);
	CatDesc SO80HDesc("80M_H_U",	"3.5",	"Single Op 3.5 MHz High Power",		H,	6, SOMag);
	CatDesc SO160HDesc("160M_H_U",	"1.8",	"Single Op 1.8 MHz High Power",		H,	7, SOMag);

	if (_mode == "SSB")
		SO80HDesc = CatDesc("80M_H_U", "3.7", "Single Op 3.7 MHz High Power", H, 6, SOMag);

	CatDesc	SOABLDesc("ALL_L_U",	 "A",	"Single Op All Band Low Power",		L,	0, SOMag);
	CatDesc SO10LDesc("10M_L_U",	"28",	"Single Op 28 MHz Low Power",		L,	1, SOMag);
	CatDesc SO15LDesc("15M_L_U",	"21",	"Single Op 21 MHz Low Power",		L,	2, SOMag);
	CatDesc SO20LDesc("20M_L_U",	"14",	"Single Op 14 MHz Low Power",		L,	3, SOMag);
	CatDesc SO40LDesc("40M_L_U",	 "7",	"Single Op 7 MHz Low Power",		L,	4, SOMag);
	CatDesc SO80LDesc("80M_L_U",	"3.5",	"Single Op 3.5 MHz Low Power",		L,	6, SOMag);
	CatDesc SO160LDesc("160M_L_U",	"1.8",	"Single Op 1.8 MHz Band Low Power", L,	7, SOMag);

	if (_mode == "SSB")
		SO80LDesc = CatDesc("80M_L_U", "3.7", "Single Op 3.7 MHz Low Power", L, 6, SOMag);

	CatDesc	SOABQDesc("ALL_Q_U",	"A",	"Single Op All Band QRP"	, Q, 0, QPMag);
	CatDesc SO10QDesc("10M_Q_U",	"28",	"Single Op 28 MHz QRP"		, Q, 1, QPMag);
	CatDesc SO15QDesc("15M_Q_U",	"21",	"Single Op 21 MHz QRP"		, Q, 2, QPMag);
	CatDesc SO20QDesc("20M_Q_U",	"14",	"Single Op 14 MHz QRP"		, Q, 3, QPMag);
	CatDesc SO40QDesc("40M_Q_U", 	"7",	"Single Op 7 MHz QRP"		, Q, 4, QPMag);
	CatDesc SO80QDesc("80M_Q_U", 	"3.5",	"Single Op 3.5 MHz QRP"		, Q, 6, QPMag);
	CatDesc SO160QDesc("160M_Q_U", 	"1.8",	"Single Op 1.8 MHz QRP"		, Q, 7, QPMag);

	if (_mode == "SSB")
		SO80QDesc = CatDesc("80M_Q_U", "3.7", "Single Op 3.7 MHz QRP", Q, 6, QPMag);

	CatDesc	SOABHADesc("ALL_H_A", 	"AA",	"Single Op All Band High Power Assisted"	, H, 0, SOMag);
	CatDesc SO10HADesc("10M_H_A", 	"28A",	"Single Op 28 MHz High Power Assisted"		, H, 1, SOMag);
	CatDesc SO15HADesc("15M_H_A", 	"21A",	"Single Op 21 MHz High Power Assisted"		, H, 2, SOMag);
	CatDesc SO20HADesc("20M_H_A", 	"14A",	"Single Op 14 MHz High Power Assisted"		, H, 3, SOMag);
	CatDesc SO40HADesc("40M_H_A", 	"7A",	"Single Op 7 MHz High Power Assisted"		, H, 4, SOMag);
	CatDesc SO80HADesc("80M_H_A", 	"3.5A",	"Single Op 3.5 MHz High Power Assisted"		, H, 6, SOMag);
	CatDesc SO160HADesc("160M_H_A", "1.8A",	"Single Op 1.8 MHz Band High Power Assisted", H, 7, SOMag);

	if (_mode == "SSB")
		SO80HADesc = CatDesc("80M_H_A", "3.7A", "Single Op 3.7 MHz High Power Assisted", H, 6, SOMag);

	CatDesc	SOABLADesc("ALL_L_A", 	"AA",	"Single Op All Band Low Power Assisted",	L, 0, SOMag);
	CatDesc SO10LADesc("10M_L_A", 	"28A",	"Single Op 28 MHz Low Power Assisted",		L, 1, SOMag);
	CatDesc SO15LADesc("15M_L_A", 	"21A",	"Single Op 21 MHz Low Power Assisted",		L, 2, SOMag);
	CatDesc SO20LADesc("20M_L_A", 	"14A",	"Single Op 14 MHz Low Power Assisted",		L, 3, SOMag);
	CatDesc SO40LADesc("40M_L_A", 	"7A",	"Single Op 7 MHz Low Power Assisted",		L, 4, SOMag);
	CatDesc SO80LADesc("80M_L_A", 	"3.5A",	"Single Op 3.5 MHz Low Power Assisted",		L, 6, SOMag);
	CatDesc SO160ALDesc("160M_L_A", "1.8A",	"Single Op 1.8 MHz Low Power Assisted",		L, 7, SOMag);

	if (_mode == "SSB")
		SO80LADesc = CatDesc("80M_L_A", "3.7A", "Single Op 3.7 MHz Low Power Assisted", L, 6, SOMag);

	CatDesc	SOABQADesc("ALL_Q_A", 	"AA",	"Single Op All Band QRP Assisted",	Q, 0, QPMag);
	CatDesc SO10QADesc("10M_Q_A", 	"28A",	"Single Op 28 MHz QRP Assisted",	Q, 1, QPMag);
	CatDesc SO15QADesc("15M_Q_A", 	"21A",	"Single Op 21 MHz QRP Assisted",	Q, 2, QPMag);
	CatDesc SO20QADesc("20M_Q_A", 	"14A",	"Single Op 14 MHz QRP Assisted",	Q, 3, QPMag);
	CatDesc SO40QADesc("40M_Q_A", 	"7A",	"Single Op 7 MHz QRP Assisted",		Q, 4, QPMag);
	CatDesc SO80QADesc("80M_Q_A", 	"3.5A", "Single Op 3.5 MHz QRP Assisted",	Q, 6, QPMag);
	CatDesc SO160QADesc("160M_Q_A", "1.8A", "Single Op 1.8 MHz QRP Assisted",	Q, 7, QPMag);

	if (_mode == "SSB")
		SO80QADesc = CatDesc("80M_Q_A", "3.7A", "Single Op 3.7 MHz QRP Assisted", Q, 6, QPMag);

	CatDesc	MSHDesc("MSH", "MSH", "Multi-Single High Power", H, 0, MSMag);
	CatDesc	MSLDesc("MSL", "MSL", "Multi-Single Low Power",	L, 0, MSMag);
	CatDesc	M2Desc("M2",	"M2", "Multi-Two",		H, 0, M2Mag);
	CatDesc	MMDesc("MM",	"MM", "Multi-Multi",	H, 0, MMMag);

	CatDesc CKDesc("CK", "CK", "Checklog", H, 0, CKMag);

	_catDescByNum.insert(100, SOABHDesc);
	_catDescByNum.insert(200, SO10HDesc);
	_catDescByNum.insert(300, SO15HDesc);
	_catDescByNum.insert(400, SO20HDesc);
	_catDescByNum.insert(500, SO40HDesc);
	_catDescByNum.insert(700, SO80HDesc);
	_catDescByNum.insert(800, SO160HDesc);

	_catDescByNum.insert(110, SOABLDesc);
	_catDescByNum.insert(210, SO10LDesc);
	_catDescByNum.insert(310, SO15LDesc);
	_catDescByNum.insert(410, SO20LDesc);
	_catDescByNum.insert(510, SO40LDesc);
	_catDescByNum.insert(710, SO80LDesc);
	_catDescByNum.insert(810, SO160LDesc);

	_catDescByNum.insert(120, SOABQDesc);
	_catDescByNum.insert(220, SO10QDesc);
	_catDescByNum.insert(320, SO15QDesc);
	_catDescByNum.insert(420, SO20QDesc);
	_catDescByNum.insert(520, SO40QDesc);
	_catDescByNum.insert(720, SO80QDesc);
	_catDescByNum.insert(820, SO160QDesc);

	_catDescByNum.insert(101, SOABHADesc);
	_catDescByNum.insert(201, SO10HADesc);
	_catDescByNum.insert(301, SO15HADesc);
	_catDescByNum.insert(401, SO20HADesc);
	_catDescByNum.insert(501, SO40HADesc);
	_catDescByNum.insert(701, SO80HADesc);
	_catDescByNum.insert(801, SO160HADesc);

	_catDescByNum.insert(111, SOABLADesc);
	_catDescByNum.insert(211, SO10LADesc);
	_catDescByNum.insert(311, SO15LADesc);
	_catDescByNum.insert(411, SO20LADesc);
	_catDescByNum.insert(511, SO40LADesc);
	_catDescByNum.insert(711, SO80LADesc);
	_catDescByNum.insert(811, SO160ALDesc);

	_catDescByNum.insert(121, SOABQADesc);
	_catDescByNum.insert(221, SO10QADesc);
	_catDescByNum.insert(321, SO15QADesc);
	_catDescByNum.insert(421, SO20QADesc);
	_catDescByNum.insert(521, SO40QADesc);
	_catDescByNum.insert(721, SO80QADesc);
	_catDescByNum.insert(821, SO160QADesc);

	_catDescByNum.insert(1000, MSHDesc);
	_catDescByNum.insert(2000, MSLDesc);
	_catDescByNum.insert(3000, M2Desc);
	_catDescByNum.insert(4000, MMDesc);

	_catDescByNum.insert(10000, CKDesc);

	foreach(int num, _catDescByNum.keys())
		_catNumByInternalDesc.insert(_catDescByNum.value(num).Internal(), num);
}

void CqwwContest::WriteTxtHeader(QTextStream & strm)
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
#if 0
	foreach(QString band, BandNameList())
	{
		s += QString("%1 %2 %3 %4 ")
			.arg(band		,4)
			.arg("Q"		,4)
			.arg("Z"		,3)
			.arg("C"		,3);
	}
#endif
	s += QString("%1 %2 %3 %4 %5 %6 %7% %8 %9\n")
			.arg("Q"		,5)
			.arg("Z"		,3)
			.arg("C"		,4)
			.arg("Claimed"	,8)
			.arg("Raw"		,8)
			.arg("Final"	,8)
			.arg("Delta"	,6)
			.arg("Time"		,4)
			.arg("Cat"		,3);

	strm << s << endl;
}

void CqwwContest::MarkNilBust()
{
	Contest::MarkNilBust();

	QFile z2File(_resDir.absoluteFilePath(Name().toLower() + ".z2"));
	z2File.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream z2Stream(&z2File);

	QFile mmFile(_resDir.absoluteFilePath(Name().toLower() + ".mm"));
	mmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream mmStream(&mmFile);

	QMap <QString, QsoList> z2Map;
	QMap <QString, QsoList> mmMap;

	foreach(Log * log, _logsByCall)		// build a list of all zone 2 claimed per received call
	{
		foreach(QsoPtr p, log->AllQso())
		{
			CqwwQso * q = static_cast<CqwwQso *> (p);
			if (q->ZoneRcvd() == 2)
				z2Map[q->CallRcvd()].append(q);

			if (q->IsMM())
				mmMap[q->CallRcvd()].append(q);
		}
	}

	foreach(QString call, z2Map.keys())
	{
		foreach(QsoPtr p, z2Map[call])
		{
			CqwwQso * q = static_cast<CqwwQso *> (p);
			if (q->ZoneExpected() != 2)
				z2Stream << q->ToString() << " expected " << q->ZoneExpected() << endl;
		}
	}

	foreach(QString call, mmMap.keys())
	{
		foreach(QsoPtr p, mmMap[call])
		{
			CqwwQso * q = static_cast<CqwwQso *> (p);
			mmStream << QString("%1 claimed MM zone %2").arg(q->ToString()).arg(q->ZoneRcvd()) << endl;
		}
	}

	z2File.close();
	mmFile.close();

	foreach(Log * l, _logsByCall)
	{
		CqwwLog * log = static_cast<CqwwLog *> (l);
		log->AssignMostCommonZoneRcvd();
	}

	// test rcvd zones against rl for Ws in the FCC data

	QFile rlxFile(_resDir.absoluteFilePath(Name().toLower() + ".rlx"));
	rlxFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream rlxStrm(&rlxFile);
	int notInFccDataCount(0);
	int usCallsCount(0);

	rlxStrm << QString("%1\t%2\t%3\t%4\t%5\n").arg("RL Call").arg("State").arg("Busted zone count").arg("Size of RL").arg("Pct busted zone");

	for (RevLogMap::iterator revLogIt = _revLogMap.begin(); revLogIt != _revLogMap.end(); ++revLogIt)
	{
		QString call = (*revLogIt).first;
		RevLog & revLog = (*revLogIt).second;
		int unmatchedZoneCount(0);

		if (_logsByCall.contains(call))		// only check rls we don't have
			continue;

		if (_logsByCall.contains(call))		// only check rls we don't have
			continue;

		Px const * px = gPxMap.FindExactMatchPx(call);			 // only check W rls
		if (!px)
			px = gPxMap.FindPx(gFormPx(call, "").first);

		if (!px || px == &gEmptyPx || !px->GetCty()->IsW())
			continue;

		QString state = gStateByFccCallMap.value(call);
		usCallsCount++;

		if (state.isEmpty())		// skip calls not in the callbook
		{
			notInFccDataCount++;
			continue;
		}

		int zoneExpected = gZoneByStateMap.value(state); // zone from fcc data

		foreach(QsoPtr p, revLog.AllQso())
		{
			if (!p->PxRcvdPtr()->GetCty()->IsW())	// only check W
				continue;

			CqwwQso * q = static_cast<CqwwQso *> (p);
			int zoneRcvd = q->ZoneRcvd();
			if (zoneExpected != zoneRcvd)
				unmatchedZoneCount++;
		}

		double pct = 100.0 * double(unmatchedZoneCount) / double(revLog.AllQso().size());
		rlxStrm << QString("%1\t%2\t%3\t%4\t%5\n")
			.arg(call)
			.arg(state)
			.arg(unmatchedZoneCount)
			.arg(revLog.AllQso().size())
			.arg(pct, 6, 'f', 1);
	}
	rlxStrm << QString("US calls count: %1 US calls not in FCC data: %2").arg(usCallsCount).arg(notInFccDataCount) << endl;
}

void CqwwContest::WriteSbxFile()
{
	QFile dxQrmFile(_resDir.absoluteFilePath(Name().toLower() + "_dx_qrm.mag"));
	dxQrmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream dxQrmStream(& dxQrmFile);

	QFile usaQrmFile(_resDir.absoluteFilePath(Name().toLower() + "_usa_qrm.mag"));
	usaQrmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream usaQrmStream(& usaQrmFile);

	foreach(Log * log, _logsByCall)
	{
		if(!log->SoapBox().isEmpty())
		{
			if(log->CtyPtr() == W_id)
				usaQrmStream << QString("%1<. . .>%2.\n").arg(log->SoapBox()).arg(log->Call());
			else
				dxQrmStream << QString("%1<. . .>%2.\n").arg(log->SoapBox()).arg(log->Call());
		}
	}
	usaQrmFile.close();
	dxQrmFile.close();
}

void CqwwContest::WriteOpsFile() { Contest::WriteOpsFile(); }
