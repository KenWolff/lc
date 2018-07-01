#include "StdAfx.h"

ArrlContest::ArrlContest() : Contest()
{
	_hqCategoryToBandMap.insert("CK", "");
	_hqCategoryToBandMap.insert("A", "");
	_hqCategoryToBandMap.insert("B", "");
	_hqCategoryToBandMap.insert("C", "");
	_hqCategoryToBandMap.insert("SAQ", "");
	_hqCategoryToBandMap.insert("SAL", "");
	_hqCategoryToBandMap.insert("SAH", "");
	_hqCategoryToBandMap.insert("160", "160");
	_hqCategoryToBandMap.insert("80", "80");
	_hqCategoryToBandMap.insert("40", "40");
	_hqCategoryToBandMap.insert("20", "20");
	_hqCategoryToBandMap.insert("15", "15");
	_hqCategoryToBandMap.insert("10", "10");
	_hqCategoryToBandMap.insert("MSL", "");
	_hqCategoryToBandMap.insert("MSH", "");
	_hqCategoryToBandMap.insert("M2", "");
	_hqCategoryToBandMap.insert("MM", "");

	_hqCategoryToPwrMap.insert("CK", "");
	_hqCategoryToPwrMap.insert("A", "A");
	_hqCategoryToPwrMap.insert("B", "B");
	_hqCategoryToPwrMap.insert("C", "C");
	_hqCategoryToPwrMap.insert("SAQ", "A");
	_hqCategoryToPwrMap.insert("SAL", "B");
	_hqCategoryToPwrMap.insert("SAH", "C");
	_hqCategoryToPwrMap.insert("160", "");
	_hqCategoryToPwrMap.insert("80", "");
	_hqCategoryToPwrMap.insert("40", "");
	_hqCategoryToPwrMap.insert("20", "");
	_hqCategoryToPwrMap.insert("15", "");
	_hqCategoryToPwrMap.insert("10", "");
	_hqCategoryToPwrMap.insert("MSL", "B");
	_hqCategoryToPwrMap.insert("MSH", "C");
	_hqCategoryToPwrMap.insert("M2", "");
	_hqCategoryToPwrMap.insert("MM", "");

	_hqCategoryToSmMap.insert("CK", "");
	_hqCategoryToSmMap.insert("A", "S");
	_hqCategoryToSmMap.insert("B", "S");
	_hqCategoryToSmMap.insert("C", "S");
	_hqCategoryToSmMap.insert("SAQ", "SAQ");
	_hqCategoryToSmMap.insert("SAL", "SAL");
	_hqCategoryToSmMap.insert("SAH", "SAH");
	_hqCategoryToSmMap.insert("160", "S");
	_hqCategoryToSmMap.insert("80", "S");
	_hqCategoryToSmMap.insert("40", "S");
	_hqCategoryToSmMap.insert("20", "S");
	_hqCategoryToSmMap.insert("15", "S");
	_hqCategoryToSmMap.insert("10", "S");
	_hqCategoryToSmMap.insert("MSL", "MSL");
	_hqCategoryToSmMap.insert("MSH", "MSH");
	_hqCategoryToSmMap.insert("M2", "M2X");
	_hqCategoryToSmMap.insert("MM", "MM");
}

ArrlContest::~ArrlContest() {}

void ArrlContest::WriteTsvFile()
{
	QFile tsvFile(_resDir.absoluteFilePath(Name().toLower() + ".tsv"));
	tsvFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream outStream(&tsvFile);
	outStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));

	WriteTsvHeader(outStream);

	foreach(Log * log, _logsByCall)
	{
		if (log->Disqualified())
			continue;

		log->WriteTsvLine(outStream);
	}
	tsvFile.close();
}

