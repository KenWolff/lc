#include "StdAfx.h"

Contest * Contest::CreateContest()
{
	QString type = gSrcMap["ContestType"];

	if (type == "ARDX")
		return new ArrlDxContest();

	if (type == "CQWW")
		return new CqwwContest();

	if (type == "CQWW-RTTY")
		return new CqwwRttyContest();

	if (type == "IARU")
		return new IaruContest();

	if (type == "OCDX")
		return new OcDxContest();

	if (type == "RTTY")
		return new ArrlRttyContest();

	if (type == "VHF-JUN" || type == "VHF-SEP" || type == "VHF-JAN" || type == "UHF-AUG")
		return new VhfContest();

	if (type == "WPX")
		return new WpxContest();

	if (type == "WPX-RTTY")
		return new WpxRyContest();

	gStdErrStrm << "Unknown contest type: " << type << endl;
	return 0;
}

Contest::Contest() :
		_bustedCallCount		(0),
		_bustedRevLogCallCount	(0),
		_bustedXchCount			(0),
		_checkableCount			(0),
		_isCrossBandCount		(0),
		_dupeCount				(0),
		_exactFreqCount			(0),
		_formatErrCount			(0),
		_goodMatchCount			(0),
		_logCount				(0),
		_matchWindowSize		(INT_MAX),
		_multStnFldNum			(10),
		_nonCheckableCount		(0),
		_notInLogCount			(0),
		_numberOfFields			(10),
		_qsoTotalCount			(0),
		_rcvdCallFldNum			(7),
		_rcvdRstFldNum			(8),
		_rcvdXchFldNum			(9),
		_rcvdXchFldNum2			(INT_MAX),
		_sentRstFldNum			(5),
		_sentXchFldNum			(6),
		_sentXchFldNum2			(INT_MAX),
		_totalScore				(0),
		_uniquesNotCounted		(0),
		_uniquesCount			(0)
{
	_contestType	= gSrcMap["ContestType"];
	_name			= gSrcMap["NameOfContest"];
	_mode			= gSrcMap["Mode"];
	_title			= gSrcMap["Title"];

	_startTime = QDateTime::fromString(gSrcMap["StartTime"], gLcDateTimeFormat);
	_startTime.setTimeSpec(Qt::UTC);

	_endTime = QDateTime::fromString(gSrcMap["EndTime"], gLcDateTimeFormat);
	_endTime.setTimeSpec(Qt::UTC);

	_certCutoffTime = QDateTime::fromString(gSrcMap["CertCutoff"], gLcDateTimeFormat);
	_certCutoffTime.setTimeSpec(Qt::UTC);

	_logsDir = gLogsDir;

	// setup results directory
	_bustDir	= SetupSubdir("bust");
	_cbmDir		= SetupSubdir("cbm");
	_dupeDir	= SetupSubdir("dupe");
	_fccDir		= SetupSubdir("fcc");
	_lgDir		= SetupSubdir("lg");
	_mulDir		= SetupSubdir("mul");
	_nilDir		= SetupSubdir("nil");
	_resDir		= SetupSubdir("results");
	_rlDir		= SetupSubdir("rl");
	_rptDir		= SetupSubdir("rpt");
	_sptDir		= SetupSubdir("spt");
	_tmDir		= SetupSubdir("tm");
	_uniDir		= SetupSubdir("uni");
	_xchDir		= SetupSubdir("xch");
	
	gDbgFile.setFileName(_resDir.absoluteFilePath(Name().toLower() + ".dbg"));
	gDbgFile.open(QIODevice::WriteOnly | QIODevice::Text);
	gDbgFileStrm.setDevice(&gDbgFile);

	LoadFccData();

	// Red and Yellow cards are deprecated -- they get combined into Disqualified.lis
	// Read Disqualify.lis RedCard.lis and Yellowcard.lis cards

	QString yellowCardFileName("YellowCard.lis");
	if (_logsDir.exists(yellowCardFileName))
	{
		QFile yellowCardFile(yellowCardFileName);
		yellowCardFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream inStream(&yellowCardFile);
		while (!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_disqualifiedSet.insert(line);
			_yellowCardSet.insert(line);
		}
		yellowCardFile.close();
	}

	QString redCardFileName("RedCard.lis");
	if (_logsDir.exists(redCardFileName))
	{
		QFile redCardFile(redCardFileName);
		QTextStream inStream(&redCardFile);
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_disqualifiedSet.insert(line);
			_redCardSet.insert(line);
		}
		redCardFile.close();
	}

	QString disqualifiedFileName("Disqualify.lis");
	if (_logsDir.exists(disqualifiedFileName))
	{
		QFile disqualifiedFile(disqualifiedFileName);
		disqualifiedFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream inStream(&disqualifiedFile);
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_disqualifiedSet.insert(line);
		}
		disqualifiedFile.close();
	}

	_disqualifiedSet += _yellowCardSet + _redCardSet;

	// read in the list of incomplete logs that might cause nils for other stations

	QFile incompleteLogFile("IncompleteLogs.lis");
	if (incompleteLogFile.exists() && incompleteLogFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream(&incompleteLogFile);
		while (!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_incompleteLogSet.insert(line);
		}
		incompleteLogFile.close();
	}

	QFile noAwardFile("NoAward.lis");
	if (noAwardFile.exists() && noAwardFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream(&noAwardFile);
		while (!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_noAwardSet.insert(line);
		}
		noAwardFile.close();
	}

	QFile administrativeChecklogFile("AdministrativeChecklog.lis");
	if (administrativeChecklogFile.exists() && administrativeChecklogFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream(&administrativeChecklogFile);
		while (!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_administrativeChecklogSet.insert(line);
		}
		administrativeChecklogFile.close();
	}

	// read in the list of logs with bad sent exchange info that might cause XCH for other stations
	QFile badSentInfoFile("BadSentInfo.lis");
	if(badSentInfoFile.exists() && badSentInfoFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &badSentInfoFile );
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_badInfoSentCallSet.insert(line);
		}
		badSentInfoFile.close();
	}

	// read in the list of logs with bad received exchange info, for example DX instead of serial number
	QFile badRcvdInfoFile("BadRcvdInfo.lis");
	if(badRcvdInfoFile.exists() && badRcvdInfoFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &badRcvdInfoFile );
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			_badInfoRcvdCallSet.insert(line);
		}
		badRcvdInfoFile.close();
	}

	// read in the list of aliases,  bad and good calls
	QFile aliasFile("AliasCalls.lis");
	if(!gSpt && !gRbn && aliasFile.exists() && aliasFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &aliasFile );
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			QStringList parts = line.toUpper().split('=');
			if(parts.size() > 1 && parts[0].trimmed() != parts[1].trimmed())
				_aliasCallMap.insert(parts[0].trimmed(), parts[1].trimmed());
		}
		aliasFile.close();
	}

	QFile badCallFile("BadCalls.lis");
	if(!gSpt && !gRbn && badCallFile.exists() && badCallFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &badCallFile );
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			QStringList  parts = line.toUpper().split("=");
			QString call = parts[0].trimmed();

			if(parts.size() > 1)
			{
				QString correctCall = parts[1].trimmed();
				if (call != correctCall)
					_badCallsMap.insert(call, correctCall);
			}
			else
				_badCallsMap.insert(call, "");
		}
		badCallFile.close();
	}

	QFile goodCallFile("GoodCalls.lis");
	if(goodCallFile.exists() && goodCallFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &goodCallFile );
		inStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));
		while(!inStream.atEnd())
		{
			QString call = inStream.readLine().toUpper();
			call = call.left(call.indexOf('#')).trimmed();
			_goodCallsSet.insert(call);
		}
		goodCallFile.close();
	}

	QFile fillRunMultFile("AutoAssignMSRunMultStn.lis");
	if(fillRunMultFile.exists() && fillRunMultFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &fillRunMultFile );
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper();
			line = line.left(line.indexOf('#')).trimmed();
			QStringList fillRunMultPair = line.simplified().split(QRegularExpression("\\s|\\t|:|="), QString::SkipEmptyParts);
			if(fillRunMultPair.size() == 2)
				_fillRunMultMap.insert(fillRunMultPair[0], fillRunMultPair[1]);
		}
		fillRunMultFile.close();
	}

	QFile piratesFile("Pirates.lis");
	if(piratesFile.exists() && piratesFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &piratesFile );
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper().simplified();
			line = line.left(line.indexOf('#')).trimmed();
			_doNotCheckSet.insert(line);
			_pirateCalls.insert(line);
		}
		piratesFile.close();
	}

	QFile aliasDistrictsFile("AliasDistricts.lis");
	if(aliasDistrictsFile.exists() && aliasDistrictsFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &aliasDistrictsFile );
		while(!inStream.atEnd())
		{
			QStringList parts = inStream.readLine().toUpper().split('=');
			if(parts.size() > 1)
				_aliasDistrictMap.insert(parts[0].trimmed(), parts[1].trimmed());
		}
		aliasFile.close();
	}

	// Tiny logs with big reductions
	QFile doNotCheckFile("DoNotCheck.lis");
	if(doNotCheckFile.exists() && doNotCheckFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream(&doNotCheckFile);
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper().simplified();
			line = line.left(line.indexOf('#')).trimmed();
			_doNotCheckSet.insert(line);
		}
		doNotCheckFile.close();
	}

	// bypasses the time received checking for logs to determine eligibility for certificates or awards

	QFile extensionsFile("Extensions.lis");
	if(extensionsFile.exists() && extensionsFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream inStream( &extensionsFile );
		inStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));
		while(!inStream.atEnd())
		{
			QString line = inStream.readLine().toUpper().trimmed();
			line = line.left(line.indexOf('#')).trimmed();
			_extensionsSet.insert(line);
		}
		extensionsFile.close();
	}

	// Text in this file appears at the top of the log checking reports
	QFile adminFile("admin.txt");
	if(adminFile.exists() && adminFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString call;
		QTextStream inStream( &adminFile );
		inStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));
		_adminText = inStream.readAll().trimmed();
		adminFile.close();
	}

	gStateAbbreviationMap.insert("CT"	, "CT");
	gStateAbbreviationMap.insert("CN"	, "CT");
	gStateAbbreviationMap.insert("MA"	, "MA");
	gStateAbbreviationMap.insert("EMA"	, "MA");
	gStateAbbreviationMap.insert("WMA"	, "MA");
	gStateAbbreviationMap.insert("ME"	, "ME");
	gStateAbbreviationMap.insert("NH"	, "NH");
	gStateAbbreviationMap.insert("RI"	, "RI");
	gStateAbbreviationMap.insert("VT"	, "VT");
	gStateAbbreviationMap.insert("NY"	, "NY");
	gStateAbbreviationMap.insert("ENY"	, "NY");
	gStateAbbreviationMap.insert("NLI"	, "NY");
	gStateAbbreviationMap.insert("NNY"	, "NY");
	gStateAbbreviationMap.insert("WNY"	, "NY");
	gStateAbbreviationMap.insert("NJ"	, "NJ");
	gStateAbbreviationMap.insert("NNJ"	, "NJ");
	gStateAbbreviationMap.insert("SNJ"	, "NJ");
	gStateAbbreviationMap.insert("DE"	, "DE");
	gStateAbbreviationMap.insert("DEL"	, "DE");
	gStateAbbreviationMap.insert("DL"	, "DE");
	gStateAbbreviationMap.insert("PA"	, "PA");
	gStateAbbreviationMap.insert("EPA"	, "PA");
	gStateAbbreviationMap.insert("WPA"	, "PA");
	gStateAbbreviationMap.insert("PAC"	, "PAC");
	gStateAbbreviationMap.insert("MDC"	, "MD");
	gStateAbbreviationMap.insert("MD"	, "MD");
	gStateAbbreviationMap.insert("DC"	, "DC");
	gStateAbbreviationMap.insert("AL"	, "AL");
	gStateAbbreviationMap.insert("GA"	, "GA");
	gStateAbbreviationMap.insert("KY"	, "KY");
	gStateAbbreviationMap.insert("NC"	, "NC");
	gStateAbbreviationMap.insert("FL"	, "FL");
	gStateAbbreviationMap.insert("FLA"	, "FL");
	gStateAbbreviationMap.insert("NFL"	, "FL");
	gStateAbbreviationMap.insert("SFL"	, "FL");
	gStateAbbreviationMap.insert("WCF"	, "FL");
	gStateAbbreviationMap.insert("SC"	, "SC");
	gStateAbbreviationMap.insert("TN"	, "TN");
	gStateAbbreviationMap.insert("VA"	, "VA");
	gStateAbbreviationMap.insert("PR"	, "PR");
	gStateAbbreviationMap.insert("VI"	, "VI");
	gStateAbbreviationMap.insert("AR"	, "AR");
	gStateAbbreviationMap.insert("LA"	, "LA");
	gStateAbbreviationMap.insert("MS"	, "MS");
	gStateAbbreviationMap.insert("NM"	, "NM");
	gStateAbbreviationMap.insert("TX"	, "TX");
	gStateAbbreviationMap.insert("NTX"	, "TX");
	gStateAbbreviationMap.insert("STX"	, "TX");
	gStateAbbreviationMap.insert("WTX"	, "TX");
	gStateAbbreviationMap.insert("OK"	, "OK");
	gStateAbbreviationMap.insert("CA"	, "CA");
	gStateAbbreviationMap.insert("EB"	, "CA");
	gStateAbbreviationMap.insert("LAX"	, "CA");
	gStateAbbreviationMap.insert("ORG"	, "CA");
	gStateAbbreviationMap.insert("SB"	, "CA");
	gStateAbbreviationMap.insert("SCV"	, "CA");
	gStateAbbreviationMap.insert("SDG"	, "CA");
	gStateAbbreviationMap.insert("SF"	, "CA");
	gStateAbbreviationMap.insert("SJV"	, "CA");
	gStateAbbreviationMap.insert("SV"	, "CA");
	gStateAbbreviationMap.insert("AZ"	, "AZ");
	gStateAbbreviationMap.insert("WA"	, "WA");
	gStateAbbreviationMap.insert("EWA"	, "WA");
	gStateAbbreviationMap.insert("WWA"	, "WA");
	gStateAbbreviationMap.insert("AK"	, "AK");
	gStateAbbreviationMap.insert("ID"	, "ID");
	gStateAbbreviationMap.insert("MT"	, "MT");
	gStateAbbreviationMap.insert("NV"	, "NV");
	gStateAbbreviationMap.insert("OR"	, "OR");
	gStateAbbreviationMap.insert("UT"	, "UT");
	gStateAbbreviationMap.insert("WY"	, "WY");
	gStateAbbreviationMap.insert("MI"	, "MI");
	gStateAbbreviationMap.insert("OH"	, "OH");
	gStateAbbreviationMap.insert("WV"	, "WV");
	gStateAbbreviationMap.insert("WVA"	, "WV");
	gStateAbbreviationMap.insert("IL"	, "IL");
	gStateAbbreviationMap.insert("IN"	, "IN");
	gStateAbbreviationMap.insert("WI"	, "WI");
	gStateAbbreviationMap.insert("WS"	, "WI");
	gStateAbbreviationMap.insert("CO"	, "CO");
	gStateAbbreviationMap.insert("IA"	, "IA");
	gStateAbbreviationMap.insert("IO"	, "IA");
	gStateAbbreviationMap.insert("KS"	, "KS");
	gStateAbbreviationMap.insert("MN"	, "MN");
	gStateAbbreviationMap.insert("MO"	, "MO");
	gStateAbbreviationMap.insert("NE"	, "NE");
	gStateAbbreviationMap.insert("ND"	, "ND");
	gStateAbbreviationMap.insert("SD"	, "SD");
	gStateAbbreviationMap.insert("NB"	, "NB");
	gStateAbbreviationMap.insert("VE9"	, "NB");
	gStateAbbreviationMap.insert("NS"	, "NS");
	gStateAbbreviationMap.insert("VE1"	, "NS");
	gStateAbbreviationMap.insert("VA1"	, "NS");
	gStateAbbreviationMap.insert("QC"	, "QC");
	gStateAbbreviationMap.insert("QU"	, "QC");
	gStateAbbreviationMap.insert("PQ"	, "QC");
	gStateAbbreviationMap.insert("VE2"	, "QC");
	gStateAbbreviationMap.insert("VA2"	, "QC");
	gStateAbbreviationMap.insert("ON"	, "ON");
	gStateAbbreviationMap.insert("ONN"	, "ON");
	gStateAbbreviationMap.insert("ONE"	, "ON");
	gStateAbbreviationMap.insert("GTA"	, "ON");
	gStateAbbreviationMap.insert("ONS"	, "ON");
	gStateAbbreviationMap.insert("ONT"	, "ON");
	gStateAbbreviationMap.insert("VE3"	, "ON");
	gStateAbbreviationMap.insert("VA3"	, "ON");
	gStateAbbreviationMap.insert("MB"	, "MB");
	gStateAbbreviationMap.insert("VE4"	, "MB");
	gStateAbbreviationMap.insert("VA4"	, "MB");
	gStateAbbreviationMap.insert("SK"	, "SK");
	gStateAbbreviationMap.insert("SASK"	, "SK");
	gStateAbbreviationMap.insert("SAS"	, "SK");
	gStateAbbreviationMap.insert("VE5"	, "SK");
	gStateAbbreviationMap.insert("VA5"	, "SK");
	gStateAbbreviationMap.insert("AB"	, "AB");
	gStateAbbreviationMap.insert("ALB"	, "AB");
	gStateAbbreviationMap.insert("VE6"	, "AB");
	gStateAbbreviationMap.insert("VA6"	, "AB");
	gStateAbbreviationMap.insert("BC"	, "BC");
	gStateAbbreviationMap.insert("VE7"	, "BC");
	gStateAbbreviationMap.insert("VA7"	, "BC");
	gStateAbbreviationMap.insert("NWT"	, "NWT");
	gStateAbbreviationMap.insert("NT"	, "NWT");
	gStateAbbreviationMap.insert("VE8"	, "NWT");
	gStateAbbreviationMap.insert("NL"	, "NF");
	gStateAbbreviationMap.insert("NF"	, "NF");
	gStateAbbreviationMap.insert("VO1"	, "NF");
	gStateAbbreviationMap.insert("LB"	, "LB");
	gStateAbbreviationMap.insert("LAB"	, "LB");
	gStateAbbreviationMap.insert("VO2"	, "LB");
	gStateAbbreviationMap.insert("NU"	, "NU");
	gStateAbbreviationMap.insert("VY0"	, "NU");
	gStateAbbreviationMap.insert("YT"	, "YT");
	gStateAbbreviationMap.insert("VY1"	, "YT");
	gStateAbbreviationMap.insert("PE"	, "PEI");
	gStateAbbreviationMap.insert("PEI"	, "PEI");
	gStateAbbreviationMap.insert("VY2"	, "PEI");

	gStateAbbreviationMap.insert("ALA"	, "AL");
	gStateAbbreviationMap.insert("ARI"	, "AZ");
	gStateAbbreviationMap.insert("ARK"	, "AR");
	gStateAbbreviationMap.insert("CAL"	, "CA");
	gStateAbbreviationMap.insert("COL"	, "CO");
	gStateAbbreviationMap.insert("CON"	, "CT");
	gStateAbbreviationMap.insert("DEL"	, "DE");
	gStateAbbreviationMap.insert("FLO"	, "FL");
	gStateAbbreviationMap.insert("GEO"	, "GA");
	gStateAbbreviationMap.insert("IDA"	, "ID");
	gStateAbbreviationMap.insert("ILL"	, "IL");
	gStateAbbreviationMap.insert("IND"	, "IN");
	gStateAbbreviationMap.insert("IOW"	, "IA");
	gStateAbbreviationMap.insert("KAN"	, "KS");
	gStateAbbreviationMap.insert("KEN"	, "KY");
	gStateAbbreviationMap.insert("LOU"	, "LA");
	gStateAbbreviationMap.insert("MAI"	, "ME");
	gStateAbbreviationMap.insert("MAS"	, "MA");
	gStateAbbreviationMap.insert("MIC"	, "MI");
	gStateAbbreviationMap.insert("MIN"	, "MN");
	gStateAbbreviationMap.insert("MIS"	, "MS");
	gStateAbbreviationMap.insert("MON"	, "MT");
	gStateAbbreviationMap.insert("NDA"	, "ND");
	gStateAbbreviationMap.insert("NEB"	, "NE");
	gStateAbbreviationMap.insert("NEB"	, "NE");
	gStateAbbreviationMap.insert("NME"	, "NM");
	gStateAbbreviationMap.insert("OHI"	, "OH");
	gStateAbbreviationMap.insert("OKL"	, "OK");
	gStateAbbreviationMap.insert("ORE"	, "OR");
	gStateAbbreviationMap.insert("PEN"	, "PA");
	gStateAbbreviationMap.insert("SDA"	, "SD");
	gStateAbbreviationMap.insert("TEN"	, "TN");
	gStateAbbreviationMap.insert("TEX"	, "TX");
	gStateAbbreviationMap.insert("TEXAS", "TX");
	gStateAbbreviationMap.insert("UTA"	, "UT");
	gStateAbbreviationMap.insert("VER"	, "VT");
	gStateAbbreviationMap.insert("VIR"	, "VA");
	gStateAbbreviationMap.insert("WAS"	, "WA");
	gStateAbbreviationMap.insert("WIS"	, "WI");
	gStateAbbreviationMap.insert("WYO"	, "WY");

	gDistrictBySectionMap.insert("CT"	, 1);
	gDistrictBySectionMap.insert("EMA"	, 1);
	gDistrictBySectionMap.insert("WMA"	, 1);
	gDistrictBySectionMap.insert("ME"	, 1);
	gDistrictBySectionMap.insert("NH"	, 1);
	gDistrictBySectionMap.insert("RI"	, 1);
	gDistrictBySectionMap.insert("VT"	, 1);
	gDistrictBySectionMap.insert("ENY"	, 2);
	gDistrictBySectionMap.insert("NLI"	, 2);
	gDistrictBySectionMap.insert("NNY"	, 2);
	gDistrictBySectionMap.insert("WNY"	, 2);
	gDistrictBySectionMap.insert("NNJ"	, 2);
	gDistrictBySectionMap.insert("SNJ"	, 2);
	gDistrictBySectionMap.insert("DE"	, 3);
	gDistrictBySectionMap.insert("EPA"	, 3);
	gDistrictBySectionMap.insert("WPA"	, 3);
	gDistrictBySectionMap.insert("MDC"	, 3);
	gDistrictBySectionMap.insert("AL"	, 4);
	gDistrictBySectionMap.insert("GA"	, 4);
	gDistrictBySectionMap.insert("KY"	, 4);
	gDistrictBySectionMap.insert("NC"	, 4);
	gDistrictBySectionMap.insert("FLA"	, 4);
	gDistrictBySectionMap.insert("NFL"	, 4);
	gDistrictBySectionMap.insert("SFL"	, 4);
	gDistrictBySectionMap.insert("WCF"	, 4);
	gDistrictBySectionMap.insert("SC"	, 4);
	gDistrictBySectionMap.insert("TN"	, 4);
	gDistrictBySectionMap.insert("VA"	, 4);
	gDistrictBySectionMap.insert("PR"	, 4);
	gDistrictBySectionMap.insert("VI"	, 4);
	gDistrictBySectionMap.insert("AR"	, 5);
	gDistrictBySectionMap.insert("LA"	, 5);
	gDistrictBySectionMap.insert("MS"	, 5);
	gDistrictBySectionMap.insert("NM"	, 5);
	gDistrictBySectionMap.insert("NTX"	, 5);
	gDistrictBySectionMap.insert("STX"	, 5);
	gDistrictBySectionMap.insert("WTX"	, 5);
	gDistrictBySectionMap.insert("OK"	, 5);
	gDistrictBySectionMap.insert("EB"	, 6);
	gDistrictBySectionMap.insert("LAX"	, 6);
	gDistrictBySectionMap.insert("ORG"	, 6);
	gDistrictBySectionMap.insert("SB"	, 6);
	gDistrictBySectionMap.insert("SCV"	, 6);
	gDistrictBySectionMap.insert("SDG"	, 6);
	gDistrictBySectionMap.insert("SF"	, 6);
	gDistrictBySectionMap.insert("SJV"	, 6);
	gDistrictBySectionMap.insert("SV"	, 6);
	gDistrictBySectionMap.insert("PAC"	, 6);
	gDistrictBySectionMap.insert("AZ"	, 7);
	gDistrictBySectionMap.insert("AK"	, 7);
	gDistrictBySectionMap.insert("EWA"	, 7);
	gDistrictBySectionMap.insert("WWA"	, 7);
	gDistrictBySectionMap.insert("ID"	, 7);
	gDistrictBySectionMap.insert("MT"	, 7);
	gDistrictBySectionMap.insert("NV"	, 7);
	gDistrictBySectionMap.insert("OR"	, 7);
	gDistrictBySectionMap.insert("UT"	, 7);
	gDistrictBySectionMap.insert("WY"	, 7);
	gDistrictBySectionMap.insert("MI"	, 8);
	gDistrictBySectionMap.insert("OH"	, 8);
	gDistrictBySectionMap.insert("WV"	, 8);
	gDistrictBySectionMap.insert("IL"	, 9);
	gDistrictBySectionMap.insert("IN"	, 9);
	gDistrictBySectionMap.insert("WI"	, 9);
	gDistrictBySectionMap.insert("WS"	, 9);
	gDistrictBySectionMap.insert("CO"	, 0);
	gDistrictBySectionMap.insert("IA"	, 0);
	gDistrictBySectionMap.insert("IO"	, 0);
	gDistrictBySectionMap.insert("KS"	, 0);
	gDistrictBySectionMap.insert("MN"	, 0);
	gDistrictBySectionMap.insert("MO"	, 0);
	gDistrictBySectionMap.insert("NE"	, 0);
	gDistrictBySectionMap.insert("ND"	, 0);
	gDistrictBySectionMap.insert("SD"	, 0);
	gDistrictBySectionMap.insert("MAR"	, 1);
	gDistrictBySectionMap.insert("NL"	, 1);
	gDistrictBySectionMap.insert("QC"	, 2);
	gDistrictBySectionMap.insert("ON"	, 3);
	gDistrictBySectionMap.insert("ONT"	, 3);
	gDistrictBySectionMap.insert("ONN"	, 3);
	gDistrictBySectionMap.insert("ONE"	, 3);
	gDistrictBySectionMap.insert("GTA"	, 3);
	gDistrictBySectionMap.insert("ONS"	, 3);
	gDistrictBySectionMap.insert("MB"	, 4);
	gDistrictBySectionMap.insert("SK"	, 5);
	gDistrictBySectionMap.insert("AB"	, 6);
	gDistrictBySectionMap.insert("BC"	, 6);
	gDistrictBySectionMap.insert("NT"	, 8);

	gZoneByStateMap.insert("AK"		, 1);
	gZoneByStateMap.insert("CT"		, 5);
	gZoneByStateMap.insert("MA"		, 5);
	gZoneByStateMap.insert("EMA"	, 5);
	gZoneByStateMap.insert("WMA"	, 5);
	gZoneByStateMap.insert("ME"		, 5);
	gZoneByStateMap.insert("NH"		, 5);
	gZoneByStateMap.insert("RI"		, 5);
	gZoneByStateMap.insert("VT"		, 5);
	gZoneByStateMap.insert("NY"		, 5);
	gZoneByStateMap.insert("ENY"	, 5);
	gZoneByStateMap.insert("NLI"	, 5);
	gZoneByStateMap.insert("NNY"	, 5);
	gZoneByStateMap.insert("WNY"	, 5);
	gZoneByStateMap.insert("NJ"		, 5);
	gZoneByStateMap.insert("NNJ"	, 5);
	gZoneByStateMap.insert("SNJ"	, 5);
	gZoneByStateMap.insert("DE"		, 5);
	gZoneByStateMap.insert("DEL"	, 5);
	gZoneByStateMap.insert("PA"		, 5);
	gZoneByStateMap.insert("EPA"	, 5);
	gZoneByStateMap.insert("WPA"	, 5);
	gZoneByStateMap.insert("MDC"	, 5);
	gZoneByStateMap.insert("MD"		, 5);
	gZoneByStateMap.insert("DC"		, 5);
	gZoneByStateMap.insert("AL"		, 4);
	gZoneByStateMap.insert("GA"		, 5);
	gZoneByStateMap.insert("KY"		, 4);
	gZoneByStateMap.insert("NC"		, 5);
	gZoneByStateMap.insert("FL"		, 5);
	gZoneByStateMap.insert("FLA"	, 5);
	gZoneByStateMap.insert("NFL"	, 5);
	gZoneByStateMap.insert("SFL"	, 5);
	gZoneByStateMap.insert("WCF"	, 5);
	gZoneByStateMap.insert("SC"		, 5);
	gZoneByStateMap.insert("TN"		, 4);
	gZoneByStateMap.insert("VA"		, 5);
	gZoneByStateMap.insert("AR"		, 4);
	gZoneByStateMap.insert("LA"		, 4);
	gZoneByStateMap.insert("MS"		, 4);
	gZoneByStateMap.insert("NM"		, 4);
	gZoneByStateMap.insert("TX"		, 4);
	gZoneByStateMap.insert("NTX"	, 4);
	gZoneByStateMap.insert("STX"	, 4);
	gZoneByStateMap.insert("WTX"	, 4);
	gZoneByStateMap.insert("OK"		, 4);
	gZoneByStateMap.insert("CA"		, 3);
	gZoneByStateMap.insert("EB"		, 3);
	gZoneByStateMap.insert("LAX"	, 3);
	gZoneByStateMap.insert("ORG"	, 3);
	gZoneByStateMap.insert("SB"		, 3);
	gZoneByStateMap.insert("SCV"	, 3);
	gZoneByStateMap.insert("SDG"	, 3);
	gZoneByStateMap.insert("SF"		, 3);
	gZoneByStateMap.insert("SJV"	, 3);
	gZoneByStateMap.insert("SV"		, 3);
	gZoneByStateMap.insert("AZ"		, 3);
	gZoneByStateMap.insert("WA"		, 3);
	gZoneByStateMap.insert("EWA"	, 3);
	gZoneByStateMap.insert("WWA"	, 3);
	gZoneByStateMap.insert("ID"		, 3);
	gZoneByStateMap.insert("MT"		, 4);
	gZoneByStateMap.insert("NV"		, 3);
	gZoneByStateMap.insert("OR"		, 3);
	gZoneByStateMap.insert("UT"		, 3);
	gZoneByStateMap.insert("WY"		, 4);
	gZoneByStateMap.insert("MI"		, 4);
	gZoneByStateMap.insert("OH"		, 4);
	gZoneByStateMap.insert("WV"		, 5);
	gZoneByStateMap.insert("WVA"	, 5);
	gZoneByStateMap.insert("IL"		, 4);
	gZoneByStateMap.insert("IN"		, 4);
	gZoneByStateMap.insert("WI"		, 4);
	gZoneByStateMap.insert("WS"		, 4);
	gZoneByStateMap.insert("CO"		, 4);
	gZoneByStateMap.insert("IA"		, 4);
	gZoneByStateMap.insert("IO"		, 4);
	gZoneByStateMap.insert("KS"		, 4);
	gZoneByStateMap.insert("MN"		, 4);
	gZoneByStateMap.insert("MO"		, 4);
	gZoneByStateMap.insert("NE"		, 4);
	gZoneByStateMap.insert("ND"		, 4);
	gZoneByStateMap.insert("SD"		, 4);
	gZoneByStateMap.insert("NB"		, 4);
	gZoneByStateMap.insert("VE9"	, 5);
	gZoneByStateMap.insert("NS"		, 5);
	gZoneByStateMap.insert("VE1"	, 5);
	gZoneByStateMap.insert("QC"		, 5);
	gZoneByStateMap.insert("PQ"		, 5);
	gZoneByStateMap.insert("VE2"	, 5);
	gZoneByStateMap.insert("ON"		, 4);
	gZoneByStateMap.insert("ONN"	, 4);
	gZoneByStateMap.insert("ONE"	, 4);
	gZoneByStateMap.insert("GAT"	, 4);
	gZoneByStateMap.insert("ONS"	, 4);
	gZoneByStateMap.insert("ONT"	, 4);
	gZoneByStateMap.insert("VE3"	, 4);
	gZoneByStateMap.insert("MB"		, 4);
	gZoneByStateMap.insert("VE4"	, 4);
	gZoneByStateMap.insert("SK"		, 4);
	gZoneByStateMap.insert("SASK"	, 4);
	gZoneByStateMap.insert("SAS"	, 4);
	gZoneByStateMap.insert("VE5"	, 4);
	gZoneByStateMap.insert("AB"		, 4);
	gZoneByStateMap.insert("ALB"	, 4);
	gZoneByStateMap.insert("VE6"	, 4);
	gZoneByStateMap.insert("BC"		, 3);
	gZoneByStateMap.insert("VE7"	, 3);
	gZoneByStateMap.insert("NWT"	, 1);
	gZoneByStateMap.insert("VE8"	, 1);
	gZoneByStateMap.insert("NF"		, 5);
	gZoneByStateMap.insert("VO1"	, 5);
	gZoneByStateMap.insert("LB"		, 2);
	gZoneByStateMap.insert("VO2"	, 2);
	gZoneByStateMap.insert("NU"		, 1);
	gZoneByStateMap.insert("VY0"	, 4);
	gZoneByStateMap.insert("YT"		, 1);
	gZoneByStateMap.insert("VY1"	, 1);
	gZoneByStateMap.insert("PE"		, 5);
	gZoneByStateMap.insert("PEI"	, 5);
	gZoneByStateMap.insert("VY2"	, 5);

	gZoneByStateMap.insert("ALA"	, 4);
	gZoneByStateMap.insert("ARI"	, 3);
	gZoneByStateMap.insert("ARK"	, 4);
	gZoneByStateMap.insert("CAL"	, 3);
	gZoneByStateMap.insert("COL"	, 4);
	gZoneByStateMap.insert("CON"	, 5);
	gZoneByStateMap.insert("DEL"	, 5);
	gZoneByStateMap.insert("FLO"	, 5);
	gZoneByStateMap.insert("GEO"	, 4);
	gZoneByStateMap.insert("IDA"	, 3);
	gZoneByStateMap.insert("ILL"	, 4);
	gZoneByStateMap.insert("IND"	, 4);
	gZoneByStateMap.insert("IOW"	, 4);
	gZoneByStateMap.insert("KAN"	, 4);
	gZoneByStateMap.insert("KEN"	, 4);
	gZoneByStateMap.insert("LOU"	, 4);
	gZoneByStateMap.insert("MAI"	, 4);
	gZoneByStateMap.insert("MAS"	, 4);
	gZoneByStateMap.insert("MIC"	, 4);
	gZoneByStateMap.insert("MIN"	, 4);
	gZoneByStateMap.insert("MIS"	, 4);
	gZoneByStateMap.insert("MON"	, 4);
	gZoneByStateMap.insert("NDA"	, 4);
	gZoneByStateMap.insert("NEB"	, 4);
	gZoneByStateMap.insert("NME"	, 4);
	gZoneByStateMap.insert("OHI"	, 4);
	gZoneByStateMap.insert("OKL"	, 4);
	gZoneByStateMap.insert("ORE"	, 3);
	gZoneByStateMap.insert("PEN"	, 5);
	gZoneByStateMap.insert("SDA"	, 4);
	gZoneByStateMap.insert("TEN"	, 4);
	gZoneByStateMap.insert("TEX"	, 4);
	gZoneByStateMap.insert("TEXAS"	, 4);
	gZoneByStateMap.insert("UTA"	, 3);
	gZoneByStateMap.insert("VER"	, 5);
	gZoneByStateMap.insert("VIR"	, 5);
	gZoneByStateMap.insert("WAS"	, 3);
	gZoneByStateMap.insert("WIS"	, 4);
	gZoneByStateMap.insert("WYO"	, 4);
}

void Contest::AcquireLogs()
{
	QTime timer;
	timer.start();

	QStringList filters;
	filters << "*.log";
	_logsDir.setFilter(QDir::Files | QDir::Readable);
	_logsDir.setNameFilters(filters);

	QFileInfoList fileInfoList = _logsDir.entryInfoList(); 						// List of files ending in .log
	gStdErrStrm << QString("\n%1 logs found in %2\n").arg(QLocale().toString(fileInfoList.size())).arg(_logsDir.absolutePath()) << endl;

	QList <Log *> logPtrs;

	if (gMP)
		logPtrs = QtConcurrent::blockingMapped(fileInfoList, Log::CreateLog);	// this is where Logs are allocated
	else
		foreach(QFileInfo fileInfo, fileInfoList)
			logPtrs << Log::CreateLog(fileInfo);

	foreach(Log * log, logPtrs)
	{
		if (!log)
			continue;

		if (_logsByCall.contains(log->Call()))
		{
			gContest->AppendErrMsgList(log->FileInfo().fileName(), QString("%1 %2 already used in %3")
				.arg("CALLSIGN:", -22)
				.arg(log->Call())
				.arg(_logsByCall[log->Call()]->FileInfo().fileName()));
			continue;
		}

		_logsByCall.insert(log->Call(), log);

		if (log->IsChecklog())
			_doNotCheckSet.insert(log->Call());

		if (_doNotCheckSet.contains(log->Call()))
			log->DoNotCheck(true);

		if (_noAwardSet.contains(log->Call()))
			log->IsNoAward(true);

		if (_administrativeChecklogSet.contains(log->Call()))
			log->IsAdministrativeChecklog(true);

		_qsoTotalCount += log->LogSize();
	}

	FillLogsByCallRcvdMap();

	_logCount = _logsByCall.size();
	gStdErrStrm << QString("%1 LoadLogs").arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
}

void Contest::MarkRunSp()
{
	// find passes
	// compute run/s&p from real qrg
	// compute run/s&p from rl qrg

	int const countNeeded = (Mode() == "CW") ? 3 : 2;		// initial qso plus three (CW) or two(SSB) matches

	foreach(Log * log, _logsByCall)
	{
		if (!log->AllQso().size())		// std::vector::size() returns an unsigned int
			continue;					// unsigned - 1 is a very large number, see below

		for (size_t i = 0; i + 1 < log->AllQso().size(); i++)			// search for passes
		{
			QsoPtr q = log->AllQso()[i];

			for (size_t j = i + 1; j < log->AllQso().size(); j++)
			{
				QsoPtr nq = log->AllQso()[j];

				int diffTime = q->DateTime().secsTo(nq->DateTime());
				if (diffTime > 300)
					break;

				if (q->CallRcvd() == nq->CallRcvd() && q->Band() != nq->Band())
				{
					nq->Running("P");
					nq->RlRunning("P");
				}
			}
		}

		// determine which bands have real qrgs by
		// looking at matching qsos with real qrg to see if band has real qrgs
		// band edges are not real

		foreach(QString band, log->GetQsoListByBandMap().keys())
		{
			int matchedQrgCount(0);
			int matchedQsoCount(0);

			foreach(QsoPtr q, log->GetQsoListByBandMap().value(band))
			{
				// count matched with real qrgs
				if (!q->IsBandEdge() && q->Matched() && !q->Matched()->IsBandEdge())
				{
					matchedQsoCount++;

					// count matched within 1 kHz
					if (abs(q->Qrg() - q->Matched()->Qrg()) <= 1)
					{
						matchedQrgCount++;
						// mark some that don't meet the 80% rule
						q->IsExactQrg(true);
						q->Matched()->IsExactQrg(true);
						_exactFreqCount++;
					}
				}
			}

			bool  exactQrgThisBand = float(matchedQrgCount) / float(matchedQsoCount) >= 0.80;

			log->InsertExactQrgByBand(band, exactQrgThisBand);
			if (exactQrgThisBand)
				foreach(QsoPtr q, log->GetQsoListByBandMap().value(band))				// mark individual qsos exact
				if (!q->IsBandEdge())
					q->IsExactQrg(true);
		}
	}

	foreach(Log * log, _logsByCall)
	{
		// mark R, S&P when we have good qrgs to work with

		foreach(QString band, log->GetQsoListByBandMap().keys())
		{
			if (log->ExactQrgByBand(band))
			{
				int lastRunQrg(0);
				QListIterator <QsoPtr> it(log->GetQsoListByBandMap().value(band));
				QListIterator <QsoPtr> searchIt(it);

				while (it.hasNext())
				{
					QsoPtr q = it.next();

					// not a possible run qrg

					if (q->Matched() && q->Matched()->IsExactQrg() && !q->IsExactQrg())
						q->MyComputedQrg(q->Matched()->Qrg());						// use his qrg

					if (!q->IsExactQrg() || q->Running() == "P")
						continue;

					int count(0);
					int nonExactCount(0);

					searchIt = it;

					while (searchIt.hasNext() && count < countNeeded)				// find 3 or 2 in  row
					{
						QsoPtr nq = searchIt.next();

						if (!q->IsExactQrg())
						{
							nonExactCount++;
							if (nonExactCount == 5)
								break;
						}

						// different rules by mode

						if ((Mode() == "CW" && q->Qrg() == nq->Qrg()) || (Mode() != "CW" && abs(q->Qrg() - nq->Qrg()) <= 1))
							count++;
						else
							break;
					}

					if (count == countNeeded)										// found a run qrg
					{
						lastRunQrg = q->Qrg();
						q->Running("R");

						while (it.hasNext())
						{
							q = it.peekNext();

							if (q->Matched() && q->Matched()->IsExactQrg())
								q->MyComputedQrg(q->Matched()->Qrg());


							if (!q->IsExactQrg() || q->Running() == "P")
							{
								it.next();
								continue;
							}


							if ((Mode() == "CW" && q->Qrg() == lastRunQrg) || (Mode() != "CW" && abs(q->Qrg() - lastRunQrg) <= 1))
							{
								lastRunQrg = q->Qrg();								// keep going to end of run
								q->Running("R");
								q->IsMyRun(true);
								it.next();
								continue;
							}
							break;
						}
					}
					else
						q->Running("S");
				}

				QsoPtr lastRunQ = 0;
				it.toFront();

				while (it.hasNext())
				{
					QsoPtr q = it.next();

					if (!q->IsExactQrg())
						continue;

					if (q->Running() == "P")													// skip passes
						continue;

					if (q->Running() == "R")													// found a run qso
					{
						lastRunQ = q;
						continue;
					}

					if (lastRunQ && lastRunQ->DateTime().secsTo(q->DateTime()) > 300)		// five minutes is up
					{
						lastRunQ = 0;
						continue;
					}

					if (lastRunQ && ((Mode() == "CW" && q->Qrg() == lastRunQrg) || (Mode() == "SSB" && abs(q->Qrg() - lastRunQrg) <= 1)))
					{
						q->Running("R+");
					}
				}

				lastRunQ = 0;
				it.toBack();
				while (it.hasPrevious())
				{
					QsoPtr q = it.previous();

					if (!q->IsExactQrg())
						continue;

					if (q->Running() == "P")													// pass
						continue;

					if (q->Running() == "R")													// found a run qso
					{
						lastRunQ = q;
						continue;
					}

					if (lastRunQ && q->DateTime().secsTo(lastRunQ->DateTime()) > 300)		// five minutes is up
					{
						lastRunQ = 0;
						continue;
					}

					if (lastRunQ && ((Mode() == "CW" && q->Qrg() == lastRunQrg) || (Mode() == "SSB" && abs(q->Qrg() - lastRunQrg) <= 1)))
					{
						q->Running("R-");
					}
				}
			}

			if (true)			// compute from RL, even if we have exact QRGs
			{
				int	lastRunQrg = 0;
				QListIterator <QsoPtr> it(log->GetQsoListByBandMap().value(band));

				while (it.hasNext())
				{
					QsoPtr q = it.next();

					if (q->Running() == "P")
						continue;

					if (!q->Matched() || !q->Matched()->IsExactQrg())					// not a possible run qrg
						continue;

					q->MyComputedQrg(q->Matched()->Qrg());								// use his qrg

					QListIterator <QsoPtr> nextIt(it);
					int count(0);
					int nonExactCount(0);
					while (nextIt.hasNext() && count < countNeeded)
					{
						QsoPtr nq = nextIt.next();
						if (!nq->Matched() || !nq->Matched()->IsExactQrg())				// only test exact rl qrgs
							continue;

						if (abs(q->Matched()->Qrg() - nq->Matched()->Qrg()) <= 1)
							count++;
						else
						{
							nonExactCount++;
							if (nonExactCount == 5)
								break;
						}
					}

					if (count == countNeeded)										// found a run qrg
					{
						lastRunQrg = q->Matched()->Qrg();
						q->RlRunning("R");

						while (it.hasNext())
						{
							q = it.peekNext();
							if (!q->Matched() || !q->Matched()->IsExactQrg() || q->Running() == "P")
							{
								it.next();
								continue;
							}

							if (abs(q->Matched()->Qrg() - lastRunQrg) <= 1)
							{
								lastRunQrg = q->Matched()->Qrg();						// keep going to end of run
								q->RlRunning("R");
								it.next();
								continue;
							}
							break;
						}
					}
					else
						q->RlRunning("S");
				}

				QsoPtr lastRunQ = 0;
				QDateTime lastRunTime;
				it.toFront();
				while (it.hasNext())
				{
					QsoPtr q = it.next();

					if (q->Running() == "P")												// pass
						continue;

					if (q->RlRunning() == "R")											// found a RL run qso
					{
						lastRunQ = q;
						lastRunQrg = q->MyComputedQrg();
						lastRunTime = q->DateTime();
						continue;
					}

					if (lastRunQ)
					{
						q->MyComputedQrg(lastRunQrg);
						if (lastRunTime.secsTo(q->DateTime()) > 300)						// five minutes is up
						{
							if (q->Matched() && q->Matched()->IsExactQrg())
							{
								q->RlRunning("S+");				// S&P with exact QRTG in my log
							}
							else
							{
								q->RlRunning("U");				// Give up, can't determine run or s&p
								q->MyComputedQrg(0);
							}
							lastRunQ = 0;
							lastRunQrg = 0;
							continue;
						}

						if (!q->Matched() || !q->Matched()->IsExactQrg())				// still running
						{
							q->RlRunning("R+M");
							lastRunTime = q->DateTime();
						}
						else
						{
							if ((Mode() == "CW" && q->Matched()->Qrg() == lastRunQrg)		// found another matching qrg
								|| (Mode() == "SSB" && abs(q->Matched()->Qrg() - lastRunQrg) <= 1))
							{
								q->RlRunning("R*+");
								lastRunTime = q->DateTime();
							}
							else												// found a S&P QSO
							{
								if (q->Matched() && q->Matched()->IsExactQrg())
									q->MyComputedQrg(q->Matched()->Qrg());
								else
									q->MyComputedQrg(0);
								q->RlRunning("S+");
							}
						}
					}
					else
					{
						q->RlRunning("");
						lastRunQrg = 0;
					}
				}

				lastRunQrg = 0;
				lastRunQ = 0;
				it.toBack();

				// search backwards for a remnent run qso

				while (it.hasPrevious())
				{
					QsoPtr q = it.previous();

					if (q->Running() == "P")						// pass
						continue;

					if (q->RlRunning().contains('R'))
					{
						lastRunQ = q;
						lastRunQrg = q->MyComputedQrg();
						lastRunTime = q->DateTime();
						continue;
					}

					// just look for timeout and random R-

					if (lastRunQ)
					{
						if (q->DateTime().secsTo(lastRunTime) > 600)		// ten minutes is up
						{
							lastRunQrg = 0;
							lastRunQ = 0;
							continue;
						}

						if (q->Matched() && q->Matched()->IsExactQrg() && lastRunQrg == q->Matched()->Qrg()) // extra run qso
						{
							q->MyComputedQrg(lastRunQrg);
							q->RlRunning("R-");
							lastRunTime = q->DateTime();
						}
					}
				}
			}
		}
	}
}

int Contest::WriteRevLogs()
{
	int revLogQsoCount = 0;

	// Fill the reverse logs for every call received from every log
	foreach(Log * log, _logsByCall)
		foreach(QsoPtr q, log->AllQso())				// load rl map by call here
			_revLogMap[q->CallRcvd()].AllQso().push_back(q);

	QList <RevLog *> rlPtrs;
	for (RevLogMap::iterator it = _revLogMap.begin(); it != _revLogMap.end(); ++it)
		rlPtrs.append(&(it->second));

	if (gMP)
		QtConcurrent::blockingMap(rlPtrs, RevLog::SortRevLogs);
	else
		foreach(RevLog * ptr, rlPtrs)
			RevLog::SortRevLogs(ptr);

	QString revCountFileName = _resDir.absoluteFilePath(Name().toLower() + ".rl");
	QFile revCountFile(revCountFileName);
	revCountFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream revCountTextStream(&revCountFile);
	revCountTextStream << QString("%1 %2").arg("Call", -12).arg("Number of QSO") << endl;

	QMultiMap <size_t, QString> revLogCounts;

	for (RevLogMap::iterator rlmIt = _revLogMap.begin(); rlmIt != _revLogMap.end(); rlmIt++)
	{
		QString call = rlmIt->first;
		if (!GetLog(call))
			revLogCounts.insert(rlmIt->second.AllQso().size(), call);
	}

	QMapIterator< size_t, QString > it(revLogCounts);
	it.toBack();
	while (it.hasPrevious())
	{
		it.previous();
		revCountTextStream << QString("%1 %2\n").arg(it.value(), -12).arg(it.key());
	}

	revLogCounts.clear();
	revCountFile.close();

	if (gMP)
		QtConcurrent::blockingMap(rlPtrs, RevLog::WriteRevLogs);
	else
		foreach(RevLog * rlPtr, rlPtrs)
			RevLog::WriteRevLogs(rlPtr);

	return revLogQsoCount;
}

void Contest::MarkNilBust()
{
	foreach(Log * log, _logsByCall)
		Log::CheckBusts(log);
	RunMultiThreaded(Log::CheckXchs);
}

void Contest::CheckLogs()
{
	QTime timer;
	timer.start();

	AlignTimes();				gStdErrStrm << QString("%1 AlignTimes")				.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteRevLogs();				gStdErrStrm << QString("%1 WriteRevLogs")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	DeriveExchInfo();			gStdErrStrm << QString("%1 DeriveExchInfo")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	RepairCrossBandQsos();		gStdErrStrm << QString("%1 RepairCrossBandQsos")	.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkMultiOpBCV();			gStdErrStrm << QString("%1 MarkMultiOpBCV")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkFccBusts();				gStdErrStrm << QString("%1 MarkFccBusts")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkRunSp();				gStdErrStrm << QString("%1 MarkRunSp")				.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkNilBust();				gStdErrStrm << QString("%1 MarkNilBust")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkDupes();				gStdErrStrm << QString("%1 MarkDupes")				.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkBadCalls();				gStdErrStrm << QString("%1 MarkBadCalls")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkRubberClockedQsos();	gStdErrStrm << QString("%1 MarkRubberClockedQsos")	.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteCllFile();				gStdErrStrm << QString("%1 WriteCllFile")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteDupeFiles();			gStdErrStrm << QString("%1 WriteDupeFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteFccBusts();			gStdErrStrm << QString("%1 WriteFccBusts")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteRepairedCalls();		gStdErrStrm << QString("%1 WriteRepairedCalls")		.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteUniques();				gStdErrStrm << QString("%1 WriteUniques")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteUniqueMults();			gStdErrStrm << QString("%1 WriteUniqueMults")		.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteUniqueNils();			gStdErrStrm << QString("%1 WriteUniqueNils")		.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteSbxFile();				gStdErrStrm << QString("%1 WriteSbxFile")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteErrFiles();			gStdErrStrm << QString("%1 WriteErrFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	FillReportLists();			gStdErrStrm << QString("%1 FillReportLists")		.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	ComputeScores();			gStdErrStrm << QString("%1 ComputeScores")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	UpdateCounts();				gStdErrStrm << QString("%1 UpdateCounts")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteCbmFiles();			gStdErrStrm << QString("%1 WriteCbmFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteResultFiles();			gStdErrStrm << QString("%1 WriteResultFiles")		.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteLgFiles();				gStdErrStrm << QString("%1 WriteLgFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteTabFile();				gStdErrStrm << QString("%1 WriteTabFile")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteTsvFile();				gStdErrStrm << QString("%1 WriteTsvFile")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteResultsTxtFile();		gStdErrStrm << QString("%1 WriteResultsTxtFile")	.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteMultFiles();			gStdErrStrm << QString("%1 WriteMultFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteObtFile();				gStdErrStrm << QString("%1 WriteObtFile")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteRptFiles();			gStdErrStrm << QString("%1 WriteRptFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteSptFiles();			gStdErrStrm << QString("%1 WriteSptFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	MarkCertificateWinners();	gStdErrStrm << QString("%1 MarkCertificateWinners")	.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteMagazineFile();		gStdErrStrm << QString("%1 WriteMagazineFile")		.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteAllFiles();			gStdErrStrm << QString("%1 WriteAllFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteClubsFile();			gStdErrStrm << QString("%1 WriteClubsFile")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteCtyFiles();			gStdErrStrm << QString("%1 WriteCtyFiles")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
	WriteOpsFile();				gStdErrStrm << QString("%1 WriteOpsFile")			.arg(float(timer.restart()) / 1000, 6, 'f', 1) << endl;
}

Log * Contest::GetLog(QString call)
{
	if (!_logsByCall.contains(call))
		return 0;
	else
		return _logsByCall.value(call);
}

void Contest::LoadFccData()
{
	QString fccDataFileName("fcc.dat");
	QDir fccDataDir = _logsDir;

	if (fccDataDir.exists(fccDataFileName))
	{
		QFile fccDataFile(_logsDir.absoluteFilePath(fccDataFileName));
		fccDataFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream fccDataStream(&fccDataFile);
		while (!fccDataStream.atEnd())
		{
			QString line = fccDataStream.readLine().simplified();
			QStringList parts = line.split(' ', QString::SkipEmptyParts);
			if (parts.size() > 1)
				gStateByFccCallMap.insert(parts[0], parts[1]);

			if (parts.size() > 2)
				gLicenseByFccCallMap.insert(parts[0], parts[2]);
		}
		fccDataFile.close();
	}
}

void Contest::PrintStats() const
{
	gStdOutStrm << endl;
	gStdOutStrm << QString("%1 Logs\n").arg(QLocale().toString(_logsByCall.size()), 9);
	gStdOutStrm << QString("%1 QSOs\n").arg(QLocale().toString(int(_qsoTotalCount)), 9);
	gStdOutStrm << QString("%1 Countries\n").arg(QLocale().toString(_qsoListsByStdPx.size()), 9);

	gStdOutStrm << QString("%1 (%2%) cross band qso\n")
		.arg(QLocale().toString(_isCrossBandCount), 9)
		.arg((100.0 * _isCrossBandCount) / _qsoTotalCount,  4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) qso with exact frequencies\n")
		.arg(QLocale().toString(_exactFreqCount), 9)
		.arg((100.0 * _exactFreqCount) / _qsoTotalCount,  4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) qso with unique calls\n")
		.arg(QLocale().toString(_uniquesCount), 9)
		.arg((100.0 * _uniquesCount) / _qsoTotalCount,  4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) Cabrillo format errors\n")
		.arg(QLocale().toString(_formatErrCount), 9)
		.arg((100.0 * _formatErrCount) / _qsoTotalCount, 4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) qso checked against another log\n")
		.arg(QLocale().toString(_checkableCount), 9)
		.arg((100.0 * _checkableCount) / _qsoTotalCount,  4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) qso checked good when checked against another log\n")
		.arg(QLocale().toString(_goodMatchCount), 9)
		.arg((100.0 * _goodMatchCount) / _checkableCount, 4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) unique calls busted\n")
		.arg(QLocale().toString(_uniquesNotCounted), 9)
		.arg((100.0 * _uniquesNotCounted) / _uniquesCount, 4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) dupes\n")
		.arg(QLocale().toString(_dupeCount), 9)
		.arg((100.0 * _dupeCount) / _qsoTotalCount, 4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) busted exchange\n")
		.arg(QLocale().toString(_bustedXchCount), 9)
		.arg((100.0 * _bustedXchCount) / _checkableCount, 4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) busted calls\n")
		.arg(QLocale().toString(_bustedCallCount), 9)
		.arg((100.0 * _bustedCallCount) / _checkableCount, 4, 'f', 1);

	//gStdOutStrm << QString("%1 (%2%) busted reverse log calls\n")
	//	.arg(QLocale().toString(_bustedRevLogCallCount), 9)
	//	.arg((100.0 * _bustedRevLogCallCount)   / _checkableCount, 4, 'f', 1);

	gStdOutStrm << QString("%1 (%2%) not in log\n")
		.arg(QLocale().toString(_notInLogCount), 9)
		.arg((100.0 * _notInLogCount) / _checkableCount, 4, 'f', 1);

	gStdOutStrm << QString("%1 Total Score\n").arg(QLocale().toString(uint(_totalScore)));
	gStdOutStrm << endl;
}

void Contest::WriteTabFile()
{
	QFile tabFile(_resDir.absoluteFilePath(Name().toLower() + ".tab"));
	tabFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream outStream(&tabFile);
	outStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));

	WriteTabHeader(outStream);

	foreach(Log * log, _logsByCall)
	{
		if (log->Disqualified())
			continue;

		log->WriteTabLine(outStream);
	}
	tabFile.close();
}

void Contest::WriteSprHdr(QTextStream & sprStrm)
{
	sprStrm
		<< "Call"                      			<< '\t'
		<< "Px"                        			<< '\t'
		<< "Cont"                      			<< '\t'
		<< "Zone"                      			<< '\t'
		<< "Dist"                      			<< '\t'
		<< "Cat"                       			<< '\t'
		<< "Score"                     			<< '\t'
		<< "World"                     			<< '\t'
		<< "EU"                     			<< '\t'
		<< "US"                     			<< '\t'
		<< "160M exact QRG"             		<< '\t'
		<< "80M exact QRG"             			<< '\t'
		<< "40M exact QRG"             			<< '\t'
		<< "20M exact QRG"             			<< '\t'
		<< "15M exact QRG"             			<< '\t'
		<< "10M exact QRG"             			<< '\t'
		<< "ALL QSO"							<< '\t'
		<< "Run QSO"							<< '\t'
		<< "Pass QSO"							<< '\t'
		<< "Unknown QSO"						<< '\t'
		<< "S&P QSO"							<< '\t'
		<< "S&P Hits"							<< '\t'
		<< "% S&P of All QSO"					<< '\t'
		<< "% S&P of All Mults"					<< '\t'
		<< "% S&P Hits of All QSO"				<< '\t'
		<< "% S&P Hits of S&P QSO"				<< '\t'
		<< "ALL Mults"							<< '\t'
		<< "S&P Mults"							<< '\t'
		<< "S&P Mult Hits"						<< '\t'
		<< "% S&P Mult of All Mult"				<< '\t'
		<< "% S&P Mult Hits of All Mult"		<< '\t'
		<< "% S&P Mult Hits of All S&P"			<< '\t'
		<< "% S&P Mult Hits of S&P Mults"		<< '\t'
		<< "% S&P Mult Hits of S&P Hits"		<< '\t'
		<< "ALL QSO Day1"						<< '\t'
		<< "Run QSO Day1"						<< '\t'
		<< "Pass QSO Day1"						<< '\t'
		<< "Unknown QSO Day1"					<< '\t'
		<< "S&P QSO Day1"						<< '\t'
		<< "S&P Hits Day1"						<< '\t'

		<< "% S&P of All QSO Day1"				<< '\t'
		<< "% S&P of All Mults Day1"			<< '\t'
		<< "% S&P Hits of All QSO Day1"			<< '\t'
		<< "% S&P Hits of S&P QSO Day1"			<< '\t'

		<< "ALL Mults Day1"						<< '\t'
		<< "S&P Mults Day1"						<< '\t'
		<< "S&P Mult Hits Day1"					<< '\t'

		<< "% S&P Mult of All Mult Day1"		<< '\t'
		<< "% S&P Mult Hits of All Mult Day1"	<< '\t'
		<< "% S&P Mult Hits of All S&P Day1"	<< '\t'
		<< "% S&P Mult Hits of S&P Mults Day1"	<< '\t'
		<< "% S&P Mult Hits of S&P Hits Day1"	<< '\t'

		<< "ALL QSO Day2"						<< '\t'
		<< "Run QSO Day2"						<< '\t'
		<< "Pass QSO Day2"						<< '\t'
		<< "Unknown QSO Day2"					<< '\t'
		<< "S&P QSO Day2"						<< '\t'
		<< "S&P Hits Day2"						<< '\t'

		<< "% S&P of All QSO Day2"				<< '\t'
		<< "% S&P of All Mults Day1"			<< '\t'
		<< "% S&P Hits of All QSO Day2"			<< '\t'
		<< "% S&P Hits of S&P QSO Day2"			<< '\t'

		<< "ALL Mults Day2"						<< '\t'
		<< "S&P Mults Day2"						<< '\t'
		<< "S&P Mult Hits Day2"					<< '\t'

		<< "% S&P Mult of All Mult Day2"		<< '\t'
		<< "% S&P Mult Hits of All Mult Day2"	<< '\t'
		<< "% S&P Mult Hits of All S&P Day2"	<< '\t'
		<< "% S&P Mult Hits of S&P Mults Day2"	<< '\t'
		<< "% S&P Mult Hits of S&P Hits Day2"	<< '\t'

		<< '\n';
	}

void Contest::WriteOpsFile() {}

void Contest::WriteRedAndYellowCards(QTextStream & magStream)
{
	_disqualifiedSet += _yellowCardSet + _redCardSet;
	if(_disqualifiedSet.size())
	{
		QStringList dqList =  _disqualifiedSet.toList();
		dqList.sort();
		magStream << "\nDisqualified: " << dqList.join(", ") << '!' << endl;
	}
}

void Contest::WriteHqCountsFile() {}

void Contest::WriteSbxFile()
{
	QFile sbxFile(_resDir.absoluteFilePath(Name().toLower() + ".sbx"));
	sbxFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream sbxStream(& sbxFile);

	foreach(Log * log, _logsByCall)
		if(!log->SoapBox().isEmpty())
			sbxStream << QString("%1...%2\n").arg(log->SoapBox()).arg(log->Call());
	sbxFile.close();
}

QDir Contest::SetupSubdir(QString name)
{
	QDir dir = _logsDir;
	dir.cdUp();

	if (!dir.exists(name))
	{
		gStdErrStrm << QString("Creating directory: %1").arg(name) << endl;
		dir.mkdir(name);
	}
	dir.cd(name);

	foreach(QFileInfo fileInfo, dir.entryInfoList())
		QFile(fileInfo.filePath()).remove();
	return dir;
}

int Contest::CountOffsets(int maxOffset)
{
	int count(0);

	foreach(Log * log, _logsByCall)
		foreach(QsoPtr q, log->AllQso())
			if (abs(q->MatchedTimeDiff()) / 60 > maxOffset)
				count++;
	return count;
}

Contest::~Contest()
{
	QMutableMapIterator <QString, Log * > logIterator(_logsByCall);
	while (logIterator.hasNext())
		delete logIterator.next().value();
}

void Contest::AlignTimes()
{
	WriteOcpFile();
	// initial match and reporting 
	int maxOffset = 2;
	RunMultiThreaded(Log::MatchQsos);
	RunMultiThreaded(Log::ReportTimeMatching);

	foreach(Log * log, _logsByCall)	// this is because we can't send a function with parameters through QT concurrent
		log->IsFinalTm(true);

	QFile offFile(_resDir.absoluteFilePath(Name().toLower() + ".tm.initial"));
	offFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream offStream(&offFile);

	offStream << QString("Initial matched pairs with offset > %1 minutes\n\n").arg(maxOffset);

	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr q, log->AllQso())
		{
			if (!q->Matched())
				continue;

			int offset = q->MatchedTimeDiff();

			if (abs(offset) / 60 > maxOffset)
				offStream << QString("%1  %2\n").arg(q->ToString()).arg(offset / 60, 7);
		}
	}
	offStream << QString("Total qsos with > %1 minute offset %2").arg(maxOffset).arg(CountOffsets(maxOffset)) << endl;;
	offFile.close();

	// normalize times in the logs
	// first adjust on a full log basis
	for (int i = 0; i < 5; i++)
	{
		int unalignedCount(0);
		QList <bool> mediansList;

		if (gMP)
			mediansList = QtConcurrent::blockingMapped(_logsByCall.values(), Log::AdjustTimes);
		else
			foreach(Log * log, _logsByCall)
				mediansList << Log::AdjustTimes(log);

		foreach(bool res, mediansList)
			unalignedCount += res;

		if (unalignedCount == 0)
			break;

		RunMultiThreaded(Log::MatchQsos);		// re-match
	}

	QFile  tmeFile(_resDir.absoluteFilePath(Name().toLower() + ".tm.adj_log"));
	tmeFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream tmeStream(&tmeFile);

	int adjustedCount(0);

	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr q, log->AllQso())
			if (q->TimeAdjustment())
				adjustedCount++;

		if (log->TimeAdjustment())
			tmeStream << QString("%1 adjusted by %2 minutes\n").arg(log->Call(), -12).arg(log->TimeAdjustment() / 60, 5);
	}

	tmeStream << QString("Total qsos adjusted by shifting entire logs: %1").arg(adjustedCount) << endl;
	tmeStream << QString("Total qsos with > %1 minute offset: %2").arg(maxOffset).arg(CountOffsets(maxOffset)) << endl;;
	tmeFile.close();

	// adjust logs with bad blocks of time
	QFile tmbFile(_resDir.absoluteFilePath(Name().toLower() + ".tm.adj_blocks"));
	tmbFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream tmbStream(&tmbFile);
	adjustedCount = 0;

	foreach(Log * log, _logsByCall)
	{
		bool modified(false);
		int minDiff = 20 * 60;			// offset must be over minDiff 
		int maxJump = 10 * 60;			// offset can't jump more than maxJump

		for (auto it = log->AllQso().begin(); it != log->AllQso().end(); ++it)
		{
			int count(1);
			QsoPtr q = *it;
			if (!q->Matched() || abs(q->MatchedTimeDiff()) < minDiff) { continue; }


			auto sit = it;
			int lastOffset = q->MatchedTimeDiff();

			// found offset > minDiff at start of block
			for (; it != log->AllQso().end(); ++it)
			{
				q = *it;
				if (!q->Matched())
					continue;			// skip unmatched qsos

										// find end of a sequence of offsets > minDiff
				if (abs(q->MatchedTimeDiff() - lastOffset) > maxJump || abs(q->MatchedTimeDiff()) < minDiff)
					break;

				lastOffset = q->MatchedTimeDiff();
				count++;
			}
			auto eit = it;

			if (count >= 4)
			{
				int initialOffset = (*sit)->MatchedTimeDiff();

				// adjust non-matched qsos too

				tmbStream << "Start of time correction  " << count << endl;
				it = sit;

				for (auto i = sit; i != eit; ++i)
				{
					QsoPtr q = *i;
					QsoPtr rq = q->Matched();
					int delta = rq ? q->MatchedTimeDiff() : initialOffset;

					tmbStream << QString("%1  adjusted by %2 minutes\n").arg(q->ToString()).arg(delta / 60, 5);
					q->AdjustTime(delta);
					adjustedCount++;
				}
				tmbStream << "End of time correction\n" << endl;
				modified = true;
			}
			if (it == log->AllQso().end())
				break;
		}
		if (modified)			// modification may change time order
			log->SortAllQso();
	}

	tmbStream << QString("Total qsos adjusted by shifting blocks %1").arg(adjustedCount) << endl;
	tmbStream << QString("Total qsos remaining with > %1 minute offset %2").arg(maxOffset).arg(CountOffsets(maxOffset)) << endl;;
	tmbFile.close();

	RunMultiThreaded(Log::MatchQsos);			// re-match after adjustments
	RunMultiThreaded(Log::ReportTimeMatching);	// final time matches for each log

	QFile f(_resDir.absoluteFilePath(Name().toLower() + ".tm.final"));
	f.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream outstrm(&f);

	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr q, log->AllQso())
		{
			if (!q->Matched())
				continue;

			int offset = q->MatchedTimeDiff();

			if (abs(offset) > maxOffset * 60)
				outstrm << QString("%1  %2\n").arg(q->ToString()).arg(offset / 60, 7);
		}
	}
	outstrm << QString("Total qsos remaining with > %1 minute offset %2").arg(maxOffset).arg(CountOffsets(maxOffset)) << endl;
	f.close();

	// final match with new window size
	MatchWindowSize(10 * 60);
	RunMultiThreaded(Log::MatchQsos);
}

void Contest::AppendErrMsgList(QString msg, QString arg) { _errMsgList.append(QString("%1 %2").arg(msg, -16).arg(arg)); }

void Contest::MarkFccBusts()
{
	if (gStateByFccCallMap.size())
	{
		foreach(Log * log, _logsByCall)					// check against FCC Data, excluded list
		{
			foreach(QsoPtr q, log->AllQso())
			{
				if (q->Status() != NO_LOG)
					continue;

				if (!log->DoNotCheck() &&
					q->CtyRcvd()->IsInFccData() &&
					q->CallRcvd().size() > 3 &&
					!_goodCallsSet.contains(q->CallRcvd()) &&
					!q->IsBadCallByList() &&
					!q->CallRcvd().contains('/') &&		// portable in US don't get checked
					!gStateByFccCallMap.contains(CallFormer().RootCall(q->CallRcvd())))
				{
					q->Status(BUSTED_FCC_CALL);
					log->AppendBustedFccCallList(q);
					if (q->Matched() && !q->IsExcluded())
						q->Matched()->GetLogPtr()->AppendBustCausedList(q);
					_badFccCallCounts[q->CallRcvd()]++;
				}
			}
		}
	}

	foreach(QString call, _badFccCallCounts.keys())
	{
		QString line = QString("%1 %2").arg(call, -12).arg(_badFccCallCounts.value(call), 4);
		_fccByCount.insert(_badFccCallCounts.value(call), line);
	}
}

void Contest::MarkMultiOpBCV()	{ RunMultiThreaded(Log::CheckBandChanges);	}

void Contest::MarkUniques()		{ RunMultiThreaded(Log::MarkUniques);		}

void Contest::Compute20MinRates()
{
	// only compute 20 minute rates when checking spots
	if (!gSpt && !gRbn)
		return;

	RunMultiThreaded(Log::MatchQsos);
	RunMultiThreaded(Log::FillTwentyMinRates);
}

void Contest::WriteSptFiles()
{
	if (!gSpt && !gRbn)
		return;

	Compute20MinRates();

	int rbnCount = 0;
	int sptCount = 0;
	QString rbnInFileName("rbn.tab");
	QString sptInFileName("spots.tab");
	QTime timer;
	timer.start();

	if (gRbn && _logsDir.exists(rbnInFileName))
	{
		QFile rbnFile(_logsDir.filePath(rbnInFileName));
		QTextStream rbnStrm(&rbnFile);

		if (rbnFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			while (!rbnStrm.atEnd())
			{
				QString line = rbnStrm.readLine();
				Spot * spt = new Spot(line);
				if (spt)
				{
					_spotsByBandCallTimeMap[spt->Band()][spt->Call()].push_back(spt);
					rbnCount++;
				}
				else
					AppendErrMsgList("RBN:", "spot number " + QString::number(rbnCount) + " is bad");
			}
			rbnFile.close();
		}
	}

	if (gSpt && _logsDir.exists(sptInFileName))				// fill _spotsByBandCallTimeMap
	{
		QFile sptFile(_logsDir.filePath(sptInFileName));
		QTextStream sptStrm(&sptFile);

		if (sptFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			while (!sptStrm.atEnd())
			{
				QString line = sptStrm.readLine();
				Spot * spt = new Spot(line);
				if (spt)
				{
					_spotsByBandCallTimeMap[spt->Band()][spt->Call()].push_back(spt);
					sptCount++;
				}
				else
					AppendErrMsgList("RBN:", line);
			}
			sptFile.close();
		}
	}

	foreach(QString band, _spotsByBandCallTimeMap.keys())
		foreach(QString call, _spotsByBandCallTimeMap[band].keys())
	{
		std::vector <Spot *> & spots = _spotsByBandCallTimeMap[band][call];
		std::stable_sort(spots.begin(), spots.end(), gSpotTimeLessThanSpotTime);

		Spot * firstSpt = *spots.begin();

		firstSpt->First(true);
		if (spots.size() == 1)
			firstSpt->IsUnique(true);

		double lastQrg = firstSpt->Qrg();

		for (auto it(spots.begin()); it != spots.end(); it++)
		{
			SpotPtr spt = *it;

			// find isolated spots (+/- 90 min)
			if (it != spots.begin() && it + 1 != spots.end() && (*(it - 1))->DateTime().secsTo(spt->DateTime()) > 60 * 90 && spt->DateTime().secsTo((*(it + 1))->DateTime()) > 60 * 90)
				spt->Isolated(true);

			// find spots that qsy
			if (abs(spt->Qrg() - lastQrg) >= 1.5)
			{
				spt->NewFreq(true);
				lastQrg = spt->Qrg();
			}
		}
	}

	RunMultiThreaded(Log::ComputeSpotInfo);
	WriteSprFile();
	WriteRlsFile();
}

void Contest::ComputeScores() 
{ 
	RunMultiThreaded(Log::ComputeScore);
	// total score for entire contest
	foreach(Log * log, _logsByCall)
		_totalScore += log->Score();
}

void Contest::FillCtyLists()
{
	foreach(Log * log, _logsByCall)
		foreach(QsoPtr q, log->AllQso())							// make lists of countrys worked for credit
			if (q->CountsForCredit())
				_qsoListsByStdPx[q->CtyRcvd()->StdPx()].append(q);	// count countries

	foreach(auto key, _qsoListsByStdPx.keys())
		std::stable_sort(_qsoListsByStdPx[key].begin(), _qsoListsByStdPx[key].end(), gQsoTimeLessThanQsoTime);
}

void Contest::FillLogsByCallRcvdMap()
{
	foreach(Log * log, _logsByCall)
		foreach(auto q, log->AllQso())
			_logByCallRcvdMap[q->CallRcvd()].insert(log->Call());
}

void Contest::FillReportLists()
{
	foreach(Log * log, _logsByCall)
		log->FillReportLists();
}

void Contest::MarkBadCalls()
{
	foreach(Log * log, _logsByCall)						// check for explicit busts
		foreach(QsoPtr q, log->AllQso())
		if (q->IsBadCallByList() && !q->IsExcluded())
		{
			q->Status(BUSTED_LIST_CALL);
			if (q->Matched() && !q->IsExcluded())
				q->IsBadCallByList(q->Matched()->GetLogPtr());
		}
}

void Contest::MarkDupes() { RunMultiThreaded(Log::CheckDupes); }

void Contest::MarkRubberClockedQsos()
{
	foreach(Log * log, _logsByCall)
		foreach(QsoPtr q, log->RubberClockedByBandMap())
		if (q->IsRubberClocked())
			q->Status(RUBBER_CLOCK_VIOLATION);
}

void Contest::RepairCrossBandQsos()
{
	int fixedCrossBandCount = 0;
	int singleFixedCrossBandCount = 0;

	foreach(Log * log, _logsByCall)			// attempt to fix cross band qsos
	{
		for (std::vector<Qso *>::const_iterator it = log->AllQso().begin(); it != log->AllQso().end(); ++it)
		{
			if (!(*it)->CrossBandMatch())		// enter the while loop when we have a  cross bander
				continue;

			std::vector<Qso *>::const_iterator startIt = it;
			int count(0);
			int cbCount(0);

			while (true)
			{
				count++;
				QsoPtr q = *it;
				QsoPtr rq = q->CrossBandMatch();

				if (rq)
					cbCount++;

				std::vector<Qso *>::const_iterator next = it + 1;		// peek next
				if ((next == log->AllQso().end()) || !(*next)->CrossBandMatch())
					break;
				++it;
			}

			if (cbCount == 1)
			{
				QsoPtr q = *startIt;
				QsoPtr rq = q->CrossBandMatch();

				if (q->Running() == "R" && rq->Running() != "R")
				{
					singleFixedCrossBandCount++;
					q->Status(GOOD_QSO);
				}

				if (q->Running() != "R" && rq->Running() == "R")
				{
					singleFixedCrossBandCount++;
					rq->Status(GOOD_QSO);
				}
			}

			if (cbCount >= 3)
			{
				QString correctBand = (*startIt)->Band();			// first is guaranteed to be a cross bander
				for (int i = 0; i < count; i++)
				{
					QsoPtr q = startIt[i];
					QsoPtr rq = q->CrossBandMatch();

					if (rq)											// give the innocent the qso
					{
						rq->Status(GOOD_QSO);
						fixedCrossBandCount++;
					}
					else
					{
						if (q->Band() != correctBand)
							q->Band(correctBand);					// FIX THIS need to go back move qso from old to new band lists
					}
				}
			}
		}
	}
}

void Contest::UpdateCounts()
{
	// update various contest counts
	foreach(Log * log, _logsByCall)
	{
		_goodMatchCount += log->GoodList().size();
		_dupeCount += log->DupeList().size();
		_notInLogCount += log->NilList().size();
		_bustedCallCount += log->BustedCallList().size() + log->BadRevLogList().size();
		_bustedRevLogCallCount += log->BadRevLogList().size();
		_bustedXchCount += log->BustedXchList().size();
		_formatErrCount += log->ErrorMsgList().size();
		_checkableCount += log->CheckableCount();
		_nonCheckableCount += log->NotCheckedList().size();
		_isCrossBandCount += log->CrossBandList().size();
	}
}

void Contest::WriteCbmFiles()
{
	QMapIterator < QString, Log * >  logsByCallIterator(_logsByCall);
	while (logsByCallIterator.hasNext())
	{
		Log * log = logsByCallIterator.next().value();

		if (log->CrossBandList().size())
		{
			double percentCrossBand = log->CheckableCount() ? 100.0 * log->CrossBandList().size() / log->CheckableCount() : 0;
			QString line = QString("%1 (%2%) %3 of %4 cross band qsos")
				.arg(log->Call(), -12)
				.arg(percentCrossBand, 6, 'f', 1)
				.arg(log->CrossBandList().size(), 4)
				.arg(log->CheckableCount(), 5);
			_crossBandByCount.insert(log->CrossBandList().size(), line);
		}

		if (log->CrossBandList().size())		 		// write out cross band
		{
			QString cbmFileName = _cbmDir.absoluteFilePath(log->FileInfo().baseName() + ".cbm");
			QFile cbmFile(cbmFileName);
			cbmFile.open(QIODevice::WriteOnly | QIODevice::Text);
			QTextStream cbmTextStream(&cbmFile);

			foreach(QsoPtr q, log->CrossBandList())
			{
				if (!q)
				{
					gStdErrStrm << "Zero Qso Ptr in crossband list" << endl;
					continue;
				}

				QsoPtr rq = q->CrossBandMatch();

				if (!rq)
				{
					gStdErrStrm << "Crossband qso not found" << endl;
					continue;
				}

				QsoListsByBandCtyRcvdMap  const multMap = log->GetMultMap();

				cbmTextStream << QString("%1 %2\n%3\n\n")
					.arg(q->ToString())
					.arg(multMap.value(q->Band()).contains(q->CtyRcvd()->StdPx()) ? '-' : 'M')
					.arg(rq->ToString());
			}
			cbmFile.close();
		}
	}

	QFile cbmFile(_resDir.absoluteFilePath(Name().toLower() + ".cbm"));
	cbmFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream cbmStream(&cbmFile);
	cbmStream << "    **** CROSS BAND QSOS ****\n\n";

	QMapIterator< int, QString > cbmIt(_crossBandByCount);
	cbmIt.toBack();
	while (cbmIt.hasPrevious())
	{
		cbmIt.previous();
		cbmStream << cbmIt.value() << '\n';
	}
	cbmFile.close();
}

void Contest::WriteClubsFile()
{
	QFile clubFile(_resDir.absoluteFilePath(Name().toLower() + ".clb"));
	clubFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream clubStream(&clubFile);

	foreach(Log * log, _logsByCall)							// club scores
	{
		if (log->Club().size() < 2)
			continue;

		clubStream << QString("%1\t%2\t%3\t%4\n")
			.arg(log->Call())
			.arg(log->CtyPtr()->StdPx())
			.arg(log->Club())
			.arg(log->Score());

		_logsByClub[log->Club()].append(log);
	}
	clubFile.close();
}

void Contest::WriteCllFile()
{
	// Generate and write .cll
	// .cll has only calls that counted for credit
	// .clx has all raw calls
	QMap < QString, QList <QsoPtr> >		qsoListsByCallRcvd;
	QMap < QString, QMap <QString, int > >	countByCallRcvdBand;
	
	FillCtyLists();				

	foreach(Log * log, _logsByCall)
	{
		auto & qsoList = qsoListsByCallRcvd[log->Call()];
		auto & countByBand = countByCallRcvdBand[log->Call()];

		foreach(QsoPtr q, log->AllQso())
		{
			if (q->CountsForCredit() && !q->IsBadCallByList() && q->IsValidCty() && !q->IsExcluded())	// use only qsos for credit
			{
				qsoList.append(q);
				countByBand[q->Band()]++;
				countByBand["ALL"]++;
			}
		}
	}

	// all good qsos are now in qsoList
	QFile cllFile(_resDir.absoluteFilePath(Name().toLower() + ".cll"));
	cllFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream cllStream(&cllFile);

	cllStream << QString("Call\tClaimed 1\tClaimed 2\tClaimed 3\tClaimed 4\tClaimed 5\t"
		"Cont\tZone\tStdPx\tTotal\t160M\t80M\t40M\t20M\t15M\t10M") << endl;

	foreach(QString callRcvd, qsoListsByCallRcvd.keys())
	{
		QsoList qList = qsoListsByCallRcvd.value(callRcvd);

		if (!qList.size())
			continue;

		QMap < QString, int >  countByBand = countByCallRcvdBand.value(callRcvd);
		QsoPtr q = qList.first();
		cllStream << callRcvd << '\t';

		for (int i = 0; i < 5; i++)
		{
			if (i < qList.size())
				cllStream << qList[i]->CallRcvd() << '\t';
			else
				cllStream << '-' << '\t';
		}

		cllStream << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10")
			.arg(q->Cont())
			.arg(q->PxRcvdPtr()->CqwwZone())
			.arg(q->StdPx())
			.arg(countByBand.value("ALL"))
			.arg(countByBand.value("160M"))
			.arg(countByBand.value("80M"))
			.arg(countByBand.value("40M"))
			.arg(countByBand.value("20M"))
			.arg(countByBand.value("15M"))
			.arg(countByBand.value("10M")) << endl;
	}
	cllFile.close();

	// CLX FILE
	QMap < QString, QsoPtr >				qsoByRawCallRcvd;
	QMap < QString, QMap <QString, int > >	countByRawCallRcvdBand;
	QMap < QString, QMap <QChar, int > >	countByRawCallRcvdStatus;

	foreach(auto pair, _revLogMap)
	{
		QString		call = pair.first;
		RevLog	&	rl = pair.second;
		Qso		*	q = rl.AllQso()[0];

		QString	rawCall = q->RawCallRcvd();
		QString	band = q->Band();
		QChar status = q->StatusChar();

		qsoByRawCallRcvd[rawCall] = q;

		auto & countByBand = countByRawCallRcvdBand[rawCall];
		auto & countByStatus = countByRawCallRcvdStatus[rawCall];

		foreach(Qso * q, rl.AllQso())
		{
			q; // suppress the initialized but not referenced warning;
			countByBand[band]++;
			countByBand["ALL"]++;
			countByStatus[status]++;
		}
	}

	QFile clxFile(_resDir.absoluteFilePath(Name().toLower() + ".clx"));
	clxFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream clxStream(&clxFile);
	clxStream << QString("Raw Call\tClean Call\tCont\tZone\tStd Px\tALL\t160M\t80M\t40M\t20M\t15M\t10M\t-\t*\tB\tD\tF\tG\tL\tR\tV\tX\n");

	foreach(auto q, qsoByRawCallRcvd)
	{
		QMap <QString, int> countByBand = countByRawCallRcvdBand.value(q->RawCallRcvd());
		QMap <QChar, int>   countByStatus = countByRawCallRcvdStatus.value(q->RawCallRcvd());

		clxStream << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\t%15\t%16\t%17\t%18\t%19\t%20\t%21\t%22\n")
			.arg(q->RawCallRcvd())
			.arg(q->CallRcvd())
			.arg(q->Cont())
			.arg(q->PxRcvdPtr()->CqwwZone())
			.arg(q->StdPx())
			.arg(countByBand.value("ALL"))
			.arg(countByBand.value("160M"))
			.arg(countByBand.value("80M"))
			.arg(countByBand.value("40M"))
			.arg(countByBand.value("20M"))
			.arg(countByBand.value("15M"))
			.arg(countByBand.value("10M"))
			.arg(countByStatus.value('-'))
			.arg(countByStatus.value('*'))
			.arg(countByStatus.value('B'))
			.arg(countByStatus.value('D'))
			.arg(countByStatus.value('F'))
			.arg(countByStatus.value('G'))
			.arg(countByStatus.value('L'))
			.arg(countByStatus.value('R'))
			.arg(countByStatus.value('V'))
			.arg(countByStatus.value('X'));
	}
	clxFile.close();
}

void Contest::WriteCtyFiles()
{
	QFile ctyTabFile(_resDir.absoluteFilePath(Name().toLower() + ".cty.tab"));
	ctyTabFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream ctyTabStream(&ctyTabFile);

	QFile ctyTxtFile(_resDir.absoluteFilePath(Name().toLower() + ".cty.txt"));
	ctyTxtFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream ctyTxtStrm(&ctyTxtFile);
	QMap <int, QsoList> ctyMapBySize;

	FillCtyLists();
	foreach(QString stdPx, _qsoListsByStdPx.keys())
	{
		QsoList list = _qsoListsByStdPx[stdPx];
		if (!list.size())
			continue;

		ctyMapBySize[list.size()] = list;
	}

	foreach(QsoList list, ctyMapBySize)
	{
		ctyTabStream << QString("%1\t%2\t%3\n")
			.arg(list.first()->StdPx())
			.arg(QLocale().toString(list.size()))
			.arg(list.first()->ToTabString());

		ctyTxtStrm << QString("%1 %2 %3\n")
			.arg(list.first()->StdPx(), -5)
			.arg(QLocale().toString(list.size()), 8)
			.arg(list.first()->ToString());
	}

	ctyTabFile.close();
	ctyTxtFile.close();
}

void Contest::WriteErrFiles()
{
	// write warning and error messages
	// sort the messages in maps for easier reading
	// fill maps to sort output
	QMultiMap <QString, QString> contestMsgMap;
	QMultiMap <QString, QString> logMsgMap;
	QMultiMap <QString, QString> qsoMsgMap;

	foreach(QString msg, _errMsgList)				// contest errors
		contestMsgMap.insert(msg, msg);

	foreach(Log * log, _logsByCall)
	{
		foreach(QString msg, log->ErrorMsgList())	// log errors
			logMsgMap.insert(log->FileInfo().fileName(), QString("%1 %2").arg(log->FileInfo().fileName(), -16).arg(msg));

		foreach(Qso * q, log->AllQso())				// qso errors
			foreach(QString msg, q->WrnMsgList())
				qsoMsgMap.insert(log->FileInfo().fileName(), QString("%1 line: %2  %3   %4").arg(log->FileInfo().fileName(), -16).arg(q->LineNum(), 5).arg(msg, 16).arg(q->ToString()));
	}

	QFile contestErrFile;
	QFile logErrFile;
	QFile qsoErrFile;

	contestErrFile.setFileName(_resDir.absoluteFilePath(Name().toLower() + ".contest.err"));
	logErrFile.setFileName(_resDir.absoluteFilePath(Name().toLower() + ".log.err"));
	qsoErrFile.setFileName(_resDir.absoluteFilePath(Name().toLower() + ".qso.err"));

	contestErrFile.open(QIODevice::WriteOnly | QIODevice::Text);
	logErrFile.open(QIODevice::WriteOnly | QIODevice::Text);
	qsoErrFile.open(QIODevice::WriteOnly | QIODevice::Text);

	QTextStream contestErrStrm;
	QTextStream logErrStrm;
	QTextStream qsoErrStrm;

	contestErrStrm.setDevice(&contestErrFile);
	logErrStrm.setDevice(&logErrFile);
	qsoErrStrm.setDevice(&qsoErrFile);

	// output to .err files
	foreach(QString msg, contestMsgMap)
		contestErrStrm << msg << endl;

	foreach(QString msg, qsoMsgMap)
		qsoErrStrm << msg << endl;

	foreach(QString msg, logMsgMap)
		logErrStrm << msg << endl;

	WriteHqCountsFile();
}

void Contest::WriteFccBusts()
{
	QString fccFileName(_resDir.absoluteFilePath(Name().toLower() + ".fcc"));
	QFile fccFile(fccFileName);
	fccFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream fccStream(&fccFile);
	fccStream << "This file contains the calls and number of times busted in the FCC data\n\n";

	QMapIterator< int, QString > fccIt(_fccByCount);
	fccIt.toBack();
	while (fccIt.hasPrevious())
	{
		fccIt.previous();
		fccStream << fccIt.value() << '\n';
	}
	fccFile.close();

	foreach(Log * log, _logsByCall)
	{
		if (log->BustedFccCallList().size())		 // write out FCC busts
		{
			QString fccFileName = _fccDir.absoluteFilePath(log->FileInfo().baseName() + ".fcc");
			QFile fccFile(fccFileName);
			if (!fccFile.open(QIODevice::WriteOnly | QIODevice::Text))
				gStdErrStrm << "Could not open fcc file for " << fccFileName << endl;
			QTextStream fccTextStream(&fccFile);

			foreach(QsoPtr q, log->BustedFccCallList())
				fccTextStream << q->ToString() << "  not in FCC data\n";
			fccFile.close();
		}
	}
}

void Contest::WriteLvdResults()
{
	QFile lvdFile(_resDir.absoluteFilePath(Name().toLower() + ".lvd"));
	lvdFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream lvdStrm(&lvdFile);

	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr q, log->AllQso())
		{
			QsoPtr rq = q->Matched();
			if (rq && q->Status() == BUSTED_CALL)
				lvdStrm << QString("%1 %2\n%3\n\n").arg(q->ToString()).arg(q->LvDistance()).arg(rq->ToString());
		}
	}
	lvdFile.close();
}

void Contest::WriteMultFiles() { RunMultiThreaded(Log::WriteMultFiles); }

void Contest::WriteAllFiles()
{
	if (!gWriteAllFile)
		return;

	// write the full list of QSO's in the contest sorted by qrg, then time
	QFile allFile(_resDir.absoluteFilePath(Name().toLower() + ".all"));
	allFile.open(QIODevice::WriteOnly);
	QTextStream allStream(&allFile);

	std::multimap <int, std::multimap <QDateTime, Qso *> > entireContestByQrgAndTime;
	foreach(Log * log, _logsByCall)
	{
		foreach(QsoPtr q, log->AllQso())
		{
			if (!entireContestByQrgAndTime.count(q->Qrg()))
				entireContestByQrgAndTime.insert(std::pair<int, std::multimap <QDateTime, Qso *> >(q->Qrg(), std::multimap <QDateTime, Qso *>()));
			auto dt = q->DateTime();
			auto pair = std::pair<QDateTime, Qso *>(dt, q);
			entireContestByQrgAndTime.find(q->Qrg())->second.insert(pair);
		}
	}

	for (auto qrgIt = entireContestByQrgAndTime.cbegin(); qrgIt != entireContestByQrgAndTime.cend(); ++qrgIt)
		for (auto timeIt = qrgIt->second.cbegin(); timeIt != qrgIt->second.cend(); timeIt++)
			allStream << timeIt->second->ToString() << '\n';

	allFile.close();
}

void Contest::WriteSprFile()
{
	if (!gSpt && !gRbn)
		return;

	QFile sprFile(_resDir.absoluteFilePath(Name().toLower() + ".spr"));
	sprFile.open(QIODevice::WriteOnly);
	QTextStream sprStrm(&sprFile);

	WriteSprHdr(sprStrm);									// write .spr header
	foreach(Log * log, _logsByCall)						// write  line for each log
		log->WriteSprLine(sprStrm);
}

void Contest::WriteXchCausedFiles()
{
	foreach(Log * log, _logsByCall)
	{
		if (log->BustedXchCausedList().size())
		{
			// write out xch caused list

			QString fileName = _xchDir.absoluteFilePath(log->FileInfo().baseName() + ".xch");
			QFile file(fileName);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
				gStdErrStrm << "Could not open xch file for " << fileName << endl;
			QTextStream xchTextStream(&file);

			xchTextStream << log->Call() << " caused the following exchange errors\n" << endl;
			QMultiMap < QDateTime, QsoPtr> tmpMap;
			foreach(QsoPtr q, log->BustedXchCausedList())
				tmpMap.insertMulti(q->DateTime(), q);
			foreach(QsoPtr q, tmpMap)
				xchTextStream << QString("%1 correct %2").arg(q->ToString()).arg(q->CorrectXch()) << '\n';
			file.close();
		}
	}
}

void Contest::WriteObtFile()
{
	QString obtFileName = _resDir.absoluteFilePath(Name().toLower() + ".obt");	// write obt file
	QFile obtFile(obtFileName);
	if (obtFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream obtStrm(&obtFile);
		foreach(Log * log, _logsByCall)
			foreach(QsoPtr q, log->AllQso())
			if (q->IsBustedOffByTwo())
				obtStrm << QString("\n%1\n%2\n").arg(q->ToString()).arg(q->Matched()->ToString());
		obtFile.close();
	}
}

void Contest::WriteResultsTxtFile()
{
	QFile txtFile(_resDir.absoluteFilePath(Name().toLower() + ".txt"));
	txtFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream outStrm(&txtFile);
	WriteTxtHeader(outStrm);

	// fill the result maps
	std::map <QString, std::multimap <int, QString>	> resultByCatAndScoreMap;
	foreach(Log * log, _logsByCall)
		resultByCatAndScoreMap[log->Category()].insert(std::pair <int, QString> (log->Score(), log->ResultLine()));
	
	// write them biggest to smallest
	for (auto catIt = resultByCatAndScoreMap.begin(); catIt != resultByCatAndScoreMap.end(); catIt++)
		for (auto it = catIt->second.rbegin(); it != catIt->second.rend(); it++)
			outStrm << it->second << endl;
}

void Contest::WriteLgFiles() { RunMultiThreaded(Log::WriteLgFiles); }

void Contest::WriteResultFiles()
{
	if (true) //nils rcvd
	{
		StringByDoubleMap map;
		foreach(Log * log, _logsByCall)
		{
			if (log->NilList().size())
			{
				QString line = QString("%1% %2  (%3 of %4) not in log")
					.arg(log->PercentNil(), 5, 'f', 1)
					.arg(log->Call(), -12)
					.arg(log->NilList().size(), 4)
					.arg(log->CheckableCount(), 4);
				map.insert(log->PercentNil(), line);
			}
		}
		QFile file(_resDir.absoluteFilePath(Name().toLower() + ".nil_r"));
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream strm(&file);
		strm << "    **** NILS RCVD BY PERCENTAGE ****\n\n";
		QMapIterator< double, QString > it(map);
		it.toBack();
		while (it.hasPrevious())
		{
			it.previous();
			strm << it.value() << '\n';
		}
		file.close();
	}

	if (true)	// busts rcvd
	{
		StringByDoubleMap map;
		foreach(Log * log, _logsByCall)
		{
			if (log->BustedCallList().size())
			{
				QString line = QString("%1% %2  (%3 of %4) busted call")
					.arg(log->PercentBust(), 5, 'f', 1)
					.arg(log->Call(), -12)
					.arg(log->BustedCallList().size(), 4)
					.arg(log->CheckableCount(), 4);
				map.insert(log->PercentBust(), line);
			}
		}
		QFile file(_resDir.absoluteFilePath(Name().toLower() + ".bust_r"));
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream strm(&file);
		strm << "    **** BUSTS RCVD BY PERCENTAGE ****\n\n";
		QMapIterator< double, QString > it(map);
		it.toBack();
		while (it.hasPrevious())
		{
			it.previous();
			strm << it.value() << '\n';
		}
		file.close();
	}

	if(true) // exch busts rcvd
	{
		StringByDoubleMap map;
		foreach(Log * log, _logsByCall)
		{
			if (log->BustedXchList().size())
			{
				QString line = QString("%1% %2  (%3 of %4) bad exchange")
					.arg(log->PercentXch(), 5, 'f', 1)
					.arg(log->Call(), -12)
					.arg(log->BustedXchList().size(), 4)
					.arg(log->CheckableCount(), 4);
				map.insert(log->PercentXch(), line);
			}
		}
		QString fileName(_resDir.absoluteFilePath(Name().toLower() + ".xch_r"));
		QFile file(fileName);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream xchStream(&file);
		xchStream << "    **** BUSTED EXCHANGES RCVD BY PERCENTAGE ****\n\n";
		QMapIterator< double, QString > it = map;
		it.toBack();
		while (it.hasPrevious())
		{
			it.previous();
			xchStream << it.value() << '\n';
		}
		file.close();
	}

	if (true) // nils caused
	{
		StringByDoubleMap map;
		foreach(Log * log, _logsByCall)
		{
			if (log->NilCausedList().size())
			{
				QString line = QString("%1% %2  (%3 of %4) caused not in log")
					.arg(log->PercentNilCaused(), 5, 'f', 1)
					.arg(log->Call(), -12)
					.arg(log->NilCausedList().size(), 4)
					.arg(log->CheckableCount(), 4);
				_nilCausedByPercentage.insert(log->PercentNilCaused(), line);
			}
		}
		QString fileName(_resDir.absoluteFilePath(Name().toLower() + ".nil_c"));
		QFile file(fileName);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream nilStream(&file);
		nilStream << "    **** NILS CAUSED BY PERCENTAGE ****\n\n";
		QMapIterator< double, QString > it(_nilCausedByPercentage);
		it.toBack();
		while (it.hasPrevious())
		{
			it.previous();
			nilStream << it.value() << '\n';
		}
		file.close();
	}

	if (true) // busts caused
	{
		StringByDoubleMap map;
		foreach(Log * log, _logsByCall)
		{
			if (log->BustCausedList().size())
			{
				QString line = QString("%1% %2  (%3 of %4) caused busted call")
					.arg(log->PercentBustCaused(), 5, 'f', 1)
					.arg(log->Call(), -12)
					.arg(log->BustCausedList().size(), 4)
					.arg(log->CheckableCount(), 4);
				map.insert(log->PercentBustCaused(), line);
			}
		}
		QString fileName(_resDir.absoluteFilePath(Name().toLower() + ".bust_c"));
		QFile file(fileName);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream xchStream(&file);
		xchStream << "    **** BUST CAUSED BY PERCENTAGE ****\n\n";
		QMapIterator< double, QString > it = map;
		it.toBack();
		while (it.hasPrevious())
		{
			it.previous();
			xchStream << it.value() << '\n';
		}
		file.close();
	}

	if (true) // exch busts caused
	{
		StringByDoubleMap map;
		foreach(Log * log, _logsByCall)
		{
			if (log->BustedXchCausedList().size())
			{
				QString line = QString("%1% %2  (%3 of %4) caused bad exchange")
					.arg(log->PercentXchCaused(), 5, 'f', 1)
					.arg(log->Call(), -12)
					.arg(log->BustedXchCausedList().size(), 4)
					.arg(log->CheckableCount(), 4);
				map.insert(log->PercentXchCaused(), line);
			}
		}
		QString fileName(_resDir.absoluteFilePath(Name().toLower() + ".xch_c"));
		QFile file(fileName);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream xchStream(&file);
		xchStream << "    **** BUSTED EXCHANGED CAUSED BY PERCENTAGE ****\n\n";
		QMapIterator< double, QString > it = map;
		it.toBack();
		while (it.hasPrevious())
		{
			it.previous();
			xchStream << it.value() << '\n';
		}
		file.close();
	}
}

void Contest::WriteRptFiles() { RunMultiThreaded(Log::WriteRptFiles); }

void Contest::WriteRlsFile()
{
	if (!gRbn && !gSpt)
		return;
	
	QMap <QString, int> claimedQsoCountMap;
	QMap <QString, int> bustedQsoCountMap;
	foreach(Log * log, _logsByCall)
	{
		claimedQsoCountMap[log->Category()] += log->RawQso();
		bustedQsoCountMap[log->Category()] += log->BustedCallCount();
	}

	// create rls file
	QFile rlsFile(_resDir.absoluteFilePath(Name().toLower() + ".rls"));
	rlsFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream rlsStrm(&rlsFile);

	rlsStrm << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\n")
		.arg("Date")
		.arg("Time")
		.arg("QRG")
		.arg("CallRcvd")
		.arg("CallSent")
		.arg("Status")
		.arg("Cat")
		.arg("Asst")
		.arg("CorrectCall")
		.arg("SpotDate")
		.arg("SpotTime")
		.arg("SpotQRG");

	for (RevLogMap::iterator revLogIt = _revLogMap.begin(); revLogIt != _revLogMap.end(); ++revLogIt)
	{
		QString call = (*revLogIt).first;

		if (gRbn || gSpt)							// for RBN checking, do not test if we have log or rl is large
		{
			if (gContest->GetLog(call))				// if we have a log, probably not a busted spot
				continue;

			if (!_revLogMap[call].AllQso().size())
				continue;
		}

		QStringList outStrList;
		foreach(QsoPtr q, _revLogMap[call].AllQso())
			{
				if (!q->SpotHit() || !(q->Status() == BUSTED_CALL || q->Status() == BUSTED_FCC_CALL || q->Status() == BUSTED_LIST_CALL))
					continue;

				bool assisted = !q->GetLogPtr()->Category().endsWith('U');

				QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\n")
					.arg(q->DateStr())
					.arg(q->TimeStr())
					.arg(float(q->Qrg()), 0, 'f', 1)
					.arg(q->CallRcvd())
					.arg(q->CallSent())
					.arg(q->StatusChar())
					.arg(q->GetLogPtr()->Category())
					.arg(assisted ? "A" : "NA")
					.arg(q->CorrectCall())
					.arg(q->SpotHit()->DateStr())
					.arg(q->SpotHit()->TimeStr())
					.arg(q->SpotHit()->Qrg(), 0, 'f', 1);
				outStrList.append(outStr);
			}
		foreach(QString outStr, outStrList)
			rlsStrm << outStr;
	}
	rlsFile.close();
}

void Contest::WriteOcpFile()
{
	QFile ocxFile(_resDir.absoluteFilePath(Name().toLower() + ".ocp"));
	ocxFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream ocxStream(&ocxFile);
	ocxStream << "QSOs claimed outside the contest period\n" << endl;

	foreach(Log * log, _logsByCall)
		foreach(QsoPtr q, log->AllQso())
			if (q->DateTime() < _startTime || q->DateTime() >= _endTime)
				ocxStream << q->ToString() << endl;

	ocxFile.close();
}

void Contest::WriteUniqueMults()
{
	QFile file(_resDir.absoluteFilePath(Name().toLower() + ".uniMults.txt"));
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream strm(&file);

	foreach(Log * log, _logsByCall)
		log->WriteMultUniqueToMe(strm);
}

void Contest::WriteUniqueNils()
{
return;
	// attempt to find nils caused by badly mangled calls. IE near uniques and nils at the same time and freq
	QFile unilFile(_resDir.absoluteFilePath(Name().toLower() + ".unl"));
	unilFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream unilStrm(&unilFile);
	QMap <int, QsoList> unlMap;
	const int unlWinSize = 5 * 60;

	foreach(Log * log, _logsByCall)
	{
		// try to find near uniques that correlate with nils

		if (!gContest->RevLogMap().count(log->Call()))		// nobody claimed me! 
			continue;

		std::vector<Qso *> & qsos = gContest->RevLogMap().at(log->Call()).AllQso();	// my reverse log

		foreach(QsoPtr q, log->AllQso())
		{
			if (q->Status() != NO_LOG)
				continue;

			// only process received calls that are near uniques		

			size_t claimedCount = gContest->RevLogMap().at(q->CallRcvd()).AllQso().size();
			if (claimedCount > 3)	// not a near unique
				continue;

			// search from qtr -unlWinSize to qtr +unlWinSize

			for (std::vector<Qso *>::iterator it = std::lower_bound(qsos.begin(), qsos.end(), q->DateTime().addSecs(-unlWinSize), gQsoTimeLessThanTime); it != qsos.end() && q->DateTime().secsTo((*it)->DateTime()) <= unlWinSize; it++)
			{
				QsoPtr rq = *it;

				if (rq->Status() != NOT_IN_LOG || q->Band() != rq->Band() || rq->Matched()) // these are not candidates
					continue;

				// report U/N pairs that share all but 0 or 1 characters

				QSet <QChar> uSet, nSet, unSet, nuSet;
				foreach(QChar c, q->CallRcvd())
					uSet.insert(c);

				foreach(QChar c, rq->CallSent())
					nSet.insert(c);

				unSet = uSet - nSet;	// remove the intersection
				nuSet = nSet - uSet;	// remove the intersection

				int remainder = unSet.size() + nuSet.size();
				if (remainder <= 1)
				{
					q->Status(BUSTED_CALL);
					q->CorrectCall(rq->CallSent());
					q->Matched(rq);
					rq->GetLogPtr()->AppendBustCausedList(rq);
					rq->Status(GOOD_QSO);
					rq->Matched(q);
					unlMap[remainder].append(rq);
					continue;
				}

				uint lvd;

				if (Mode() == "CW")
				{
					QString rcvdCallMorse = gStrToMorse(q->CallRcvd());
					QString sentCallMorse = gStrToMorse(rq->CallSent());

					lvd = gLvd(sentCallMorse, rcvdCallMorse);

					if (lvd <= 3)
					{
						q->Status(BUSTED_CALL);
						q->CorrectCall(rq->CallSent());
						q->Matched(rq);
						rq->GetLogPtr()->AppendBustCausedList(rq);
						rq->Status(GOOD_QSO);
						rq->Matched(q);
						unlMap[lvd].append(rq);
						continue;
					}
				}

				lvd = gLvd(rq->CallSent(), q->CallRcvd());
				if (lvd <= 2)
				{
					q->Status(BUSTED_CALL);
					q->CorrectCall(rq->CallSent());
					q->Matched(rq);
					rq->GetLogPtr()->AppendBustCausedList(rq);
					rq->Status(GOOD_QSO);
					rq->Matched(q);
					unlMap[lvd].append(rq);
				}
			}
		}
	}

	if (unlMap.size())
	{
		foreach(int dist, unlMap.keys())
		{
			QsoList list = unlMap.value(dist);
			foreach(QsoPtr rq, list)
			{
				unilStrm << QString("%1 claimed: %2 times\n").arg(rq->Matched()->ToString()).arg(gContest->RevLogMap().at(rq->Matched()->CallRcvd()).AllQso().size());
				QString msg = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 nil lvd: %11\n\n")
					.arg(rq->Qrg(), 5)
					.arg(rq->Mode(), 2)
					.arg(rq->DateStr())
					.arg(rq->TimeStr())
					.arg(rq->CallRcvd(), -10)
					.arg(rq->XchRcvd(), 5)
					.arg(rq->XchRcvd2(), 5)
					.arg(rq->CallSent(), -10)
					.arg(rq->XchSent(), 5)
					.arg(rq->XchSent2(), 5)
					.arg(dist);
				unilStrm << msg;
			}
		}
		unilStrm << '\n';
	}
}

void Contest::WriteUniques()
{
	MarkUniques();

	QFile uniFile(_resDir.absoluteFilePath(Name().toLower() + ".uni"));
	uniFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream uniStream(&uniFile);

	// report uniques and busted uniques
	foreach(Log * log, _logsByCall)
	{
		_uniquesCount += log->UniList().size();

		foreach(QsoPtr q, log->UniList())
		{
			if (!q->CountsForCredit())
				_uniquesNotCounted++;

			QStringList foundTestCalls;
			if (q->Status() == NO_LOG)
			{
				foreach(QString testCall, gGenOffByOneCallSet(q->CallRcvd()))
				{
					if (_revLogMap.find(testCall) == _revLogMap.end())
						continue;

					Log * hisLog = GetLog(testCall);

					if (hisLog)
						continue;

					foundTestCalls << testCall;
				}

				if (foundTestCalls.size())
				{
					uniStream << q->ToString() << endl;
					foreach(QString call, foundTestCalls)
					{
						RevLog revLog = _revLogMap.at(call);
						uniStream << QString("   %1 (%2)\n").arg(call, -8).arg(revLog.AllQso().size());
					}
					uniStream << endl;
				}
			}

			if (log->UniList().size())
			{
				// write out unique list

				QString uniFileName = _uniDir.absoluteFilePath(log->FileInfo().baseName() + ".uni");
				QFile uniFile(uniFileName);
				uniFile.open(QIODevice::WriteOnly | QIODevice::Text);
				QTextStream uniTextStream(&uniFile);

				uniTextStream << "List of calls that are unique to " << log->Call() << '\n' << endl;
				foreach(ConstQsoPtr q, log->UniList())
					uniTextStream << q->StatusChar() << " " << q->ToString() << '\n';

				uniFile.close();
			}
		}
	}
}

void Contest::WriteDupeFiles() { RunMultiThreaded(Log::WriteDupeFiles); }

void Contest::WriteRepairedCalls()
{
	QFile repairedFile(_resDir.absoluteFilePath(Name().toLower() + "-repaired.txt"));
	repairedFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream repairedStream(&repairedFile);
	foreach(Log * log, _logsByCall)
		foreach(QsoPtr q, log->AllQso())
		if (q->IsRepaired())
			repairedStream << QString("%1 = %2").arg(q->RawCallRcvd(), -12).arg(q->CallRcvd()) << endl;
	repairedFile.close();
}

void Contest::RunMultiThreaded(void(&func) (Log *))
{
	if (gMP)
		QtConcurrent::blockingMap(_logsByCall, func);
	else
		foreach(Log * log, _logsByCall)
			func(log);
}

int					Contest::LogCount()					const { return _logCount;		}
int					Contest::MultStnFldNum()			const { return _multStnFldNum;	}
int					Contest::NumberOfFields()			const { return _numberOfFields;	}
int					Contest::MatchWindowSize()			const { return _matchWindowSize;}
int					Contest::RcvdCallFldNum()			const { return _rcvdCallFldNum; }
int					Contest::RcvdRstFldNum()			const { return _rcvdRstFldNum;	}
int					Contest::RcvdXchFldNum()			const { return _rcvdXchFldNum;	}
int					Contest::RcvdXchFldNum2()			const { return _rcvdXchFldNum2; }
int					Contest::SentXchFldNum()			const { return _sentXchFldNum;	}
int					Contest::SentXchFldNum2()			const { return _sentXchFldNum2; }
int					Contest::SentRstFldNum()			const { return _sentRstFldNum;	}
QString		const	Contest::AdminText()				const { return _adminText;		}
QString		const	Contest::Name()						const { return _name;			}
QString		const	Contest::Mode()						const { return _mode;			}
QString		const	Contest::Title()					const { return _title;			}
QStringList	const	Contest::BandNameList()				const { return _bandNameList;	}
QDateTime	const	Contest::StartTime()				const { return _startTime;		}
QDateTime	const	Contest::EndTime()					const { return _endTime;		}
QDateTime	const	Contest::CertCutoffTime()			const { return _certCutoffTime; }
QMap <QString, Log * >			Contest::LogsByCall()	const { return _logsByCall;		}
std::map < QString, RevLog > &	Contest::RevLogMap()		  { return _revLogMap;		}

QDir		const	Contest::DupeDir()					const { return _dupeDir; }
QDir		const	Contest::LgDir()					const { return _lgDir; }
QDir		const	Contest::MulDir()					const { return _mulDir; }
QDir		const	Contest::RlDir()					const { return _rlDir;	}
QDir		const	Contest::RptDir()					const { return _rptDir; }
QDir		const	Contest::TmDir()					const { return _tmDir;	}
QDir		const	Contest::SptDir()					const { return _sptDir;	}

QMap <QString, QString>	const   Contest::BadCallMap()				const { return _badCallsMap;			}

CallSet					const	Contest::BadInfoSentSet()			const { return _badInfoSentCallSet;		}
CallSet					const	Contest::BadInfoRcvdSet()			const { return _badInfoRcvdCallSet;		}
CallSet					const	Contest::ExtensionsSet()			const { return _extensionsSet;			}
CallSet					const	Contest::DoNotCheckSet()			const { return _doNotCheckSet;			}
CallSet					const	Contest::GoodCallsSet()				const { return _goodCallsSet;			}
CallSet					const	Contest::IncompleteLogSet()			const { return _incompleteLogSet;		}
CallSet					const	Contest::PirateCallsSet()			const { return _pirateCalls;			}

QMap <QString, QString> const	Contest::AliasCallMap()				const { return _aliasCallMap;			}
QMap <QString, QString> const	Contest::AliasDistrictMap()			const { return _aliasDistrictMap;		}
QMap <QString, QString>	const   Contest::RevHqCategoryMap()			const { return _revHqCategoryMap;		}
QMap <QString, QList <Log *> > const & Contest::LogsByClub()		const { return _logsByClub;				}
QMap <QString, QString>	const   Contest::FillRunMultMap()			const { return _fillRunMultMap;			}
QSet <QString>			const	Contest::RedCardSet()				const { return _redCardSet;				}
QSet <QString>			const	Contest::YellowCardSet()			const { return _yellowCardSet;			}
QSet <QString>			const	Contest::DisqualifiedSet()			const { return _disqualifiedSet;		}

QMap <QString, QMap <QString, std::vector <Spot *> > >	Contest::SpotsByBandCallTimeMap()	{ return  _spotsByBandCallTimeMap;}
QMap <QString, QMap < QDateTime, QMap <int, Qso *> > >	Contest::AllByCallTimeQrgMap()		{ return _allByCallTimeQrgMap;}
QMap < QString, RevLog>	const	Contest::LargeRlMap()			{ return _largeRlMap; }
QMap < QString, RevLog>	const	Contest::SmallRlMap()			{ return _smallRlMap; }
QMap <QString, QString> const	Contest::AliasStateMap() const	{ return _aliasStateMap; }

void Contest::FillRunMultMap(QString call, QString data)		{ _fillRunMultMap.insert(call, data); }
void Contest::MatchWindowSize(int winSize)						{ _matchWindowSize = winSize; }
QMap <QString, QSet <QString> > const Contest::LogSetByCallRcvd() const{ return _logByCallRcvdMap; }
void Contest::WriteTsvFile() {}
QString Contest::ContestType() { return _contestType; }
