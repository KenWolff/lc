#ifndef _CQWWQSO
#define _CQWWQSO

class CqwwQso : public Qso
{
public:
	CqwwQso(Log * log);
	
	void	Parse();
	bool	CompareExchange(QsoPtr rq);
	int		ZoneRcvd()	const;
	int		ZoneExpected()	const;
	void	AssignZoneExpected();

protected:
	int		_zoneRcvd;
	int		_zoneSent;
	int		_zoneExpected;
};
#endif // _CQWWQSO
