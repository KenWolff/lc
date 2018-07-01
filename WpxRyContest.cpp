
#include "StdAfx.h"
#include "WpxContest.h"

void WpxRyContest::WriteSbxFile()
{
	QFile dxQrmFile(_resDir.absoluteFilePath(Name().toLower() + "_dx_qrm.mag"));
	dxQrmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream dxQrmStream(& dxQrmFile);

	QFile usaQrmFile(_resDir.absoluteFilePath(Name().toLower() + "_usa_qrm.mag"));
	usaQrmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream usaQrmStream(& usaQrmFile);

	foreach(Log * log, _logsByCall)
		if(!log->SoapBox().isEmpty())
		{
			if(log->CtyPtr() == W_id)
				usaQrmStream << QString("%1<. . .>%2.\n").arg(log->SoapBox()).arg(log->Call());
			else
				dxQrmStream << QString("%1<. . .>%2.\n").arg(log->SoapBox()).arg(log->Call());
		}
	usaQrmFile.close();
	dxQrmFile.close();
}

void WpxRyContest::MarkCertificateWinners()
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

		if(!bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr][bandNum] || 
			score > bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr][bandNum]->Score())
		{
			bestLogByCatCtyDistPwrBand[magOrderNum][cty][dist][pwr][bandNum] = log;
		}

		// overlay cats are by cty,  district, power and band

		if(pwr == Q)
			pwr = L;

		if(log->CategoryOverlay() == "ROOKIE")
			if( !bestRkLogByCtyDistPwrBand[cty][dist][pwr][bandNum] ||
				score > bestRkLogByCtyDistPwrBand[cty][dist][pwr][bandNum]->Score())
					bestRkLogByCtyDistPwrBand[cty][dist][pwr][bandNum] = log;

		if(log->CategoryOverlay() == "TB-WIRES")
			if( !bestTbLogByCtyDistPwrBand[cty][dist][pwr][bandNum] ||
				score > bestTbLogByCtyDistPwrBand[cty][dist][pwr][bandNum]->Score())
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

void WpxRyContest::WriteMagazineFile()
{
	QFile magFile(_resDir.absoluteFilePath(Name().toLower() + ".mag"));
	magFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream magStream(& magFile);

	QMap <int, QString> contByNum;
	contByNum.insert( 1, "NORTH AMERICA" );
	contByNum.insert( 2, "AFRICA"		 );
	contByNum.insert( 3, "ASIA"			 );
	contByNum.insert( 4, "EUROPE"		 );
	contByNum.insert( 5, "OCEANIA"	     );
	contByNum.insert( 6, "SOUTH AMERICA" );

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
	QMap <int, QMap <int, QMap <int, QMap <int, QMap <int, QMap < int, QList <Log *> > > > > > > resultsMap;

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

		int magOrderNum	= log->MagOrder();
		int pwr			= log->PwrNum();
		int bandNum		= log->BandNum();

		int contNum     = numByCont     .value(log->Cont()      );
		int ctyNum		= gCtyNumByName .value(log->CtyName()   );
		int districtNum = callAreaMap   .value(log->District()  );

		if(log->PwrNum() == Q)		   // QRP gets bunched together
		{
			contNum		= -1;
			ctyNum		= -1;
			districtNum = -1;
			pwr			= -1;
		}

		Cty const * cty = log->CtyPtr();
		if( !_ctysByDistrictSet.contains(cty))
			districtNum = 1;

		if(log->Category() == "MM")
			districtNum = 1;

		if(magOrderNum != 10000)
	        resultsMap[magOrderNum][contNum][ctyNum][districtNum][pwr][bandNum].append(log);

		if(log->CategoryOverlay() == "ROOKIE")
		{
			if(pwr == Q)
				pwr = L;
			int usDx = log->CtyName() == W_id->Name() ? 0 : gCtyNumByName.value("DX");
	        resultsMap[RK_MAG_ORDER][0][usDx][1][pwr][bandNum].append(log);
		}

		if(log->CategoryOverlay() == "TB-WIRES")
		{
			if(pwr == Q)
				pwr = L;
			int usDx = log->CtyName() == W_id->Name() ? 0 : gCtyNumByName.value("DX");
	        resultsMap[TB_MAG_ORDER][0][usDx][1][pwr][bandNum].append(log);
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
					foreach(int pwr, resultsMap[op][cont][ctyNum][districtNum].keys())
					{
						QString lastBandName;

						foreach(int band, resultsMap[op][cont][ctyNum][districtNum][pwr].keys())
						{
							QMap <int, Log *> logsByScore;
							foreach(Log * log, resultsMap[op][cont][ctyNum][districtNum][pwr][band])
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

								QString tmp = _catDescByNum.value(log->CategoryNum()).Abbreviation();
								QString bandName = (lastBandName == tmp) ?  "\"" : tmp;
								lastBandName = tmp;
	
								if(op == MSH_MAG_ORDER || op == MSL_MAG_ORDER  || op == M2_MAG_ORDER || op == MM_MAG_ORDER)
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
								
								if(!log->Operators().isEmpty() && !log->IsMultiSingle() && !log->IsMultiTwo() && !log->IsMultiMulti () &&
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
					                    outLine += QString("\n%1").arg(guestLine);
									}
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

	magStream << "\n\n=== Check Logs ===#\n" << endl;
	QString outText;
	foreach(QString call, checkLogMap.keys())
		outText += call.replace('0', "$$") + ", ";
	outText.chop(2);
	magStream << outText << endl;

	magStream << "\n\nTop Scores#\n" << endl;

	enum {WORLD, US, EU};
	QMap <int, QString> topByNum;
	topByNum.insert(WORLD,  "World#");
	topByNum.insert(US,     "United States#");
	topByNum.insert(EU,     "Europe#");

	QMap <int,  QMap <int, QMap <int, QMap <int,  QList <Log *> > > > > topMap;

	foreach(Log * log, _logsByCall)
	{
		if(log->HasRedCard() || log->HasYellowCard())
			continue;

		int magOrderNum    = log->MagOrder();
		int pwr			= log->PwrNum();
		int bandNum	 = log->BandNum();

		if(magOrderNum >= CK_MAG_ORDER)
			continue;

		// resultsMap[RK_MAG_ORDER][0][usDx][1][pwr][bandNum].append(log);

		topMap[WORLD][magOrderNum][pwr][bandNum].append(log);

	    if(log->CtyPtr() == W_id)
		    topMap[US][magOrderNum][pwr][bandNum].append(log);

		if(log->Cont() == "EU")
	        topMap[EU][magOrderNum][pwr][bandNum].append(log);

		if(log->CategoryOverlay() == "ROOKIE" && log->CategoryOperator() == "SINGLE-OP")
		{
			if(pwr == Q)
				pwr = L;
	        topMap[WORLD][RK_MAG_ORDER][pwr][bandNum].append(log);
	        if(log->CtyPtr() == W_id)
		        topMap[US][RK_MAG_ORDER][pwr][bandNum].append(log);
	        if(log->Cont() == "EU")
		        topMap[EU][RK_MAG_ORDER][pwr][bandNum].append(log);
		}

		if(log->CategoryOverlay() == "TB-WIRES" && log->CategoryOperator() == "SINGLE-OP")
		{
			if(pwr == Q)
				pwr = L;
	        topMap[WORLD][TB_MAG_ORDER][pwr][bandNum].append(log);
	        if(log->CtyPtr() == W_id)
		        topMap[US][TB_MAG_ORDER][pwr][bandNum].append(log);
	        if(log->Cont() == "EU")
		        topMap[EU][TB_MAG_ORDER][pwr][bandNum].append(log);
		}
	}

	foreach(int top, topMap.keys())
	{
		magStream << topByNum.value(top) << endl;
		foreach(int magOrderNum, topMap[top].keys())
		{
			magStream << _magOpDescByMagOrderNumMap.value(magOrderNum).Desc() << endl;
			foreach(int pwrNum, topMap[top][magOrderNum].keys())
			{
				foreach(int bandNum, topMap[top][magOrderNum][pwrNum].keys())
				{
					QMap <int, Log *> logsByScore;
					foreach(Log * log, topMap[top][magOrderNum][pwrNum][bandNum])
					logsByScore.insertMulti(log->Score(), log);

					QList <Log *> reverseLogList;
					foreach(Log * log, logsByScore)
						reverseLogList.push_front(log);

					int count = 0;
					foreach(Log * log, reverseLogList)
					{
						if(log == reverseLogList.first())
							magStream << _catDescByNum.value(log->CategoryNum()).Abbreviation() << endl;
						QString rawCall = log->RawCall();
						rawCall.replace('0', "$$");
						
						if(pwrNum == L)
							rawCall.prepend('*');

						if (log->IsPortable())
							rawCall += '/' + log->PortDigit();

						count++;
						if(!log->GuestOp().isEmpty())
						{
							QString guestOp = log->GuestOp();
							guestOp.replace('0', "$$");
							magStream << QString("%1 (%2)\t%3\n").arg(rawCall).arg(guestOp).arg(QLocale().toString(log->Score()));
						}
						else
							magStream << QString("%1\t%2&\n").arg(rawCall).arg(QLocale().toString(log->Score()));
						if(count >= 10)
							break;
					}
					magStream << endl;
				}
			}
		}
	}
	WriteRedAndYellowCards(magStream);
	magFile.close();
}

void WpxRyContest::Init()
{
	gSpt = false;
	gRbn = false;
	_mode = "RTTY";

	_ctysByDistrictSet.insert(W_id);
	_ctysByDistrictSet.insert(VE_id);
	_ctysByDistrictSet.insert(UA_id);
	_ctysByDistrictSet.insert(UA0_id);
	_ctysByDistrictSet.insert(JA_id);

	_bandNameList << B80.Name() << B40.Name() << B20.Name() << B15.Name() << B10.Name();

	enum { SO, QRP, TRIBANDER, ROOKIE, MS, M2, MM, CK};

	MagOpDesc SOMag		("SINGLE OPERATOR#",								SO_MAG_ORDER	);
	MagOpDesc QRPMag	("QRP#",											QRP_MAG_ORDER	);
	MagOpDesc MSHMag	("MULTI-OPERATOR#\nSINGLE_TRANSMITTER HIGH POWER#",	MSH_MAG_ORDER	);
	MagOpDesc MSLMag	("MULTI-OPERATOR#\nSINGLE-TRANSMITTER LOW POWER#",	MSL_MAG_ORDER	);
	MagOpDesc M2Mag		("MULTI-OPERATOR#\nTWO TRANSMITTER#",				M2_MAG_ORDER	);
	MagOpDesc MMMag		("MULTI-OPERATOR#\nMULTI-TRANSMITTER#",				MM_MAG_ORDER	);
	MagOpDesc RKMag		("ROOKIE#",											RK_MAG_ORDER	);
	MagOpDesc TBMag		("TRIBANDER/SINGLE-ELEMENT#",						TB_MAG_ORDER	);
	MagOpDesc CKMag		("CHECK LOGS#",										CK_MAG_ORDER	);

	_magOpDescByMagOrderNumMap.insert(SOMag.MagOrder(),		SOMag);
	_magOpDescByMagOrderNumMap.insert(QRPMag.MagOrder(),	QRPMag);
	_magOpDescByMagOrderNumMap.insert(MSHMag.MagOrder(),	MSHMag);
	_magOpDescByMagOrderNumMap.insert(MSLMag.MagOrder(),	MSLMag);
	_magOpDescByMagOrderNumMap.insert(M2Mag.MagOrder(),		M2Mag);
	_magOpDescByMagOrderNumMap.insert(MMMag.MagOrder(),		MMMag);
	_magOpDescByMagOrderNumMap.insert(RKMag.MagOrder(),		RKMag);
	_magOpDescByMagOrderNumMap.insert(TBMag.MagOrder(),		TBMag);

	CatDesc	SOABHDesc	( "ALL_H",	"A",	"Single Op All Band High Power",			H, 0, SOMag);
	CatDesc SO10HDesc	( "10M_H",	"28",	"Single Op 28 MHz High Power",				H, 1, SOMag);
	CatDesc SO15HDesc	( "15M_H",	"21",	"Single Op 21 MHz High Power",				H, 2, SOMag);
	CatDesc SO20HDesc	( "20M_H",	"14",	"Single Op 14 MHz High Power",				H, 3, SOMag);
	CatDesc SO40HDesc	( "40M_H",	"7",	"Single Op 7 MHz High Power",				H, 4, SOMag);
	CatDesc SO80HDesc	( "80M_H",	"3.5",	"Single Op 3.5 MHz High Power",				H, 6, SOMag);
	
	CatDesc	SOABLDesc	(  "ALL_L",	"A",	"Single Op All Band Low Power",				L, 0, SOMag);
	CatDesc SO10LDesc	( "10M_L",	"28",	"Single Op 28 MHz Low Power",				L, 1, SOMag);
	CatDesc SO15LDesc	( "15M_L",	"21",	"Single Op 21 MHz Low Power",				L, 2, SOMag);
	CatDesc SO20LDesc	( "20M_L",	"14",	"Single Op 14 MHz Low Power",				L, 3, SOMag);
	CatDesc SO40LDesc	( "40M_L",	"7",	"Single Op 7 MHz Low Power",				L, 4, SOMag);
	CatDesc SO80LDesc	( "80M_L",	"3.5",	"Single Op 3.5 MHz Low Power",				L, 6, SOMag);

	CatDesc	SOABQDesc	(  "ALL_Q",	"A",	"Single Op All Band QRP",					Q, 0, QRPMag);
	CatDesc SO10QDesc	( "10M_Q",	"28",	"Single Op 28 MHz QRP",						Q, 1, QRPMag);
	CatDesc SO15QDesc	( "15M_Q",	"21",	"Single Op 21 MHz QRP",						Q, 2, QRPMag);
	CatDesc SO20QDesc	( "20M_Q",	"14",	"Single Op 14 MHz QRP",						Q, 3, QRPMag);
	CatDesc SO40QDesc	( "40M_Q",	"7",	"Single Op 7 MHz QRP",						Q, 4, QRPMag);
	CatDesc SO80QDesc	( "80M_Q",	"3.5",	"Single Op 3.5 MHz QRP",					Q, 6, QRPMag);

	CatDesc	MSHDesc		( "MSH",	"MSH",	"Multi-Single High Power",					H, 0, MSHMag);
	CatDesc	MSLDesc		( "MSL",	"MSL",	"Multi-Single Low Power",					L, 0, MSLMag);
	CatDesc	M2Desc		( "M2",		"M2",	"Multi-Two",								H, 0, M2Mag);
	CatDesc	MMDesc		( "MM",		"MM",	"Multi-Multi",								H, 0, MMMag);

	CatDesc CKDesc		( "CK",		"CK",	"Checklog",									H, 0, CKMag);

	_catDescByNum.insert(100,	SOABHDesc	);	
	_catDescByNum.insert(200,	SO10HDesc	);	
	_catDescByNum.insert(300,	SO15HDesc	);	
	_catDescByNum.insert(400,	SO20HDesc	);	
	_catDescByNum.insert(500,	SO40HDesc	);	
	_catDescByNum.insert(700,	SO80HDesc	);	
								
	_catDescByNum.insert(110,	SOABLDesc	);	
	_catDescByNum.insert(210,	SO10LDesc	);	
	_catDescByNum.insert(310,	SO15LDesc	);	
	_catDescByNum.insert(410,	SO20LDesc	);	
	_catDescByNum.insert(510,	SO40LDesc	);	
	_catDescByNum.insert(710,	SO80LDesc	);	
								
	_catDescByNum.insert(120,	SOABQDesc	);	
	_catDescByNum.insert(220,	SO10QDesc	);	
	_catDescByNum.insert(320,	SO15QDesc	);	
	_catDescByNum.insert(420,	SO20QDesc	);	
	_catDescByNum.insert(520,	SO40QDesc	);	
	_catDescByNum.insert(720,	SO80QDesc	);	
								
	_catDescByNum.insert(1000,	MSHDesc		);		
	_catDescByNum.insert(1010,	MSLDesc		);		
	_catDescByNum.insert(2000,	M2Desc		);	
	_catDescByNum.insert(3000,	MMDesc		);	
								
	_catDescByNum.insert(10000,	CKDesc		);

	foreach(int num, _catDescByNum.keys())
		_catNumByInternalDesc.insert(_catDescByNum.value(num).Internal(), num);
}

void WpxRyContest::WriteTabHeader(QTextStream & strm)
{
	strm <<  "Call"			<<	'\t'
		 <<  "Cty Px"		<<	'\t'
		 <<  "Cont"			<<	'\t'
		 <<  "Loc"			<<	'\t'
		 <<  "Dist"			<<	'\t'
		 <<  "Cat"			<<	'\t'
		 <<  "Cert"			<<	'\t'
		 <<  "Overlay Cert"	<<	'\t'
		 <<	 "Mail Cert"	<<	'\t'
		 <<  "Late"			<<	'\t'
		 <<  "Claimed"		<<	'\t'
		 <<  "Raw Scr"		<<	'\t'
		 <<  "Final Scr"	<<	'\t'
		 <<  "Reduct"		<<	'\t'
		 <<  "Raw Q"		<<	'\t'
		 <<  "Dupes"		<<	'\t'
		 <<  "B"			<<	'\t'
		 <<  "NIL"			<<	'\t'
		 <<  "X"			<<	'\t'
		 <<  "BCV"			<<	'\t'
		 <<  "TV"			<<	'\t'
		 <<  "Uni"			<<	'\t'
		 <<  "C B"			<<	'\t'
		 <<  "C NIL"		<<	'\t'
		 <<  "C X"			<<	'\t'
		 <<  "Mult"			<<	'\t'
		 <<  "Qso"			<<	'\t'
		 <<  "160M"			<<	'\t'
		 <<  "80M"			<<	'\t'
		 <<  "40M"			<<	'\t'
		 <<  "20M"			<<	'\t'
		 <<  "15M"			<<	'\t'
		 <<  "10M"			<<	'\t'
		 <<  "CatOp"		<<	'\t'
		 <<  "CatTx"		<<	'\t'
		 <<  "CatTime"		<<	'\t'
		 <<  "CatOverlay"	<<	'\t'
		 <<  "CatBand"		<<	'\t'
		 <<  "CatPwr"		<<	'\t'
		 <<  "CatMode"		<<	'\t'
		 <<  "CatAssisted"	<<	'\t'
		 <<  "CatStn"		<<	'\t'
		 <<  "Club"			<<	'\t'
		 <<  "Operators"	<<	'\t'
		 <<  "Name"			<<	'\t'
		 <<  "Email"		<<	'\t'
		 <<  "HQ-FROM"		<<	'\t'
		 <<  "Soap Box"		<<	'\t'
		 <<  "Created By"	<<	'\t'
		 <<  "Op Hrs"		<<	'\t'
		 <<  "City"			<<	'\t'
		 <<  "State"		<<	'\t'
		 <<  "Postcode"		<<	'\t'
		 <<  "Country"		<<	'\t'
		 <<  "Addr 1"		<<	'\t'
		 <<  "Addr 2"		<<	'\t'
		 <<  "Addr 3"		<<	'\t'
		 <<  "Addr 4"		<<	'\t'
		 <<  "Addr 5"		<<	'\t'
		 <<  "Addr 6"		<<	'\t'
		 << endl;
}

void WpxRyContest::WriteTxtHeader(QTextStream & strm)
{
	strm << "Raw Call     Category PX     Cont  QSO  Mult   Score D    B    N    X   U\n" << endl;
}