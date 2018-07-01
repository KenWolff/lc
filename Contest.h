#ifndef _CONTEST
#define _CONTEST

typedef QSet		<QString>						CallSet;
typedef QMultiMap	<double, QString>				StringByDoubleMap;   // print out percentages in order
typedef QMultiMap	<int, QString>					StringByIntMap;      // print out counts in order
typedef QMap		<int, int>						ZoneCountMap;
typedef QMap		<QString, ZoneCountMap>			ZoneCountsMappedByCallAndZone;
typedef QMap		<QString, int>					InfoCountMap;
typedef QMap		<QString, int>					CtyCountMap;
typedef QMap		<QString, InfoCountMap>			InfoCountsMappedByCallAndInfo;
typedef std::map	<QString, RevLog>				RevLogMap;
typedef QMap		<QString, QsoList>				QsoListsByMultMap;
typedef QMap		<QString, QsoListsByMultMap>	QsoListsByBandMultMap;

enum ContestTypes { CQWW, CQWW_RY, WPX, WPX_RY, ARDX, RTTY, IARU, VHF_JAN, VHF_JUN, VHF_SEP, OC };
enum { A = 0, H = 1, L = 2, Q = 3 };

BandEdges const & gFreqToBand(double);

class Contest
{
public:
	Contest();
	virtual ~Contest();
	
	bool				CheckFccData()		const;
	int					CountOffsets(int maxOffset);
	int					LogCount()			const;
	int					MultStnFldNum()		const;
	int					NumberOfFields()	const;
	int					MatchWindowSize()	const;
	int					RcvdCallFldNum()	const;
	int					RcvdRstFldNum()		const;
	int					RcvdXchFldNum()		const;
	int					RcvdXchFldNum2()	const;
	int					SentXchFldNum()		const;
	int					SentXchFldNum2()	const;
	int					SentRstFldNum()		const;
	QString		const	AdminText()			const;
	QString		const	Name()				const;
	QString		const	Mode()				const;
	QString		const	MultType()			const;
	QString		const	MultType1()			const;
	QString		const	Title()				const;
	QStringList	const	BandNameList()		const;
	QDateTime	const	StartTime()			const;
	QDateTime	const	EndTime()			const;
	QDateTime	const	CertCutoffTime()	const;
	QMap <QString, Log * >			LogsByCall() const;
	std::map < QString, RevLog > &	RevLogMap();
	QMap < QString, RevLog>	const LargeRlMap();
	QMap < QString, RevLog>	const SmallRlMap();

	Log	*	GetLog(QString call);
	void	RunMultiThreaded(void(&func) (Log *));

	int		WriteRevLogs();
	void    AlignTimes();
	void    AppendErrMsgList(QString msg, QString arg);
	void    CheckLogs();
	void    Compute20MinRates();
	void    ComputeScores();
	void    FillCtyLists();
	void    FillLogsByCallRcvdMap();
	void    FillReportLists();
	void    LoadFccData();
	void    MarkBadCalls();
	void    MarkDupes();
	void    MarkFccBusts();
	void    MarkMultiOpBCV();
	void    MarkRubberClockedQsos();
	void    MarkRunSp();
	void    MarkUniques();
	void    MatchWindowSize(int winSize);
	void    PrintStats() const;
	void    RepairCrossBandQsos();
	void    UpdateCounts();
	void    WriteAllFiles();
	void    WriteCbmFiles();
	void    WriteCllFile();
	void    WriteClubsFile();
	void    WriteCtyFiles();
	void    WriteDupeFiles();
	void    WriteErrFiles();
	void    WriteFccBusts();
	void    WriteLgFiles();
	void    WriteLvdResults();
	void    WriteMultFiles();
	void    WriteObtFile();
	void    WriteOcpFile();
	void    WriteRepairedCalls();
	void    WriteResultFiles();
	void    WriteResultsTxtFile();
	void    WriteRlsFile();
	void    WriteRptFiles();
	void    WriteSprFile();
	void    WriteSprHdr(QTextStream & strm);
	void    WriteSptFiles();
	void    WriteUniqueMults();
	void    WriteUniqueNils();
	void    WriteUniques();
	void    WriteXchCausedFiles();

	QDir		SetupSubdir(QString name);
	QDir const	DupeDir()	const;
	QDir const	LgDir()		const;
	QDir const	MulDir()	const;
	QDir const	RlDir()		const;
	QDir const	RptDir()	const;
	QDir const	SptDir()	const;
	QDir const	TmDir()		const;

	QMap <QString, QString>	const   BadCallMap				()	const;

	CallSet	const					BadInfoSentSet			()	const;
	CallSet const					BadInfoRcvdSet			()	const;
	CallSet const					ExtensionsSet			()	const;
	CallSet const					DoNotCheckSet			()	const;
	CallSet	const					GoodCallsSet			()	const;
	CallSet const					IncompleteLogSet		()	const;
	CallSet const					PirateCallsSet			()	const;

	QMap <QString, QString> const	AliasStateMap			()	const;
	QMap <QString, QString> const	AliasCallMap			()	const;
	QMap <QString, QString> const	AliasDistrictMap		()	const;
	QMap <QString, QString>	const   RevHqCategoryMap		()	const;
	QMap <QString, QList <Log *> > const & LogsByClub		()	const;
	QMap <QString, QString>	const   FillRunMultMap			()	const;
	QSet <QString>			const	RedCardSet				()	const;
	QSet <QString>			const	YellowCardSet			()	const;
	QSet <QString>			const	DisqualifiedSet			()	const;

	QMap <QString, QSet <QString> > const LogSetByCallRcvd	()	const;
	QMap <QString, QMap < QDateTime, QMap <int, Qso *> > >	AllByCallTimeQrgMap();
	QMap <QString, QMap <QString, std::vector <Spot *> > >	SpotsByBandCallTimeMap();
	
	QString ContestType();
	static	Contest * CreateContest();
	virtual void	MarkCertificateWinners()		= 0;
	virtual void	WriteTabHeader(QTextStream &)	= 0;
	virtual void	WriteTxtHeader(QTextStream &)	= 0;

	virtual void	AcquireLogs();
	virtual void	MarkNilBust();
	virtual void	DeriveExchInfo() {};
	virtual	void	FillRunMultMap(QString call, QString data);
	virtual void	Init() {}
	virtual void	WriteAllFile() {}
	virtual void	WriteMagazineFile() {}
	virtual void	WriteOpsFile();
	virtual void	WriteRedAndYellowCards(QTextStream & strm);
	virtual	void	WriteHqCountsFile();
	virtual void	WriteSbxFile();
	virtual void	WriteTabFile();
	virtual void	WriteTsvFile();
protected:
	QString			_adminText;
	QString			_contestType;
	QString			_mode;
	QString			_name;
	QString			_title;
	int				_bustedCallCount;
	int				_bustedRevLogCallCount;
	int				_bustedXchCount;
	int				_checkableCount;
	int				_isCrossBandCount;
	int				_dupeCount;
	int				_exactFreqCount;
	int				_formatErrCount;
	int				_goodMatchCount;
	int				_logCount;
	int				_matchWindowSize;
	int				_multStnFldNum;
	int				_nonCheckableCount;
	int				_notInLogCount;
	int				_numberOfFields;
	size_t			_qsoTotalCount;
	int				_rcvdCallFldNum;
	int				_rcvdRstFldNum;
	int				_rcvdXchFldNum;
	int				_rcvdXchFldNum2;
	int				_sentRstFldNum;
	int				_sentXchFldNum;
	int				_sentXchFldNum2;
	int64_t			_totalScore;
	int				_uniquesNotCounted;
	int				_uniquesCount;
	QStringList		_bandNameList;
	QStringList		_errMsgList; 
	CallSet			_administrativeChecklogSet; // set for KR2Q
	CallSet			_doNotCheckSet;				// set of logs we will not check
	CallSet			_incompleteLogSet;			// list of causers of NIL
	CallSet			_badInfoRcvdCallSet;		// list of calls with bad info rcvd
	CallSet			_badInfoSentCallSet;		// list of causers of XCH
	CallSet			_goodCallsSet;				// set of known good calls
	CallSet			_noAwardSet;				// set of logs that can't recv award
	CallSet			_pirateCalls;				// set of known pirate calls

	StringByDoubleMap				_crossBandByPercentage;
	StringByIntMap					_crossBandByCount;
	StringByDoubleMap				_nilCausedByPercentage;
	StringByIntMap					_fccByCount;
	QMap <QString, QSet <QString> > _logByCallRcvdMap;

	QMap <QString,	QString>		_aliasStateMap;
	QMap <QString,	QString>		_fillRunMultMap;			// compute run/mult: YES or NO
	QMap <QString,	QString>		_aliasCallMap;
	QMap <QString,	QString>		_aliasDistrictMap;
	QMap <QString, QString>			_badCallsMap;				// map of known bad calls and correct call
	QMap <QString, int>				_badFccCallCounts;
	QMap <int, QString>				_badFccCallsMap;			// set of calls not in fcc data
	QMap <QString,	QString>		_bandNames;
	QMap <QString,	QList<QsoPtr> >	_qsoListsByStdPx;
	QMap <QString,	QString>		_hqCatMap;
	QMap <QString,	QString>		_revHqCategoryMap;
	QMap <QString,	QString>		_hqCategoryToSmMap;
	QMap <QString,	QString>		_hqCategoryToPwrMap;
	QMap <QString,	QString>		_hqCategoryToBandMap;
	QMap <QString, RevLog>			_largeRlMap;
	QMap <QString, RevLog>			_smallRlMap;
	QMap <QString,	QList<Log *> >	_logsByClub;
	std::map <QString,	RevLog>		_revLogMap;
	QMap <QString,	int>			_rlbIniMap;
	QMap <QString,	int >			_scoresByClub;
	QMap <QString,	int >			_callCounts;
	QSet <Cty const *>				_ctysByDistrictSet;
	QDir							_bustDir;
	QDir							_cbmDir;
	QDir							_dupeDir;
	QDir							_fccDir;
	QDir							_lgDir;
	QDir							_logsDir;
	QDir							_mulDir;
	QDir							_nilDir;
	QDir							_resDir;
	QDir							_rlDir;
	QDir							_rptDir;
	QDir							_sptDir;
	QDir							_tmDir;
	QDir							_uniDir;
	QDir							_xchDir;
	QDateTime						_startTime;
	QDateTime						_endTime;
	QDateTime						_certCutoffTime;
	QFile							_errFile;
	QFile							_wrnFile;
	QMap <QString, QMap < QDateTime, QMap <int, Qso *> > > _allByCallTimeQrgMap;
	QMap <QString, QMap <QString, std::vector <Spot *> > > _spotsByBandCallTimeMap;
	QMap <QString, QMap <QString, QMap <QDateTime, BustedRbnSpot *> > > _bustedRbnSpotsByBandCallTime;
	QMap <QString, Log *>			_logsByCall;
	QMutex							_hqCountMutex;
	QSet <QString>					_disqualifiedSet;
	QSet <QString>					_extensionsSet;		// extensions for late logs
	QSet <QString>					_redCardSet;
	QSet <QString>					_yellowCardSet;
};

typedef Contest * ContestPtr;
#endif // _CONTEST
