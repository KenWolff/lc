#ifndef _ARRLRTTYCONTEST
#define _ARRLRTTYCONTEST

class ArrlRttyContest : public Contest
{
public:
	ArrlRttyContest () : Contest()  {}
	virtual ~ArrlRttyContest() {}
	void	Init();
	void	MarkCertificateWinners() {}
	void	WriteLg(QTextStream & strm);
	void	WriteTabHeader	(QTextStream & strm);
	void	WriteTsvHeader	(QTextStream & strm);
	void	WriteTxtHeader	(QTextStream & strm);
	void	DeriveExchInfo();
private:
};

#endif // _ARRLRTTYCONTEST
