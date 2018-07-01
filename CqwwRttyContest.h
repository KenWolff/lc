#ifndef _CQWWRTTYCONTEST
#define _CQWWRTTYCONTEST

class CqwwRttyContest : public CqwwContest
{
public:
	CqwwRttyContest() : CqwwContest() {}
	virtual ~CqwwRttyContest() {}

	void    Init();
	void    MarkCertificateWinners();
	void	WriteSbxFile();
	void    WriteTxtHeader(QTextStream & strm);
	void    WriteTabHeader(QTextStream & strm);
	void	AcquireLogs();

private:
};
#endif // _CQWWRTTYCONTEST
