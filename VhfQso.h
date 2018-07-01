#ifndef _VHFQSO
#define _VHFQSO

class VhfQso : public Qso
{
public:
	VhfQso(Log * log);

	void	Parse();
	QString	ToString()				const;
	QString	ToTabString()			const;
	QString	ToFullReverseString()	const;
	bool	CompareExchange(Qso * rq);
private:
};

typedef VhfQso * VhfQsoPtr;
#endif // _VHFQSO
