#include "StdAfx.h"
#include "Cty.h"

QMap <int, QString> gCtyNameByNum;
QStringList			gCtyNames;
QMap <QString, int> gCtyNumByName;
QSet <Cty const *>	gUsPossessionSet;
int					Cty::CtyCount;

Cty const *W_id, *KH0_id, *KH1_id, *KH2_id, *KH3_id, *KH4_id, *KH5_id, *KH6_id, *KH8_id, *KH9_id, *KC4_id, *BY_id, *KG4_id, *KC6_id, *KL7_id, *KP1_id, *KP2_id, *KP4_id, *KP5_id, *VE_id, *CY9_id, *CY0_id, *UA_id, *UA0_id, *CE_id, *EA_id, *VK_id, *SP_id, *PA_id, *JA_id, *LU_id, *PY_id, *ZS_id, *ZL_id;

QList <Cty const *> CtyFileParser::Parse(QString fileName, PxMap & px_map, bool cqwwFlag)
{
	QList <Cty const *> ctyList;
	QFile ctyFile(fileName);

	if (!ctyFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		gStdErrStrm << " could not open" << fileName << endl;
		exit(-1);
	}

	QTextStream ctyStream(&ctyFile);
	QStringList lines;
	QString fileStr;

	while (!ctyStream.atEnd())
	{
		QString line = ctyStream.readLine();
		if (!line.startsWith('#'))
			fileStr += line;
	}
	ctyFile.close();

	fileStr = fileStr.simplified();						// remove all excess white space

	// parse countries,

	QStringList ctyLines = fileStr.split(';', QString::SkipEmptyParts); // break into country records
	foreach(QString line, ctyLines)
	{
		QString   std_px;
		QString   cont;
		int       cqwwZone;
		int	      iaruZone;
		double	  lat;
		double	  lon;
		double	  time_offset;

		// split a country record into component parts

		QStringList ctyParts = line.split(':');
		if (ctyParts.size() < 9)
		{
			gContest->AppendErrMsgList("CTY.DAT:", QString("Missing ':' in %1 line: %2").arg(fileName).arg(line));
			continue;
		}

		QString name;
		QString pxStr;

		name = ctyParts[0];
		cqwwZone = ctyParts[1].toInt();
		iaruZone = ctyParts[2].toInt();
		cont = ctyParts[3].simplified();
		lat = ctyParts[4].toDouble();
		lon = ctyParts[5].toDouble();
		time_offset = ctyParts[6].toDouble();
		std_px = ctyParts[7].simplified();
		pxStr = ctyParts[8].simplified();

		if (std_px.startsWith('*'))				// px starting with '*' is special for cqww
		{
			std_px.remove(0, 1);
			if (!cqwwFlag)
				continue;
		}
		name = name.simplified();
		Cty * cty = new Cty(ctyList.size() + 1, name, std_px, cqwwZone, iaruZone, cont, time_offset, lat, lon, true);

		gCtyNames.append(name);
		ctyList.append(cty);

		if(cty->Name().contains(QRegularExpression("United States|Alaska|Puerto Rico|U.S. Virgin Islands|Midway I|Mariana Is|Baker & Howland Is|Guam|Johnston I|Hawaii|Kure I")))
			cty->IsInFccData(true);

		if (cty->Name().contains("United States"))
			W_id = cty;

		if (cty->Name() == "Canada")
			VE_id = cty;

		if (cty->Name() == "Northern Mariana Islands")
			KH0_id = cty;

		if(cty->Name() == "Baker & Howland Islands")
			KH1_id = cty;

		if(cty->Name() == "Guam")
			KH2_id = cty;

		if(cty->Name() == "Johnston Island")
			KH3_id = cty;

		if(cty->Name() == "Midway Island")
			KH4_id = cty;

		if(cty->Name() == "Palmyra & Jarvis Islands")
			KH5_id = cty;

		if(cty->Name() == "Hawaii")
			KH6_id = cty;

		if(cty->Name() == "American Samoa")
			KH8_id = cty;

		if(cty->Name() == "Wake Island")
			KH9_id = cty;

		if(cty->Name() == "Alaska")
			KL7_id = cty;

		if (cty->StdPx() == "KP1")
			KP1_id = cty;

		if (cty->Name() == "U.S. Virgin Islands")
			KP2_id = cty;

		if(cty->Name() == "Puerto Rico")
			KP4_id = cty;

		if (cty->StdPx() == "KP5")
			KP5_id = cty;

		if(cty->Name() == "Guantanamo Bay")
			KG4_id = cty;

		if (cty->Name() == "Canada")
			VE_id = cty;

		if (cty->StdPx() == "CY9")
			CY9_id = cty;

		if (cty->StdPx() == "CY0")
			CY0_id = cty;

		if(cty->Name() == "Asiatic Russia")
			UA0_id = cty;

		if(cty->Name() == "European Russia")
			UA_id = cty;

		if(cty->Name() == "Chile")
			CE_id = cty;

		if(cty->Name() == "Japan")
			JA_id = cty;

		if(cty->Name() == "Brazil")
			PY_id = cty;

		if(cty->Name() == "Antarctica")
			KC4_id = cty;

		if(cty->Name() == "China")
			BY_id = cty;

		if(cty->Name() == "Spain")
			EA_id = cty;

		if(cty->Name() == "Australia")
			VK_id = cty;

		if(cty->Name() == "Belau")
			KC6_id = cty;

		if(cty->Name() == "Argentina")
			LU_id = cty;

		if(cty->Name() == "Netherlands")
			PA_id = cty;

		if(cty->Name() == "Poland")
			SP_id = cty;

		if(cty->Name() == "South Africa")
			ZS_id = cty;

		if(cty->Name() == "New Zealand")
			ZL_id = cty;

		// fill set of US possessions for stuipid FCC portable problem

		gUsPossessionSet.insert(KH0_id);
		gUsPossessionSet.insert(KH1_id);
		gUsPossessionSet.insert(KH2_id);
		gUsPossessionSet.insert(KH3_id);
		gUsPossessionSet.insert(KH4_id);
		gUsPossessionSet.insert(KH5_id);
		gUsPossessionSet.insert(KH6_id);
		gUsPossessionSet.insert(KL7_id);
		gUsPossessionSet.insert(KH8_id);
		gUsPossessionSet.insert(KH9_id);
		gUsPossessionSet.insert(KC4_id);
		gUsPossessionSet.insert(KG4_id);
		gUsPossessionSet.insert(KC6_id);
		gUsPossessionSet.insert(KP2_id);
		gUsPossessionSet.insert(KP4_id);

		// split prefix list into prefixes

		// look for provinces
		QString province;
		auto lis = pxStr.split('$');
		for (int i = 0; i < lis.size(); i++)
		{
			if (i % 2 != 0)			// even numbers are provinces
			{
				province = lis[i];
				QRegularExpression re("\\((.+)\\)");
				QRegularExpressionMatch m = re.match(province);
				if(m.hasMatch())
					province = m.captured(1);
			}
			else
			{
				QStringList pxStrs = lis[i].split(',');
				pxStr.remove(QRegularExpression("\\s+"));

				foreach(QString part, pxStrs)
				{
					part = part.trimmed();
					QString	pxCont = cont;
					int		pxCqwwZone = cqwwZone;
					int		pxIaruZone = iaruZone;
					double	pxLat = lat;
					double	pxLon = lon;
					bool	specialCqwwZone = false;
					bool	specialIaruZone = false;
					bool	specialCont = false;
					bool	exactMatch = false;

					QRegularExpression cqZoneRe("\\((.+)\\)");
					QRegularExpression iaruZoneRe("\\[(.+)\\]");
					QRegularExpression contRe("\\{(.+)\\}");
					QRegularExpressionMatch match;

					match = cqZoneRe.match(part);
					if (match.hasMatch())				// cqww zone override
					{
						specialCqwwZone = true;
						pxCqwwZone = match.captured(1).toInt();
						part.remove(cqZoneRe);
					}

					match = iaruZoneRe.match(part);
					if (match.hasMatch())				// iaru zone override
					{
						specialIaruZone = true;
						pxIaruZone = match.captured(1).toInt();
						part.remove(iaruZoneRe);
					}

					match = contRe.match(part);
					if (match.hasMatch())				// continent override
					{
						specialCont = true;
						pxCont = match.captured(1);
						part.remove(contRe);
					}

					exactMatch = part.startsWith('=');
					if (exactMatch)
						part.remove(0, 1);	   // get rid of '=' flag

					part = part.simplified();
					PxPtr pxPtr = new Px(part, pxCqwwZone, pxIaruZone, province, pxCont, pxLat, pxLon, cty, specialCqwwZone, specialIaruZone, specialCont, exactMatch);

					px_map.InsertPx(part, pxPtr, exactMatch);
					cty->AppendPx(pxPtr);
				}
			}
		}
	}
	return ctyList;
}

Cty::~Cty()
{
	foreach(PxPtr p, _pxList)
		delete p;
}

QString		const	Cty::Name()				const	{ return _name;							}
QString		const	Cty::StdPx()			const	{ return _std_px;						}
QString		const	Cty::Cont()				const	{ return _cont;							}
int					Cty::Zone()				const	{ return _zone;							}
int					Cty::IaruZone()			const	{ return _iaruZone;						}
int					Cty::Pts()				const	{ return _pts;							}
double				Cty::TimeAdjustment()	const	{ return _timeZoneOffset;				}
double				Cty::Lat()				const	{ return _lat;							}
double				Cty::Lon()				const	{ return _lon;							}
bool				Cty::IsInFccData()		const	{ return _isInFccData;					}
bool				Cty::IsKG4()			const	{ return this == KG4_id;				}
bool				Cty::IsUsaPossession()	const	{ return this != W_id && _isInFccData;	}
bool				Cty::IsWVe()			const	{ return this == W_id || this == VE_id; }
bool				Cty::IsW()				const	{ return this == W_id;					}
PxPtrList	const	Cty::PxList()			const	{ return _pxList;						}

void				Cty::AppendPx(Px * pxPtr)		{ _pxList.append(pxPtr);				}
void				Cty::IsInFccData(bool b)		{ _isInFccData = b;						}

