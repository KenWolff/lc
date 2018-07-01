#ifndef _IARULOG
#define _IARULOG

typedef QMap < int, QsoList >						QsoListsByZoneRcvdMap;
typedef QMap < QString, QsoListsByZoneRcvdMap >		QsoListsByBandZoneRcvdMap;
typedef QMap < QString, QsoList >					QsoListsByHqRcvdMap;
typedef QMap < QString, QsoListsByHqRcvdMap >		QsoListsByBandHqRcvdMap;

class IaruLog : public Log
{
public:
	IaruLog();
	IaruQso * NewQso() { return new IaruQso(this); }

	void	CheckBandChanges();
	void	ComputeCategory();
	void	ComputePts();
	void	ComputeMults();
	void	ComputeScore();
	void	InitByContestType();
	void	WriteLg				(QTextStream & strm);
	void	WriteRptMults		(QTextStream & strm);
	void	WriteMultFile		(QTextStream & strm);
	void	WriteRptSummary		(QTextStream & strm);
	void	WriteTabLine		(QTextStream & strm);
	void	WriteTsvLine		(QTextStream & strm);
	void	WriteMultUniqueToMe	(QTextStream & strm);

	QString	ResultLine();
	int		Zone() const;
	int		MostRcvdZone() const;

protected:
	void	ClearMultMaps();
	QsoListsByBandCtyRcvdMap	_ctyMultMap;
	QsoListsByBandCtyRcvdMap	_rawCtyMultMap;
	QsoListsByBandZoneRcvdMap	_zoneMultMap;
	QsoListsByBandZoneRcvdMap	_rawZoneMultMap;
	QsoListsByBandHqRcvdMap		_hqMultMap;
	QsoListsByBandHqRcvdMap		_rawHqMultMap;
	int							_zone;
	int							_mostRcvdZone;
	bool						_isHq;
	bool						_isWrtc;
};

class IaruRevLog : public RevLog
{
public:
	IaruRevLog() : _zone(0) {}
	int		ZoneRcvd()			const	{ return _zone; }
	void	ZoneRcvd(int zone)			{ _zone = zone; }
private:
	int		_zone;
};
#endif // IARULOG
