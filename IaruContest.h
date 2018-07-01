#ifndef IARUCONTEST
#define IARUCONTEST

class IaruContest : public Contest
{
public:
	IaruContest() : Contest() {}
	virtual ~IaruContest() {}

	bool	IsHqAbrv		(QString abrv)	const;
	bool	IsHqFromAbrv	(QString abrv)	const;
	bool	IsHqFromCall	(QString call)	const;
	bool	IsWrtcStn		(QString call)	const;

	void	Init();
	void	MarkCertificateWinners() {};
	void	WriteAllFile();
	void	WriteHqCountsFile();
	void	WriteTabHeader(QTextStream & strm);
	void	WriteTsvHeader(QTextStream & strm);
	void	WriteTxtHeader(QTextStream & strm);
	void	WriteTabFile();
	QString	HqAbrvFromCall(QString call)	const;
	QString	HqCallFromAbrv(QString abrv)	const;

private:
	QMultiMap<QString, QString>	_hqCallByAbrvMap;
	QMap<QString, QString>		_hqAbrvByCallMap;
	QMap<QString, QString>		_wrtcStnMap;
	QSet <QString>				_hqAbrvSet;
};
#endif // IARUCONTEST
