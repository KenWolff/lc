#include "StdAfx.h"

BustedRbnSpot::BustedRbnSpot(QString spotter, double qrg, QDateTime dt, QString call, QString correctSpotter, double correctQrg, QDateTime correctDt, QString correctCall) :
     Spot(spotter, qrg, dt, call),  _correctQrg(correctQrg), _correctDateTime(correctDt),  _correctSpotter(correctSpotter)
{
    _correctCall = correctCall;
}

QString BustedRbnSpot::ToString() const
{
    return QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
        .arg(_qrg, 7 , 'f', 1)
        .arg(_dateTime.date().toString(Qt::ISODate))
        .arg(_dateTime.time().toString("hhmm"))
        .arg(_call				,-12)
        .arg(_spotter			,-12)
        .arg(_correctQrg, 7		, 'f', 1)
        .arg(_correctDateTime.date().toString(Qt::ISODate))
        .arg(_correctDateTime.time().toString("hhmm"))
        .arg(_correctCall		,-12)
        .arg(_correctSpotter	 ,-12);
}

QString BustedRbnSpot::ToTabString() const
{
    return QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10")
        .arg(_qrg, 7			, 'f', 1)
        .arg(_dateTime.date().toString(Qt::ISODate))
        .arg(_dateTime.time().toString("hhmm"))
        .arg(_call				,-12)
        .arg(_spotter			,-12)
        .arg(_correctQrg		, 7,'f', 1)
        .arg(_correctDateTime.date().toString(Qt::ISODate))
        .arg(_correctDateTime.time().toString("hhmm"))
        .arg(_correctCall		,-12)
        .arg(_correctSpotter	,-12);
}

Spot::Spot(QString spotter, double qrg, QDateTime dt, QString call) :
    _first		(false),
    _isolated	(false),
    _newFreq	(false),
    _rbnBust	(false),
    _isUnique	(false),
    _valid		(true),
    _qrg		(qrg),
    _dateTime	(dt),
    _call		(call),
    _spotter	(spotter)
{
    _band = gFreqToBand(_qrg).Name();
}

Spot::Spot(QString date, QString time, double qrg, QString call, QString spotter) :
    _first		(false),
    _isolated	(false),
    _newFreq	(false),
    _rbnBust	(false),
    _isUnique	(false),
    _valid		(true),
    _qrg		(qrg),
    _call		(call),
    _spotter	(spotter)
{
    _dateTime = QDateTime::fromString(date + " " + time, "yyyy-MM-dd hhmm");
    _dateTime.setTimeSpec(Qt::UTC);
    _band = gFreqToBand(_qrg).Name();
}

Spot::Spot(QString line) :
    _first(false),
    _isolated(false),
    _newFreq(false),
    _rbnBust(false),
    _isUnique(false),
    _valid(false),
    _qrg(0.0)
{
    QStringList segs = line.remove(' ').split('\t');

    while(segs.size() < 5)	
        segs << QString();

    _dateTime	= QDateTime::fromString(segs[0] + " " + segs[1], "yyyy-MM-dd hhmm");
    _dateTime.setTimeSpec(Qt::UTC);
    _call		= segs[2].toUpper();
    _qrg		= segs[3].toDouble();
    _spotter	= segs[4].toUpper();
    _band       = gFreqToBand(_qrg).Name();
}

QString Spot::ToString() const
{
    return QString("%1 %2 %3 %4 %5 %6 %7")
        .arg(_qrg, 7 , 'f', 1)
        .arg("  "		   ,2)
        .arg(_dateTime.date().toString(Qt::ISODate))
        .arg(_dateTime.time().toString("hhmm"))
        .arg(_call			,-12)
        .arg(_spotter	   ,-12)
        .arg(_comment);
}

QString Spot::ToTabString() const
{
    return QString("%1\t%2\t%3\t%4\t%5\t%6")
        .arg(_qrg, 7 , 'f', 1)
        .arg(_dateTime.date().toString(Qt::ISODate))
        .arg(_dateTime.time().toString("hhmm"))
        .arg(_call			,-12)
        .arg(_spotter	   ,-12)
        .arg(_comment);
}

Spot::Spot(void) :
        _first(false),
        _isolated(false),
        _newFreq(false),
        _rbnBust(false),
        _isUnique(false),
        _valid(false),
        _qrg(0.0),
        _call("Empty")
    {}

Spot::Spot(Spot const & sp) :		// copy constructor
        _first		(sp._first),
        _isolated	(sp._isolated),
        _newFreq	(sp._newFreq),
        _rbnBust    (sp._rbnBust),
        _isUnique	(sp._isUnique),
        _valid      (sp._valid),
        _qrg		(sp._qrg),
        _band		(sp._band),
        _dateTime	(sp._dateTime),
        _call		(sp._call),
        _comment	(sp._comment),
        _spotter	(sp._spotter),
        _correctCall(sp._correctCall)
        {}

Spot::~Spot(void)
{
}

bool gSpotTimeLessThan(Spot const *  sp1, Spot const *  sp2)
{
    return sp1->DateTime() < sp2->DateTime();
}
