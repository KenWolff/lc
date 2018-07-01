#ifndef _OCDXQSO
#define _OCDXQSO

class OcDxLog;
class OcDxQso : public Qso
{
public:
	OcDxQso(OcDxLog * log);
	void	 Parse();
	bool	 CompareExchange(QsoPtr rq);
};

#endif // _OCDXQSO
