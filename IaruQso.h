#ifndef _IARUQSO
#define _IARUQSO

class IaruQso : public Qso
{
public:
	IaruQso(Log * log);
	
	void	Parse();
	bool	CompareExchange(QsoPtr rq);
	bool	IsHq()		const;
	int		ZoneSent()	const;
	int		ZoneRcvd()	const;

protected:
	int		_zoneRcvd;
	bool	_isHq;
};
#endif // _IARUQSO
