#pragma once
#include "StdAfx.h"

class CatDesc;
class MagOpDesc;

class CqContest : public Contest
{
public:
	CqContest();
	~CqContest();
	CatDesc	const CatDescrFromNum(int) const;
	int	CatNumFromInternalDesc(QString s) const;

protected:
	QMap <int, CatDesc>		_catDescByNum;
	QMap <int, MagOpDesc>	_magOpDescByMagOrderNumMap;
	QMap <QString, int> 	_catNumByInternalDesc;
};

class MagOpDesc			// magazine operator description and ordering integer
{
public:
	MagOpDesc() : _desc("NG"), _magOrder(-1) {}
	MagOpDesc(QString desc, int magOrder) : _desc(desc), _magOrder(magOrder) {}

	QString	Desc()		const { return _desc; }
	int		MagOrder()	const { return _magOrder; }

private:
	QString	_desc;
	int		_magOrder;
};

class CatDesc			// category description strings
{
public:
	CatDesc() : _internal("Not Initialized"), _abbreviation("NG"), _full("Not a Category") {}
	CatDesc(QString internal, QString abbrev, QString full, int pwrNum, int bandNum, MagOpDesc magOpDesc = MagOpDesc()) :
		_internal(internal),
		_abbreviation(abbrev),
		_full(full),
		_pwrNum(pwrNum),
		_bandNum(bandNum),
		_magOpDesc(magOpDesc)
	{}

	QString		Abbreviation()		const { return _abbreviation; }
	QString		Full()				const { return _full; }
	QString		Internal()			const { return _internal; }
	int			PwrNum()			const { return _pwrNum; }
	int			BandNum()			const { return _bandNum; }
	MagOpDesc	MagOpDescription()	const { return _magOpDesc; }

private:
	QString		_internal;
	QString		_abbreviation;
	QString		_full;
	int			_pwrNum;
	int			_bandNum;
	MagOpDesc	_magOpDesc;
};
