#ifndef _ARRLDXQSO
#define _ARRLDXQSO

class ArrlDxQso : public Qso
{
public:
	ArrlDxQso(Log * log);

	void	Parse();
	QString StringToPower(QString s) const;
	QString	ToString() const;
};

#endif // _ARRLDXQSO
