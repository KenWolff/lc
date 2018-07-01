#ifndef _VHFCONTEST
#define _VHFCONTEST

class VhfContest : public ArrlContest
{
public:
	VhfContest();
	virtual ~VhfContest() {}

	void	CheckDupes();
	void	MarkNilBust();
	void	Init();
	void	MarkCertificateWinners()		{}
	void	WriteTxtHeader	(QTextStream & strm);
	void	WriteTabHeader	(QTextStream & strm);
	void	WriteTsvHeader	(QTextStream & strm);
	QMap <QString, QString> HqCategoryMap() { return _hqCategoryMap; }

protected:
	QMap <QString, QString>		_hqCategoryMap;
};

#endif // _VHFCONTEST
