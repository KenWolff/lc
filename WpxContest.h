#ifndef _WPXCONTEST
#define _WPXCONTEST

class WpxContest : public CqContest
{
private:
enum {	SO_MAG_ORDER, SOA_MAG_ORDER, QRP_MAG_ORDER, MSH_MAG_ORDER, MSL_MAG_ORDER, M2_MAG_ORDER, MM_MAG_ORDER, RK_MAG_ORDER, TB_MAG_ORDER, CK_MAG_ORDER };

public:
	WpxContest() : CqContest() {}

	void	Init();
	void	MarkCertificateWinners();
	void	WriteMagazineFile();
	void	WriteOpsFile();
	void	WriteSbxFile();
	void	WriteTxtHeader(QTextStream & strm);
	void	WriteTabHeader(QTextStream & strm);
	void	WriteWpxPortableFile();
private:
};

#endif // _WPXCONTEST
