#include "StdAfx.h"
#include "WpxContest.h"

void WpxContest::WriteSbxFile()
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

void WpxContest::WriteTxtHeader(QTextStream & strm)  { strm << "Call           Raw   D   N   B   X  BCV   U    Q    M    Final      Raw  Claimed   Delta  Hrs Cat\n\n"; }

void WpxContest::MarkCertificateWinners()
{
	// mark general certificate winners by cat, cty, dist, pwr, band

	QMap <int, QMap <Cty const *, QMap <int,  QMap <int,  QMap <int, Log *> > > > > bestLogByCatCtyDistPwrBand;
	QMap <Cty const *, QMap <int,  QMap <int,  QMap <int, Log *> > > > bestRkLogByCtyDistPwrBand;
	QMap <Cty const *, QMap <int,  QMap <int,  QMap <int, Log *> > > > bestTbLogByCtyDistPwrBand;

	foreach(Log * log, _logsByCall)
	{
		if (!log->IsCertEligible())
			continue;

		int magOrderNum = log->CategoryNum();
		if(magOrderNum == 10000)
			continue;

		Cty const * cty	= log->CtyPtr();
		int dist		= log->District().toInt();
		int pwr			= log->PwrNum();
		int bandNum		= log->BandNum();
		int score		= log->Score();

		if( !_ctysByDistrictSet.contains(cty))
			dist = 1;

		if(!bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr][bandNum] || score > bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr][bandNum]->Score())
			bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr][bandNum] = log;

		// overlay cats are by cty,  district, power and band

		if(pwr == Q)
			pwr = L;

		if(log->CategoryOverlay() == "ROOKIE")
			if( !bestRkLogByCtyDistPwrBand[cty][dist][pwr][bandNum] || score > bestRkLogByCtyDistPwrBand[cty][dist][pwr][bandNum]->Score())
				bestRkLogByCtyDistPwrBand[cty][dist][pwr][bandNum] = log;

		if(log->CategoryOverlay() == "TB-WIRES")
			if( !bestTbLogByCtyDistPwrBand[cty][dist][pwr][bandNum] || score > bestTbLogByCtyDistPwrBand[cty][dist][pwr][bandNum]->Score())
				bestTbLogByCtyDistPwrBand[cty][dist][pwr][bandNum] = log;
	}

	foreach(int magOrderNum, bestLogByCatCtyDistPwrBand.keys())
		foreach(Cty const * cty, bestLogByCatCtyDistPwrBand[magOrderNum].keys())
			foreach(int dist, bestLogByCatCtyDistPwrBand[magOrderNum][cty].keys())
	            foreach(int pwr, bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist].keys())
		            foreach(int bandNum, bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr].keys())
						bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr][bandNum]->Certificate(true);

	foreach(Cty const * cty, bestRkLogByCtyDistPwrBand.keys())
		foreach(int dist, bestRkLogByCtyDistPwrBand[cty].keys())
			foreach(int pwr, bestRkLogByCtyDistPwrBand[cty][dist].keys())
		        foreach(int bandNum, bestRkLogByCtyDistPwrBand[cty][dist][pwr].keys())
			        bestRkLogByCtyDistPwrBand[cty][dist][pwr][bandNum]->OverlayCertificate(true);

	foreach(Cty const * cty, bestTbLogByCtyDistPwrBand.keys())
		foreach(int dist, bestTbLogByCtyDistPwrBand[cty].keys())
			foreach(int pwr, bestTbLogByCtyDistPwrBand[cty][dist].keys())
		        foreach(int bandNum, bestTbLogByCtyDistPwrBand[cty][dist][pwr].keys())
			        bestTbLogByCtyDistPwrBand[cty][dist][pwr][bandNum]->OverlayCertificate(true);
}

void WpxContest::WriteMagazineFile()
{
	QFile magFile(_resDir.absoluteFilePath(Name().toLower() + ".mag"));
	magFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream magStream(& magFile);

	QMap <int, QString> contByNum;
	contByNum.insert( 1, "NORTH AMERICA");
	contByNum.insert( 2, "AFRICA"		);
	contByNum.insert( 3, "ASIA"			);
	contByNum.insert( 4, "EUROPE"		);
	contByNum.insert( 5, "OCEANIA"		);
	contByNum.insert( 6, "SOUTH AMERICA");

	QMap <QString, int> numByCont;
	numByCont.insert("NA", 1 );
	numByCont.insert("AF", 2 );
	numByCont.insert("AS", 3 );
	numByCont.insert("EU", 4 );
	numByCont.insert("OC", 5 );
	numByCont.insert("SA", 6 );

	QMap <QString, int> callAreaMap;
	callAreaMap["1"] = 1;
	callAreaMap["2"] = 2;
	callAreaMap["3"] = 3;
	callAreaMap["4"] = 4;
	callAreaMap["5"] = 5;
	callAreaMap["6"] = 6;
	callAreaMap["7"] = 7;
	callAreaMap["8"] = 8;
	callAreaMap["9"] = 9;
	callAreaMap["0"] = 10;
	QStringList ctyNames(gCtyNames);
	ctyNames.sort();
	ctyNames.removeAt(ctyNames.indexOf("United States"));	   // countries sorted by name, except United States first
	ctyNames.prepend("United States");

	int ctyNum = 0;
	foreach(QString name, ctyNames)
	{
		gCtyNameByNum.insert(ctyNum, name);
		gCtyNumByName.insert(name, ctyNum);
		ctyNum++;
	}
	gCtyNumByName.insert("DX", ctyNum);
	gCtyNameByNum.insert(ctyNum, "DX");

	// order of finsh by world/eu/usa, then categroy, then score
	QMap <QString, QMap < QString, QMap <int, Log * > > > finishMap;

	// opCat, cont, cty, call area, pwr, band.  Full results, set up to create output in magazine order
	QMap <int, QMap <int, QMap <int, QMap <int, QMap <int, QMap < int, QMap < int, QList <Log *> > > > > > > > resultsMap;

	QMap <QString, QString > checkLogMap;
	QString opCat;

	// fill in finishMap for rankings within category in World, EU, USA
	// fill in resultsMap for all scores by opcat, cont, cty, callarea, pwr, band

	foreach(Log * log, _logsByCall)
	{
		// fill in results for world/eu/us by cat 
		// use this data to mark bust call && spot hit 

		if(log->HasRedCard() || log->HasYellowCard() || log->Disqualified())
			continue;

		finishMap["World"][log->Category()].insertMulti(log->Score(), log);			// entire World by score
		if(log->Cont() == "EU")
			finishMap["Europe"][log->Category()].insertMulti(log->Score(), log);	// Europe by score
		if(log->CtyPtr() == W_id)
			finishMap["USA"][log->Category()].insertMulti(log->Score(), log);		// USA by score

		if(log->Category() == "CK")
		{
			checkLogMap.insert(log->Call(),log->Call());							// special check log map
			continue;
		}

		int assisted	= log->Category().endsWith("A");
		int magOrderNum	= log->MagOrder();
		int pwr			= log->PwrNum();
		int bandNum		= log->BandNum();

		int contNum		= numByCont     .value(log->Cont()      );
		int ctyNum		= gCtyNumByName .value(log->CtyName()   );
		int districtNum = callAreaMap   .value(log->District()  );

		if(log->PwrNum() == Q)		   // QRP gets bunched together
		{
			contNum		= 1;
			ctyNum		= 1;
			districtNum	= -1;
			pwr			= 1;
		}

		Cty const * cty = log->CtyPtr();
		if( !_ctysByDistrictSet.contains(cty))
			districtNum = -1;

		if(log->Category() == "MM")
			districtNum = -1;

		if(magOrderNum != 10000)
	        resultsMap[magOrderNum][contNum][ctyNum][districtNum][pwr][bandNum][assisted].append(log);

		if(log->CategoryOverlay() == "ROOKIE")
		{
			if(log->CategoryPower() == "QRP")
				pwr = L;
			int usDx = log->CtyName() == W_id->Name() ? 0 : gCtyNumByName.value("DX");
	        resultsMap[RK_MAG_ORDER][0][usDx][-1][pwr][bandNum][assisted].append(log);
		}

		if(log->CategoryOverlay() == "TB-WIRES")
		{
			if(log->CategoryPower() == "QRP")
				pwr = L;
			int usDx = log->CtyName() == W_id->Name() ? 0 : gCtyNumByName.value("DX");
	        resultsMap[TB_MAG_ORDER][0][usDx][-1][pwr][bandNum][assisted].append(log);
		}
	}
	resultsMap.remove(CK_MAG_ORDER);

// opCat, cont, cty, call area, pwr, band 

	foreach(MagOpDesc magOpDesc, _magOpDescByMagOrderNumMap)
	{
		int op = magOpDesc.MagOrder();
		magStream << magOpDesc.Desc() << endl;			// for example, SINGLE OPERATOR#
		
		foreach(int cont, resultsMap[op].keys())
		{
			// qrp and overlay cats don't list by cont

			if(op != QRP_MAG_ORDER && op != RK_MAG_ORDER && op != TB_MAG_ORDER)
				magStream << contByNum.value(cont) << "@\n";

			// overlay cats only use US and DX as countries
			foreach(int ctyNum, resultsMap[op][cont].keys())
			{
				QString ctyName = gCtyNameByNum.value(ctyNum);
				if(op != QRP_MAG_ORDER )			// QRP doesn't list by country
				{
					QString ctyName = gCtyNameByNum.value(ctyNum);
					ctyName.replace("Is.", "Islands");
					magStream << ctyName << "%\n";
				}

				foreach(int districtNum, resultsMap[op][cont][ctyNum].keys())
				{
					if(districtNum >= 0)
						magStream << QString("%1 - District %2%\n").arg(ctyName).arg(districtNum % 10);
					
					for(int assisted = 0; assisted < 2; assisted++)
					{
						foreach(int pwr, resultsMap[op][cont][ctyNum][districtNum].keys())
						{
							QString lastBandName;

							foreach(int band, resultsMap[op][cont][ctyNum][districtNum][pwr].keys())
							{
								QMap <int, Log *> logsByScore;
								foreach(Log * log, resultsMap[op][cont][ctyNum][districtNum][pwr][band][assisted])
									logsByScore.insertMulti(log->Score(), log);

								QList <Log *> reverseLogList;
								foreach(Log * log, logsByScore)
									reverseLogList.push_front(log);

								foreach(Log * log, reverseLogList)
								{
									QString rawCall = log->RawCall();
									rawCall.replace('0', "$$");
									
									if(pwr == L)
										rawCall.prepend('*');

									if (log->IsPortable())
										rawCall += '/' + log->PortDigit();

									QString tmp = _catDescByNum.value(log->CategoryNum()).Abbreviation();
									QString bandName = (lastBandName == tmp) ?  "\"" : tmp;
									lastBandName = tmp;
	
									if(op == MSH_MAG_ORDER || op == MSL_MAG_ORDER || op == M2_MAG_ORDER || op == MM_MAG_ORDER)
										bandName = "";

									QString outLine = QString("%1\t%2\t%3\t%4\t%5")
										.arg(rawCall)
										.arg(bandName)
										.arg(QLocale().toString(log->Score()))
										.arg(log->NetQso())
										.arg(log->NetMults());

									bool isOverlayCategory = (op == RK_MAG_ORDER || op == TB_MAG_ORDER);
									bool isCert = ((!isOverlayCategory && log->Certificate())	|| 
												  (  isOverlayCategory && log->OverlayCertificate()));
									if(isCert)
										outLine.append('^');

									if(log->SubmittedLate())
										outLine.append('!');
								
									if(log->Operators().size() && log->Category() != "MSL"	&& log->Category() != "MSH"	&& 
										log->Category() != "M2" && log->Category() != "MM"	&&
										(!log->OpsList().isEmpty() || !log->HostCall().isEmpty()))
									{
										QString guestOp;
										QString hostOp;
										QString guestOpString;

										if(!log->OpsList().isEmpty()					&& 
											log->Call() != log->OpsList().value(0)		&&
											log->OpsList().value(0).contains(QRegularExpression("[A-Z]\\d+[A-Z]"))
										   )
										{
											log->GuestOp(log->OpsList().value(0));
											guestOp = log->GuestOp();
										}

										if(!log->HostCall().isEmpty() && log->HostCall() != log->Call())
											hostOp = '@' + log->HostCall();
									
										if(!guestOp.isEmpty() && !hostOp.isEmpty())
											guestOpString = log->GuestOp() + ' ' + hostOp;

										if(log->GuestOp().isEmpty() && !log->HostCall().isEmpty())
											guestOpString = hostOp;

										if(!log->GuestOp().isEmpty() && log->HostCall().isEmpty())
											guestOpString = guestOp;

										if(!guestOpString.isEmpty())
										{
											guestOpString.replace('0', "$$");
											QString guestLine = QString("(OP:%1)").arg(guestOpString);
											if(isCert)
											   guestLine.append('^');
											if(log->SubmittedLate())
												guestLine.append('!');
											outLine += QString("\n%1").arg(guestLine);
										}
									}
									magStream << outLine << '\n';
								}
							}
						}
					}
					magStream << '\n';
				}
			}
		}
	}

	magStream << "\n\n=== Check Logs ===#\n" << endl;
	QString outText;
	foreach(QString call, checkLogMap.keys())
		outText += call.replace('0', "$$") + ", ";
	outText.chop(2);
	magStream << outText << endl;
	WriteRedAndYellowCards(magStream);
	magFile.close();

	QFile topScoresFile(_resDir.absoluteFilePath(Name().toLower() + "_top_scores.mag"));
	if(!topScoresFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		gStdErrStrm << _resDir.absoluteFilePath(Name().toLower() + "_top_scores.mag") << " failed to open" << endl;
		exit(-1);
	}
	QTextStream topScoresStrm(& topScoresFile);

	topScoresStrm << "Top Scores#\n" << endl;

	enum {WORLD, US, EU};
	QMap <int, QString> topByNum;
	topByNum.insert(WORLD,  "World#"		);
	topByNum.insert(US,     "United States#");
	topByNum.insert(EU,     "Europe#"		);

	QMap <int,  QMap <int, QMap <int, QMap <int, QMap <int,  QList <Log *> > > > > > topMap;

	foreach(Log * log, _logsByCall)
	{
		if(log->Category() == "CK")
			continue;

		if(log->SubmittedLate())
			continue;

		if(log->HasRedCard() || log->HasYellowCard() || log->Disqualified())
			continue;

		int assisted	= log->Category().endsWith("A");
		int magOrderNum	= log->MagOrder();
		int pwr			= log->PwrNum();
		int bandNum		= log->BandNum();

		topMap[WORLD][magOrderNum][assisted][pwr][bandNum].append(log);

	    if(log->CtyPtr() == W_id)
		    topMap[US][magOrderNum][assisted][pwr][bandNum].append(log);

		if(log->Cont() == "EU")
	        topMap[EU][magOrderNum][assisted][pwr][bandNum].append(log);

		if(log->CategoryOverlay() == "ROOKIE" && log->CategoryOperator() == "SINGLE-OP")
		{
			if(pwr == Q)
				pwr = L;
	        topMap[WORLD][RK_MAG_ORDER][false][pwr][bandNum].append(log);
	        if(log->CtyPtr() == W_id)
		        topMap[US][RK_MAG_ORDER][false][pwr][bandNum].append(log);
	        if(log->Cont() == "EU")
		        topMap[EU][RK_MAG_ORDER][false][pwr][bandNum].append(log);
		}

		if(log->CategoryOverlay() == "TB-WIRES" && log->CategoryOperator() == "SINGLE-OP")
		{
			if(pwr == Q)
				pwr = L;
	        topMap[WORLD][TB_MAG_ORDER][false][pwr][bandNum].append(log);
	        if(log->CtyPtr() == W_id)
		        topMap[US][TB_MAG_ORDER][false][pwr][bandNum].append(log);
	        if(log->Cont() == "EU")
		        topMap[EU][TB_MAG_ORDER][false][pwr][bandNum].append(log);
		}
	}

	foreach(int top, topMap.keys())						// output World, USA, EU
	{
		topScoresStrm << topByNum.value(top) << endl;
		foreach(int magOrderNum, topMap[top].keys())
		{
			if(magOrderNum == RK_MAG_ORDER)
				topScoresStrm << QString("%1").arg("\nROOKIE^\n") << endl;

			if(magOrderNum == TB_MAG_ORDER)
				topScoresStrm << QString("%1").arg("\nTribander/Single Element^\n") << endl;

			foreach(int assisted, topMap[top][magOrderNum].keys())
			{
				foreach(int pwr, topMap[top][magOrderNum][assisted].keys())
				{
					foreach(int bandNum, topMap[top][magOrderNum][assisted][pwr].keys())
					{
						QMap <int, Log *> logsByScore;
						foreach(Log * log, topMap[top][magOrderNum][assisted][pwr][bandNum])
							logsByScore.insertMulti(log->Score(), log);

						QList <Log *> reverseLogList;
						foreach(Log * log, logsByScore)
							reverseLogList.push_front(log);
		
						int count = 0;
						foreach(Log * log, reverseLogList)
						{
							if(log == reverseLogList.first())
								topScoresStrm << _catDescByNum.value(log->CategoryNum()).Full() << '^' << endl;

							if(_catDescByNum.value(log->CategoryNum()).Abbreviation() == "NG")
								gStdErrStrm << QString("%1 %2").arg(log->Call(), -12).arg(log->CategoryNum()) << endl;

							QString rawCall = log->RawCall();

							rawCall.replace('0', "$$");
							
							if (log->IsPortable())
								rawCall += '/' + log->PortDigit();

							QString guestOpString = log->MakeGuestOpStr();
							if (!guestOpString.isEmpty())
								rawCall += QString(" (%1)").arg(guestOpString.toUpper());

							topScoresStrm << QString("%1\t%2&\n")
								.arg(rawCall)
								.arg(QLocale().toString(log->Score()));

							if(++count >= 10)
								break;
						}
						topScoresStrm << endl;
					}
				}
			}
		}
	}

//	WriteWpxPortableFile();
}

void WpxContest::Init()
{
	if(_mode == "PH")
		_mode = "SSB";

	if(_mode != "CW" && _mode != "SSB")
		AppendErrMsgList("MODE:", _mode + "must be CW or SSB");

	_ctysByDistrictSet.insert(W_id);
	_ctysByDistrictSet.insert(VE_id);
	_ctysByDistrictSet.insert(UA_id);
	_ctysByDistrictSet.insert(UA0_id);
	_ctysByDistrictSet.insert(JA_id);

	_bandNameList << B160.Name() << B80.Name() << B40.Name() << B20.Name() << B15.Name() << B10.Name();
	
	enum { SO, QRP, ASSISTED, TRIBANDER, ROOKIE, MS, M2, MM, CK};

	MagOpDesc SOMag		("SINGLE OPERATOR#",									SO_MAG_ORDER	);
	MagOpDesc SOAMag	("ASSISTED#",											SOA_MAG_ORDER	);
	MagOpDesc QRPMag	("QRP#",												QRP_MAG_ORDER	);
	MagOpDesc MSHMag	("MULTI-OPERATOR#\nSINGLE-TRANSMITTER#",				MSH_MAG_ORDER	);
	MagOpDesc MSLMag	("MULTI-OPERATOR#\nSINGLE-TRANSMITTER#",				MSL_MAG_ORDER	);
	MagOpDesc M2Mag		("MULTI-OPERATOR#\nTWO-TRANSMITTER#",					M2_MAG_ORDER	);
	MagOpDesc MMMag		("MULTI-OPERATOR#\nMULTI-TRANSMITTER#",					MM_MAG_ORDER	);
	MagOpDesc RKMag		("ROOKIE#",												RK_MAG_ORDER	);
	MagOpDesc TBMag		("TRIBANDER/SINGLE ELEMENT#",							TB_MAG_ORDER	);
	MagOpDesc CKMag		("CHECK LOGS#",											CK_MAG_ORDER	);

	_magOpDescByMagOrderNumMap.insert(SOMag.MagOrder(),		SOMag);
	_magOpDescByMagOrderNumMap.insert(QRPMag.MagOrder(),	QRPMag);
	_magOpDescByMagOrderNumMap.insert(SOAMag.MagOrder(),	SOAMag);
	_magOpDescByMagOrderNumMap.insert(MSHMag.MagOrder(),	MSHMag);
	_magOpDescByMagOrderNumMap.insert(MSLMag.MagOrder(),	MSLMag);
	_magOpDescByMagOrderNumMap.insert(M2Mag.MagOrder(),		M2Mag);
	_magOpDescByMagOrderNumMap.insert(MMMag.MagOrder(),		MMMag);
	_magOpDescByMagOrderNumMap.insert(RKMag.MagOrder(),		RKMag);
	_magOpDescByMagOrderNumMap.insert(TBMag.MagOrder(),		TBMag);

	CatDesc	SOABHDesc	( "ALL_H_U",   "A",	"Single Op All Band High Power",				H, 0, SOMag);
	CatDesc SO10HDesc	( "10M_H_U",  "28",	"Single Op 28 MHz High Power",					H, 1, SOMag);
	CatDesc SO15HDesc	( "15M_H_U",  "21",	"Single Op 21 MHz High Power",					H, 2, SOMag);
	CatDesc SO20HDesc	( "20M_H_U",  "14",	"Single Op 14 MHz High Power",					H, 3, SOMag);
	CatDesc SO40HDesc	( "40M_H_U",   "7",	"Single Op 7 MHz High Power",					H, 4, SOMag);
	CatDesc SO80HDesc	( "80M_H_U", "3.5", "Single Op 3.5 MHz High Power",					H, 6, SOMag);
	CatDesc SO160HDesc	("160M_H_U", "1.8", "Single Op 1.8 MHz High Power",					H, 7, SOMag);

	if(_mode == "SSB")
		SO80HDesc = CatDesc("80M_H_U", "3.7","Single Op 3.7 MHz High Power",				H, 6, SOMag);
	
	CatDesc	SOABLDesc	(  "ALL_L_U",   "A","Single Op All Band Low Power",					L, 0, SOMag);
	CatDesc SO10LDesc	( "10M_L_U",  "28",	"Single Op 28 MHz Low Power",					L, 1, SOMag);
	CatDesc SO15LDesc	( "15M_L_U",  "21",	"Single Op 21 MHz Low Power",					L, 2, SOMag);
	CatDesc SO20LDesc	( "20M_L_U",  "14",	"Single Op 14 MHz Low Power",					L, 3, SOMag);
	CatDesc SO40LDesc	( "40M_L_U",   "7",	"Single Op 7 MHz Low Power",					L, 4, SOMag);
	CatDesc SO80LDesc	( "80M_L_U", "3.5",	"Single Op 3.5 MHz Low Power",					L, 6, SOMag);
	CatDesc SO160LDesc	("160M_L_U", "1.8",	"Single Op 1.8 MHz Band Low Power",				L, 7, SOMag);

	if(_mode == "SSB")
		SO80LDesc = CatDesc( "80M_L_U", "3.7",	"Single Op 3.7 MHz Low Power",				L, 6, SOMag);

	CatDesc	SOABQDesc	(  "ALL_Q_U",   "A",	"Single Op All Band QRP",						Q, 0, QRPMag);
	CatDesc SO10QDesc	( "10M_Q_U",  "28",		"Single Op 28 MHz QRP",							Q, 1, QRPMag);
	CatDesc SO15QDesc	( "15M_Q_U",  "21",		"Single Op 21 MHz QRP",							Q, 2, QRPMag);
	CatDesc SO20QDesc	( "20M_Q_U",  "14",		"Single Op 14 MHz QRP",							Q, 3, QRPMag);
	CatDesc SO40QDesc	( "40M_Q_U",   "7",		"Single Op 7 MHz QRP",							Q, 4, QRPMag);
	CatDesc SO80QDesc	( "80M_Q_U", "3.5",		"Single Op 3.5 MHz QRP",						Q, 6, QRPMag);
	CatDesc SO160QDesc	("160M_Q_U", "1.8",		"Single Op 1.8 MHz QRP",						Q, 7, QRPMag);

	if(_mode == "SSB")
		SO80QDesc = CatDesc( "80M_Q_U", "3.7",	"Single Op 3.7 MHz QRP",						Q, 6, QRPMag);

	CatDesc	SOABHADesc	(  "ALL_H_A",   "AA",	"Single Op All Band High Power Assisted",		H, 0, SOMag);
	CatDesc SO10HADesc	( "10M_H_A",  "28A",	"Single Op 28 MHz High Power Assisted",			H, 1, SOMag);
	CatDesc SO15HADesc	( "15M_H_A",  "21A",	"Single Op 21 MHz High Power Assisted",			H, 2, SOMag);
	CatDesc SO20HADesc	( "20M_H_A",  "14A",	"Single Op 14 MHz High Power Assisted",			H, 3, SOMag);
	CatDesc SO40HADesc	( "40M_H_A",   "7A",	"Single Op 7 MHz High Power Assisted",			H, 4, SOMag);
	CatDesc SO80HADesc	( "80M_H_A", "3.5A",	"Single Op 3.5 MHz High Power Assisted",		H, 6, SOMag);
	CatDesc SO160HADesc	("160M_H_A", "1.8A",	"Single Op 1.8 MHz Band High Power Assisted",	H, 7, SOMag);

	if(_mode == "SSB")
		SO80HADesc = CatDesc("80M_H_A", "3.7A", "Single Op 3.7 MHz High Power Assisted",		H, 6, SOMag);

	CatDesc	SOABLADesc	(  "ALL_L_A",   "AA",	"Single Op All Band Low Power Assisted",		L, 0, SOMag);
	CatDesc SO10LADesc	( "10M_L_A",  "28A",	"Single Op 28 MHz Low Power Assisted",			L, 1, SOMag);
	CatDesc SO15LADesc	( "15M_L_A",  "21A",	"Single Op 21 MHz Low Power Assisted",			L, 2, SOMag);
	CatDesc SO20LADesc	( "20M_L_A",  "14A",	"Single Op 14 MHz Low Power Assisted",			L, 3, SOMag);
	CatDesc SO40LADesc	( "40M_L_A",   "7A",	"Single Op 7 MHz Low Power Assisted",			L, 4, SOMag);
	CatDesc SO80LADesc	( "80M_L_A", "3.5A",	"Single Op 3.5 MHz Low Power Assisted",			L, 6, SOMag);
	CatDesc SO160ALDesc	("160M_L_A", "1.8A",	"Single Op 1.8 MHz Low Power Assisted",			L, 7, SOMag);

	if(_mode == "SSB")
		SO80LADesc = CatDesc( "80M_L_A", "3.7A","Single Op 3.7 MHz Low Power Assisted",			L, 6, SOMag);

	CatDesc	SOABQADesc	(  "ALL_Q_A",   "AA",	"Single Op All Band QRP Assisted",				Q, 0, QRPMag);
	CatDesc SO10QADesc	( "10M_Q_A",  "28A",	"Single Op 28 MHz QRP Assisted",				Q, 1, QRPMag);
	CatDesc SO15QADesc	( "15M_Q_A",  "21A",	"Single Op 21 MHz QRP Assisted",				Q, 2, QRPMag);
	CatDesc SO20QADesc	( "20M_Q_A",  "14A",	"Single Op 14 MHz QRP Assisted",				Q, 3, QRPMag);
	CatDesc SO40QADesc	( "40M_Q_A",   "7A",	"Single Op 7 MHz QRP Assisted",					Q, 4, QRPMag);
	CatDesc SO80QADesc	( "80M_Q_A", "3.5A",	"Single Op 3.5 MHz QRP Assisted",				Q, 6, QRPMag);
	CatDesc SO160QADesc	("160M_Q_A", "1.8A",	"Single Op 1.8 MHz QRP Assisted",				Q, 7, QRPMag);

	if(_mode == "SSB")
		SO80QADesc = CatDesc( "80M_Q_A", "3.7A","Single Op 3.7 MHz QRP Assisted",				Q, 6, QRPMag);

	CatDesc	MSHDesc		( "MSH",	  "MSH",	"Multi-Single High Power",						H, 0, MSHMag);
	CatDesc	MSLDesc		( "MSL",	  "MSL",	"Multi-Single Low Power",						L, 0, MSLMag);
	CatDesc	M2Desc		( "M2",		  "M2",		"Multi-Two",									H, 0, M2Mag);
	CatDesc	MMDesc		( "MM",		  "MM",		"Multi-Multi",									H, 0, MMMag);
	CatDesc CKDesc		( "CK",		  "CK",		"Checklog",										H, 0, CKMag);

	_catDescByNum.insert(100,	SOABHDesc	);	
	_catDescByNum.insert(200,	SO10HDesc	);	
	_catDescByNum.insert(300,	SO15HDesc	);	
	_catDescByNum.insert(400,	SO20HDesc	);	
	_catDescByNum.insert(500,	SO40HDesc	);	
	_catDescByNum.insert(600,	SO80HDesc	);	
	_catDescByNum.insert(700,	SO160HDesc	);	
								
	_catDescByNum.insert(110,	SOABLDesc	);	
	_catDescByNum.insert(210,	SO10LDesc	);	
	_catDescByNum.insert(310,	SO15LDesc	);	
	_catDescByNum.insert(410,	SO20LDesc	);	
	_catDescByNum.insert(510,	SO40LDesc	);	
	_catDescByNum.insert(610,	SO80LDesc	);	
	_catDescByNum.insert(710,	SO160LDesc	);	
								
	_catDescByNum.insert(120,	SOABQDesc	);	
	_catDescByNum.insert(220,	SO10QDesc	);	
	_catDescByNum.insert(320,	SO15QDesc	);	
	_catDescByNum.insert(420,	SO20QDesc	);	
	_catDescByNum.insert(520,	SO40QDesc	);	
	_catDescByNum.insert(620,	SO80QDesc	);	
	_catDescByNum.insert(720,	SO160QDesc	);	
								
	_catDescByNum.insert(101,	SOABHADesc	);	
	_catDescByNum.insert(201,	SO10HADesc	);	
	_catDescByNum.insert(301,	SO15HADesc	);	
	_catDescByNum.insert(401,	SO20HADesc	);	
	_catDescByNum.insert(501,	SO40HADesc	);	
	_catDescByNum.insert(601,	SO80HADesc	);	
	_catDescByNum.insert(701,	SO160HADesc	);	
								
	_catDescByNum.insert(111,	SOABLADesc	);
	_catDescByNum.insert(211,	SO10LADesc	);	
	_catDescByNum.insert(311,	SO15LADesc	);	
	_catDescByNum.insert(411,	SO20LADesc	);	
	_catDescByNum.insert(511,	SO40LADesc	);	
	_catDescByNum.insert(611,	SO80LADesc	);	
	_catDescByNum.insert(711,	SO160ALDesc	);	
								
	_catDescByNum.insert(121,	SOABQADesc	);	
	_catDescByNum.insert(221,	SO10QADesc	);	
	_catDescByNum.insert(321,	SO15QADesc	);	
	_catDescByNum.insert(421,	SO20QADesc	);	
	_catDescByNum.insert(521,	SO40QADesc	);	
	_catDescByNum.insert(621,	SO80QADesc	);	
	_catDescByNum.insert(721,	SO160QADesc	);	
								
	_catDescByNum.insert(1000,	MSHDesc		);		
	_catDescByNum.insert(2000,	MSLDesc		);		
	_catDescByNum.insert(3000,	M2Desc		);	
	_catDescByNum.insert(4000,	MMDesc		);	
								
	_catDescByNum.insert(10000,	CKDesc		);

	foreach(int num, _catDescByNum.keys())
		_catNumByInternalDesc.insert(_catDescByNum.value(num).Internal(), num);
}

void WpxContest::WriteTabHeader(QTextStream & strm)
{
	strm 
		<<  "Call"				<< '\t'
		<<  "Cty Px"			<< '\t'
		<<  "Cont"				<< '\t'
		<<  "Loc"				<< '\t'
		<<  "Dist"				<< '\t'
		<<  "Cat"				<< '\t'
		<<  "Cert"				<< '\t'
		<<  "Overlay Cert"		<< '\t'
		<<  "Late"				<< '\t'
		<<  "Claimed"			<< '\t'
		<< "Raw Scr"			<< '\t'
		<< "Final Scr"			<< '\t'
		<<  "Reduct"			<< '\t'
		<<  "Raw Q"				<< '\t'
		<<  "Dupes"				<< '\t'
		<<  "B"					<< '\t'
		<<  "NIL"				<< '\t'
		<<  "X"					<< '\t'
		<<  "BCV"				<< '\t'
		<<  "TV"				<< '\t'
		<<  "Uni"				<< '\t'
		<<  "C B"				<< '\t'
		<<  "C NIL"				<< '\t'
		<<  "C X"				<< '\t'
		<<  "Mult"				<< '\t'
		<<  "Qso"				<< '\t'
		<<  "160M"				<< '\t'
		<<  "80M"				<< '\t'
		<<  "40M"				<< '\t'
		<<  "20M"				<< '\t'
		<<  "15M"				<< '\t'
		<<  "10M"				<< '\t'
		<<  "CatOp"				<< '\t'
		<<  "CatTx"				<< '\t'
		<<  "CatTime"			<< '\t'
		<<  "CatOverlay"		<< '\t'
		<<  "CatBand"			<< '\t'
		<<  "CatPwr"			<< '\t'
		<<  "CatMode"			<< '\t'
		<<  "CatAssisted"		<< '\t'
		<<  "CatStn"			<< '\t'
		<<  "Club"				<< '\t'
		<<  "Operators"			<< '\t'
		<<  "Name"				<< '\t'
		<<  "Email"				<< '\t'
		<<  "HQ-FROM"			<< '\t'
		<<  "Soap Box"			<< '\t'
		<<  "Created By"		<< '\t'
		<<  "Op Hrs"			<< '\t'
		<<  "City"				<< '\t'
		<<  "State"				<< '\t'
		<<  "Postcode"			<< '\t'
		<<  "Country"			<< '\t'
		<<  "Addr 1"			<< '\t'
		<<  "Addr 2"			<< '\t'
		<<  "Addr 3"			<< '\t'
		<<  "Addr 4"			<< '\t'
		<<  "Addr 5"			<< '\t'
		<<  "Addr 6"			<< '\t'
		<< endl;
}

void WpxContest::WriteWpxPortableFile()
{
	QFile wpxPortFile(_resDir.absoluteFilePath(Name().toLower() + "_portables_conversion.txt"));
	wpxPortFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream wpxPortstream(&wpxPortFile);

	QMap <QString, WpxQso *> wpxQsoByCallRcvd;
	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr qPtr, log->AllQso())
		{
			WpxQso * q = (WpxQso *)qPtr;
			if (q->RawCallRcvd().contains('/'))
				wpxQsoByCallRcvd.insert(q->RawCallRcvd(), q);
		}
	}
	foreach(WpxQso * q, wpxQsoByCallRcvd)
		wpxPortstream << q->ToString() << "   " << q->WpxStr() << endl;
}

void WpxContest::WriteOpsFile() { Contest::WriteOpsFile(); }
