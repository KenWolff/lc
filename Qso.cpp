#include "StdAfx.h"

QMap <QString, int> callCounts;

// call is passed in without the /<digit>

QString Qso::ComputeWpxStr(QString wpxString, QString portDigit)
{
	if (wpxString.contains('/'))
	{
		QStringList  parts = wpxString.split('/');
		foreach(QString part, parts)			// FM/K5ZD or FM/K5ZD/FS take first
			if (part.size() < wpxString.size())
				wpxString = part;
	}

	QRegularExpression re("[A-Z](\\d+)");
	QRegularExpressionMatch m = re.match(wpxString);
	if (m.hasMatch())
	{
		wpxString.truncate(m.capturedEnd(1));	// remove the suffix
		if (!portDigit.isEmpty())				// replace number
		{
			wpxString.chop(1);
			wpxString.append(_portDigit);
		}
	}
	else
	{
		if (wpxString.size() > 2)				// all letter calls use first two letters
			wpxString.truncate(2);

		wpxString.append("0");					// append zero
	}

	if (_fixFccPort && !portDigit.isEmpty())
		wpxString = "K" + portDigit;

	return wpxString;
}

bool Qso::CompareExchange(QsoPtr rq)
{
	if (rq->XchSent() != _xchRcvd)
	{
		CorrectXch(rq->XchSent());
		return false;
	}
	return true;
}

void Qso::ParseQsoLine(QsoLineData & lineData)
{
	QString initialLine(lineData.Line());
	_lineNum			= lineData.LineNum();
	_isExcluded			= lineData.IsExcluded();
	_isRubberClocked	= lineData.IsRubberClocked();
	_isWrongBand		= lineData.IsWrongBand();
	_isOutOfBand		= lineData.IsOutOfBand();

	QStringList segs = lineData.Line().split(' ');		// QString::simplified() was called earlier when line was tokenized

	if(segs.size() < NumberOfFields())
	{
		_status   = FORMAT_ERR;
		AppendErrMsgList("FIELD CNT:", QString::number(segs.size()));
		return;
	}

	_band	= segs[0];
	_mode	= segs[1];
	_date	= segs[2];
	_time	= segs[3];

	// this code breaks VHF because "1.2G" has a decimal point
	if (!gContest->ContestType().startsWith("VHF") && !gContest->ContestType().startsWith("UHF"))
	{
		int idx = _band.indexOf('.');			// some loggers send us a decimal point, which we ignore
		if (idx != -1)
			_band = _band.left(idx);
	}

	if(segs.size() > SentRstFldNum())
		_rstSent	= segs[SentRstFldNum()];

	if(segs.size() > SentXchFldNum())
		_xchSent	= segs[SentXchFldNum()];

	if(segs.size() > SentXchFldNum2())
		_xchSent2   = segs[SentXchFldNum2()];

	if(segs.size() > RcvdCallFldNum())
		_rawCallRcvd= segs[RcvdCallFldNum()];

	if(segs.size() > RcvdRstFldNum())
		_rstRcvd	= segs[RcvdRstFldNum()];

	if(segs.size() > RcvdXchFldNum())
		_rawXchRcvd = segs[RcvdXchFldNum()];

	if(segs.size() > RcvdXchFldNum2())
		_rawXchRcvd2  = segs[RcvdXchFldNum2()];

	if(segs.size() > MultStnFldNum())
		_isMultStn = (segs[MultStnFldNum()] == "1");

	_rawCallRcvd = _rawCallRcvd.toUpper();
	_rawCallRcvd.replace('-', '/');
	
	_rawCallRcvd.remove(QRegularExpression("[^A-Z0-9/]"));

	_callRcvd = _rawCallRcvd;
	auto callRcvdStd = _callRcvd.toStdString();

	bool badCallFlag	= BadCallMap().contains(_rawCallRcvd);
	bool aliasCallFlag	= AliasCallMap().contains(_rawCallRcvd);
	bool goodCallFlag	= GoodCallsSet().contains(_rawCallRcvd);

	if (badCallFlag)
	{
		_isRepaired = false;
		_isBadCallByList = true;
		if (BadCallMap().value(_rawCallRcvd).size())
			_callRcvd = _correctCall = BadCallMap().value(_rawCallRcvd);
		_status = BUSTED_LIST_CALL;
	}

	if (aliasCallFlag)
	{
		_callRcvd = AliasCallMap().value(_rawCallRcvd);
		_isRepaired = false;
	}

	if (goodCallFlag)
	{
		_callRcvd = _rawCallRcvd;
		_isRepaired = false;
	}

	_isCorrectable = !(badCallFlag && aliasCallFlag && goodCallFlag && !gSpt && !gRbn && gFixTypos);

	QRegularExpressionMatch m;

	if(_callRcvd.endsWith("/MM"))
		_isMM = true;

	QRegularExpression re;

	if (_isCorrectable)
	{
		if (_callRcvd.size() > 5 && (_callRcvd.size() % 2 == 0))	// find double calls 'K5ZDK5ZD'
		{
			QString left = _callRcvd.left(_callRcvd.size() / 2);
			QString right = _callRcvd.right(_callRcvd.size() / 2);
			if (left == right)
				_callRcvd = left;
		}

		_callRcvd.remove(QRegularExpression(EndsPortableMultipleDigitPat));

		re.setPattern(MultipleSlashesPat);
		m = re.match(_callRcvd);
		if (m.hasMatch())
		{
			_callRcvd.replace(re, "/");		// compress multiple slashes
			_isRepaired = true;
		}

		_callRcvd.remove(QRegularExpression(StartAndEndNonsensePat));
		_callRcvd.replace(QRegularExpression(MiddleNonsensePat), "/");

		re.setPattern(StartsIllegalLetterZeroPat);
		m = re.match(_callRcvd);
		if (m.hasMatch())
		{
			_callRcvd[m.capturedStart(1)] = 'O';
			_isRepaired = true;
		}

		re.setPattern(EndsLettersNumbersLettersZeroPat);
		m = re.match(_callRcvd);
		if (m.hasMatch())
		{
			_callRcvd[m.capturedStart(1)] = 'O';
			_isRepaired = true;
		}

		re.setPattern(EndsLettersNumbersLettersNumbersPat);
		m = re.match(_callRcvd);
		if (m.hasMatch())	// UW5Y2 becomes UW5Y
		{
			_callRcvd.truncate(m.capturedStart(1));
			_isRepaired = true;
		}

		re.setPattern(StartsOhLetterOhPat);
		m = re.match(_callRcvd);
		if (m.hasMatch())	// '0' Letter '0'
		{
			_callRcvd[m.capturedStart(1)] = '0';
			_isRepaired = true;
		}

		re.setPattern(StartsZeroLetterDigitPat);
		m = re.match(_callRcvd);	// Zero letter Digit
		if (m.hasMatch())
		{
			_callRcvd[m.capturedStart(1)] = 'O';
			_isRepaired = true;
		}

		re.setPattern(StartsDigitZeroPat);
		m = re.match(_callRcvd);
		if (m.hasMatch())
		{
			_callRcvd[m.capturedStart(1)] = 'O';		// beginning digit plus 0 IE(403 = 4O3)
			_isRepaired = true;
		}

		//re.setPattern(EndsDoubleZeroPat);
		//m = re.match(_callRcvd);
		//if (m.hasMatch())
		//{
		//	_callRcvd.replace(m.capturedStart(1), 2, "OO");	// ending 00 becomes OO
		//	_isRepaired = true;
		//}

		re.setPattern(EndsLetterZeroPat);
		m = re.match(_callRcvd);
		if (m.hasMatch() && !_callRcvd.contains("/"))
		{
			_callRcvd[m.capturedStart(1)] = 'O';
			_isRepaired = true;
		}
	}

	re.setPattern("[A-Z]+(\\d)");
	m = re.match(_callRcvd);				// directly from call
	if (m.hasMatch())
		_district = m.captured(1).toInt();

	re.setPattern(EndsPortableSingleDigitPat);
	m = re.match(_callRcvd);
	if (m.hasMatch())
	{
		_portDigit = m.captured(2);
		_district = _portDigit.toInt();
		_callRcvd.truncate(m.capturedStart(1)); // strip portable digit
	}

	_xchRcvd	= _rawXchRcvd;
	_xchRcvd2	= _rawXchRcvd2;

	_dateTime.setTimeSpec(Qt::UTC);
	_dateTime.setDate(QDate::fromString(_date, Qt::ISODate));
	_dateTime.setTime(QTime::fromString(_time, "hhmm"));
	_rawDateTime = _dateTime;

	if(!_dateTime.isValid())
	{
		_status = FORMAT_ERR;
		AppendErrMsgList("DATE/TIME:", _date + ' ' + _time);
	}

	if(PirateCallsSet().contains(_callRcvd))
	{
		_status = DOES_NOT_COUNT;
		return;
	}

	if (!badCallFlag && !aliasCallFlag && !goodCallFlag)
	{
		QStringList parts = _callRcvd.split('/');
		if (parts.size() > 1)
		{
			// convert SP5PY/PA to PA/SP5PY
			QMultiMap <int, QString> partsBySizeMap;
			foreach(QString part, parts)
				partsBySizeMap.insert(part.size(), part);
			QList <QString> partsList = partsBySizeMap.values();

			_callRcvd = partsList.join("/");
		}
	}

	if(_callRcvd.isEmpty() || _callRcvd == CallSent())
	{
		_status = DOES_NOT_COUNT;
		return;
	}

	_pxRcvd = gPxMap.FindExactMatchPx(_rawCallRcvd);			 // check for exact match first -- always use raw call because cty.dat has RA0UF/8(17)

	if (!_pxRcvd)
	{
		QPair < QString, bool> returnedPair = gFormPx(_callRcvd, _portDigit);	// pxRcvd from K1EA = K1EA from FS/K1EA = FS
		QString pxTested = returnedPair.first;
		_fixFccPort = returnedPair.second;
		_pxRcvd = gPxMap.FindPx(pxTested);
	}

	_isValidCty = (_pxRcvd == &gEmptyPx) ? false : true;
	_ctyRcvdPtr = _pxRcvd->GetCty();

	Parse();		// parsing by contest via virtual methods

//	{QMutexLocker lck(&gStdOutMutex); gStdOutStrm << QString("%1 %2").arg(ToString()).arg(_isExcluded) << endl; }
	re.setPattern(LegalCallPat);
	if (!_callRcvd.contains(re) && !goodCallFlag && !aliasCallFlag && !badCallFlag)
	{
		AppendErrMsgList("CALL:", _rawCallRcvd);
		_isLegalCall = false;
		_isExcluded = true;
	}

	if(!_isValidCty) // this is called after Parse() to get qrg info into ToString()
		AppendErrMsgList("CTRY:", _rawCallRcvd);

	if (badCallFlag)
		_status = BUSTED_LIST_CALL;

	if(IncompleteLogSet().contains(_rawCallRcvd))
		_status = GOOD_QSO;
}

QChar Qso::StatusChar()  const
{
	if (_isDupe)
		return _isEarlyDupe ? 'e' : 'D';

	if (_isExcluded)
		return('Z');

	switch (_status)
	{
	case NO_LOG:
		return '-';

	case GOOD_QSO:
		return 'G';

	case NOT_IN_LOG:
		return 'N';

	case BUSTED_CALL:
		return 'B';

	case BUSTED_LIST_CALL:
		return 'L';

	case BUSTED_FCC_CALL:
		return 'F';

	case BUSTED_REVLOG_CALL:
		return 'R';

	case RUBBER_CLOCK_VIOLATION:
		return 'A';
		
	case BUSTED_EXCHANGE:
		return 'X';

	case FORMAT_ERR:
		return 'E';

	case DOES_NOT_COUNT:
		return 'C';

	case BAND_CHANGE_VIOLATION:
		return 'V';

	case OUT_OF_BAND:
		return 'O';

	default:
		return ' ';
	}
}

QPair< QString, bool> gFormPx(QString pxStr, QString portSingleDigit = QString())				// pass in a full call, reduce to a px
{
	QString shortest = pxStr;
	QString longest;					// longest used to see if root is US possession
	bool isFccNonLower48(false);
	QPair< QString, bool> returnPair;

	QStringList parts = pxStr.split('/');
	foreach(QString s, parts)
		if (s.length() > longest.length())
			longest = s;

	Px const * px = gPxMap.FindPx(longest);
	if (px != &gEmptyPx)
		isFccNonLower48 = px->GetCty()->IsUsaPossession();

	foreach(QString s, parts)
		if (s.length() < pxStr.length())
			pxStr = s;

	if (!portSingleDigit.isEmpty())		// replace last digit of first number
	{									// after at least one letter with the single portable digit
		if (isFccNonLower48)			// stupid FCC
		{
			returnPair.first = QString("K") + portSingleDigit;
			returnPair.second = true;
			return returnPair;
		}

		QRegularExpression re("[A-Z]+(\\d+)");
		QRegularExpressionMatch m = re.match(shortest);
		if (m.hasMatch())
			pxStr.replace(m.capturedEnd(1) - 1, 1, portSingleDigit);
		else
			pxStr += portSingleDigit;
	}

	returnPair.first = pxStr;
	returnPair.second = false;
	return returnPair;
}

QString CallFormer::RootCall(QString call) const
{
	if (gContest->AliasCallMap().contains(call))		// override in AliasCalls.lis
		return gContest->AliasCallMap().value(call);

	QStringList parts = call.split("/");
	if (parts.size() == 2 && parts[0].size() > parts[1].size())	// shortest first
		call = parts[1] + "/" + parts[0];

	return call;
}

Qso::Qso(Log * log) :
	_fixFccPort			(false),
	_isBadCallByList	(false),
	_isBandChange		(false),
	_isBandEdge			(false),
	_isBustedOffByTwo	(false),
	_isCorrectable		(false),
	_isCrossBand		(false),
	_isDupe				(false),
	_isEarlyDupe		(false),
	_isExactQrg			(false),
	_isExcluded	 		(false),
	_isHeRunning		(false),
	_isLegalCall		(true),
	_isLostMult	 		(false),
	_isLostMult2		(false),
	_isLostMult3		(false),
	_isMM				(false),
	_isMult				(false),
	_isMult2			(false),
	_isMult3			(false),
	_isMultStn			(false),
	_isOutOfBand		(false),
	_isOutOfTime		(false),
	_isPortable			(false),
	_isRawMult			(false),
	_isRawMult2			(false),
	_isRawMult3			(false),
	_isRepaired			(false),
	_isRubberClocked	(false),
	_isRun				(false),
	_isUnique		 	(false),
	_isValidCty			(false),
	_isWrongBand		(false),
	_district			(0),
	_hisComputedQrg		(0),
	_lineNum			(0),
	_lvDistance			(0),
	_matchedTimeDiff	(0),
	_myComputedQrg		(0),
	_pts				(0),
	_qrg				(0),
	_rate20				(0),
	_timeOffset			(0),
	_timeToPktSpot		(0),

	// QSO_STATUS
	_status(NO_LOG),

	//Cty ptr
	_ctyRcvdPtr(&gEmptyCty),

	// Log *
	_log(log),

	// PxPtr
	_pxRcvd(&gEmptyPx),

	// QsoPtr
	_crossBandMatched(0),
	_matched(0),

	// SpotPtr
	_spotHit(0)
	{}

QString Qso::ToString() const
{
	return QString("%1 %2 %3 %4 %5 %6 %7 %8")
		.arg(_qrg			, 5)
		.arg(_mode			, 2)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(CallSent()		, -10)
		.arg(_xchSent		, 5)
		.arg(_rawCallRcvd	,-10)
		.arg(_rawXchRcvd	, 5);
}

QString Qso::ToTabString() const
{
	return QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8")
		.arg(_qrg)
		.arg(_mode)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(CallSent())
		.arg(_xchSent)
		.arg(_rawCallRcvd)
		.arg(_rawXchRcvd);
}

QString Qso::ToReverseString() const
{
	return QString("%1 %2 %3 %4 %5 %6 %7 %8")
		.arg(_qrg, 5)
		.arg(_mode, 2)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(_rawCallRcvd, -10)
		.arg(_rawXchRcvd, 5)
		.arg(CallSent(), -10)
		.arg(_xchSent, 5);
}

QString Qso::ToFullReverseString() const
{
	return QString("QSO: %1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
		.arg(Qrg(), 5)
		.arg(Mode(), 2)
		.arg(DateStr())
		.arg(TimeStr())
		.arg(_rawCallRcvd, -10)
		.arg(_rstRcvd, 3)
		.arg(_rawXchRcvd, 8)
		.arg(CallSent(), -10)
		.arg(_rstSent, 3)
		.arg(_xchSent);
}

bool Qso::CountsForCredit()	const
{
	return (_status == GOOD_QSO || _status == NO_LOG) && CountsForRawCredit();
}

bool Qso::CountsForRawCredit()	const
{
	return !_isDupe && !_isExcluded && _isValidCty && _isLegalCall && !_isBadCallByList;
}

bool gSpotTimeLessThanSpotTime	(Spot		const *sp1, Spot		const	*sp2) { return sp1->DateTime()	< sp2->DateTime();	}
bool gTimeLessThanSpotTime		(QDateTime	const   dt, Spot		const	*sp1) { return dt				< sp1->DateTime();	}

bool gQsoTimeLessThanQsoTime	(Qso		const * q1, Qso			const	*q2) { return q1->DateTime()	< q2->DateTime();	}
bool gTimeLessThanQsoTime		(QDateTime	const   dt, Qso			const	* q) { return dt				< q->DateTime();	}
bool gQsoTimeLessThanTime		(Qso		const * q,	QDateTime	const	 dt) { return q->DateTime()		< dt;				}

bool gLostMultLessThanByDateTime(LostMult const &lm1, LostMult const &lm2) { return lm1.QPtr()->DateTime()	< lm2.QPtr()->DateTime(); }

QString Qso::CallSent() const { return GetLogPtr()->Call(); }

QMap <QString, QString> const	Qso::AliasCallMap()			const { return _log->ContestPtr()->AliasCallMap();		}
QMap <QString, QString> const	Qso::AliasDistrictMap()		const { return _log->ContestPtr()->AliasDistrictMap();  }
QMap <QString, QString> const	Qso::BadCallMap()			const { return _log->ContestPtr()->BadCallMap();		}
QSet <QString>			const	Qso::GoodCallsSet()			const { return _log->ContestPtr()->GoodCallsSet();		}
QSet <QString>			const	Qso::PirateCallsSet()		const { return _log->ContestPtr()->PirateCallsSet();	}
QSet <QString>			const	Qso::IncompleteLogSet()		const { return _log->ContestPtr()->IncompleteLogSet();	}

QStringList				const	Qso::ErrMsgList()			const { return _errMsgList; }
QStringList				const	Qso::WrnMsgList()			const { return _errMsgList; }

CallSet const Qso::BadInfoRcvdSet() const { return _log->ContestPtr()->BadInfoRcvdSet(); }
CallSet const Qso::BadInfoSentSet() const { return _log->ContestPtr()->BadInfoSentSet(); }

int				Qso::NumberOfFields	()	const { return _log->ContestPtr()->NumberOfFields();	}
int				Qso::RcvdCallFldNum	()	const { return _log->ContestPtr()->RcvdCallFldNum();	}
int				Qso::RcvdRstFldNum	()	const { return _log->ContestPtr()->RcvdRstFldNum();		}
int				Qso::RcvdXchFldNum	()	const { return _log->ContestPtr()->RcvdXchFldNum();		}
int				Qso::RcvdXchFldNum2	()	const { return _log->ContestPtr()->RcvdXchFldNum2();	}
int				Qso::SentXchFldNum	()	const { return _log->ContestPtr()->SentXchFldNum();		}
int				Qso::SentXchFldNum2	()	const { return _log->ContestPtr()->SentXchFldNum2();	}
int				Qso::SentRstFldNum	()	const { return _log->ContestPtr()->SentRstFldNum();		}
int				Qso::MatchWindowSize()	const { return _log->ContestPtr()->MatchWindowSize();	}
int				Qso::MultStnFldNum	()	const { return _log->ContestPtr()->MultStnFldNum();		}

void			Qso::IsBandChange		(bool b)	{ _isBandChange		= b; }
void			Qso::IsBadCallByList	(bool b)	{ _isBadCallByList	= b; }
void			Qso::IsBustedOffByTwo	(bool b)	{ _isBustedOffByTwo = b; }
void			Qso::IsDupe				(bool b)	{ _isDupe			= b; }
void			Qso::IsEarlyDupe		(bool b)	{ _isEarlyDupe		= b; }
void			Qso::IsExactQrg			(bool b)	{ _isExactQrg		= b; }
void			Qso::IsExcluded			(bool b)	{ _isExcluded		= b; }
void			Qso::IsLostMult			(bool b)	{ _isLostMult		= b; }
void			Qso::IsLostMult2		(bool b)	{ _isLostMult2		= b; }
void			Qso::IsLostMult3		(bool b)	{ _isLostMult3		= b; }
void			Qso::IsMult				(bool b)	{ _isMult			= b; }
void			Qso::IsMult2			(bool b)	{ _isMult2			= b; }
void			Qso::IsMult3			(bool b)	{ _isMult3			= b; }
void			Qso::IsMultStn			(bool b)	{ _isMultStn		= b; }
void			Qso::IsMyRun			(bool b)	{ _isRun			= b; }
void			Qso::IsHisRun			(bool b)	{ _isHeRunning		= b; }
void			Qso::IsOutOfTime		(bool b)	{ _isOutOfTime		= b; }
void			Qso::IsRawMult			(bool b)	{ _isRawMult		= b; }
void			Qso::IsRawMult2			(bool b)	{ _isRawMult2		= b; }
void			Qso::IsRawMult3			(bool b)	{ _isRawMult3		= b; }
void			Qso::IsUnique			(bool b)	{ _isUnique			= b; }

QDateTime		Qso::RawDateTime()		const	{ return _rawDateTime;		}
QDateTime		Qso::DateTime()			const	{ return _dateTime;			}
QString			Qso::Band()				const	{ return _band;				}
QString			Qso::Mode()				const	{ return _mode;				}
QString			Qso::CallRcvd()			const	{ return _callRcvd;			}
QString			Qso::Cont()				const	{ return _pxRcvd->Cont();}
QString			Qso::CorrectCall()		const	{ return _correctCall;		}
QString			Qso::CorrectXch()		const	{ return _correctXch;		}
QString			Qso::DateStr()			const	{ return _dateTime.date().toString("yyyy-MM-dd"); }
QString			Qso::RawCallRcvd()		const	{ return _rawCallRcvd;		}
QString			Qso::Running()			const	{ return _running;			}
QString			Qso::RlRunning()		const	{ return _rlRunning;		}
QString			Qso::RstRcvd()			const	{ return _rstRcvd;			}
QString			Qso::RstSent()			const	{ return _rstSent;			}
QString			Qso::TimeStr()			const	{ return _dateTime.time().toString("hhmm"); }
QString			Qso::StdPx()			const	{ return _pxRcvd->StdPx(); }
QString			Qso::XchSent()			const	{ return _xchSent;			}
QString			Qso::XchSent2()			const	{ return _xchSent2;			}
QString			Qso::XchRcvd()			const	{ return _xchRcvd;			}
QString			Qso::XchRcvd2()			const	{ return _xchRcvd2;			}
QString			Qso::RawXchRcvd()		const	{ return _rawXchRcvd;		}

time_t 			Qso::TimeInSeconds()	const	{ return _dateTime.toTime_t();}
QSO_STATUS		Qso::Status()			const	{ return _status;			}

bool			Qso::IsExcluded()		const	{ return _isExcluded;		}
bool			Qso::IsBadCallByList()	const	{ return _isBadCallByList;	}
bool			Qso::IsBandChange()		const	{ return _isBandChange;		}
bool			Qso::IsBustedOffByTwo()	const	{ return _isBustedOffByTwo;	}
bool			Qso::IsDupe()			const	{ return _isDupe;			}
bool			Qso::IsExactQrg()		const	{ return _isExactQrg;		}
bool			Qso::IsMM()				const	{ return _isMM;				}
bool			Qso::IsMult()			const	{ return _isMult;			}
bool			Qso::IsMult2()			const	{ return _isMult2;			}
bool			Qso::IsMult3()			const	{ return _isMult3;			}
bool			Qso::IsOutOfBand()		const	{ return _isOutOfBand;		}
bool			Qso::IsOutOfTime()		const	{ return _isOutOfTime;		}
bool			Qso::IsPortable()		const	{ return _isPortable;		}
bool			Qso::IsRepaired()		const	{ return _isRepaired;		}
bool			Qso::IsLegalCall()		const	{ return _isLegalCall;		}
bool			Qso::IsLostMult()		const	{ return _isLostMult;		}
bool			Qso::IsLostMult2()		const	{ return _isLostMult2;		}
bool			Qso::IsLostMult3()		const	{ return _isLostMult3;		}
bool			Qso::IsRawMult()		const	{ return _isRawMult;		}
bool			Qso::IsRawMult2()		const	{ return _isRawMult2;		}
bool			Qso::IsRawMult3()		const	{ return _isRawMult3;		}
bool			Qso::IsValidCty()		const	{ return _isValidCty;		}
bool			Qso::IsUsaPossession()	const	{ return MyCty()->IsUsaPossession(); }
bool			Qso::IsRcvdCallRootUsaPossession() const { return _fixFccPort;	}
bool			Qso::IsRubberClocked()	const	{ return _isRubberClocked;	}
bool			Qso::IsWVe()			const	{ return MyCty()->IsWVe();	}
bool			Qso::IsWVeRcvd()		const	{ return _ctyRcvdPtr->IsWVe();}
bool			Qso::IsMultStn()		const	{ return _isMultStn;		}
bool			Qso::IsMyRun()			const	{ return _isRun;			}
bool			Qso::IsHisRun()			const	{ return _isHeRunning;		}
bool			Qso::IsBandEdge()		const	{ return _isBandEdge;		}
bool			Qso::IsStnOne()			const	{ return !_isMultStn;		}
bool			Qso::IsStnTwo()			const	{ return _isMultStn;		}
bool			Qso::IsUnique()			const	{ return _isUnique;			}

int				Qso::District()			const	{ return _district;			}
int				Qso::HisComputedQrg()	const	{ return _hisComputedQrg;	}
int				Qso::LineNum()			const	{ return _lineNum;			}
uint			Qso::LvDistance()		const	{ return _lvDistance;		}
int				Qso::MatchedTimeDiff()	const	{ return _matched ? _dateTime.secsTo(_matched->DateTime()) : 0; }
int				Qso::MyComputedQrg()	const	{ return _myComputedQrg;	}
int 			Qso::Pts()				const	{ return _pts;				}
int				Qso::Qrg()				const	{ return _qrg;				}
int 			Qso::Rate20()			const	{ return _rate20;			}
int 			Qso::TimeAdjustment()	const	{ return _timeOffset;		}
int 			Qso::TimeToPktSpot()	const	{ return _timeToPktSpot;	}

QMutex	&		Qso::Lock()						{ return _lock;					}
Log *			Qso::GetLogPtr()		const	{ return _log;					}

Qso *			Qso::Matched()			const	{ return _matched;				}
QsoPtr			Qso::CrossBandMatch()	const	{ return _crossBandMatched;		}
Cty		const *	Qso::CtyRcvd()			const	{ return _ctyRcvdPtr;			}
Cty		const *	Qso::MyCty()			const	{ return GetLogPtr()->CtyPtr();	}
Spot	const *	Qso::SpotHit()			const	{ return _spotHit;				}
Px		const *	Qso::PxRcvdPtr()		const	{ return _pxRcvd;			}
QString			Qso::WpxStr()			const	{ return _wpxStr;				}
QString			Qso::PortDigit()		const	{ return _portDigit;			}

BandEdges const & Qso::BandEdge()		const	{ return _bandEdge;				}

void			Qso::AdjustTime			(int n)					{ _dateTime = _dateTime.addSecs(n); _timeOffset += n; }
void			Qso::HisComputedQrg		(int qrg)				{ _hisComputedQrg	= qrg;		}
void			Qso::LvDistance			(uint lvd)				{ _lvDistance		= lvd;		}
void			Qso::MyComputedQrg		(int qrg)				{ _myComputedQrg	= qrg;		}
void			Qso::Pts				(int pts)				{ _pts				= pts;		}
void			Qso::Rate20				(int rate)				{ _rate20			= rate;		}
void			Qso::TimeToPktSpot		(int t)					{ _timeToPktSpot	= t;		}
void			Qso::Matched			(Qso * q)				{ _matched			= q;		}
void			Qso::CrossBandMatch		(Qso * q)				{ _crossBandMatched	= q;		}
void			Qso::Status				(QSO_STATUS status)		{ _status			= status;	}
void 			Qso::Band				(QString band)			{ _band				= band;		}
void			Qso::CorrectCall		(QString call)			{ _correctCall		= call;		}
void			Qso::CorrectXch			(QString xch)			{ _correctXch		= xch;		}
void			Qso::RlRunning			(QString r)				{ _rlRunning		= r;		}
void			Qso::Running			(QString r)				{ _running			= r;		}
void			Qso::XchSent			(QString xch)			{ _xchSent			= xch;		}
//void			Qso::XchSent2			(QString xch)			{ _xchSent2			= xch;		}
void			Qso::SpotHit			(Spot const * sptPtr)	{ _spotHit			= sptPtr;	}
void			Qso::IsCorrectable		(bool b)				{ _isCorrectable	= b;		}

void Qso::AppendErrMsgList(QString msg, QString arg)
{ 
	QString outStr = QString("%1 %2").arg(msg, 10).arg(arg, -10);
	_errMsgList.append(outStr); 
}
