#include "StdAfx.h"
//QMap < QPair <QChar, QChar>, QsoList>	gBustPairs;
//	{ QMutexLocker l(&gStdOutMutex); gStdOutStrm << QString("%1 %2").arg("").arg("") << endl; }

Contest * gContest(0);
bool gListMultiplierQsos(false);
bool gRunAutoFill(false);
bool gSpt(false);
bool gRbn(false);
bool gMP(true);
bool gUseOffByTwo(true);
bool gWriteAllFile(false);
bool gFixTypos(true);
bool gUseExpected(false);

// example QMutexLocker mutexLocker(&gStdOutMutex);
// example QMutexLocker mutexLocker(&gDbgMutex);
// gDbgMutex.lock();
// gDbgFileStrm << QString("%1 %2").arg(RawCall(), -12).arg(_district) << "  create log" << endl;
// gDbgMutex.unlock();

QString		gWrtcYear;
QFile		gDbgFile;
QMutex		gDbgMutex;
QTextStream	gDbgFileStrm;

QMutex		gStdErrMutex;
QTextStream	gStdOutStrm(stdout);	// take over stdout

QMutex		gStdOutMutex;
QTextStream	gStdErrStrm(stderr);	// take over stderr

QDir		gLogsDir;

QMap <QString, QString> gSrcMap;
QString		const gLcDateTimeFormat("yyyy-MM-dd hhmm");

QMutex							gObtMutex;
QMutex							gBustPairsMutex;
QPair< QString, bool>			gFormPx(QString, QString);
QSet <QString>					GenOffByOneCallSet(QString call);
QMap <QString, QStringList *>	gOffByOneCallListByCallMap;

QString BadCallCharPat("[^A-Z0-9/]");		// letters numbers and slash allowed in calls
QString EndsDoubleZeroPat("(00)$");
QString EndsLettersNumbersLettersNumbersPat("[A-Z]+\\d+[A-Z]+(\\d+)$");
QString EndsLettersNumbersLettersZeroPat("[A-Z]+\\d+[A-Z]+(0)$");
QString EndsPortableSingleDigitPat("(/(\\d))$");
QString EndsPortableMultipleDigitPat("/\\d\\d+$");
QString EndsLetterZeroPat("[A-Z]+(0)$");
QString LegalCallPat("[A-Z]+\\d+[A-Z]");
QString StartsDigitZeroPat("^\\d(0)");
QString StartsIllegalLetterZeroPat("^[ACDEHJLOPTVXYZ](0)");
QString StartsOhLetterOhPat("^O[A-Z](O)");
QString StartsZeroLetterDigitPat("^(0)[A-Z]\\d");
QString MultipleSlashesPat("(//+)");
QString StartAndEndNonsensePat("^/|/$|/O$|/D$|/F$|/P$|/M$|/B$|/D$|/A$|/H$|/J$|/E$|/AG$|/AD$|/AE$|/KT$|/AA$|/M\\d?$|/MM$|/DUPE$|/AM$|/Q.*$");
QString MiddleNonsensePat("/R/|/P\\d*/|/M\\d*/|/MM/");

int gDeriveZone(QString call, Px const * px, Log * log)
{
	if (!px || call.isEmpty())
		return 0;

	int zone = px->CqwwZone();				// zone from cty.dat

	if (log)							// this path when loading logs
	{
		if (log->CtyPtr()->IsWVe())		// US get zone from state
		{
			if (log->IsW())
				zone = gZoneByStateMap.value(log->Location());
			else
				zone = px->CqwwZone();	// VE get zone from cqww.dat

			return zone;
		}
	}
	else								// this path when loading qsos
	{
		if (gContest->LogsByCall().contains(call))
		{
			zone = static_cast<CqwwLog *> (gContest->LogsByCall().value(call))->Zone();
			return zone;
		}
	}

	if (px->IsSpecialCqwwZone())
		return zone;

	if (px->GetCty()->IsW() && gStateByFccCallMap.contains(call))
	{
		zone = gZoneByStateMap.value(gStateByFccCallMap.value(call));
		return zone;
	}
	return zone;
}

BandEdges const & gFreqToBand(double freq)
{
	if (freq >= B10.LowerEdge() && freq <= B10.UpperEdge())  return  B10;
	if (freq >= B15.LowerEdge() && freq <= B15.UpperEdge())  return  B15;
	if (freq >= B20.LowerEdge() && freq <= B20.UpperEdge())  return  B20;
	if (freq >= B40.LowerEdge() && freq <= B40.UpperEdge())  return  B40;
	if (freq >= B80.LowerEdge() && freq <= B80.UpperEdge())  return  B80;
	if (freq >= B160.LowerEdge() && freq <= B160.UpperEdge()) return  B160;
	return BNone;
}

uint gLvd(QString s, QString t, bool fullObt)
{
	const int MAX_CALL_LENGTH(63);
	uint ar[MAX_CALL_LENGTH + 1][MAX_CALL_LENGTH + 1];
	int sLen = s.length();
	int tLen = t.length();

	if (s.length() > MAX_CALL_LENGTH || t.length() > MAX_CALL_LENGTH)	// bounds checking
		return UINT_MAX;

	for (int i = 0; i <= sLen; i++)
		ar[i][0] = i;

	for (int j = 0; j <= tLen; j++)
		ar[0][j] = j;

	for (int i = 1; i <= sLen; i++)
		for (int j = 1; j <= tLen; j++)
		{
			uint v[4];
			uint cost = (s[i - 1] == t[j - 1]) ? 0 : 1;
			v[0] = ar[i - 1][j - 1] + cost;	// substitution

			if (fullObt)
			{
				v[1] = ar[i - 1][j + 0] + 1;	// insertion
				v[2] = ar[i + 0][j - 1] + 1;	// deletion
				if (i > 1 && j > 1 && s[i - 1] == t[j - 2] && s[i - 2] == t[j - 1])		// swap
					v[3] = ar[i - 2][j - 2] + cost;
				else
					v[3] = UINT_MAX;
			}
			else
				v[1] = v[2] = v[3] = UINT_MAX;

			ar[i][j] = v[0];				// find smallest
			for (int k = 1; k < 4; k++)
				if (v[k] < ar[i][j])
					ar[i][j] = v[k];
		}

	return ar[sLen][tLen];
}

QString gStrToMorse(QString str)
{
	static std::map <QChar, QString> charToMorseMap =
	{
		{'A',"01"},		{'B',"1000" },	{'C',"1010"},	{'D',"100"},	{'E',"0"},		{'F',"0010"},	{'G',"110"},	{'H',"0000"},	{'I',"00"},	{'J',"0111"},	{'K',"101"},
		{'L',"0100"},	{'M',"11" },	{'N',"10"},		{'O',"111"},	{'P',"0110"},	{'Q',"1101"},	{'R',"010"},	{'S',"000"},	{'T', "1"},	{'U',"001"},	{'V',"0001"},
		{'W',"011"},	{'X',"1001"},	{'Y',"1011"},	{'Z',"1100"},	{'/',"10010"},	{'1',"01111"},	{'2',"00111"},	{'3',"00011"},	{'4',"00001" },	{'5',"00000"},{'6',"10000"},
		{'7',"11000"},	{'8',"11100"},	{'9',"11110"},	{'0',"11111"}
	};

	// dot = '0' dash = '1' character space ' '

	QString morseStr;
	foreach(QChar c, str)
		morseStr.append(charToMorseMap[c]).append(' ');

	return morseStr.trimmed();
}

QString gFromCutNumber(QString s)
{
	s.remove(QRegularExpression("[^\\dAENTO]"));
	s.replace('A', '1');
	s.replace('E', '5');
	s.replace('N', '9');
	s.replace('T', '0');
	s.replace('O', '0');

	s.setNum(s.toInt());				// idiom to strip leading zeros
	return s;
}

QSet <QString> gGenOffByOneCallSet(QString call)		// this makes a list of all possible off-by-ones
{
	static QByteArray ASCII_array("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/-");
	QString testCall;
	QSet <QString> testSet;								// use a set to avoid duplicates
	std::map < QString, RevLog > &	revLogMap = gContest->RevLogMap();	// don't use any test call not in reverse logs

	for (int i = 0; i < call.length(); i++)
	{
		foreach(QChar c, ASCII_array)
		{
			if (call[i] != c)
			{
				testCall = call;
				testCall[i] = c;						// substitute char
				if (revLogMap.count(testCall))
					testSet.insert(testCall);
			}
			testCall = call;
			testCall.insert(i, c);						// insert char
			if (revLogMap.count(testCall))
				testSet.insert(testCall);
		}
		testCall = call;
		testCall.remove(i, 1);							// remove char
		if (revLogMap.count(testCall))
			testSet.insert(testCall);
	}

	foreach(QChar c, ASCII_array)
	{
		testCall = call;
		testCall.append(c);								// append a character
		if (revLogMap.count(testCall))
			testSet.insert(testCall);
	}

	for (int i = 0; i < call.length() - 1; i++)
	{
		if (call[i] != call[i + 1])
		{
			testCall = call;
			testCall[i] = call[i + 1];					// twiddle
			testCall[i + 1] = call[i];
			if (revLogMap.count(testCall))
				testSet.insert(testCall);
		}
	}

	QRegularExpression endsThreeLetters("([A-Z][A-Z][A-Z])$");
	QRegularExpressionMatch match = endsThreeLetters.match(call);

	if (match.hasMatch())					// check double letter in three letter suffix
	{
		QList <QChar> letterList;
		testCall = call;

		int idx = match.capturedStart(1);
		if (idx != -1)
		{
			if (call[idx + 0] == call[idx + 1] &&	// first and second match
				call[idx + 0] != call[idx + 2])		// first and third don't match
			{
				testCall[idx + 0] = call[idx + 2];	// swap K1AAB for K1BBA
				testCall[idx + 1] = call[idx + 2];
				testCall[idx + 2] = call[idx + 0];
				if (revLogMap.count(testCall))
					testSet.insert(testCall);
			}
			else
			{
				if (call[idx + 0] == call[idx + 2] &&	// first and third match
					call[idx + 0] != call[idx + 1])		// first and second don't match
				{
					testCall[idx + 0] = call[idx + 1];	// swap K1ABA for K1BAB
					testCall[idx + 1] = call[idx + 0];
					testCall[idx + 2] = call[idx + 1];
					if (revLogMap.count(testCall))
						testSet.insert(testCall);
				}
				else
				{
					if (call[idx + 1] == call[idx + 2] &&	// second and third match
						call[idx + 0] != call[idx + 2])		// first and third don't match
					{
						testCall[idx + 0] = call[idx + 2];	// swap K1ABB for K1BAA
						testCall[idx + 1] = call[idx + 0];
						testCall[idx + 2] = call[idx + 0];
						if (revLogMap.count(testCall))
							testSet.insert(testCall);
					}
				}
			}
		}
	}
	return testSet;
}
