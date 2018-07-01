#ifndef _QSO
#define _QSO

class Qso;
class Log;
class Contest;

typedef Qso *					QsoPtr;
typedef Qso const *				ConstQsoPtr;
typedef QList<Qso *>			QsoList;
typedef QListIterator<Qso *>	QsoListIterator;
typedef QSet<Qso *>				QsoSet;
typedef QMap<QString, Qso *>	FirstQsoByCallMap;
typedef QPair<QString, int>		QsoLinePair;


enum QSO_STATUS 
{	
	NO_LOG,
	GOOD_QSO,
	DO_NOT_CHECK,
	NOT_IN_LOG,
	LOG_NO_MATCH,
	LOG_NO_GRID_MATCH,
	BUSTED_CALL,
	BUSTED_FCC_CALL,
	BUSTED_REVLOG_CALL,
	BUSTED_LIST_CALL,
	BUSTED_EXCHANGE,
	BUSTED_REVLOG_EXCHANGE,
	FORMAT_ERR,
	DOES_NOT_COUNT,
	EXCLUDED,
	BAND_CHANGE_VIOLATION,
	RUBBER_CLOCK_VIOLATION,
	OUT_OF_BAND
};

class CallFormer
{
public:
	CallFormer() {}
	QString operator() (QString call) const { return RootCall(call); }
	QString RootCall(QString call) const;
};

class QsoLineData
{
public:
	QsoLineData()	: _isExcluded(false), _lineNum(-1) {}
	QsoLineData(QString line, int lineNum, bool excluded, bool outOfBand, bool wrongBand, bool rubberClocked) :
		_line(line), 
		_lineNum(lineNum),
		_isExcluded(excluded),
		_isOutOfBand(outOfBand),
		_isWrongBand(wrongBand),
		_isRubberClocked(rubberClocked)
{}

	bool	IsExcluded()		const { return _isExcluded; }
	bool	IsOutOfBand()		const { return _isOutOfBand; }
	bool	IsWrongBand()		const { return _isWrongBand; }
	bool	IsRubberClocked()	const { return _isRubberClocked; }
	int		LineNum()			const { return _lineNum;	}
	QString	Line()				const { return _line;		}

private:
	bool	_isExcluded;
	bool	_isOutOfBand;
	bool	_isWrongBand;
	bool	_isRubberClocked;
	int		_lineNum;
	QString _line;
};

class Qso
{
public:
	Qso(Log * log);
virtual ~Qso()  {}

public:
	virtual void	Parse()          = 0;
	virtual void	ParseQsoLine(QsoLineData &);
	virtual bool	CompareExchange(Qso *);
	virtual QString	ToReverseString()		const;
	virtual QString ToFullReverseString()	const;
	virtual QString	ToString()				const;
	virtual QString	ToTabString()			const;
	virtual void	AssignLocationExpected() {}

	QDateTime RawDateTime()		const;
	QDateTime DateTime()		const;
	QString   Band()			const;
	QString   Mode()			const;
	QString   CallSent()		const;
	QString   CallRcvd()		const;
	QString   Cont()			const;
	QString   CorrectCall()		const;
	QString   CorrectXch()		const;
	QString   DateStr()			const;
	QString   PxSent()			const;
	QString   RawCallRcvd()		const;
	QString   PxTested()		const;
	QString   Running()			const;
	QString   RlRunning()		const;
	QString	  RstRcvd()			const;
	QString	  RstSent()			const;
	QString   TimeStr()			const;
	QString   StdPx()			const;
	QString   XchSent()			const;
	QString   XchSent2()		const;
	QString   XchRcvd()			const;
	QString   XchRcvd2()		const;

	QString   RawXchRcvd()		const;

	time_t    TimeInSeconds()	const;
	QSO_STATUS	Status()		const;

	bool	CountsForCredit()	const;
	bool	CountsForRawCredit()const;
	bool	IsBandChange()		const;
	bool	IsBandEdge()		const;
	bool	IsCorrectable()		const;
	bool	IsDupe()			const;
	bool	IsExactQrg()		const;
	bool	IsExcluded()		const;
	bool	IsBadCallByList()	const;
	bool	IsBustedOffByTwo()	const;
	bool	IsOutOfBand()		const;
	bool	IsOutOfTime()		const;
	bool	IsMM()				const;
	bool	IsMult()			const;
	bool	IsRawMult()			const;
	bool	IsMult2()			const;
	bool	IsPortable()		const;
	bool	IsRawMult2()		const;
	bool	IsRepaired()		const;
	bool	IsMult3()			const;
	bool	IsRawMult3()		const;
	bool	IsLegalCall()		const;
	bool	IsValidCty()		const;
	bool	IsUsaPossession()	const;
	bool	IsRcvdCallRootUsaPossession() const;
	bool	IsRubberClocked()	const;
	bool	IsWVe()				const;
	bool	IsWVeRcvd()			const;
	bool	IsLostMult()		const;
	bool	IsLostMult2()		const;
	bool	IsLostMult3()		const;
	bool	IsMultStn()			const;
	bool	IsMyRun()			const;
	bool	IsHisRun()			const;
	bool	IsStnOne()			const;
	bool	IsStnTwo()			const;
	bool	IsUnique()			const;
	bool	IsZ2()				const;

	int		District()			const;
	int		IaruZone()			const;
	int		Zone()				const;
	int		HisComputedQrg()	const;
	int		LineNum()			const;
	uint	LvDistance()		const;
	int		MatchedTimeDiff()	const;
	int		MyComputedQrg()		const;
	int 	Pts()				const;
	int		Qrg()				const;
	int 	Rate20()			const;
	int 	TimeAdjustment()	const;
	int 	TimeToPktSpot()		const;

	Qso			  *	Matched()			const;
	Qso			  * CrossBandMatch()	const;
	Log			  *	GetLogPtr()			const;
	Cty		const *	CtyRcvd()			const;
	Cty		const *	MyCty()				const;
	Spot	const *	SpotHit()			const;
	QChar			StatusChar()		const;
	Px		const *	PxRcvdPtr()			const;
	QString			WpxStr()			const;
	QString			PortDigit()			const;

	QString			ComputeWpxStr(QString call, QString portDigit);

	QSet <QString>			const	BadInfoRcvdSet()	const;	// stored in Contest
	QSet <QString>			const	BadInfoSentSet()	const;	// stored in Contest
	QMap <QString, QString> const	AliasCallMap()		const;
	QMap <QString, QString> const	AliasDistrictMap()	const;
	QMap <QString, QString> const	BadCallMap()		const;
	QSet <QString>			const	GoodCallsSet()		const;
	QSet <QString>			const	PirateCallsSet()	const;
	QSet <QString>			const	IncompleteLogSet()	const;
	QStringList				const	ErrMsgList()		const;
	QStringList				const	WrnMsgList()		const;

	BandEdges				const & BandEdge() const;

	int	MatchWindowSize	()	const;
	int	MultStnFldNum	()	const;
	int	NumberOfFields	()	const;
	int	RcvdCallFldNum	()	const;
	int	RcvdRstFldNum	()	const;
	int	RcvdXchFldNum	()	const;
	int	RcvdXchFldNum2	()	const;
	int	SentXchFldNum	()	const;
	int	SentXchFldNum2	()	const;
	int	SentRstFldNum	()	const;

	QMutex & Lock();

	void	IsBadCallByList		(bool b);
	void	IsBandChange		(bool b);
	void	IsBustedOffByTwo	(bool b);
	void	IsCorrectable		(bool b);
	void	IsDupe				(bool b);
	void	IsEarlyDupe			(bool b);
	void	IsExcluded			(bool b);
	void	IsExactQrg			(bool b);
	void	IsMult				(bool b);
	void	IsRawMult			(bool b);
	void	IsMult2				(bool b);
	void	IsRawMult2			(bool b);
	void	IsMult3				(bool b);
	void	IsRawMult3			(bool b);
	void	IsLostMult			(bool b);
	void	IsLostMult2			(bool b);
	void	IsLostMult3			(bool b);
	void	IsOutOfTime			(bool b);
	void	IsUnique			(bool b);
	void	IsMultStn			(bool b);
	void	IsMyRun				(bool b);
	void	IsHisRun			(bool b);

	void	AdjustTime			(int n);
	void	AppendErrMsgList	(QString msg, QString arg);
	void	HisComputedQrg		(int qrg);
	void	LvDistance			(uint lvd);
	void	MyComputedQrg		(int qrg);
	void	Pts					(int pts);
	void	Rate20				(int rate);
	void	TimeToPktSpot		(int t);
	void	Matched				(Qso * q);
	void	CrossBandMatch		(Qso * q);
	void	Status				(QSO_STATUS status);
	void 	Band				(QString band);
	void	CorrectCall			(QString call);
	void	CorrectXch			(QString xch);
	void	RlRunning			(QString r);
	void	Running				(QString r);
	void	XchSent				(QString xch);
//	void	XchSent2			(QString xch);
	void	SpotHit				(Spot const * sptPtr);

protected:
	bool  _fixFccPort		: 1;
	bool  _isBadCallByList	: 1;
	bool  _isBandChange		: 1;
	bool  _isBandEdge		: 1;
	bool  _isBustedOffByTwo	: 1;
	bool  _isCorrectable	: 1;
	bool  _isCrossBand		: 1;
	bool  _isDupe			: 1;
	bool  _isEarlyDupe		: 1;
	bool  _isExactQrg		: 1;
	bool  _isExcluded		: 1;
	bool  _isHeRunning		: 1;
	bool  _isLegalCall		: 1;
	bool  _isLostMult		: 1;
	bool  _isLostMult2		: 1;
	bool  _isLostMult3		: 1;
	bool  _isMM				: 1;
	bool  _isMult			: 1;
	bool  _isMult2			: 1;
	bool  _isMult3			: 1;
	bool  _isMultStn		: 1;
	bool  _isOutOfBand		: 1;
	bool  _isOutOfTime		: 1;
	bool  _isPortable		: 1;
	bool  _isRawMult		: 1;
	bool  _isRawMult2		: 1;
	bool  _isRawMult3		: 1;
	bool  _isRepaired		: 1;
	bool  _isRubberClocked	: 1;
	bool  _isRun			: 1;
	bool  _isUnique			: 1;
	bool  _isValidCty		: 1;
	bool  _isWrongBand		: 1;

	int  _district;
	int  _hisComputedQrg;
	int  _lineNum;
	uint _lvDistance;
	int  _matchedTimeDiff;
	int  _myComputedQrg;
	int  _pts;
	int  _qrg;
	int  _rate20;
	int  _timeOffset;
	int  _timeToPktSpot;

	QString	_band;
	QString	_callRcvd;
	QString	_computedWpxStr;
	QString	_correctCall;
	QString	_correctXch;
	QString	_date;
	QString	_mode;
	QString	_portDigit;
	QString	_rawCallRcvd;
	QString	_rawXchRcvd;
	QString	_rawXchRcvd2;
	QString	_rlRunning;
	QString	_running;
	QString	_rstRcvd;
	QString	_rstSent;
	QString	_time;
	QString	_wpxStr;
	QString	_xchRcvd;
	QString	_xchRcvd2;
	QString	_xchSent;
	QString	_xchSent2;

	QStringList	_errMsgList;
	
	QDateTime			_dateTime;
	QDateTime			_rawDateTime;
	QSO_STATUS			_status;
	QMutex				_lock;
	Cty		const	*	_ctyRcvdPtr;
	Log				*	_log;
	Px		const	*	_pxRcvd;
	Qso				*	_crossBandMatched;
	Qso				*	_matched;
	Spot	const	*	_spotHit;
	BandEdges			_bandEdge;
};

class LostMult
{
public:
	LostMult(QsoPtr qp, QString multType) : _qPtr(qp), _multType(multType) {}

	QsoPtr			QPtr()		const { return _qPtr; }
	QString	const	MultType()	const { return _multType; }
private:
	LostMult() {}
	QsoPtr	_qPtr;
	QString	_multType;
};

bool gSpotTimeLessThanSpotTime	(Spot		const *sp1,	Spot		const *sp2);
bool gTimeLessThanSpotTime		(QDateTime	const   dt,	Spot		const *sp1);

bool gQsoTimeLessThanQsoTime	(Qso		const * q1,	Qso			const  *q2);
bool gTimeLessThanQsoTime		(QDateTime	const   dt, Qso			const  * q);
bool gQsoTimeLessThanTime		(Qso		const  * q, QDateTime	const   dt);

bool gLostMultLessThanByDateTime(LostMult const &lm1, LostMult const &lm2);

extern QMap <QString, int> callCounts;
#endif // _QSO

