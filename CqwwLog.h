#ifndef _CQWWLOG
#define _CQWWLOG

typedef QMap < int, QsoList >						QsoListsByZoneRcvdMap;
typedef QMap < QString, QsoListsByZoneRcvdMap >		QsoListsByBandZoneRcvdMap;

class CqwwLog : public Log
{
public:
	CqwwLog();
	void	InitByContestType();
	int		Score() const { return _score; }
	int		OverlayScore() const { return _overlayScore; }

	CqwwQso * NewQso() { return new CqwwQso(this); }

	QString	ResultLine();
	void	AssignMSBandChanges();
	void	AssignZones();
	void	AssignMostCommonZoneRcvd();
	void	CheckBandChanges();
	void	CheckMSBandChanges();
	void	ClearMultMaps();
	void	ComputeCategory();
	void	ComputePts();
	void	ComputeMults();
	void	ComputeScore();
	void	WriteLg(QTextStream & strm);
	void	WriteRptMults(QTextStream & strm);
	void	WriteMultFile(QTextStream & strm);
	void	WriteRptSummary(QTextStream & strm);
	void	WriteSprLine(QTextStream & strm);
	void	WriteTabLine(QTextStream & strm);
	void	WriteMultUniqueToMe(QTextStream & strm);

	QsoListsByBandCtyRcvdMap  const GetMultMap() const { return _ctyMultMap; }	// for KR2Q

	int		Zone() const;
	int		MostRcvdZone() const;

protected:
	QsoListsByBandCtyRcvdMap	_ctyMultMap;
	QsoListsByBandCtyRcvdMap	_rawCtyMultMap;
	QsoListsByBandZoneRcvdMap	_zoneMultMap;
	QsoListsByBandZoneRcvdMap	_rawZoneMultMap;
	QsoListsByBandCtyRcvdMap	_overlayCtyMultMap;
	QsoListsByBandZoneRcvdMap	_overlayZoneMultMap;
	QsoListsByBandCtyRcvdMap	_overlayRawCtyMultMap;
	QsoListsByBandZoneRcvdMap	_overlayRawZoneMultMap;
	int							_zone;
	int							_mostRcvdZone;
};

class CqwwRevLog : public RevLog
{
public:
	CqwwRevLog() : _zone(0) {}
	int		ZoneRcvd()			const	{ return _zone; }
	void	ZoneRcvd(int zone)			{ _zone = zone; }
private:
	int		_zone;
};
#endif // _CQWWLOG
