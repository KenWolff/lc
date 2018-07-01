#ifndef _SPOT
#define _SPOT

class Spot
{
public:
    Spot(void);

    Spot(QString str);
    Spot(QString spotter, double qrg, QDateTime dt, QString call);
    Spot(QString date, QString time, double qrg, QString call, QString spotter);
    Spot(Spot const & sp);
    ~Spot(void);

    bool    IsUnique()		const		{ return _isUnique;		}
    bool	First()			const		{ return _first;		}
    bool	Isolated()		const		{ return _isolated;		}
    bool	NewFreq()		const		{ return _newFreq;		}
    double	Qrg()			const		{ return _qrg;			}
    QString	Band()			const		{ return _band;			}
    QString	Call()			const		{ return _call;			}
    QString	CorrectCall()	const		{ return _correctCall;	}
    QString	Comment()		const		{ return _comment;		}
    QString	Spotter()		const		{ return _spotter;		}
    QDateTime	DateTime()	const		{ return _dateTime;		}
    QString	ToString()		const;
    QString	ToTabString()	const;
    QString	DateStr()		const		{ return _dateTime.date().toString("yyyy-MM-dd");	}
    QString	TimeStr()		const		{ return _dateTime.time().toString("hhmm");		}

    void		First		(bool b)		{ _first		= b;	}
    void		Isolated	(bool b)		{ _isolated		= b;	}
    void		NewFreq		(bool b)		{ _newFreq		= b;	}
    void		RbnBust		(bool b)		{ _rbnBust		= b;	}
    void		IsUnique	(bool b)		{ _isUnique		= b;	}
    void		CorrectCall	(QString s)		{ _correctCall	= s;	}
    void		Band		(QString band)	{ _band			= band;	}

protected:
    bool		_first		: 1;
    bool		_isolated	: 1;
    bool		_newFreq	: 1;
    bool		_rbnBust	: 1;
    bool		_isUnique	: 1;
    bool		_valid		: 1;
    double		_qrg;
    QString		_band;
    QDateTime	_dateTime;
    QString		_call;
    QString		_comment;
    QString		_spotter;
    QString		_correctCall;
};

class BustedRbnSpot : public Spot
{
public:
    BustedRbnSpot(QString spotter, double qrg, QDateTime dt, QString call, QString correctSpotter, double correctQrg, QDateTime correctDt, QString correctCall);
    QString		ToString()	const;
    QString		ToTabString()	const;

private:
    double		_correctQrg;
    int         _correctMinute;
    QString		_correctBand;
    QDateTime	_correctDateTime;
    QString		_correctCall;
    QString		_correctSpotter;
};

typedef Spot		*	SpotPtr;
typedef Spot const	*	ConstSpotPtr;

class SpotLessBySpotter
{
public:
    SpotLessBySpotter()	{}
    bool operator() (ConstSpotPtr a, ConstSpotPtr b) const
    {
        if(a->Spotter() == b->Spotter())
            return a->DateTime() < b->DateTime();
        return a->Spotter()	< b->Spotter();
    }
};

class SpotLessByFreq
{
public:
    SpotLessByFreq()		{}
    bool operator() (ConstSpotPtr a, ConstSpotPtr b) const
    {
        if(a->Qrg() == b->Qrg())
            return a->DateTime() < b->DateTime();
        return a->Qrg() < b->Qrg();
    }
};

typedef QSet <ConstSpotPtr>	    SpotSet;
typedef QMap <QString,			ConstSpotPtr			>	SpotByCallMap;
typedef QMap <QDateTime,		SpotByCallMap			>	SpotByTimeSpotterMap;
typedef QMap <QString,			SpotByTimeSpotterMap	>	SpotByCallTimeSpotterMap;

bool gSpotTimeLessThan(Spot const *  sp1, Spot const *  sp2);
#endif // _SPOT
