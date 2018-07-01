#ifndef _OCDXCONTEST
#define _OCDXCONTEST

class OcDxContest : public CqContest
{
private:
	enum { SO_MAG_ORDER, QRP_MAG_ORDER, MS_MAG_ORDER, M2_MAG_ORDER, MM_MAG_ORDER, CK_MAG_ORDER };
public:
	OcDxContest () : CqContest() {}

	void	Init();
	void	ClearMultMaps();
	void	MarkCertificateWinners() {}
	void	WriteTxtHeader(QTextStream & strm);
	void	WriteTabHeader(QTextStream & strm);
private:
};

#endif // _OCDXCONTEST
