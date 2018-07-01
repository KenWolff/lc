#pragma once

extern	QString	BadCallCharPat;		// letters numbers and slash allowed in calls
extern	QString	EndsDoubleZeroPat;
extern	QString	EndsLettersNumbersLettersNumbersPat;
extern	QString	EndsLettersNumbersLettersZeroPat;
extern	QString	EndsPortableSingleDigitPat;
extern	QString	EndsPortableMultipleDigitPat;
extern	QString	EndsLetterZeroPat;
extern	QString	LegalCallPat;
extern	QString	StartsDigitZeroPat;
extern	QString	StartsIllegalLetterZeroPat;
extern	QString	StartsOhLetterOhPat;
extern	QString	StartsZeroLetterDigitPat;
extern	QString	MultipleSlashesPat;
extern	QString	StartAndEndNonsensePat;
extern	QString	MiddleNonsensePat;

extern  QSet <QString> gGenOffByOneCallSet(QString call);

extern	QFile						gDbgFile;
extern	bool						gMP;
extern	bool						gWriteAllFile;
extern	bool						gListMultiplierQsos;
extern	bool						gRunAutoFill;
extern	bool						gSpt;
extern	bool						gRbn;
extern	bool						gFixTypos;
extern	bool						gUseExpected;
extern	bool						gUseOffByTwo;
extern	int							gDeriveZone(QString, Px const *, Log * = 0);
extern	QMutex						gDbgMutex;
extern	QMutex						gStdErrMutex;
extern	QMutex						gStdOutMutex;
extern	QMutex						gWrnMutex;
extern	QTextStream					gDbgFileStrm;
extern	QTextStream					gStdErrStrm;		// take over stderr
extern	QTextStream					gStdOutStrm;		// take over stdout
extern	QDir						gLogsDir;
extern	QMap <QString, QString>		gSrcMap;
extern	QString	const				gLcDateTimeFormat;
extern	QString						gWrtcYear;
extern	Contest *					gContest;
extern	QString						gFromCutNumber(QString);

//extern	QMutex					gBustPairsMutex;
//extern	QMap < QPair <QChar, QChar>, QsoList> gBustPairs;
extern	unsigned int				gLvd(QString s, QString t, bool fullObt = true);
extern	QString						gStrToMorse(QString str);
extern	QPair< QString, bool>		gFormPx(QString, QString);
