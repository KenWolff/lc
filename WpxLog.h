#ifndef _WPXLOG
#define _WPXLOG

class WpxLog : public Log
{
public:
	WpxLog();
	WpxQso * NewQso() { return new WpxQso(this); }

	void	CheckBandChanges	();
	void	ClearMultMaps		();
	void	ComputeCategory		();
	void	ComputeMults		();
	void	ComputePts			();
	void	WriteRptMults		(QTextStream & strm);
	void	WriteLg				(QTextStream & strm);
	void	WriteMultFile		(QTextStream & strm);
	void	WriteRptSummary		(QTextStream & strm);
	void	WriteTabLine		(QTextStream & strm);

	QString ResultLine();

protected:
	QMap  <QString, QsoList>	_wpxMultMap;
	QMap  <QString, QsoList>	_rawWpxMultMap;
};
#endif // _WPXLOG
