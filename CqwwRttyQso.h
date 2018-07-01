#ifndef _CQWWRTTYQSO
#define _CQWWRTTYQSO

class CqwwRttyQso : public CqwwQso
{
public:
	CqwwRttyQso(Log * log);
	bool	CompareExchange		(QsoPtr rq);
	void	Parse				();
	QString	ToString			() const;
	QString	ToTabString			() const;
	QString	ToFullReverseString	() const;
	QString	LocationExpected	() const;
	void	AssignLocationExpected();
	bool	IsStateValid		() const;

private:
	QString _locationExpected;
};
#endif // _CQWWRTTYQSO
