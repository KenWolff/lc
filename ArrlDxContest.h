#ifndef _ARRLDXCONTEST
#define _ARRLDXCONTEST

class ArrlDxContest : public Contest
{
public:
	ArrlDxContest();
	virtual ~ArrlDxContest() {}

	void	Init();
	void	MarkCertificateWinners() {}
	void	WriteTabFile();
	void	WriteTabHeader(QTextStream & strm);
	void	WriteTsvHeader(QTextStream & strm);
	void	WriteTxtHeader(QTextStream & strm);
	void    DeriveExchInfo ();

};
#endif // _ARRLDXCONTEST
