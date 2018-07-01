#ifndef _ARRLDXLOG
#define _ARRLDXLOG

class ArrlDxLog : public ArrlLog
{
public:
	ArrlDxLog();
	ArrlDxQso *	NewQso			() { return new ArrlDxQso(this); }
	void	CheckBandChanges    ();
	void	ComputeCategory     ();
	void	ComputePts          ();
	void	ComputeScore		();
	void	WriteRptMults		(QTextStream & strm);
	void	WriteRptSummary		(QTextStream & strm);
	void	WriteTabLine		(QTextStream & strm);
	void	WriteTsvLine		(QTextStream & strm);
	void	WriteMultFile		(QTextStream & strm);

	QString ResultLine();

	QList <Qso *> list;
	virtual	int											MultTotal() = 0;
	virtual	QMap  <QString, QMap < QString, QsoList> >	MultMap()	= 0;

protected:
};

class ArrlDxDxLog : public ArrlDxLog
{
public:
	ArrlDxDxLog() : ArrlDxLog() {}
	void	ClearMultMaps();
	void	ComputeMults();
	void	WriteLg(QTextStream & strm);
	int		MultTotal();
	QMap  <QString, QMap < QString, QsoList> > MultMap() { return _multByBandStateMap; }

protected:
	QMap  <QString, QMap < QString, QsoList> > _multByBandStateMap;
	QMap  <QString, QMap < QString, QsoList> > _rawMultByBandStateMap;
};

class ArrlDxWVeLog : public ArrlDxLog
{
public:
	ArrlDxWVeLog();
	void	ClearMultMaps();
	void	ComputeMults();
	void	WriteLg(QTextStream & strm);
	int		MultTotal();
	QMap  <QString, QMap < QString, QsoList> > MultMap() { return _multByBandCtyMap; }

protected:
	QMap  <QString, QMap < QString,  QsoList> > _multByBandCtyMap;
	QMap  <QString, QMap < QString,  QsoList> > _rawMultByBandCtyMap;
};

#endif // _ARRLDXLOG
