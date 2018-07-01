#ifndef _WPXQSO
#define _WPXQSO

class WpxQso : public Qso
{
public:
	WpxQso(Log * log);

	void	Parse();
	bool	CompareExchange(QsoPtr rq);
private:
};

#endif // _WpxQso
