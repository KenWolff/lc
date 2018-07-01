#ifndef _WPXRYLOG
#define _WPXRYLOG

class WpxRttyLog : public WpxLog
{
public:
	WpxRttyLog();
	WpxRyQso * NewQso() { return new WpxRyQso(this); }

	void	ComputeCategory		();
	void	ComputePts			();
	void	CheckBandChanges	();
	void	ClearMultMaps		();
	void	WriteRptSummary		(QTextStream & strm);
	void	WriteTabLine		(QTextStream & strm);

	QString ResultLine();

protected:
};
#endif // _WPXRYLOG
