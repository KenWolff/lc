#ifndef _WPXRYCONTEST
#define _WPXRYCONTEST

class WpxRyContest : public WpxContest
{
private:
	enum { SO_MAG_ORDER, QRP_MAG_ORDER, MSH_MAG_ORDER, MSL_MAG_ORDER, M2_MAG_ORDER,	MM_MAG_ORDER, RK_MAG_ORDER, TB_MAG_ORDER, CK_MAG_ORDER};
public:
	WpxRyContest () : WpxContest() {}

	void	Init();
	void	MarkCertificateWinners();
	void	WriteMagazineFile();
	void	WriteSbxFile();
	void	WriteTxtHeader(QTextStream & strm);
	void	WriteTabHeader(QTextStream & strm);
private:
};

#endif // _WPXRYCONTEST
