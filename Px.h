#ifndef _PX
#define _PX

class Cty;
extern Cty  gEmptyCty;

class Px
{
public:
   Px(QString const & pxStr, int pxCqwwZone, int pxIaruZone, QString province, QString PxCont, double lat, double lon, Cty * cty, bool specialCqwwZone, bool specialIaruZone, bool specialCont, bool exactMatch) : 
		_pxStr				(pxStr			),
		_zone				(pxCqwwZone		),
		_iaruZone			(pxIaruZone		),
		_province			(province		),
		_cont				(PxCont			),
		_lat				(lat			),
		_lon				(lon			),
		_cty				(cty			),
		_specialCqwwZone	(specialCqwwZone),
		_specialIaruZone	(specialIaruZone),
		_specialCont		(specialCont	),
		_exactMatch			(exactMatch		)
		{ }

   bool operator==(const Px p)				  {return _pxStr == p._pxStr;}
   QString		  Province()			const { return _province;		}
   QString        PxStr()				const { return _pxStr;			}
   QString        CtyNameStr()			const { return _cty->Name();	}
   QString        Cont()				const { return _cont;			}
   QString        StdPx()				const { return _cty->StdPx();	}
   Cty   const *  GetCty()				const { return _cty;			}
   int            CqwwZone()			const { return _zone;			}
   int            IaruZone()			const { return _iaruZone;		}
   double         Lat()					const { return _lat;			}
   double         Lon()					const { return _lon;			}
   bool           IsSpecialCqwwZone()	const { return _specialCqwwZone;}
   bool           IsSpecialIaruZone()	const { return _specialIaruZone;}
   bool           IsSpecialCont()		const { return _specialCont;	}
   bool           ExactMatch()			const { return _exactMatch;		}

private:
   QString     _pxStr;
   int         _zone;
   int         _iaruZone;
   QString	   _province;
   QString     _cont;
   double      _lat;
   double      _lon;
   Cty      *  _cty;
   bool        _specialCqwwZone;
   bool        _specialIaruZone;
   bool        _specialCont;
   bool        _exactMatch;
};

typedef Px *        PxPtr;
typedef Px const *  ConstPxPtr;
typedef QList <PxPtr> PxPtrList;

class PxMap
{
public:
   PxMap() {}
   virtual ~PxMap();

   int          Size()      { return _pxMap.size(); }
   void         InsertPx(QString call, Px *, bool exact);
   Px const *   FindPx(QString pxStr);
   Px const *   FindExactMatchPx(QString call);
private:
	QMap < QString, Px *>   _pxMap;
	QMap < QString, Px *>   _exactPxMap;
};

class CtyFileParser
{
public:
   CtyFileParser() {}
   QList <Cty const *> Parse (QString file, PxMap & px_map, bool cqww_cty = false);
};

extern PxMap		gPxMap;
extern Px	const gEmptyPx;

#endif // _PX
