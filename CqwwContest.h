#ifndef _CQWWCONTEST
#define _CQWWCONTEST

class CqwwContest : public CqContest
{
private:
	enum { SO_MAG_ORDER, QRP_MAG_ORDER, MS_MAG_ORDER, M2_MAG_ORDER, MM_MAG_ORDER, RK_MAG_ORDER, CL_MAG_ORDER, CK_MAG_ORDER };

public:
	CqwwContest();
	virtual ~CqwwContest() {}
	bool	IsRtty()								{ return _isRtty; }		// some things in RTTY version of CQWW are different

	void	MarkNilBust();
	void	Init();
	void	IsRtty(bool ry)							{ _isRtty = ry; }		// some things in RTTY version of CQWW are different
	void	MarkCertificateWinners();
	void	WriteAllFile();
	void	WriteBohBustFile();
	void	WriteOpsFile();
	void	WriteTxtHeader(QTextStream & strm);
	void	WriteTabHeader(QTextStream & strm);
	void	WriteTabFile();
	void	WriteMagazineFile();
	void	WriteSbxFile();
	void	WriteWorkedBustedSpotFile();
	void	AcquireLogs();
private:
	bool	_isRtty;
};
#endif // _CQWWCONTEST
