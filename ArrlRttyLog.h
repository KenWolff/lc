#ifndef _ARRLRTTYLOG
#define _ARRLRTTYLOG

class ArrlRttyLog : public ArrlLog
{
public:
	ArrlRttyLog();
	ArrlRttyQso *	NewQso() { return new ArrlRttyQso(this); }

	void	CheckBandChanges	();
	void	ClearMultMaps		();
	void	ComputeCategory		();
	void	ComputeMults();
	void	ComputeOperatingTime();
	void	ComputePts();
	void	WriteRptMults		(QTextStream & strm);
	void	WriteLg				(QTextStream & strm);
	void	WriteMultFile		(QTextStream & strm);
	void	WriteRptSummary		(QTextStream & strm);
	void	WriteTabLine		(QTextStream & strm);
	void	WriteTsvLine		(QTextStream & strm);

	QString ResultLine			();

protected:
	QMap  <QString, QsoList> _rawArrlRttyMultMap;
	QMap  <QString, QsoList> _arrlRttyMultMap;
};
#endif // _ARRLRTTYLOG
