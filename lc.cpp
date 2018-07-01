#include "StdAfx.h"

int main(int argc, char * argv[])
{
	QCoreApplication app(argc, argv);
	QFileInfo fileInfo(qApp->applicationFilePath());

	QTime timer;
	timer.start();

	for(int i = 1; i < argc; i++)
	{
		if(argv[i][0] != '-')
			gLogsDir.setPath(QString(argv[1]));

		QString argStr = QString(argv[i]).toLower();

		if (argStr == "-run_auto_fill")
			gRunAutoFill = true;

		if(argStr == "-rbn")
			gRbn = true;

		if (argStr == "-no_rbn")
			gRbn = false;

		if (argStr == "-obt")
			gUseOffByTwo = true;

		if (argStr == "-no_obt")
			gUseOffByTwo = false;

		if(argStr == "-spt")
			gSpt = true; 

		if(argStr == "-no_spt")
			gSpt = false;

		if(argStr == "-mp")
			gMP = true;

		if (argStr == "-no_mp")
			gMP = false;

		if (argStr == "-no_fix_typos")
			gFixTypos = false;

		if (argStr == "-write_all_file" || argStr == "-all")
			gWriteAllFile = true;

		if (argStr == "-uez")
			gUseExpected = true;
	}

	if (gRbn || gSpt)
		gWriteAllFile = true;

	QString lcFileName = "lc.dat";

	if(gLogsDir.dirName().isEmpty())
		gLogsDir = QDir::current();

	if(!gLogsDir.exists())						// see if LC directory exists
	{
		gStdErrStrm <<  QString("Directory %1 does not exist").arg(gLogsDir.path()) << endl;
		exit(-1);
	}

	QDir::setCurrent(gLogsDir.absolutePath());

	if(!gLogsDir.exists(lcFileName))
	{
		gStdErrStrm << lcFileName << " does not exist in " << gLogsDir.absolutePath() << endl;
		exit(-1);
	}

	QFile lcFile(lcFileName);
	lcFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream lcStream(& lcFile);
	QString line;
	while(!lcStream.atEnd())
	{
		QString line = lcStream.readLine();
		QStringList parts = line.split('=');
		if(parts.size() > 1)
			gSrcMap[parts[0].simplified()] = parts[1].simplified().toUpper();
	}
	lcFile.close();

	if(!gSrcMap.contains("ContestType"))
	{
		gStdErrStrm << "ContestType not found -- cannot proceed" << endl;
		exit(-1);
	}

	if(!gSrcMap.contains("NameOfContest"))
	{
		gStdErrStrm << "NameOfContest not found -- cannot proceed" << endl;
		exit(-1);
	}

	if(!gSrcMap.contains("Mode"))
	{
		gStdErrStrm << "Mode not found -- cannot proceed" << endl;
		exit(-1);
	}

	if(!gSrcMap.contains("StartTime"))
	{
		gStdErrStrm << "StartTime not found -- cannot proceed" << endl;
		exit(-1);
	}

	if (!gSrcMap.contains("EndTime"))
	{
		gStdErrStrm << "EndTime not found -- cannot proceed" << endl;
		exit(-1);
	}

	if (!gSrcMap.contains("CertCutoff"))
		gStdErrStrm << "CertCutOff not found -- no logs will be marked late" << endl;

	QDate compileDate = QLocale("en_US").toDate(QString(__DATE__).simplified(), "MMM d yyyy");
	QTime compileTime = QLocale("en_US").toTime(QString(__TIME__).simplified(), "hh:mm:ss");
	QDateTime compileDateTime(compileDate, compileTime);

	gContest = Contest::CreateContest();
	gStdErrStrm << endl;
	gStdOutStrm << QString("Title:       %1")	.arg(gSrcMap.value("Title"))				<< endl;
	gStdOutStrm << QString("Contest:     %1")	.arg(gContest->Name().toUpper())			<< endl;
	//gStdOutStrm << QString("Created:     %1")	.arg(fileInfo.created().toString())			<< endl;
	//gStdOutStrm << QString("Modified:    %1")	.arg(fileInfo.lastModified().toString())	<< endl;
	gStdOutStrm << QString("Compiled:    %1")	.arg(compileDateTime.toString())			<< endl;
	gStdOutStrm << QString("QT Version:  %1")	.arg(qVersion())							<< endl;
	gStdOutStrm << QString("CPU Count:   %1")	.arg(QThread::idealThreadCount())			<< endl;
	gStdOutStrm << QString("LC Start:    %1")	.arg(QDateTime::currentDateTimeUtc().toString()) << endl;

	QString contestType = gContest->ContestType();
	bool isCQContest = contestType == "CQWW" || contestType == "WPX" || contestType == "WPX-RTTY" || contestType == "CQWW-RTTY";

	QString ctyFileName = isCQContest ? "cqww.dat" : "cty.dat";
	if (!gLogsDir.exists(ctyFileName))
	{
		if (isCQContest) // didn't find cqww.dat, try cty.dat
		{
			ctyFileName = "cty.dat";
			if (!gLogsDir.exists(ctyFileName))
			{
				gStdErrStrm << "Can't find " << ctyFileName << endl;
				exit(-1);
			}
		}
		else
		{
			gStdErrStrm << "Can't find " << ctyFileName << endl;
			exit(-1);
		}
	}

	ctyFileName = gLogsDir.absoluteFilePath(ctyFileName);
	CtyFileParser().Parse(ctyFileName, gPxMap, isCQContest);
	
	gContest->Init();									// always init *after* loading cty file
	QTime acquireTimer;
	acquireTimer.start();
	gContest->AcquireLogs();
	gContest->CheckLogs();
	gContest->PrintStats();
	gStdErrStrm << QString("%1 Total Time").arg(QLocale().toString(timer.elapsed() / 1000), 4) << endl;
	app.exit();
	return 0;
}
