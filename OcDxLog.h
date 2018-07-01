#ifndef _OCDXLOG
#define _OCDXLOG

class OcDxLog : public Log
{
public:
	OcDxLog();
	OcDxQso * NewQso() { return new OcDxQso(this); }

	void	ClearMultMaps();
	void	ComputeCategory();
	void	ComputePts();
	void	CheckBandChanges();
	void	ComputeMults();
	void	ComputeScore();

	void	WriteLg			(QTextStream & strm);
	void	WriteMultFile	(QTextStream & strm);
	void	WriteRptMults	(QTextStream & strm);
	void	WriteRptSummary	(QTextStream & strm);
	void	WriteTabLine	(QTextStream & strm);
	void	FillPaperLogFromParts(QStringList parts);

	QString ResultLine();

protected:
	QMap  <QString, QMap<QString, QsoList> >	_multByBandPxMap;
	QMap  <QString, QMap<QString, QsoList> >	_rawMultByBandPxMap;
};
#endif // _OCDXLOG
