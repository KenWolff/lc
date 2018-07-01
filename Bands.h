#ifndef _BANDS
#define _BANDS

// band edge definitions

typedef int FREQ;
enum SIDEBAND  {USB = 0, LSB = 1};

class BandEdges
{
public:
   BandEdges() : _name("ALL"), _lowerEdge(0), _upperEdge(0), _sideband(LSB) {}

   BandEdges(
	  QString name,
	  FREQ low,
	  FREQ high,
	  FREQ defaultCW,
	  FREQ defaultSSB,
	  FREQ defaultRTTY,
	  SIDEBAND sb)
	  :  _name				(name),
		 _lowerEdge			(low),
		 _upperEdge			(high),
		 _defaultFreqCW		(defaultCW),
		 _defaultFreqSSB	(defaultSSB),
		 _defaultFreqRTTY	(defaultRTTY),
		 _sideband			(sb)
   {}

   virtual ~BandEdges() {}

   BandEdges(BandEdges const & b)
	  :  _name				( b._name			),
		 _lowerEdge			( b._lowerEdge		),
		 _upperEdge			( b._upperEdge		),
		 _defaultFreqCW		( b._defaultFreqCW	),
		 _defaultFreqSSB	( b._defaultFreqSSB	),
		 _defaultFreqRTTY	( b._defaultFreqRTTY),
		 _sideband			( b._sideband		)
   {}

   BandEdges & operator=(BandEdges const & b)
   {
	  _name				= b._name;
	  _lowerEdge		= b._lowerEdge;
	  _upperEdge		= b._upperEdge;
	  _defaultFreqCW	= b._defaultFreqCW;
	  _defaultFreqSSB	= b._defaultFreqSSB;
	  _defaultFreqRTTY  = b._defaultFreqRTTY;
	  _sideband			= b._sideband;
	  return *this;
   }

   FREQ        LowerEdge()            const { return _lowerEdge;      }
   void        LowerEdge(FREQ f)            { _lowerEdge = f;         }
   FREQ        UpperEdge()            const { return _upperEdge;      }
   void        UpperEdge(FREQ f)            {   _upperEdge = f;       }
   FREQ        DefaultFreqCW()        const { return _defaultFreqCW;  }
   void        DefaultFreqCW(FREQ f)        { _defaultFreqCW = f;     }
   FREQ        DefaultFreqSSB()       const { return _defaultFreqSSB; }
   void        DefaultFreqSSB(FREQ f)       { _defaultFreqSSB = f;    }
   FREQ        DefaultFreqRTTY()      const { return _defaultFreqRTTY;}
   void        DefaultFreqRTTY(FREQ f)      { _defaultFreqRTTY = f;   }
   SIDEBAND    Sideband()             const { return _sideband;       }
   void        Sideband(SIDEBAND sb)        { _sideband = sb;         }
   QString     Name()                 const { return _name;           }
   void        Name(char const * name)      { _name = name;           }

private:
   QString     _name;
   FREQ        _lowerEdge;
   FREQ        _upperEdge;
   FREQ        _defaultFreqCW;
   FREQ        _defaultFreqSSB;
   FREQ        _defaultFreqRTTY;
   SIDEBAND    _sideband;
};
extern BandEdges  B160  ;
extern BandEdges  B80   ;
extern BandEdges  B40   ;
extern BandEdges  B30   ;
extern BandEdges  B20   ;
extern BandEdges  B17   ;
extern BandEdges  B15   ;
extern BandEdges  B12   ;
extern BandEdges  B10   ;
extern BandEdges  B6    ;
extern BandEdges  B2    ;
extern BandEdges  B222  ;
extern BandEdges  B432  ;
extern BandEdges  B903  ;
extern BandEdges  B1296 ;
extern BandEdges  B2304 ;
extern BandEdges  B3456 ;
extern BandEdges  B5760 ;
extern BandEdges  B10G  ;
extern BandEdges  B24G  ;
extern BandEdges  B47G  ;
extern BandEdges  B75G  ;
extern BandEdges  B119G ;
extern BandEdges  B142G ;
extern BandEdges  B241G ;
extern BandEdges  BLight;
extern BandEdges  BSat;
extern BandEdges  BNone;
#endif // _BANDS
