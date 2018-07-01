#ifndef _ARRLRTTYQSO
#define _ARRLRTTYQSO

class ArrlRttyQso : public Qso
{
public:
	ArrlRttyQso(Log * log);
	void	Parse	   ();
	bool	CompareExchange(QsoPtr rq);
};

#endif // _ARRLRTTYQSO
