#ifndef _VHFLOG
#define _VHFLOG

typedef QMap  < QString, QsoList>				QsoListByGrid;
typedef QMap  < QString, QsoListByGrid>			QsoListByGridGrid;
typedef QMap  < QString, QsoListByGridGrid>		QsoListByBandGridGrid;
typedef QMap  < QString, QsoListByBandGridGrid>	QsoListByCallBandGridGrid;
typedef QMap  < QString, QsoListByGrid>			QsoListByBandGrid;
typedef QMap  < QString, QsoListByBandGrid>		QsoListByCallBandGrid;

class VhfContest;

class VhfLog : public ArrlLog
{
public:
	VhfLog();
	VhfQso * NewQso() { return new VhfQso(this); }
	QsoPtr	Find(QString call, QsoPtr q, int window, bool chkOtherBands = false);

	void	AddToQsoLists		(QsoPtr q);
	void	CheckBandChanges	()  {}
	void	CheckDupes			();
	void	ClearMultMaps		();
	void	ComputeCategory		();
	void	ComputeMults		();
	void	ComputeOperatingTime();
	void	ComputePts			();
	void	WriteLg				(QTextStream & strm);
	void	WriteMultFile		(QTextStream &)	{}
	void	WriteRptSummary		(QTextStream & strm);
	void	WriteRptMults		(QTextStream & strm);
	void	WriteTabLine		(QTextStream & strm);
	void	WriteTsvLine		(QTextStream & strm);

	bool	IsRover()				{ return _isRover; }
	int		GridsActivatedCount() { return _gridsSentMap.size(); }
	QStringList	GridsActivated() { return _gridsSentMap.keys(); }

	QString	ResultLine();
	QsoListByCallBandGrid		GetQsoListByCallBandGridSentMap() { return _qsoListByCallBandGridSentMap; }

	QsoList	GetQsoListByCallBandGridSentGridRcvd(QString call, QString band, QString gridSent, QString gridRcvd);
	QsoList	GetQsoListByCallBandGridSent(QString call, QString band, QString gridSent);
	QsoList	GetQsoListByCallBandGridRcvd(QString call, QString band, QString gridRcvd);
	QMap < QString, QsoPtr> & GetGridsSentMap();
	QList < QPair <QsoList, QsoList> >	GetQsoLists();
	QsoList GetQsoListByCall(QString call, QsoPtr q);

protected:
	bool						_isRover;
	QsoListByCallBandGridGrid	_qsoListByCallBandGridSentGridRcvdMap;
	QsoListByCallBandGrid		_qsoListByCallBandGridSentMap;
	QsoListByCallBandGrid		_qsoListByCallBandGridRcvdMap;
	QsoListByBandGrid			_rawMultByBandGridRcvdMap;
	QsoListByBandGrid			_multByBandGridRcvdMap;
	
	QMap < QString, QsoPtr>		_gridsSentMap;						// extra mult for rovers after chking
};

#endif // _VHFLOG
