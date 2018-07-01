#ifndef _LOG
#define _LOG

typedef QMap	< QString, QString * >					CabrilloTokenMap;
typedef QPair	< QString, QString >					TokenDataPair;
typedef QList	< TokenDataPair >						TokenDataPairList;

typedef QMap	< QString, QsoList >					QsoListByBandMap;
typedef QMap	< QString, QMap < QString, QsoList>	>	QsoListByBandCall;
typedef QMap	< QString, QMap < QString, QsoList>	>	QsoListByCallBand;
typedef QMap	< QString, QsoList>						QsoListsByCtyRcvdMap;
typedef QMap	< QString, FirstQsoByCallMap>			FirstQsoByMultCallMap;
typedef QMap	< QString, FirstQsoByMultCallMap>		FirstQsoByBandMultCallMap;
typedef QMap	< QString, QsoListsByCtyRcvdMap>		QsoListsByBandCtyRcvdMap;
typedef QMultiMap < QDateTime, QsoPtr >					QsoMapByTime;

class Log
{
public:
	Log();
    virtual  ~Log();

    QsoList				GetQsoListByCallBand(QString call, QString band) const;
    QsoListByBandMap	const GetQsoListByBandMap()		const;
    Contest const *	ContestPtr()				const;
	Cty		const *	CtyPtr()					const;
    Px		const *	PxPtr()                		const;
    QString	const	Address()                 	const;
    QString const	AddressCity()				const;
    QString const	AddressStateProvince()		const;
    QString const	AddressPostalCode()			const;
    QString const	AddressCountry()        	const;
    QString const	AdminText()					const;
    QString const	Category()                	const;
    QString const	Call()                    	const;
    QString const	CategoryAssisted()        	const;
    QString const	CategoryBand()            	const;
    QString const	CategoryMode()            	const;
    QString const	CategoryOperator()        	const;
    QString const	CategoryOverlay()         	const;
    QString const	CategoryPower()           	const;
    QString const	CategoryStation()         	const;
    QString const	CategoryTime()            	const;
    QString const	CategoryTransmitter()     	const;
    QString const	ClaimedScore()            	const;
    QString const	Club()                    	const;
    QString const	Cont()                    	const;
    QString const	CreatedBy()					const;
    QString const	CtyName()                 	const;
    QString const	StdPx()                 	const;
    QString const	District()                 	const;
    QString const	ExchSent()                	const;
    QString const	GuestOp()                 	const;
    QString const	HostCall()                 	const;
    QString const	HqFrom()                  	const;
    QString const	HqDateRcvd()              	const;
    QString const	Location()                	const;
    QString const	Mode()						const;
    QString const	Name()                    	const;
    QString	const	PortDigit()					const;
    QString const	Operators()               	const;
    QString const	RawCall()                 	const;
    QString const	SoapBox()                 	const;
    QString const	TypeStr()					const;
    QString const	CategoryString()			const;

    QStringList	OpsList()						const;
    QStringList	BigTimeOffsetList()				const;
    QFileInfo 	FileInfo()                		const;
	QDateTime	LastQsoTime()					const;
	bool      	DoNotCheck()             		const;
    bool      	Certificate()             		const;
    bool      	OverlayCertificate()      		const;
    bool      	DontBustCall()            		const;
    bool      	DontBustNil()             		const;
    bool      	DontBustXch()             		const;
    bool	  	ExactQrgByBand(QString band)	const;
    bool      	DontFillRunMult()				const;
    bool      	FillRunMult()             		const;
    bool		HasRedCard()					const;
    bool		HasYellowCard()					const;
	bool		Disqualified()					const;
	bool		IsAdministrativeChecklog()		const;
	bool		IsChecklog()					const;
	bool		IsIncomplete()					const;
	bool      	IsAssisted()           			const;
	bool      	IsBadSentInfo()           		const;
	bool      	IsBadRcvdInfo()           		const;
	bool		IsCertEligible()				const;
	bool		IsFinalTm()						const;
	bool      	IsMultiSingle()           		const;
    bool      	IsMultiTwo()              		const;
	bool      	IsMultiMulti()              	const;
	bool      	IsNoAward()		             	const;
	bool      	IsPortable()	              	const;
    bool		IsSingleOp()					const;
    bool		IsMarMobile()					const;
    bool      	IsWVe()                   		const;
    bool      	IsW()                   		const;
    bool      	IsInFccData()                   const;
    bool      	PaperLog()                		const;
    bool      	TimeNormalized()          		const;
    bool      	SubmittedLate()          		const;
    bool		IsOverlayClassic()				const;
    bool		IsOverlayRookie()				const;
    bool		IsOverlay()						const;

    int			OverlayNetQso	(QString band)	const;
    int			OverlayNetPts	(QString band)	const;
    int			OverlayNetMults	(QString band)	const;
    int			OverlayNetMults2(QString band)	const;
    int			OverlayNetMults3(QString band)	const;
                                                     ;
    int			OverlayNetQso()					const;
    int			OverlayNetPts()					const;
    int			OverlayNetMults()				const;
    int			OverlayNetMults2()				const;
    int			OverlayNetMults3()				const;
    int			CategoryNum()					const;
    int			Score()                   		const;
    int			RawScore()                		const;
	size_t		LogSize()						const;
    int			OverlayScore()					const;

    int			NetQso()                  		const;
    int			NetPts()                  		const;
    int			NetMults()                		const;
    int			NetMults2()               		const;
    int			NetMults3()               		const;

    int			RawQso()						const;
    int			RawPts()						const;
    int			RawMults()						const;
    int			RawMults2()						const;
    int			RawMults3()						const;

    int			NetQso		(QString band)		const;
    int			NetPts		(QString band)		const;
    int			NetMults	(QString band)		const;
    int			NetMults2	(QString band)		const;
    int			NetMults3	(QString band)		const;

    int			RawQso		(QString band)		const;
    int			RawPts		(QString band)		const;
    int			RawMults	(QString band)		const;
    int			RawMults2	(QString band)		const;
    int			RawMults3	(QString band)		const;

    int			BandNum()               		const;
    int			MagOrder()						const;
    int			PwrNum()               			const;

    size_t		QsoCount()						const;
    int			RunCount()						const;
    int			PassCount()						const;
    int			UnknownCount()					const;
    int			MultCount()						const;
    int			HitCount()						const;
    int			MultHitCount()					const;
    int			SnpCount()						const;
    int			SnpHitCount()					const;
    int			SnpMultCount()					const;
    int			SnpMultHitCount()				const;

    int			QsoCountDay1()					const;
    int			RunCountDay1()					const;
    int		  	PassCountDay1()					const;
    int		  	UnknownCountDay1()				const;
    int		  	MultCountDay1()					const;
    int		  	HitCountDay1()					const;
    int		  	MultHitCountDay1()				const;
    int		  	SnpCountDay1()					const;
    int		  	SnpHitCountDay1()				const;
    int		  	SnpMultCountDay1()				const;
    int		  	SnpMultHitCountDay1()			const;

    int       	BandChangeViolationCount()		const;
    int			QsoCountDay2()					const;
    int			RunCountDay2()					const;
    int		  	PassCountDay2()					const;
    int		  	UnknownCountDay2()				const;
    int		  	MultCountDay2()					const;
    int		  	HitCountDay2()					const;
    int		  	MultHitCountDay2()				const;
    int		  	SnpCountDay2()					const;
    int		  	SnpHitCountDay2()				const;
    int		  	SnpMultCountDay2()				const;
    int		  	SnpMultHitCountDay2()			const;
    int		  	CheckableCount()				const;
    int       	GoodCount()               		const;
    int       	BustedCallCount()         		const;
    int       	BadXchCount()             		const;
    int       	NilCount()                		const;
    int       	BustedCallCausedCount()   		const;
    int       	XchCausedCount()          		const;
    int       	NilCausedCount()          		const;
    int       	UniCount()                		const;
    int       	DupeCount()						const;
    int       	TimeViolationCount()			const;
    int			WorldFinish()					const;
    int			EuFinish()						const;
    int			UsaFinish()						const;

    double		PercentCheckedGood()			const;
	double		PercentBust()					const;
	double		PercentNil()					const;
	double		PercentXch()					const;
	double		PercentBustCaused()				const;
	double		PercentNilCaused()				const;
	double		PercentXchCaused()				const;
	double		PercentCrossBand()				const;
    double		OperatingTime()					const;
    double		Reduction()						const;
    double		ErrorRate()						const;
    time_t		TimeAdjustment()				const;

	void	IsAdministrativeChecklog(bool b);
	void	IsFinalTm			(bool b);
	void	IsNoAward			(bool b);
	void	DontFillRunMult		(bool b);
    void	FillRunMult			(bool fill);
    void	IsIncomplete		(bool b);
    void	CategoryNum			(int num);
    void	FinalScore			(int final);
    void	AppendErrMsgList	(QString msg, QString arg);
    void	District			(QString d);
	void	Score				(int score);
	void	Call				(QString s);
    void	Category			(QString cat);
    void	CategoryAssisted	(QString s);
    void	CategoryBand		(QString s);
    void	CategoryMode		(QString s);
    void	CategoryOperator	(QString s);
    void	CategoryPower		(QString s);
    void	CategoryStation		(QString s);
    void	CategoryTime		(QString s);
    void	CategoryTransmitter	(QString s);
    void	CategoryOverlay		(QString s);
    void	ClaimedScore		(QString claimed);
    void	Cont				(QString cont);
    void	Club				(QString club);
    void	ExchSent			(QString sent);
    void	GuestOp				(QString op);
    void	Location			(QString loc);
    void	Mode				(QString mode);
    void	Operators			(QString ops);
    void	PortDigit			(QString digit);
    void	RawCall				(QString call);
    void	FileInfo			(QFileInfo info);
    void	PxPtr				(Px const * pxPtr);
    void	Certificate			(bool b);
    void	DoNotCheck			(bool b);
    void	DontBustCall		(bool b);
    void	DontBustXch			(bool b);
    void	DontBustNil			(bool b);
    void	IsBadRcvdInfo		(bool b);
	void	IsBadSentInfo		(bool b);
	void	IsAssisted			(bool b);
    void	SetRedCard			(bool b);
    void	SetYellowCard		(bool b);
    void	SetMarMobile		(bool b);
    void	Disqualified		(bool b);
    void	OverlayCertificate	(bool b);
    void	TimeNormalized		(bool b);
    void	SubmittedLate		(bool b);

    void	NetQso				(int n);
    void	NetMults			(int n);
    void	NetMults2			(int n);
    void	SetNetQso			(int n);
    void	SetNetMults			(int n);
    void	SetNetMults2		(int n);
    void	SetNetMults3		(int n);
	void	SetCty				(Cty const * cty);
	void	TimeAdjustment		(time_t t);

    void	WorldFinish			(int fin);
    void	EuFinish			(int fin);
    void	UsaFinish			(int fin);

    void	InsertExactQrgByBand(QString band, bool exact);
    void	FillBandLists();
    void	FillReportLists();
    void	InitCabrilloMap();
    void	LoadCabrilloFile(TokenDataPairList tokenDataPairMap);
    void	MarkRunSp();
	void	ClearBandInfoMap();
	void	SortLostMultList();
	void	SortAllQso();
    void	AppendLargeTimeOffsetList		(QString s);

    void	InsertAllQsoByTime				(QsoPtr q);
    void	AppendBustedCallList			(QsoPtr q);
    void	AppendBcvList					(QsoPtr q);
    void	AppendBadRevLogList				(QsoPtr q);
    void	AppendBustCausedList			(QsoPtr q);
    void	AppendBustedFccCallList			(QsoPtr q);
    void	AppendCrossBandList				(QsoPtr q);
    void	AppendDupeList					(QsoPtr q);
    void	AppendGoodList					(QsoPtr q);
    void	AppendInvalidCallList			(QsoPtr q);
    void	InsertLogNoMatchSet				(QsoPtr q);
    void	AppendLostMultList				(QsoPtr q);
    void	AppendNilCausedList				(QsoPtr q);
    void	AppendNilList					(QsoPtr q);
    void	AppendNotCheckedList			(QsoPtr q);
	void	AppendOutOfBandList				(QsoPtr q);
	void	AppendOutOfTimeList				(QsoPtr q);
	void	AppendRubberClockedList			(QsoPtr q);
	void	AppendUniList					(QsoPtr q);
    void	AppendBustedXchList				(QsoPtr q);
	void	AppendBustedXchCausedList		(QsoPtr q);

	QsoList	const   BustedCallList()			const;
    QsoList	const	BustCausedList()			const;
    QsoList	const   BandChangeViolationList()	const;
    QsoList	const   BadRevLogList()				const;
    QsoList	const	BustedFccCallList()			const;
    QsoList	const	BustedXchCausedList()		const;
    QsoList	const   BustedXchList()				const;
    QsoList	const   CrossBandList()				const;
    QsoList	const   DupeList()					const;
    QsoList	const   GoodList()					const;
	QsoList	const	NilCausedList()				const;
	QsoList	const   NilList()					const;
    QsoList	const	NoLogList()					const;
    QsoList	const   NotCheckedList()			const;
    QsoList	const   OutOfTimeList ()			const;
	QsoList	const   RubberClockedByBandMap()	const;
	QsoList	const   UniList()					const;

    QStringList		BandNameList()				const;
    QStringList		ErrorMsgList()				const;
    QStringList		WarnLineList()				const;

	QMap<QDateTime, Qso *>	const	NilCausedMap()	const;
    QList <LostMult>		const	LostMultList()	const;
    std::vector<Qso *>		const &	AllQso()		const;

    QsoPtr	FindClosestQso(QDateTime dt, QsoList list, int window = INT_MAX);

    virtual QsoPtr	NewQso()								= 0;
    virtual QString	ResultLine()							= 0;

    virtual	void	ClearMultMaps()							= 0;
    virtual void	ComputeCategory()						= 0;
    virtual void	ComputeMults()							= 0;
    virtual void	ComputePts()							= 0;
    virtual void	CheckBandChanges()						= 0;

    virtual	void	WriteLg			(QTextStream &)			= 0;
    virtual void	WriteMultFile	(QTextStream &)			= 0;
    virtual void	WriteRptSummary	(QTextStream &)			= 0;
    virtual void	WriteRptMults	(QTextStream &)			= 0;
	virtual void	WriteTabLine	(QTextStream &)			= 0;
	virtual void	WriteTsvLine(QTextStream &) {};

	virtual void	InitByContestType() {}

    virtual void	AddToQsoLists				(QsoPtr q);
    virtual void	CheckDupes					();
    virtual void	CheckM2BandChanges			(int const count);
    virtual void	CheckMSBandChanges			(int num);			// band change limit per hour
    virtual void	ComputeOperatingTime		();
	virtual void	ComputeScore				();
    virtual void	FillM2BandChanges			(int maxQsy);
	virtual void	FillPaperLogFromParts		(QStringList) {}
	virtual	void	WriteSprLine				(QTextStream & strm);
	virtual void	WriteMultUniqueToMe			(QTextStream &) {};

    virtual QsoPtr	Find						(QString call, QsoPtr q, int window, bool chkOtherBands = false);
    virtual QsoPtr	Find						(QString call, QDateTime dt, QString band);
    virtual QsoList GetQsoListByCall			(QString call, QsoPtr q);
    virtual QString	MakeGuestOpStr				();
    virtual	QsoListsByBandCtyRcvdMap  const GetMultMap() const { return _multMap; }	// for KR2Q

    QMap < QString, QMap	<QString, QsoList> > GetQsoListByCallBandMap() { return _qsoListByCallBandMap; }
	bool			AdjustTimeOffsets();
	void			CheckBusts();
	void			CheckXchs();
	void			MarkMultiOpBCV();
	void			MarkUniques();
	void			WriteSptFiles();
	void			FindBustsInRl();
	void			FillTwentyMinRate();
	void 			MatchQsos();
	void			ReportTimeMatching();
	void			WriteLgFile();
	void			WriteDupeFile();
	void			WriteMultFile();
	void			WriteRptFile(QTextStream & strm);
	void			WriteRptFile();

	static	Log *	CreateLog(QFileInfo);
	static	void	WriteDupeFiles		(Log * log) { log->WriteDupeFile();				}
	static	void	MatchQsos			(Log * log) { log->MatchQsos();					}
	static	bool	AdjustTimes			(Log * log) { return log->AdjustTimeOffsets();	}
	static	void	CheckBusts			(Log * log) { log->CheckBusts();				}
	static	void	CheckDupes			(Log * log) { log->CheckDupes();				}
	static	void	CheckBandChanges	(Log * log) { log->CheckBandChanges();			}
	static	void	CheckXchs			(Log * log) { log->CheckXchs();					}
	static	void	MarkUniques			(Log * log) { log->MarkUniques();				}
	static	void	ComputeScore		(Log * log) { log->ComputeScore();				}
	static	void	ComputeSpotInfo		(Log * log) { log->WriteSptFiles();				}
	static	void	FillTwentyMinRates	(Log * log) { log->FillTwentyMinRate();			}
	static	void	ReportTimeMatching	(Log * log) { log->ReportTimeMatching();		}
	static	void	WriteLgFiles		(Log * log) { log->WriteLgFile();				}
	static	void	WriteMultFiles		(Log * log) { log->WriteMultFile();				}
	static	void	WriteRptFiles		(Log * log) { log->WriteRptFile();				}

	QMutex &		Lock();
	virtual QList < QPair <QsoList, QsoList> >	GetQsoLists();

protected:
	bool	_assisted;
	bool	_categoryOverlayClassic;
	bool	_categoryOverlayRookie;
	bool	_certificate;
	bool	_disqualified;
	bool	_doNotCheck;
	bool	_dontBustCall;
	bool	_dontBustNil;
	bool	_dontBustXch;
	bool	_dontFillRunMult;
	bool	_fillRunMult;
	bool	_incomplete;
	bool	_isAdministrativeChecklog;
	bool	_isBadRcvdInfo;
	bool	_isBadSentInfo;
	bool	_isChecklog;
	bool	_isFinalTm;     // hack to get write gReportTimeMatching to write .tm.initial or .tm.final
	bool	_isMultiMulti;
	bool	_isMultiSingle;
	bool	_isMultiTwo;
	bool	_isNoAward;
	bool	_isPortable;
	bool	_isRover;
	bool	_isSingleOp;
	bool	_mailCertFlag;
	bool	_marMobile;
	bool	_overlayCertificate;
	bool	_redCard;
	bool	_submittedLate;
	bool	_timeNormalized;
	bool	_yellowCard;
    int     _categoryNum;
    int     _checkableCount;
    int     _hitCount;
    int     _hitCountDay1;
    int     _hitCountDay2;
    int     _iaruZone;
    int     _logId;
    int     _maxOperatingTime;
    int     _multCount;
    int     _multCountDay1;
    int     _multCountDay2;
    int     _multHitCount;
    int     _multHitCountDay1;
    int     _multHitCountDay2;
    int     _offtimeThreshold;
	time_t	_operatingTime;
    int     _overlayScore;
    int     _passCount;
    int     _passCountDay1;
    int     _passCountDay2;
    int     _qsoCount;
    int     _qsoCountDay1;
    int     _qsoCountDay2;
	int     _rawOverlayScore;
	int     _rawScore;
	int     _runCount;
    int     _runCountDay1;
    int     _runCountDay2;
    int     _score;
    int     _snpCount;
    int     _snpCountDay1;
    int     _snpCountDay2;
    int     _snpHitCount;
    int     _snpHitCountDay1;
    int     _snpHitCountDay2;
    int     _snpMultCount;
    int     _snpMultCountDay1;
    int     _snpMultCountDay2;
    int     _snpMultHitCount;
    int     _snpMultHitCountDay1;
    int     _snpMultHitCountDay2;
    int     _timeOffset;
    int     _unknownCount;
    int     _unknownCountDay1;
    int     _unknownCountDay2;
    int     _euFinish;
    int     _usaFinish;
    int     _worldFinish;
    double	_lat;
    double	_lon;
    QString	_addressCity;
    QString	_addressCountry;
    QString	_addressPostalCode;
    QString	_addressStateProvince;
    QString	_addressStreet;
    QString	_call;
    QString	_category;
    QString	_categoryAssisted;
    QString	_categoryBand;
    QString	_categoryMode;
    QString	_categoryOperator;
    QString	_categoryOverlay;
    QString	_categoryPower;
    QString	_categoryStation;
    QString	_categoryTime;
    QString	_categoryTransmitter;
    QString	_claimedScore;
    QString	_club;
    QString	_clubCode;
    QString	_cont;
	QString	_contestName;
	QString	_cqZone;
	QString	_createdBy;
    QString	_district;
    QString	_email;
    QString	_exchSent;
    QString	_guestOp;
    QString	_hostCall;
    QString	_hqCategory;
    QString	_hqCat;
    QString	_hqDateRcvd;
    QString _hqClubAbbr;
    QString	_hqFile;
    QString	_hqFrom;
    QString	_hqSubject;
    QString	_location;
    QString	_mail;
	QString	_mailCert;
	QString	_mode;
    QString	_name;
    QString	_operators;
    QString	_portDigit;
    QString	_rawCall;
    QString	_soapBox;
    QString	_startOfLog;
    QString	_tmpLine;

    QMap <QDateTime, QsoPtr> _badRevLogByTimeMap;
    QMap <QDateTime, QsoPtr> _bcvByTimeMap;
    QMap <QDateTime, QsoPtr> _bustedCallByTimeMap;
    QMap <QDateTime, QsoPtr> _bustedCallCausedByTimeMap;
    QMap <QDateTime, QsoPtr> _bustedFccCallByTimeMap;
    QMap <QDateTime, QsoPtr> _bustedXchByTimeMap;
    QMap <QDateTime, QsoPtr> _bustedXchCausedByTimeMap;
    QMap <QDateTime, QsoPtr> _crossBandByTimeMap;
    QMap <QDateTime, QsoPtr> _dupeByTimeMap;
    QMap <QDateTime, QsoPtr> _goodQsoByTimeMap;
    QMap <QDateTime, QsoPtr> _invalidCallList;
    QMap <QDateTime, QsoPtr> _invalidCtyList;
    QMap <QDateTime, QsoPtr> _lostMult2List;
	QMap <QDateTime, QsoPtr> _lostMult3List;
	QMap <QDateTime, QsoPtr> _nilByTimeMap;
    QMap <QDateTime, QsoPtr> _nilCausedByTimeMap;
    QMap <QDateTime, QsoPtr> _noLogByTimeMap;
    QMap <QDateTime, QsoPtr> _notCheckedByTimeMap;
	QMap <QDateTime, QsoPtr> _outOfBandByTimeMap;
	QMap <QDateTime, QsoPtr> _outOfTimeByTimeMap;
	QMap <QDateTime, QsoPtr> _rubberClockedByBandMap;
	QMap <QDateTime, QsoPtr> _uniByTimeMap;

    QsoList					_logNoMatchList;
	QList <LostMult>		_lostMultList;
    QStringList				_errMsgList;
    QStringList				_opsList;
    QStringList				_largeTimeOffsetList;
    QMutex					_lock;
    Contest	const	*		_contest;
    Cty		const	*		_cty;
    Px		const	*		_px;
	QDateTime				_finalQsoDateTime;

    BandInfoMap				_bandInfoMap;
    BandInfoMap				_overlayBandInfoMap;
    CabrilloTokenMap		_cabrilloTokenMap;
    QFileInfo				_fileInfo;

    QMap <QString, bool>							_exactQrgByBand;
    QMap <int, int>									_txMultCountByDeltaRlTime0;
    QMap <int, int>									_txMultCountByDeltaRlTime1;
    QMap <int, int>									_txNonMultCountByDeltaRlTime0;
    QMap <int, int>									_txNonMultCountByDeltaRlTime1;
    QMap < QString, QList	<QsoPtr> >				_qsoListByBand;
    QMap < QString, QMap	<QString, QsoList> >	_qsoListByCallBandMap;
    std::vector<Qso *>								_allQso;
	static std::map <QString, QString>				_districtByStateMap;

    QsoListsByBandCtyRcvdMap   						_multMap;
    QsoListsByBandCtyRcvdMap   						_rawMultMap;

    QMap < QString, QMap <int, QsoList > >  		_zoneMultMap;
    QMap < QString, QMap <int, QsoList > >			_rawZoneMultMap;

    QMap < QString, QMap <QString, QsoList > >		_overlayCtyMultMap;
    QMap < QString, QMap <QString, QsoList > >		_rawOverlayMultMap;

    QMap < QString, QMap <int, QsoList > >			_overlayZoneMultMap;
    QMap < QString, QMap <int, QsoList > >			_rawOverlayZoneMultMap;

    QMap < QString, QMap <QString, QsoList > >		_overlayStateMultMap;
    QMap < QString, QMap <QString, QsoList > >		_rawOverlayStateMultMap;
};

class RevLog
{
public:
    RevLog() {}
    QString Call()			const;
    QString CorrectCall()	const			{ return _correctCall;  }
    QString CorrectXch()	const			{ return _correctInfo;  }

    void CorrectCall(QString correctCall)	{ _correctCall = correctCall;   }
    void CorrectXch(QString correctInfo)	{ _correctInfo = correctInfo;   }
	void Write();

	std::vector <Qso *>	& AllQso()			{ return _allQso; }
	static	void WriteRevLogs(RevLog * rl)	{ rl->Write(); }
	static	void SortRevLogs(RevLog* rl)	{ std::stable_sort(rl->AllQso().begin(), rl->AllQso().end(), gQsoTimeLessThanQsoTime); }
protected:
    QString				_correctCall;
    QString				_correctInfo;
	std::vector <Qso *>	_allQso;
};

struct TimeMatchData
{
    QStringList	_bigOffetStrings;
    QString		_summaryString;
};

typedef QList <Log *>  LogList;


#endif // _LOG