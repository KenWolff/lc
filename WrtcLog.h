#ifndef _WRTCLOG
#define _WRTCLOG

typedef QMap < int, QsoList >						QsoListsByZoneRcvdMap;
typedef QMap < QString, QsoListsByZoneRcvdMap >		QsoListsByBandZoneRcvdMap;

class WrtcLog : public IaruLog
{
public:
	WrtcLog();
	void	CheckBandChanges() {}
	void	ComputePts();
	void	ComputeMults();
	void	WriteRptMults(QTextStream & strm);
	void	WriteMultFile(QTextStream & strm);
	void	WriteRptSummary(QTextStream & strm);
	QString	ResultLine();
};

class WrtcRevLog : public RevLog
{
public:
	WrtcRevLog() : _zone(0) {}
	int		ZoneRcvd()			const	{ return _zone; }
	void	ZoneRcvd(int zone)			{ _zone = zone; }
private:
	int		_zone;
};
#endif // _WRTCLOG
