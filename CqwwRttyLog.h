#ifndef _CQWWRTTYLOG
#define _CQWWRTTYLOG

class CqwwRttyLog : public CqwwLog
{
public:
	CqwwRttyLog();
	void	InitByContestType();
	CqwwRttyQso * NewQso() { return new CqwwRttyQso(this);}
	QString	ResultLine			();
	QString	Qth() const;
	
	int		Score				() const	{ return _score;			}
	int		OverlayScore		() const	{ return _overlayScore;		}

	void	AssignMSBandChanges();
	void	AssignQthsExpected();
	void	ClearMultMaps();
	void	ComputePts();
	void	ComputeMults();
	void	CheckBandChanges();

	void	CheckMSBandChanges	(int cnt);
	void	WriteRptMults		(QTextStream & strm);
	void	WriteLg				(QTextStream & strm);		// mults are different
	void	WriteMultFile		(QTextStream & strm);
	void	WriteRptSummary		(QTextStream & strm);
	void	WriteTabLine		(QTextStream & strm);

protected:
	QMap	< QString, QMap < QString, QList < QsoPtr > > > _rawStateMultMap;
	QMap	< QString, QMap < QString, QList < QsoPtr > > > _stateMultMap;
	QMap	< QString, QMap < QString, QList < QsoPtr > > > _rawOverlayStateMultMap;
	QMap	< QString, QMap < QString, QList < QsoPtr > > > _overlayStateMultMap;
};

#endif // _CQWWRTTYLOG
