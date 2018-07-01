#include "StdAfx.h"

Log * Log::CreateLog(QFileInfo fileInfo)
{
	QString token;
	QString data;
	QMultiMap <QString, TokenDataPair> tokenDataPairMap;
	QFile inputFile(fileInfo.absoluteFilePath());
	inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream inputStream(&inputFile);
	QSet <QString> lineSet;

	inputStream.setCodec(QTextCodec::codecForName("ISO 8859-1"));
	while (!inputStream.atEnd())
	{
		QString line = inputStream.readLine().simplified();

		// reject duplicate lines
		if (lineSet.contains(line))
			continue;
		lineSet.insert(line);

		int idx = line.indexOf(':');							// split() and section() don't work because some lines have multiple ':'
		if (idx == -1)											// must have at least one ':' for token
			continue;
		token = line.left(idx).toUpper();						// before the colon -- all tokens are upper case
		data = line.right(line.length() - idx - 1).trimmed();	// the rest of the line without white space at ends
		tokenDataPairMap.insert(token, TokenDataPair(token, data));
	}
	inputFile.close();

	if (!tokenDataPairMap.contains("CONTEST"))
	{
		gContest->AppendErrMsgList(fileInfo.fileName(), QString("%1 line not found").arg("CONTEST:", -22));
		return false;
	}

	if (!tokenDataPairMap.contains("CALLSIGN"))
	{
		gContest->AppendErrMsgList(fileInfo.fileName(), QString("%1 line not found").arg("CALLSIGN:", -22));
		return false;
	}

	if (!tokenDataPairMap.contains("CATEGORY-OPERATOR"))
	{
		gContest->AppendErrMsgList(fileInfo.fileName(), QString("%1 missing, using: SINGLE-OP").arg("CATEGORY-OPERATOR:", -22));
		tokenDataPairMap.insert("CATEGORY-OPERATOR", TokenDataPair("CATEGORY-OPERATOR", "SINGLE-OP"));
	}

	if (!tokenDataPairMap.contains("CATEGORY-BAND"))
	{
		gContest->AppendErrMsgList(fileInfo.fileName(), QString("%1 missing, using: ALL").arg("CATEGORY-BAND:", -22));
		tokenDataPairMap.insert("CATEGORY-BAND", TokenDataPair("CATEGORY-BAND", "ALL"));
	}

	if (!tokenDataPairMap.contains("CATEGORY-POWER"))
	{
		gContest->AppendErrMsgList(fileInfo.fileName(), QString("%1 missing, using: HIGH").arg("CATEGORY-POWER:", -22));
		tokenDataPairMap.insert("CATEGORY-POWER", TokenDataPair("CATEGORY-POWER", "HIGH"));
	}

	if (!tokenDataPairMap.contains("CATEGORY-MODE"))
	{
		gContest->AppendErrMsgList(fileInfo.fileName(), QString("%1 missing, using: %2").arg("CATEGORY-MODE:", -22).arg(gContest->Mode()));
		tokenDataPairMap.insert("CATEGORY-MODE", TokenDataPair("CATEGORY-MODE", gContest->Mode()));
	}

	if (!tokenDataPairMap.contains("CATEGORY-TRANSMITTER"))
	{
		gContest->AppendErrMsgList(fileInfo.fileName(), QString("%1 missing, using: ONE").arg("CATEGORY-TRANSMITTER:", -22));
		tokenDataPairMap.insert("CATEGORY-TRANSMITTER", TokenDataPair("CATEGORY-TRANSMITTER", "ONE"));
	}

	QString rawCall;
	QString call;
	QString portDigit;
	bool marMobile(false);
	Px const * px(0);

	foreach(auto pair, tokenDataPairMap)
	{
		token = pair.first;
		data = pair.second;
		QRegularExpression re;

		if (token == "CALLSIGN")
		{
			// letters numbers and slash allowed in calls
			rawCall = call = data.toUpper().replace('-', '/').remove(QRegularExpression("[^A-Z0-9/]"));

			re.setPattern("/R$");				// for VHF Rovers
			call.replace(re, "-R");

			if (gContest->AliasCallMap().contains(rawCall))
				call = gContest->AliasCallMap().value(rawCall);

			call.remove(QRegularExpression("\\s+"));

			if (gContest->PirateCallsSet().contains(data))
			{
				gContest->AppendErrMsgList(fileInfo.fileName(), "PIRATE: " + data);
				return false;
			}

			if (call.endsWith("/MM"))
			{
				marMobile = true;
				call.remove("/MM$");
			}

			call.remove(QRegularExpression(StartAndEndNonsensePat));
			call.replace(QRegularExpression(MiddleNonsensePat), "/");

			if (!call.contains(QRegularExpression(LegalCallPat)) && !gContest->GoodCallsSet().contains(call))
			{
				gContest->AppendErrMsgList("CALL:", call);
				return false;
			}

			QRegularExpressionMatch m = QRegularExpression(EndsPortableSingleDigitPat).match(call);
			if (m.hasMatch())
			{
				portDigit = m.captured(2);
				call.truncate(m.capturedStart(1));
			}

			m = QRegularExpression(EndsLetterZeroPat).match(call);
			if (m.hasMatch())
				call[m.capturedStart(1)] = 'O';

			QStringList parts = call.split('/');

			if (parts.size() > 1 && portDigit.isEmpty()) // don't make K5ZD/1 into 1/K5ZD
			{
				// convert SP5PY/PA to PA/SP5PY
				QMultiMap <int, QString> partsBySizeMap;
				foreach(QString part, parts)
					partsBySizeMap.insert(part.size(), part);

				QList <QString> partsList = partsBySizeMap.values();
				call = partsList.join("/");
			}

			if (!marMobile)
			{
				px = gPxMap.FindExactMatchPx(rawCall);			 // check for exact match first -- raw call because cty.dat overloads contain RA0UF/8(17)
				if (!px)
				{
					px = gPxMap.FindPx(gFormPx(call, portDigit).first);
					if (!px)
						gContest->AppendErrMsgList("CTRY:", rawCall);
				}
			}
			else
				px = &gEmptyPx;
		}
	}

	if (call.isEmpty())
	{
		QMutexLocker locker(&gStdErrMutex);
		gContest->AppendErrMsgList("CALL:", "EMPTY");
		return false;
	}

	static std::map <QString, ContestTypes> contestTypeMap =
	{
		{"CQWW",	CQWW	},{"CQWW-RY",	CQWW_RY	},{"WPX",		WPX		},{"WPX-RY",	WPX_RY	},
		{"ARDX",	ARDX	},{"RTTY",		RTTY	},{"IARU",		IARU	},{"VHF-JAN",	VHF_JAN	},
		{"VHF-JUN",	VHF_JUN	},{"VHF-JUN",	VHF_JUN	},{"VHF-SEP",	VHF_SEP	},{"OC",		OC		}
	};

	Cty const * cty = px->GetCty();
	Log * log(0);
	
	switch (contestTypeMap[gContest->ContestType()])
	{
	case CQWW:
		log = new CqwwLog();
		break;

	case CQWW_RY:
		log = new CqwwRttyLog();
		break;

	case WPX:
		log = new WpxLog();
		break;

	case WPX_RY:
		log = new WpxRttyLog();
		break;

	case ARDX:
		if (cty->IsWVe())
			log = new ArrlDxWVeLog();
		else
			log = new ArrlDxDxLog();
		break;

	case RTTY:
		log = new ArrlRttyLog();
		break;

	case IARU:
		if ((static_cast <IaruContest *> (gContest))->IsWrtcStn(call))
			log = new WrtcLog();
		else
			log = new IaruLog();
		break;

	case VHF_JAN:
	case VHF_JUN:
	case VHF_SEP:
		log = new VhfLog();
		break;

	case OC:
		log = new OcDxLog();
		break;

	default:
		return 0;
	}

	log->FileInfo(fileInfo);
	log->Call(call);
	log->RawCall(rawCall);
	log->PxPtr(px);
	log->SetCty(cty);
	log->Mode(gContest->Mode());
	log->IsBadRcvdInfo(gContest->BadInfoRcvdSet().contains(rawCall));
	log->IsBadSentInfo(gContest->BadInfoSentSet().contains(rawCall));
	log->IsIncomplete(gContest->IncompleteLogSet().contains(rawCall));
	log->Disqualified(gContest->DisqualifiedSet().contains(rawCall));
	log->SubmittedLate(log->SubmittedLate() && !gContest->ExtensionsSet().contains(rawCall));
	if (log->Cont().isEmpty())
		log->Cont(px->Cont());

	if (gContest->ContestType().isEmpty())
		log->Cont(px->Cont());

	log->LoadCabrilloFile(tokenDataPairMap.values());
	return log;
}

void Log::SortLostMultList()			{ std::stable_sort(_lostMultList.begin(), _lostMultList.end(), gLostMultLessThanByDateTime); }

void Log::MarkMultiOpBCV()
{
	ComputeMults();				// get mults, they might change in checking
	CheckBandChanges();			// mark QSOs not counted
	ComputeMults();				// recompute mults
}

bool Log::AdjustTimeOffsets()
{
   if(TimeNormalized())
           return false;

   std::vector <int> timeDiffVec;
   int medianTimeDiff = 0;

    foreach(QsoPtr q, AllQso())
    {
        QsoPtr rq = q->Matched();
        if(!rq) continue;

        timeDiffVec.push_back(q->DateTime().secsTo(rq->DateTime()));			// offset in seconds
    }

    if(!timeDiffVec.size())
        return false;

    std::stable_sort(timeDiffVec.begin(), timeDiffVec.end());
    medianTimeDiff = timeDiffVec[timeDiffVec.size()/2];

    if(medianTimeDiff == 0)
    {
        TimeNormalized(true);
        return false;
    }

    foreach(QsoPtr q, AllQso())
        q->AdjustTime(medianTimeDiff);

    TimeAdjustment(medianTimeDiff);	// record adjustment in the log object
    return true;
}

void Log::ReportTimeMatching()
{
	QString ext = IsFinalTm() ? "final" : "initial";
    QString tmLogFileName = ContestPtr()->TmDir().absoluteFilePath(FileInfo().baseName() + ".tm." + ext);
    QFile tmLogFile(tmLogFileName);
    if(!tmLogFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream tmLogStream( &tmLogFile);

	tmLogStream << QString("%1 qso time offsets to other logs\n\n").arg(ext);
	tmLogStream << "************************* QSO ************************   delta  my time his time\n\n";

    foreach(Qso * q, AllQso())
    {
		QString offset = q->Matched() ? QString::number(q->MatchedTimeDiff() / 60) : "-";

        tmLogStream << QString("%1  %2    %3   %4\n")
            .arg(q->ToString())
            .arg(offset, 6)
            .arg(q->DateTime().time().toString("hhmm"))
            .arg(q->Matched() ?  q->Matched()->DateTime().time().toString("hhmm") : "");
    }

    tmLogFile.close();
}

QsoList Log::GetQsoListByCall(QString call, QsoPtr q) { return _qsoListByCallBandMap.value(call).value(q->Band()); }

QList < QPair <QsoList, QsoList> > Log::GetQsoLists()
{
	// each rcvd call band has a list a of qsos in each log
	// this returns the rcvd call band lists for this log

	QList < QPair <QsoList, QsoList> > listOfPairsOfQsoLists;
    foreach(QString callRcvd, _qsoListByCallBandMap.keys())
    {
        Log * hisLog = gContest->GetLog(callRcvd);
        if (!hisLog)
            continue;

        foreach(QString band, _qsoListByCallBandMap.value(callRcvd).keys())
        {
            QsoList myList = GetQsoListByCallBandMap().value(callRcvd).value(band);
            QsoList hisList = hisLog->GetQsoListByCallBandMap().value(_call).value(band);
            QPair <QsoList, QsoList> pairOfQsoLists(myList, hisList);
            listOfPairsOfQsoLists.append(pairOfQsoLists);
        }
    }

    return listOfPairsOfQsoLists;
}

void Log::MarkUniques()
{
	foreach(QsoPtr q, _allQso)
	{
		if (_contest->LogSetByCallRcvd()[q->CallRcvd()].size() == 1)
		{
			q->IsUnique(true);
			AppendUniList(q);
		}
	}
}

void Log::CheckXchs()			// this is called from multiple threads
{
	if (_isBadRcvdInfo)
		return;

    foreach(QsoPtr q, _allQso)
    {
        if (DoNotCheck() || q->Status() != GOOD_QSO || !q->Matched() || q->Matched()->GetLogPtr()->IsBadSentInfo())
            continue;

        QsoPtr rq = q->Matched();

        if (!q->CompareExchange(rq))
        {
            q->Status(BUSTED_EXCHANGE);
            rq->GetLogPtr()->AppendBustedXchCausedList(q);
        }
    }
}

void Log::CheckBusts()
{
	if (_doNotCheck)
	{
		foreach(QsoPtr q, _logNoMatchList)
			q->Status(GOOD_QSO);
		return;
	}

	int windowSize = _contest->MatchWindowSize();
	bool isCW = Mode() == "CW";

	foreach(QsoPtr q, _logNoMatchList)
	{
		QsoPtr rq(0);
		q->CrossBandMatch(0);

		Log * hisLog = gContest->LogsByCall().value(q->CallRcvd());
		if (hisLog->DoNotCheck() || hisLog->IsIncomplete())
		{
			q->Status(GOOD_QSO);
			continue;
		}

		QsoList offByOneList;
		QsoList offByTwoList;
		QsoList cwSet;
		auto qsos = hisLog->AllQso();

		auto it = std::lower_bound(qsos.begin(), qsos.end(), q->DateTime().addSecs(-windowSize), gQsoTimeLessThanTime);
		while (it != qsos.end() && q->DateTime().secsTo((*it)->DateTime()) <= windowSize)
		{
			rq = *it;
			int qrgOffset(0);

			// if either end doesn't have exact qrg, a wide spread diff can't help them
			if(q->IsExactQrg() && rq->IsExactQrg())
			   qrgOffset = q->Qrg() - rq->Qrg();

			if (q->Band() == rq->Band() && !rq->Matched() && abs(qrgOffset) <= 1)
			{
				uint d = gLvd(_call, rq->CallRcvd());
				if (d == 1)
					offByOneList.append(rq);
				else
				{
					if (gUseOffByTwo)
					{
						if (d == 2 && abs(q->DateTime().secsTo(rq->DateTime())) <= windowSize / 2)
							offByTwoList.append(rq);
						else
						{
							if (isCW)
							{
								d = gLvd(gStrToMorse(_call), gStrToMorse(rq->CallRcvd()));

								if (d <= 3 && abs(q->DateTime().secsTo(rq->DateTime())) <= windowSize / 2)
								{
									rq->LvDistance(d);
									cwSet.append(rq);
								}
							}
						}
					}
				}
			}
			it++;
		}

		int bestTimeDiff(INT_MAX);
		QsoPtr bestMatchPtr(0);
		int bestLvDist(INT_MAX);

		if (offByOneList.size())
		{
			foreach(QsoPtr p, offByOneList)
			{
				int timeDiff = q->DateTime().secsTo(p->DateTime());
				if (abs(timeDiff < bestTimeDiff))
				{
					bestTimeDiff = timeDiff;
					bestMatchPtr = p;
					bestLvDist = 1;
				}
			}
			bestMatchPtr->LvDistance(1);
		}
		else
		{
			if (gUseOffByTwo && offByTwoList.size())
			{
				foreach(QsoPtr p, offByTwoList)
				{
					int timeDiff = q->DateTime().secsTo(p->DateTime());
					if (abs(timeDiff < bestTimeDiff))
					{
						bestTimeDiff = timeDiff;
						bestMatchPtr = p;
						bestLvDist = 2;
						bestMatchPtr->LvDistance(2);
					}
				}
			}
			else
			{
				foreach(QsoPtr p, cwSet)
				{
					int timeDiff = q->DateTime().secsTo(p->DateTime());
					if (abs(timeDiff < bestTimeDiff))
					{
						bestTimeDiff = timeDiff;
						bestMatchPtr = p;
						bestLvDist = 3;
					}
				}
			}
		}

		if (bestMatchPtr)
		{
			rq = bestMatchPtr;
			q->LvDistance(bestLvDist);
			q->Matched(rq);
			q->Status(GOOD_QSO);

			rq->Lock().lock();
			rq->LvDistance(bestLvDist);
			rq->Matched(q);
			rq->Status(BUSTED_CALL);			// he busted this one
			rq->CorrectCall(q->CallSent());
			rq->Lock().unlock();
			AppendBustCausedList(q);		// add to my caused list
			continue;
		}

		q->Status(NOT_IN_LOG);
		if (!q->IsDupe())
			hisLog->AppendNilCausedList(q);							// this is automatically protected by Log mutex

		QsoPtr rqPtr = hisLog->Find(q->CallSent(), q, 300, true);	// test for cross band within 5 minutes
		if (rqPtr && !rqPtr->IsDupe())
		{
			q->CrossBandMatch(rqPtr);
			rqPtr->CrossBandMatch(q);
			AppendCrossBandList(q);
		}
	}
}

void Log::MatchQsos()
{
	_noLogByTimeMap.clear();
    _logNoMatchList.clear();

    foreach(QsoPtr q, _allQso)
    {
        q->IsDupe(false);
        q->Status(NO_LOG);
        q->Matched(0);
	}

    // try to match by call and band in other logs
    // generation of pairs of QsoLists are delegated through virtual GetQsoLists()

    QList < QPair <QsoList, QsoList> > listOfPairsOfQsoLists = GetQsoLists();
    QListIterator < QPair <QsoList, QsoList> > it(listOfPairsOfQsoLists);

    while (it.hasNext())
    {
        QPair <QsoList, QsoList> pairOfQsoLists = it.next();
        QsoList  myList = pairOfQsoLists.first;
        QsoList hisList = pairOfQsoLists.second;

		while (!hisList.isEmpty() && !myList.isEmpty())
        {
            int minTimeDiff = _contest->MatchWindowSize();
            QsoPtr myBestMatch(0);
            QsoPtr hisBestMatch(0);

            for (int i = 0; i < myList.size(); i++)
            {
                QsoPtr q = myList.at(i);

                for (int j = 0; j < hisList.size(); j++)
                {
                    QsoPtr rq = hisList.at(j);
					
                    int diff = abs(q->DateTime().secsTo(rq->DateTime()));
                    if (minTimeDiff && diff < minTimeDiff)
                    {
                        myBestMatch = q;
                        hisBestMatch = rq;
                        minTimeDiff = diff;
                    }
                }
            }

            if (!myBestMatch || !hisBestMatch)
                break;

            myBestMatch->Matched(hisBestMatch);
            myBestMatch->Status(GOOD_QSO);
            hisBestMatch->Matched(myBestMatch);
            hisBestMatch->Status(GOOD_QSO);
            myList.removeOne(myBestMatch);
            hisList.removeOne(hisBestMatch);
		}

        foreach(QsoPtr q, myList)
        {
            q->Status(LOG_NO_MATCH);
            _logNoMatchList.append(q);
		}
    }
}

void Log::WriteRptFile()
{
	QString rptFileName = gContest->RptDir().absoluteFilePath(FileInfo().baseName() + ".rpt");
	QFile rptFile(rptFileName);
	rptFile.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream strm(&rptFile);
	WriteRptSummary(strm);
	
	// report NIL
	if (NilList().size())
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << " Not In Log ";
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	    foreach(Qso const * q, NilList())
	        strm << QString("%1\n").arg(q->ToString());
	}
	
	// report BUST
	QMultiMap < QDateTime, QsoPtr> badCallsLog;
	
	foreach(QsoPtr q, BustedCallList())
	    badCallsLog.insert(q->DateTime(), q);
	
	foreach(QsoPtr q, BadRevLogList())
	    badCallsLog.insert(q->DateTime(), q);
	
	if (badCallsLog.size())
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << " Incorrect Call ";
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	
	    foreach(QsoPtr q, badCallsLog)
	    {
	        if (q->CorrectCall().size())
	        {
	            QString correctCall = q->CorrectCall();			 // fix -R in VHF tests
	            correctCall.replace(QRegularExpression("-R$"), "/R");
	            q->CorrectCall(correctCall);
	        }
	
	        QString msg;
	        if (q->CorrectCall().isEmpty())
	            msg = "incorrect call";
	        else
	            msg = "correct";
	
	        strm << QString("%1 %2 %3\n")
	            .arg(q->ToString())
	            .arg(msg)
	            .arg(q->CorrectCall());
	    }
	}
	
	// report busted exchanges
	if (BustedXchList().size())
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << " Incorrect Exchange Information ";
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	    foreach(ConstQsoPtr q, BustedXchList())
	        strm << QString("%1 correct %2\n").arg(q->ToString()).arg(q->CorrectXch(), 5);
	}
	
	// report out of band qsos
	if (_rubberClockedByBandMap.size())
	{
		strm << "\n";
		strm.setFieldWidth(62);
		strm.setPadChar('*');
		strm.setFieldAlignment(QTextStream::AlignCenter);
		strm << " Rubber Clocked - 10x QSO Penalty, 10x Mult Penalty ";
		strm.setFieldWidth(0);
		strm << "\n\n";
		foreach(ConstQsoPtr q, _rubberClockedByBandMap)
			strm << QString("%1").arg(q->ToString()) << '\n';
	}
	
	// report out of band qsos
	if (_outOfBandByTimeMap.size())
	{
		strm << "\n";
		strm.setFieldWidth(62);
		strm.setPadChar('*');
		strm.setFieldAlignment(QTextStream::AlignCenter);
		strm << " Out Of Band ";
		strm.setFieldWidth(0);
		strm << "\n\n";
		foreach(ConstQsoPtr q, _outOfBandByTimeMap)
			strm << QString("%1").arg(q->ToString()) << '\n';
	}
	
	// report band change violations
	if (BandChangeViolationList().size() && (IsMultiTwo() || IsMultiSingle()))
	{
		strm << "\n";
		strm.setFieldWidth(62);
		strm.setPadChar('*');
		strm.setFieldAlignment(QTextStream::AlignCenter);
		strm << " Band Change Violations ";
		strm.setFieldWidth(0);
		strm << "\n\n";
		foreach(ConstQsoPtr q, BandChangeViolationList())
			strm << QString("%1").arg(q->ToString()) << '\n';
	}
	
	// report time violations
	if (OutOfTimeList().size() && gContest->ContestType() != "CQWW")
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << " Time Violations ";
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	    foreach(ConstQsoPtr q, OutOfTimeList())
	        strm << QString("%1 %2").arg(q->ToString()).arg(q->IsMultStn() ? "Mult Stn" : " Run Stn") << '\n';
	}
	
	// report uniques
	if (UniList().size())
	{
		QString outStr;
		foreach(ConstQsoPtr q, UniList())
			if (q->Status() == NO_LOG)
				outStr += q->ToString() + '\n';

		if (outStr.size())
		{
			strm << "\n";
			strm.setFieldWidth(62);
			strm.setPadChar('*');
			strm.setFieldAlignment(QTextStream::AlignCenter);
			strm << " Unique Calls Receiving Credit (not removed) ";
			strm.setFieldWidth(0);
			strm << "\n\n";
			strm << outStr << endl;
		}
	}
	
	// report lost mults
	if (LostMultList().size())
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << " Lost Multipliers ";
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	
	    foreach(LostMult lm, LostMultList())
	    {
	        ConstQsoPtr q = lm.QPtr();
	        QString	lostMultType = lm.MultType();
	        QString reason;
	
	        if (q->IsOutOfTime())
	            reason = "Time violation";
	
	        if (q->Status() == NOT_IN_LOG)
	            reason = "Not in log";
	
	        if (q->Status() == BUSTED_EXCHANGE || q->Status() == BUSTED_REVLOG_EXCHANGE)
	        {
	            if (q->Matched() != 0)
	                reason = QString("correct %1 %2").arg(q->Matched()->XchSent(), 5).arg(q->Matched()->XchSent2(), -5);
	            else
	                reason = "Incorrect exchange";
	        }

	        if (q->Status() == BUSTED_CALL || q->Status() == BUSTED_REVLOG_CALL || q->Status() == BUSTED_LIST_CALL || q->Status() == BUSTED_FCC_CALL)
	        {
	            if (!q->CorrectCall().isEmpty())
	                reason = QString("correct %1").arg(q->CorrectCall(), -6);
	            else
	                reason = "incorrect call";
	        }
	
			if (q->Status() == OUT_OF_BAND)
				reason = "Out of band";
	
			if (q->Status() == BAND_CHANGE_VIOLATION)
				reason = "Band change violation";
	
			strm << QString("%1 %2 %3\n")
	            .arg(q->ToString())
	            .arg(reason)
	            .arg(lm.MultType());
	    }
	}

	// write multipliers by contest type
	WriteRptMults(strm);
	
	// report stations that busted my call
	if (BustCausedList().size())
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << QString(" Stations Copying %1 Incorrectly ").arg(Call());
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	
	    foreach(ConstQsoPtr q, BustCausedList())
	        strm << QString("%1\n").arg(q->Matched()->ToString());
	}
	
	// report stations that received a NIL
	if (NilCausedList().size())
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << QString(" Stations Receiving Not In Log From %1 ").arg(Call());
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	
	    foreach(ConstQsoPtr q, NilCausedList())
	        strm << QString("%1\n").arg(q->ToString());
	}
	
	// report stations that busted my exchange
	if (BustedXchCausedList().size())
	{
	    strm << "\n";
	    strm.setFieldWidth(62);
	    strm.setPadChar('*');
	    strm.setFieldAlignment(QTextStream::AlignCenter);
	    strm << QString(" Stations Copying %1 Exchange Incorrectly ").arg(Call());
	    strm.setFieldWidth(0);
	    strm << "\n\n";
	
	    foreach(QsoPtr q, BustedXchCausedList())
	        strm << QString("%1 correct %2\n").arg(q->ToString()).arg(q->CorrectXch());
	}

	QString line = QString("%1% %2 copied incorrectly %3 times in %4 logs checked")
		.arg(PercentBustCaused(), 5, 'f', 1)
		.arg(Call(), -12)
		.arg(BustCausedList().size(), 3)
		.arg(CheckableCount(), 4);
}

void Log::WriteMultFile()
{
    QString mulFileName = gContest->MulDir().absoluteFilePath(FileInfo().baseName() + ".mul");
    QFile mulFile(mulFileName);
	mulFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream strm(&mulFile);
    WriteMultFile(strm);
}

void Log::WriteLgFile()
{
	QString lgFileName = gContest->LgDir().absoluteFilePath(FileInfo().baseName() + ".lg");
    QFile lgFile(lgFileName);
    if (!lgFile.open(QIODevice::WriteOnly | QIODevice::Text))
        gStdErrStrm << "Could not open " << lgFileName << endl;

    QTextStream strm(&lgFile);
	WriteLg(strm);
}

void Log::WriteDupeFile()
{
	QString dupeFileName = gContest->DupeDir().absoluteFilePath(FileInfo().baseName() + ".dup");
	QFile dupeFile(dupeFileName);
	QTextStream dupeStrm(&dupeFile);
	dupeFile.open(QIODevice::WriteOnly | QIODevice::Text);

	foreach(QsoPtr q, _allQso)
		if (q->IsDupe())
			dupeStrm << q->ToString() << endl;

	dupeFile.close();
}

void Log::WriteLg(QTextStream & strm)		// generic output
{
    static
    QString hdr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14")
        .arg("Freq"			)
        .arg("Mode"			)
        .arg("Date"			)
        .arg("Time"			)
        .arg("Call Sent"	)
        .arg("Xch Sent"		)
        .arg("Call Rcvd"	)
        .arg("Xch Rcvd"		)
        .arg("Cty Px"		)
        .arg("Pts"			)
        .arg("Status"		)
        .arg("Run/S/P"		)
        .arg("Uni"			)
        .arg("Mult"			);

    strm << hdr << endl;

    foreach(QsoPtr q, _allQso)
    {
        QString outStr = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n")
            .arg(q->ToTabString())
            .arg(q->CtyRcvd()->StdPx())
            .arg(q->Pts())
            .arg(q->StatusChar())
            .arg(q->Running())
            .arg(q->IsUnique() ? "U" : "")
            .arg(q->IsMult() ? q->StdPx() : ((q->IsLostMult()) ? "L" : ""));
        strm << outStr;
    }
}

void Log::CheckDupes()
{
    foreach(QString call, _qsoListByCallBandMap.keys())						// each call
    {
        foreach(QString band, _qsoListByCallBandMap.value(call).keys())		// each band
        {
			// all qsos are intialized as dupes 
			// either the first good qso, or the first qso is not a dupe
			
			QsoList qList = _qsoListByCallBandMap.value(call).value(band);
			QsoPtr me(0);
			foreach(Qso * q, qList)
			{
				if (q->Status() == GOOD_QSO && !me) // no dupe here!
					me = q;
				else
					q->IsDupe(true);
			}

			if (me)
			{
				foreach(auto q, qList)
				{
					if (q != me)
						q->IsEarlyDupe(true);
					else
						break;
				}
			}
			else
				qList.first()->IsDupe(false);
        }
    }
}

void Log::WriteSptFiles()
{
static QString const sptFileHdr =
	"Q#\t"
	"Min\t"
	"Mode\t"
	"Date\t"
	"Time\t"
	"Log of\t"
	"worked\t"
	"times claimed\t"
	"RLM\t"
	"Mult1\t"
	"Mult2\t"
	"Status\t"
	"Exact QRG\t"
	"Exact RL QRG\t"
	"S&P\t"
	"My QTR\t"
	"His QTR\t"
	"QTR diff\t"
	"QTR Spot\t"
	"QTR Spot diff\t"
	"QTR My Spot\t"
	"QTR My Spot diff\t"
	"Matched\t"
	"S&P status used\t"
	"My QRG\t"
	"My Spot QRG\t"
	"My Computed S&P\t"
	"My Computed QRG\t"
	"His QRG\t"
	"His Computed S&P\t"
	"His Computed QRG\t"
	"Spot15 QRG\t"
	"Spot45 QRG\t"
	"5\t"
	"10\t"
	"15\t"
	"30\t"
	"45\t"
	"Uni\t"
	"1st\t"
	"I\t"
	"NewFreq\t"
	"rig\t"
	"Unq\t"
	"QTR Next spot\t"
	"QTR next spot diff\t"
	"Next spot QRG\t"
	"His Last Exact QRG\t"
	"His Last Exact QTR\t"
	"His Next Exact QRG\t"
	"His Next Exact QTR\t"
	"His Last QTR\t"
	"His Next QTR\t"
	"His 20 min S&P\t"
	"My Rate\t"
	"His Rate\t"
	"His QSOs\t"
	"His Band Spots\t"
	"His All spots"
	"\n";

	QDir sptDir(gContest->SptDir());
	QString sptFileName = sptDir.absoluteFilePath(_fileInfo.baseName() + ".spt");
	QFile sptFile(sptFileName);
	if(!sptFile.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	
	QTextStream sptStrm( &sptFile);
	sptStrm << sptFileHdr;

	for(auto it = _allQso.begin(); it != _allQso.end(); ++it)
	{
		QsoPtr q(*it);

		bool	isHit(false);
		bool	firstDay(false);
		bool	snp(false);
		bool	run(false);
		int 	count_5(0);
		int 	count_10(0);
		int 	count_15(0);
		int 	count_30(0);
		int 	count_45(0);
		QString deltaSpotQtr("");
		QString deltaMySpot45Qtr;
		QString deltaQtr;
		QString	spot15Qrg;
		QString	spot45Qrg;
		QString mySpot45Qrg;
		QString	bandSpot;
		QString	dateTimeSpot;
		QString	callSpot;
		QString	commentSpot;
		QString	spotterSpot;
		QString spot15Time;
		QString spot45Time;
		QString mySpot45Qtr;
		QString snpStatus("U");
		QString qtrNextSpot;
		QString qtrNextSpotDiff;
		QString nextSpotQrg;
		QString hisLastExactQrg;
		QString hisLastExactQtr;
		QString hisNextExactQrg;
		QString hisNextExactQtr;
		QString hisLastQtr;
		QString hisNextQtr;
		QString	his20MinSpRate;
		QString	hisQsoCount;
		QString	hisBandSpotCount;
		QString	hisAllBandSpotCount;

		// three ways to detect run/s&p

	    snpStatus = q->Running();

	    if(snpStatus != "P" && !q->IsExactQrg())
	    {
			if(q->Matched())
			{
	            if(q->Matched()->IsExactQrg())
	            {
	                if(q->Matched()->Running().contains("R"))
	                    snpStatus = "S";
	                else
	                    if(q->Matched()->Running().contains("S"))
	                        snpStatus = "R";
	            }
	            else
	            {
	                if(q->RlRunning().contains("S") && q->Matched()->RlRunning().contains("R"))
	                    snpStatus = "S";
	                else
	                    if(q->RlRunning().contains("R") && q->Matched()->RlRunning().contains("S"))
	                        snpStatus = "R";
	                    else
	                        snpStatus = "U MRL";
	            }
	        }
	    }
	
	    snp = snpStatus == "S";
	    run = snpStatus == "R";
	
	    q->IsMyRun(run);
	
	    if(q->Matched())
	    {
	        int secsDiff = q->DateTime().secsTo(q->Matched()->DateTime());
	        int hrsDiff = secsDiff / 3600;
	        secsDiff %= 3600;
	        int minsDiff = secsDiff / 60;
	        int diffTime = 100 * hrsDiff + minsDiff;
	        deltaQtr = QString("%1").arg(diffTime);
	        hisQsoCount = QString::number(q->Matched()->GetLogPtr()->LogSize());
	    }
	    else
	    {
	        size_t size = gContest->RevLogMap().at(q->CallRcvd()).AllQso().size();
	        hisQsoCount = QString::number(size);
	    }
		
		// search his log if we have it, otherwise his reverse log 
		std::vector<Qso *> const & qsos = q->Matched() ? q->Matched()->GetLogPtr()->AllQso() : gContest->RevLogMap().at(q->CallRcvd()).AllQso();
		
		// find his 20 minute s&p rate
		int snpCnt(0);
		for (std::vector<Qso *>::const_iterator it = std::lower_bound(qsos.begin(), qsos.end(), q->DateTime().addSecs(-600), gQsoTimeLessThanTime); it != qsos.end() && q->DateTime().secsTo((*it)->DateTime()) <= 600; it++)
			if (!(*it)->IsMyRun())
				snpCnt++;
		his20MinSpRate = QString::number(snpCnt);
	
		int count(0);	
		// find last exact qrg in his log by searching backwards. Limit search to 5 qsos
		for (auto rit(std::vector<Qso *>::const_reverse_iterator(std::upper_bound(qsos.begin(), qsos.end(), q, gQsoTimeLessThanQsoTime))); rit != qsos.rend() && count < 5; ++rit, ++count)
		{
			if (hisLastQtr.isEmpty())
				hisLastQtr = (*rit)->DateTime().toString("hhmm");
	
			if ((*rit)->IsExactQrg())
			{
				hisLastExactQrg = QString::number((*rit)->Qrg());
				hisLastExactQtr = (*rit)->DateTime().toString("hhmm");
				break;
			}
		}
	
		count = 0;		
		// find next exact qrg in his log by searching forwards. Limit search to 5 qsos
		for (std::vector<Qso *>::const_iterator exactIt = std::lower_bound(qsos.begin(), qsos.end(), q, gQsoTimeLessThanQsoTime); exactIt != qsos.end() && count < 5; ++exactIt, ++count)
		{
			if (hisNextQtr.isEmpty())
				hisNextQtr = (*exactIt)->DateTime().toString("hhmm");
	
			if ((*exactIt)->IsExactQrg())
			{
				hisNextExactQrg = QString::number((*exactIt)->Qrg());
				hisNextExactQtr = (*exactIt)->DateTime().toString("hhmm");
				break;
			}
		}
	
	    // compute total spots for him
		QMap <QString, QMap <QString, std::vector <Spot *> > > sptMaps = gContest->SpotsByBandCallTimeMap();

		size_t hisTotalSpotCount(0);
	    foreach(QString band, sptMaps.keys())
	        if(sptMaps.value(band).contains(q->CallRcvd()))
	            hisTotalSpotCount += sptMaps.value(band).value(q->CallRcvd()).size();
	
	    hisAllBandSpotCount = QString::number(hisTotalSpotCount);		// total spots for this guy
	
	    int sptWinSize = 45 * 60;										// 45 minutes

		if(sptMaps.value(q->Band()).value(q->CallRcvd()).size())		// seen at least once
	    {
			std::vector<Spot *> const & spots = sptMaps.value(q->Band()).value(q->CallRcvd());
	        hisBandSpotCount = QString::number(spots.size());

			// work backwards in time through list of spots for this call
			for (auto rit(std::vector<Spot *>::const_reverse_iterator(std::upper_bound(spots.begin(), spots.end(), q->DateTime().addSecs(120), gTimeLessThanSpotTime))); rit != spots.rend(); rit++)
			{
				Spot * sptPtr = *rit;
				bool spot15Found(false);
				bool spot45Found(false);

				//positive if spot time before QSO, negative after
				int deltaSecs(sptPtr->DateTime().secsTo(q->DateTime()));
				int deltaMins = deltaSecs / 60;

				if (deltaSecs > sptWinSize)								// if beyond spot window
					break;												// no need to check more

				if (!spot15Found && deltaMins <= 15 && deltaMins >= -2)	// in the +15/-2 minute window
				{
					spot15Found = true;
					isHit = true;
					q->SpotHit(sptPtr);

					spot15Qrg = QString("%1").arg(sptPtr->Qrg(), 0, 'f', 1);
					spot15Time = sptPtr->DateTime().toString("hhmm");
					deltaSpotQtr.setNum(deltaMins);

					// if there is a spot later, report it
					if (rit != spots.rbegin())
					{
						Spot * nextSpt = *(rit - 1);
						qtrNextSpot = nextSpt->DateTime().toString("HHmm");
						int deltaNextSecs = nextSpt->DateTime().secsTo(q->DateTime());
						int deltaNextMins = deltaNextSecs / 60;
						qtrNextSpotDiff.setNum(deltaNextMins);
						nextSpotQrg = QString("%1").arg(nextSpt->Qrg(), 0, 'f', 1);
					}
				}

				// within the 45 min window
				if (!spot45Found && deltaMins <= 45 && deltaMins >= 0)
				{
					spot45Found = true;
					spot45Qrg = QString("%1").arg(sptPtr->Qrg(), 0, 'f', 1);
					spot45Time = sptPtr->DateTime().toString("hhmm");
				}

				// counts for spots within the window
				if (spot45Found)
				{
					if (deltaMins <= 5) count_5++;
					if (deltaMins <= 10) count_10++;
					if (deltaMins <= 15) count_15++;
					if (deltaMins <= 30) count_30++;
					if (deltaMins <= 45) count_45++;
				}
			}
	    }
	
		if(sptMaps.contains(q->Band()) && sptMaps.value(q->Band()).contains(_call)) // spots of me
	    {
			bool mySpot45Found(false);

			std::vector <Spot *> const spots = sptMaps.value(q->Band()).value(_call);

			// work backwards in time through list of spots for this call
			for (auto rit(std::vector<Spot *>::const_reverse_iterator(std::upper_bound(spots.begin(), spots.end(), q->DateTime().addSecs(60), gTimeLessThanSpotTime))); rit != spots.rend(); rit++)
			{
				Spot * sptPtr = *rit;
				int deltaSecs = sptPtr->DateTime().secsTo(q->DateTime());
				int deltaMins = deltaSecs / 60;

				// within spot window
				if(deltaSecs > sptWinSize)							
	                break;
	
				// I was spotted in the last 45 minutes
	            if(!mySpot45Found && deltaMins <= 45)	 			
	            {
                    mySpot45Found = true;
                    mySpot45Qrg = QString("%1").arg(sptPtr->Qrg(), 0, 'f', 1);
                    mySpot45Qtr = sptPtr->DateTime().toString("hhmm");
                    deltaMySpot45Qtr.setNum(deltaMins);
	            }
	        }
	    }
	
	    firstDay = (gContest->StartTime().secsTo(q->DateTime()) < 86400);
	    _qsoCount++;
	    firstDay ? _qsoCountDay1++ :  _qsoCountDay2++;
	
	    if(q->IsMult() || q->IsMult2() || q->IsMult3())
	    {
	        _multCount++;
	        firstDay ? _multCountDay1++ :  _multCountDay2++;
	    }
	
	    if(isHit)
	    {
	        _hitCount++;
	        firstDay ? _hitCountDay1++ :  _hitCountDay2++;
	    }
	
	    if((q->IsMult() || q->IsMult2() || q->IsMult3()) && isHit)
	    {
	        _multHitCount++;
	        firstDay ? _multHitCountDay1++ :  _multHitCountDay2++;
	    }
	
	    if(snp)
	    {
	        _snpCount++;
	        firstDay ? _snpCountDay1++ :  _snpCountDay2++;
	
	        if(q->IsMult() || q->IsMult2() || q->IsMult3())
	        {
	            _snpMultCount++;
	            firstDay ? _snpMultCountDay1++ : _snpMultCountDay2++;
	        }
	
	        if(isHit)
	        {
	            _snpHitCount++;
	            firstDay ? _snpHitCountDay1++ : _snpHitCountDay2++;
	        }
	
	        if((q->IsMult() || q->IsMult2() || q->IsMult3()) && isHit)
	        {
	            _snpMultHitCount++;
	            firstDay ? _snpMultHitCountDay1++ : _snpMultHitCountDay2++;
	        }
	    }
	
	    if(run)
	    {
	        _runCount++;
	        firstDay ? _runCountDay1++ :  _runCountDay2++;
	    }
	
	    if(snpStatus.startsWith("U"))
	    {
	        _unknownCount++;
	        firstDay ? _unknownCountDay1++ :  _unknownCountDay2++;
	    }
	
	    if(q->Running() == "P")
	    {
	        _passCount++;
	        firstDay ? _passCountDay1++ : _passCountDay2++;
	    }
	
	    QString rlm = "";
	    if(q->Matched())
	        rlm = q->Matched()->IsMult() ? "M" : "";
	
		sptStrm <<  QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t"
	                        "%10\t%11\t%12\t%13\t%14\t%15\t%16\t%17\t%18\t%19\t"
	                        "%20\t""%21\t%22\t%23\t%24\t%25\t%26\t%27\t%28\t%29\t"
	                        "%30\t%31\t%32\t%33\t%34\t%35\t%36\t%37\t%38\t%39\t"
	                        "%40\t%41\t%42\t%43\t%44\t%45\t%46\t%47\t%48\t%49\t"
	                        "%50\t%51\t%52\t%53\t%54\t%55\t%56\t%57\t%58\t%59\n")
	    .arg(_qsoCount)
	    .arg(gContest->StartTime().secsTo(q->DateTime()) / 60)
	    .arg(q->Mode())
	    .arg(q->DateStr())
	    .arg(q->TimeStr())
	    .arg(q->CallSent())
	    .arg(q->CallRcvd())
	    .arg(gContest->RevLogMap().at(q->CallRcvd()).AllQso().size())
	    .arg(rlm)
	    .arg(q->IsMult() ? q->StdPx() : "")
		.arg(q->IsMult2() ? q->XchRcvd() : "")
		.arg(q->StatusChar())
	    .arg(q->IsExactQrg() ? 'Y' : 'N')
	    .arg(q->Matched() ? (q->Matched()->IsExactQrg() ? 'Y' : 'N') : ' ')
	    .arg(q->Running())
	    .arg(q->DateTime().toString("HHmm"))
	    .arg(q->Matched() ? q->Matched()->DateTime().toString("HHmm") : "")
	    .arg(deltaQtr)
	    .arg(spot15Time)
	    .arg(deltaSpotQtr)
	    .arg(mySpot45Qtr)
	    .arg(deltaMySpot45Qtr)
	    .arg(q->Matched() ? "Y" : "N")
	    .arg(snpStatus)
	    .arg(q->Qrg())
	    .arg(mySpot45Qrg)
	    .arg(q->RlRunning())
	    .arg(q->MyComputedQrg() ? QString("%1").arg(q->MyComputedQrg()) : "")
	    .arg(q->Matched() ? QString("%1").arg(q->Matched()->Qrg()) : "")
	    .arg(q->Matched() ? q->Matched()->RlRunning() : "U")
	    .arg((q->Matched() && q->IsExactQrg()) ? QString("%1").arg(q->Qrg()) : 0)
	    .arg(spot15Qrg)
	    .arg(spot45Qrg)
		.arg(count_5  ? QString::number(count_5)  : QString())
		.arg(count_10 ? QString::number(count_10) : QString())
		.arg(count_15 ? QString::number(count_15) : QString())
		.arg(count_30 ? QString::number(count_30) : QString())
		.arg(count_45 ? QString::number(count_45) : QString())
		.arg(q->SpotHit() && q->SpotHit()->IsUnique()	? "*" : QString())
		.arg(q->SpotHit() && q->SpotHit()->First()		? "*" : QString())
		.arg(q->SpotHit() && q->SpotHit()->Isolated()	? "*" : QString())
		.arg(q->SpotHit() && q->SpotHit()->NewFreq()	? "*" : QString())
	    .arg((IsMultiSingle() || IsMultiTwo()) ? (q->IsMultStn() ? "1" : "0") : " ")
	    .arg(q->IsUnique() ? "U" : "")
	    .arg(qtrNextSpot)
	    .arg(qtrNextSpotDiff)
	    .arg(nextSpotQrg)
	    .arg(hisLastExactQrg)
	    .arg(hisLastExactQtr)
	    .arg(hisNextExactQrg)
	    .arg(hisNextExactQtr)
	    .arg(hisLastQtr)
	    .arg(hisNextQtr)
	    .arg(his20MinSpRate)
	    .arg(QString::number(q->Rate20()))
	    .arg(q->Matched() ? QString::number(q->Matched()->Rate20()) : QString())
	    .arg(hisQsoCount)
	    .arg(hisBandSpotCount)
	    .arg(hisAllBandSpotCount);
	}
	sptFile.close();
}

void Log::ComputeOperatingTime()
{
    _operatingTime = 0;
    time_t lastTime = 0;

    foreach(QsoPtr q, _allQso)
    {
		if (CategoryBand() != "ALL" && CategoryBand() != q->Band())			// single band logs on wrong band
			continue;

		if(lastTime != 0 && q->TimeInSeconds() < (_offtimeThreshold + lastTime))
            _operatingTime += q->TimeInSeconds() - lastTime;

        if(_operatingTime > _maxOperatingTime)
        {
            q->IsOutOfTime(true);
            AppendOutOfTimeList(q);
        }

        lastTime = q->TimeInSeconds();
		if(!q->IsOutOfTime())
			_finalQsoDateTime = q->DateTime();
    }
}

void Log::CheckMSBandChanges(int maxQsy)							// ARRL band changes per hour style
{
    if(_doNotCheck)
        return;

    if( ContestPtr()->FillRunMultMap().contains(_call))
    {
        QString value = ContestPtr()->FillRunMultMap().value(_call);
        if(value == "YES")
            _fillRunMult = true;
    }

    if(LogSize() < 2)
        return;

    int count(0);
    QString band;
    int hour(-1);

    foreach(QsoPtr q, _allQso)
    {
        int currentHour = ContestPtr()->StartTime().secsTo(q->DateTime()) / 3600;
        if(currentHour != hour)
        {
            hour = currentHour;
            count = 0;
            band = q->Band();
            continue;
        }

        if(band != q->Band())
        {
            if(count == maxQsy)
            {
                if(q->Status() == GOOD_QSO || q->Status() == NO_LOG || !q->IsExcluded())
                    q->Status(BAND_CHANGE_VIOLATION);
			}
            else
            {
                count++;
                band = q->Band();
                q->IsBandChange(true);
            }
        }
    }
}

void Log::CheckM2BandChanges(int const maxQsy)
{
    if(LogSize() <= maxQsy + 1)
        return;

    if(_doNotCheck)
        return;

    if(ContestPtr()->FillRunMultMap().value(_call) == "NO")
        return;

    if(ContestPtr()->FillRunMultMap().value(_call) == "YES" || gRunAutoFill)
        _fillRunMult = true;

	int  stn0count = 0;
    int  stn1count = 0;

    if(!_fillRunMult)
    {
        foreach(QsoPtr q, _allQso)						// test for any marking
        {
             if(q->IsExcluded())
                 continue;
            if(q->IsMultStn())
                stn1count++;
        else
            stn0count++;
        }
    }

    if(!stn0count || !stn1count || gRunAutoFill)
        _fillRunMult = true;

    if(_fillRunMult)
        FillM2BandChanges(maxQsy);

    // This does the actual checking and marking

    QsoList stnOneLog;
    QsoList stnTwoLog;

    foreach(QsoPtr q, _allQso)			// make one log for each station
    {
         if(q->IsExcluded())
             continue;

        q->IsStnOne() ? stnOneLog.append(q) : stnTwoLog.append(q);
    }

    QList <QsoList> stnLogs;
    stnLogs << stnOneLog << stnTwoLog;

    int count = 0;
    QString band;
    int hour = INT_MAX;

    foreach(QsoList stnLog, stnLogs)
    {
        foreach(QsoPtr q, stnLog)
        {
            int currentHour = (q->DateTime().toTime_t() - ContestPtr()->StartTime().toTime_t()) / 3600;
            if(currentHour != hour)
            {
                hour  = currentHour;
                count = 0;
                band  = q->Band();
                continue;
            }

            if(band != q->Band())
            {
                if(count == maxQsy)
                {
//					ifCountsForCredit()
                        q->Status(BAND_CHANGE_VIOLATION);
                }
                else
                {
                    count++;
                    band = q->Band();
                    q->IsBandChange(true);
                }
            }
        }
    }
}

void Log::FillM2BandChanges(int maxQsy)
{
	if(_dontFillRunMult)
		return;

    QMap <int, QsoList> qsoByHour;
    foreach(QsoPtr q, _allQso)								// create a log per hour
    {
        if(q->IsExcluded())
             continue;

        int currentHour = ContestPtr()->StartTime().secsTo(q->DateTime()) / 3600;
        qsoByHour[currentHour].append(q);
    }

    if(LogSize() < maxQsy)
        return;

    QString runBand;
    QString multBand;

    int  runStnCount =  0;
    int  multStnCount = 0;
    foreach(QsoPtr q, _allQso)
    {
        if(q->IsExcluded())
             continue;

        if(q->IsDupe())
            continue;

        if(q->IsMultStn())
            multStnCount++;
        else
            runStnCount++;
    }

    if(_fillRunMult || !multStnCount || !runStnCount || gRunAutoFill)
        _fillRunMult =  true;

    int  runCount = 0;
    int  multCount = 0;

    int hour = INT_MAX;
    QsoList hourLog;

    for(int i = 0; i < _allQso.size() && _fillRunMult; i++)	   // assign tx numbers because submitted file did not
    {
        QsoPtr q = _allQso[i];

        if(q->IsExcluded())
             continue;

        int currentHour = ContestPtr()->StartTime().secsTo(q->DateTime()) / 3600;

        if(currentHour != hour)
        {
            hourLog.clear();
            hour = currentHour;
            runCount = 0;
            multCount = 0;

            for (int j = i + 1; j < _allQso.size(); j++)								   // create a log for this hour
            {
                Qso * qLoop = _allQso[j];
                if(currentHour == ContestPtr()->StartTime().secsTo(qLoop->DateTime()) /  3600)
                    hourLog.append(qLoop);
                else
                    break;
            }

            if(hourLog.size() == 0)
                continue;

            QListIterator<QsoPtr> hourIt(hourLog);

            runBand = hourLog.first()->Band();		// find intital stn0 and stn1 bands this hour
            while(hourIt.hasNext())
            {
                QsoPtr q = hourIt.next();
                if(q->Band() != runBand)
                {
                    multBand = q->Band();
                    break;
                }
            }

            hourIt.toFront();
            QListIterator <QsoPtr> searchHourIt(hourLog);
            while(hourIt.hasNext())
            {
                QsoPtr q = hourIt.next();

                if(q->Band() == runBand)
                {
                    q->IsMultStn(false);
                    continue;
                }

                if(q->Band() == multBand)
                {
                    q->IsMultStn(true);
                    continue;
                }

                // new band

				if (runCount < multCount)
                {
                    runBand = q->Band();
                    q->IsMultStn(false);
                    runCount++;
                    continue;
                }
				else
				{
					multBand = q->Band();
					q->IsMultStn(true);
					multCount++;
					continue;
				}

			}
        }
    }

    QsoList stnOneLog;
    QsoList stnTwoLog;

    foreach(QsoPtr q, _allQso)
    {
        if(q->IsExcluded())
             continue;

        if(q->IsStnOne())
            stnOneLog.append(q);
        else
            stnTwoLog.append(q);
    }

    QList <QsoList> stnLogs;
    stnLogs << stnOneLog << stnTwoLog;

    int count = 0;
    QString band;
    hour = INT_MAX;

    foreach(QsoList stnLog, stnLogs)
    {
        foreach(QsoPtr q, stnLog)
        {
            if(q->IsExcluded())
                 continue;

            int currentHour = (q->DateTime().toTime_t() - ContestPtr()->StartTime().toTime_t()) / 3600;
            if(currentHour != hour)
            {
                hour  = currentHour;
                count = 0;
                band  = q->Band();
                continue;
            }

            if(band != q->Band())
            {
                if(count == maxQsy)
                {
                    if(q->CountsForCredit())
                        q->Status(BAND_CHANGE_VIOLATION);
                }
                else
                {
                    count++;
                    band = q->Band();
                    q->IsBandChange(true);
                }
            }
        }
    }
}

void Log::LoadCabrilloFile(TokenDataPairList tokenDataPairMap)
{
	// This runs muti threaded
	int  lineNum(0);
	InitCabrilloMap();
	QString token;
	QString data;
	QList <QsoLineData> qsoLineDataList;

	_categoryOperator		= _categoryOperator.toUpper();
	_categoryTransmitter	= _categoryTransmitter.toUpper();
	_categoryBand			= _categoryBand.toUpper();
	_categoryPower			= _categoryPower.toUpper();
	_categoryAssisted		= _categoryAssisted.toUpper();
	_categoryStation		= _categoryStation.toUpper();

	foreach(TokenDataPair pair, tokenDataPairMap)
	{
		token = pair.first;
		data = pair.second;

		lineNum++;

		// _cabrilloTokenMap holds the destinations for most input
		// some things, like category, need a little more processing
		// data has already been simplified and upper cased when loading line

		if (token.endsWith("QSO"))
		{
			QRegularExpression re("[^A-Z0-9/\\-. ]");		// letters, numbers, slash, hyphen, space and period allowed in qso line
			data = data.toUpper().remove(re);				// qso lines are all upper case
			bool excluded = token.startsWith('X');
			bool outOfBand = token.startsWith('O');
			bool wrongBand = token.startsWith('Y');
			bool rubberClocked = token.startsWith('R');
			qsoLineDataList.append(QsoLineData(data, lineNum, excluded, outOfBand, wrongBand, rubberClocked));
		}

		if (token == "HQ-DATE-RECEIVED")
		{
			data.remove(" UTC");
			data.remove(" GMT");
			QDateTime logRcvdTime;
			logRcvdTime.setTimeSpec(Qt::UTC);
			logRcvdTime = QDateTime::fromString(data, "yyyy-MM-dd hh:mm:ss");
			logRcvdTime.setTimeSpec(Qt::UTC);

			if (gContest->CertCutoffTime().isValid() && logRcvdTime >= gContest->CertCutoffTime())
				_submittedLate = true;
		}

		if (token == "CQZONE")
		{
			_cqZone = data.toUpper();
			continue;
		}

		if (token == "CONTINENT")
		{
			_cont = data.toUpper();
			continue;
		}

		if (token == "LOCATION")
		{
			_location = data.toUpper();

			if (!IsWVe())
				_location = "DX";

			continue;
		}

		if (token == "DISTRICT")
		{
			_district = data;
			continue;
		}

		if (token == "SOAPBOX")										// don't upper case soapbox stuff
		{
			QRegularExpression emailLabelRe("E-MAIL:|EMAIL:");					// remove email addresses
			emailLabelRe.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
			data.remove(emailLabelRe);
			data = data.trimmed();

			QRegularExpression emailRe("([\\w\\-_.]+@[\\w\\-_.]+)");
			data.remove(emailRe);

			if (!_soapBox.isEmpty())
				_soapBox += " ";
			_soapBox += data;
			_soapBox = _soapBox.simplified();
			continue;
		}

		if (token == "ADDRESS")
		{
			_addressStreet = data;
			continue;
		}

		if (token == "CLUB")
		{
			QString clubLine = data.toUpper().simplified();
			if (clubLine.contains(QRegularExpression("\\b(NONE)\\b")))
				continue;
			if (_club.isEmpty())
				_club = clubLine;
			else
				_club += " | " + clubLine;

			continue;
		}

		if (token == "CLAIMED-SCORE")
		{
			data.remove(",");
			_claimedScore = data;
			QRegularExpression puntosRe("puntos\\.?");
			_claimedScore.remove(puntosRe);
			_claimedScore = _claimedScore;
			continue;
		}

		if (token == "HQ-FROM")
		{
			_hqFrom = data;
			QRegularExpression re("<(.+)>");
			QRegularExpressionMatch m = re.match(_hqFrom);
			if (m.hasMatch())
				_hqFrom = m.captured(1);
			continue;
		}

		if (token.contains("EMAIL|E-MAIL"))
		{
			_email = data;
			QRegularExpression re("<(.+)>");
			QRegularExpressionMatch m = re.match(_hqFrom);
			if (m.hasMatch())
				_email = m.captured(1);
			continue;
		}

		if (token == "OPERATORS") 
		{
			data = data.simplified().toUpper();
			data.remove(QRegularExpression("[^\\w\\s,-@]")); // get rid of garbage
			data.replace('@', " @"); // insert space in K1GQ@K1GQ
			_operators += data + ' ';
			continue;
		}

		if (token == "HQ-CATEGORY") 
		{ 
			_hqCategory = data.simplified();			
			continue; 
		} // not always upper case

		if (_cabrilloTokenMap.contains(token))
			*_cabrilloTokenMap.value(token) = data.simplified().toUpper();
	}

	if (IsWVe() && _location.isEmpty())
	{
		if (IsW())	// get state from fcc data, if needed
		{
			_location = gStateByFccCallMap.value(_call);
			AppendErrMsgList(_call, QString("Missing location field, using %1 from fcc.dat").arg(_location));
		}
		else     // get province from cqww.dat, if needed
		{
			_location = _px->Province();
			AppendErrMsgList(_call, QString("Missing location field, using %1 from cqww.dat").arg(_location));
		}
	}

	_mailCert  = _mailCert.startsWith("Y") ? true : false;

	qsoLineDataList.size();

	_soapBox.remove(QRegularExpression("\\.+$"));
	_soapBox = _soapBox.trimmed();

	if (_district.isEmpty())									// DISTICT: in log header takes precedent
	{
		QRegularExpression callRe("[A-Z]+(\\d)");				// district directly from call
		QRegularExpressionMatch m = callRe.match(_rawCall);

		if (m.hasMatch())
		{
			// Try to figure out what call district they are in
			_district = m.captured(1);

			QRegularExpression portRe("/(\\d)$");							// explicit portable wins
			QRegularExpressionMatch portMatch = portRe.match(_rawCall);

			if (!portMatch.hasMatch())
			{
				if (IsWVe())
				{
					// translate state aliases to states before lookup in _districtByStateMap
					QString location = gStateAbbreviationMap.value(_location);
					if (IsW() && !_districtByStateMap.count(location))						// get state from fcc data, if needed
						_location = gStateByFccCallMap.value(_call);

					if (_districtByStateMap.count(location) && _districtByStateMap[location] != _district)
					{
						_isPortable = true;
						QString districtFromCall = _district;
						_district = _districtByStateMap[location];
						_portDigit = _district;
					}

					if (_districtByStateMap.count(location))			// get district by state or province
						_district = _districtByStateMap[location];
				}
				else
				{
					if (_cty == JA_id && _rawCall.contains(QRegularExpression("^7[KLMN]")))	 // from location
						_district = "1";
				}
			}
			else
				_district = portMatch.captured(1); // always use portable single digit
	
			// trasnsform non-existent Russian districts
			if (_cty == UA_id && (_district == "5" || _district == "2"))
				_district = "3";

			if (_cty == UA_id && _district == "7")
				_district = "6";

			if (_cty == UA_id && (_district == "8" || _district == "9"))
				_district = "9";

			if (_cty == UA0_id && _district == "8")
				_district = "9";
		}
	}

	if (_categoryBand.isEmpty())
		_categoryBand = "ALL";

	_soapBox.remove('\n');
	_addressCountry = _addressCountry.toUpper();

	if (_contest->DisqualifiedSet().contains(_call))
		Category("DQ");
	else
		ComputeCategory();

	InitByContestType(); // this goes here because we need the location before running init to get zone in cqww

	int exactFreqCount(0);
	QDateTime lastDateTime;

	foreach (QsoLineData lineData, qsoLineDataList)
	{
		QsoPtr q = NewQso();
		q->ParseQsoLine(lineData);
		if (q->Status() != FORMAT_ERR && q->Status() != DOES_NOT_COUNT)
		{
			AddToQsoLists(q);
			_allQso.push_back(q);
			if (q->IsExactQrg())
				exactFreqCount++;
		}
	}

	// create opsList, guestOp and hostOp
	_opsList += _operators.split(QRegularExpression("\\s|,"), QString::SkipEmptyParts);
	_operators.clear();

	QMutableListIterator <QString> it(_opsList);
	while (it.hasNext())
	{
		QString op = it.next();
		if (op.startsWith("@"))
		{
			op = op.remove(0, 1);
			if(op != _call)
				_hostCall = op;
			it.remove();
		}
	}

	if (_opsList.size() && _opsList.first() != _call)
		_guestOp = _opsList.first();

	foreach(QString op, _opsList)					// add ops to _operators, including trailing space
		_operators += op.simplified() + " ";		// remove excess whitespace

	if (!_hostCall.isEmpty() && _hostCall != _call)
		_operators += '@' + _hostCall;
	else
		_operators = _operators.trimmed();
	
	std::stable_sort(_allQso.begin(), _allQso.end(), gQsoTimeLessThanQsoTime);
    FillBandLists();										// create a list of qsos per band
}

QsoPtr Log::FindClosestQso(QDateTime const dt, QsoList list, int window)
{
    int  minDelta = window;
    QsoPtr bestMatch = 0;

    foreach(QsoPtr q, list)
    {
        if(q->Matched())
            continue;

        int delta = abs(dt.secsTo(q->DateTime()));
        if(delta <= minDelta)
        {
            bestMatch = q;
            minDelta = delta;
            if(!delta)
                return bestMatch;
        }
    }
    return bestMatch;
}

QsoPtr Log::Find(QString call, QDateTime dt, QString band)
{
    return FindClosestQso(dt, _qsoListByCallBandMap.value(call).value(band));
}

QsoPtr Log::Find(QString call, QsoPtr q, int window, bool chkOtherBands)
{
    QsoList qList = _qsoListByCallBandMap.value(call).value(q->Band());
 
    if(!chkOtherBands)						// either check one band, or all but that band
        return FindClosestQso(q->DateTime(), qList, window);

    // check the other bands looking for the closest match in time

	QStringList bandNames = BandNameList();
    bandNames.removeAll(q->Band());

    QsoList otherBandQsoList;

    foreach(QString band, bandNames)		// make a list of closest for each band
    {
        qList = _qsoListByCallBandMap.value(call).value(band);
        QsoPtr rq = FindClosestQso(q->DateTime(), qList, window);

        if (rq)
            otherBandQsoList << rq;
    }

    // FindClosestQso finds the closest qso in time from a list of qsos -- perfect!

    return FindClosestQso(q->DateTime(), otherBandQsoList, window);
}

void Log::FillReportLists()
{
    foreach(QsoPtr q, _allQso)
    {
        if(q->IsExcluded())
            continue;

        if(q->IsDupe())			 // add dupes to the dupe list
        {
            AppendDupeList(q);
            q->Status(DOES_NOT_COUNT);
            continue;
        }

        if(!q->IsLegalCall())
            AppendInvalidCallList(q);

        switch (q->Status())
        {
        case GOOD_QSO:
            AppendGoodList(q);
            _checkableCount++;
            break;

        case NOT_IN_LOG:
            AppendNilList(q);
            _checkableCount++;
            break;

        case NO_LOG:
            AppendNotCheckedList(q);
            break;

        case FORMAT_ERR:
            break;

        case BUSTED_FCC_CALL:
        case BUSTED_CALL:
        case BUSTED_LIST_CALL:
            AppendBustedCallList(q);
            _checkableCount++;
            break;

        case BUSTED_REVLOG_CALL:
            AppendBadRevLogList(q);
            _checkableCount++;
            break;

        case BUSTED_EXCHANGE:
        case BUSTED_REVLOG_EXCHANGE:
            AppendBustedXchList(q);
            _checkableCount++;
            break;

        case BAND_CHANGE_VIOLATION:
            AppendBcvList(q);

        case DOES_NOT_COUNT:
            break;

		case RUBBER_CLOCK_VIOLATION:
			break;

        default:
            gStdErrStrm << QString("%1 BAD QSO STATUS: %2").arg(q->ToString()).arg(q->Status()) << endl;
        }
    }
}

void Log::FillTwentyMinRate()
{
	foreach(QsoList bandList, GetQsoListByBandMap())
	{
		QListIterator <QsoPtr> now(bandList);					// a sliding 20 minute window
		QListIterator <QsoPtr> future(bandList);				// this uses a leading future iterator
		QListIterator <QsoPtr> past(bandList);					// and a trailing past iterator
		int count(0);

		while (now.hasNext())
		{
			QsoPtr q = now.next();
			while (future.hasNext())							// 10 min in future
			{
				QsoPtr qf = future.peekNext();
				if (q->DateTime().secsTo(qf->DateTime()) < 600)
				{
					count++;
					future.next();
				}
				else
					break;
			}

			while (past.hasNext())
			{
				QsoPtr qp = past.peekNext();					// 10 min in past
				if (qp->DateTime().secsTo(q->DateTime()) > 600)
				{
					count--;
					past.next();
				}
				else
					break;
			}
			q->Rate20(3 * count);
		}
	}
}

bool Log::IsCertEligible() const
{ 
	return Category() != "DQ" && Category() != "CK" && !SubmittedLate() && CtyPtr() != &gEmptyCty && !IsNoAward() && !IsAdministrativeChecklog();
}

void Log::ComputeScore()
{
    ComputeOperatingTime();
	ComputePts();
	ComputeMults();
    SortLostMultList();
 
    _rawScore = _bandInfoMap["ALL"].RawPts() * (_bandInfoMap["ALL"].RawMults() + _bandInfoMap["ALL"].RawMults2() + _bandInfoMap["ALL"].RawMults3());
    _score	  = _bandInfoMap["ALL"].NetPts() * (_bandInfoMap["ALL"].NetMults() + _bandInfoMap["ALL"].NetMults2() + _bandInfoMap["ALL"].NetMults3());
}

void Log::InitCabrilloMap()
{
    _cabrilloTokenMap[ "START-OF-LOG"			]	=	&_startOfLog;
    _cabrilloTokenMap[ "HQ-DATE-RECEIVED"		]	=	&_hqDateRcvd;
    _cabrilloTokenMap[ "HQ-FROM"				]	=	&_hqFrom;
    _cabrilloTokenMap[ "HQ-CLUB-ABBR"			]	=	&_hqClubAbbr;
    _cabrilloTokenMap[ "HQ-SUBJECT"				]	=	&_hqSubject;
    _cabrilloTokenMap[ "HQ-FILE"				]	=	&_hqFile;
    _cabrilloTokenMap[ "HQ-CAT"					]	=	&_hqCat;
    _cabrilloTokenMap[ "HQ-CATEGORY"			]	=	&_hqCategory;
    _cabrilloTokenMap[ "CONTEST"				]	=	&_contestName;
    _cabrilloTokenMap[ "CATEGORY"				]	=	&_category;
    _cabrilloTokenMap[ "CATEGORY-ASSISTED"		]	=	&_categoryAssisted;
    _cabrilloTokenMap[ "CATEGORY-BAND"			]	=	&_categoryBand;
    _cabrilloTokenMap[ "CATEGORY-MODE"			]	=	&_categoryMode;
    _cabrilloTokenMap[ "CATEGORY-OPERATOR"		]	=	&_categoryOperator;
    _cabrilloTokenMap[ "CATEGORY-POWER"			]	=	&_categoryPower;
    _cabrilloTokenMap[ "CATEGORY-STATION"		]	=	&_categoryStation;
    _cabrilloTokenMap[ "CATEGORY-TIME"			]	=	&_categoryTime;
    _cabrilloTokenMap[ "CATEGORY-TRANSMITTER"	]	=	&_categoryTransmitter;
    _cabrilloTokenMap[ "CATEGORY-OVERLAY"		]	=	&_categoryOverlay;
    _cabrilloTokenMap[ "CLAIMED-SCORE"			]	=	&_claimedScore;
    _cabrilloTokenMap[ "CLUB"					]	=	&_club;
    _cabrilloTokenMap[ "OPERATORS"				]	=	&_tmpLine;
    _cabrilloTokenMap[ "NAME"					]	=	&_name;
    _cabrilloTokenMap[ "ADDRESS"				]	=	&_addressStreet;
    _cabrilloTokenMap[ "ADDRESS-CITY"			]	=	&_addressCity;
    _cabrilloTokenMap[ "ADDRESS-STATE-PROVINCE"	]	=	&_addressStateProvince;
    _cabrilloTokenMap[ "ADDRESS-POSTALCODE"		]	=	&_addressPostalCode;
    _cabrilloTokenMap[ "ADDRESS-COUNTRY"		]	=	&_addressCountry;
    _cabrilloTokenMap[ "SOAPBOX"				]	=	&_tmpLine;
    _cabrilloTokenMap[ "CREATED-BY"				]	=	&_createdBy;
	_cabrilloTokenMap[ "CQZONE"					]	=	&_cqZone;
	_cabrilloTokenMap[ "LOCATION"				]	=	&_location; 
	_cabrilloTokenMap[ "DISTRICT"				]	=	&_district;
    _cabrilloTokenMap[ "EMAIL"					]	=	&_email;
    _cabrilloTokenMap[ "X-EMAIL"				]	=	&_email;
    _cabrilloTokenMap[ "QSO"					]	=	&_tmpLine;
}

void Log::AddToQsoLists(QsoPtr q)
{
    if(q->Status() == FORMAT_ERR)
        return;

    _qsoListByCallBandMap[q->CallRcvd()][q->Band()].append(q);
}

void Log::FillBandLists()
{
    foreach(QsoPtr q, _allQso)
        _qsoListByBand[q->Band()].append(q);
}

int Log::MagOrder()	const {	return static_cast <CqContest *> (gContest)->CatDescrFromNum(_categoryNum).MagOpDescription().MagOrder(); }
int Log::PwrNum()	const { return static_cast <CqContest *> (gContest)->CatDescrFromNum(_categoryNum).PwrNum(); }
int Log::BandNum() const  { return static_cast <CqContest *> (gContest)->CatDescrFromNum(_categoryNum).BandNum(); }

QString Log::MakeGuestOpStr()
{
    if(!_categoryOperator.startsWith("SINGLE") && !_opsList.isEmpty())
        return "";

	QString baseCall;
	QStringList parts = _rawCall.split('/');
	foreach(QString part, parts)
		if (part.length() > baseCall.length())
			baseCall = part;

    if(_guestOp == baseCall || _guestOp == _rawCall || _guestOp == _call)
        return "";

    if(!_hostCall.isEmpty())
        return _guestOp + " @" + _hostCall;
    else
        return _guestOp;
}

Log::~Log() {}

Log::Log() :
	_assisted(false),
	_categoryOverlayClassic(false),
	_categoryOverlayRookie(false),
	_certificate(false),
	_disqualified(false),
	_doNotCheck(false),
	_dontBustCall(false),
	_dontBustNil(false),
	_dontBustXch(false),
	_dontFillRunMult(false),
	_fillRunMult(false),
	_incomplete(false),
	_isAdministrativeChecklog(false),
	_isBadRcvdInfo(false),
	_isBadSentInfo(false),
	_isChecklog(false),
	_isFinalTm(false),
	_isMultiMulti(false),
	_isMultiSingle(false),
	_isMultiTwo(false),
	_isNoAward(false),
	_isPortable(false),
	_isRover(false),
	_isSingleOp(true),
	_mailCertFlag(false),
	_marMobile(false),
	_overlayCertificate(false),
	_redCard(false),
	_submittedLate(false),
	_timeNormalized(false),
	_yellowCard(false),

	_categoryNum(0),
	_checkableCount(0),
	_hitCount(0),
	_hitCountDay1(0),
	_hitCountDay2(0),
	_iaruZone(0),
	_logId(0),
	_maxOperatingTime(INT_MAX),
	_multCount(0),
	_multCountDay1(0),
	_multCountDay2(0),
	_multHitCount(0),
	_multHitCountDay1(0),
	_multHitCountDay2(0),
	_offtimeThreshold(1800),
	_operatingTime(0),
	_overlayScore(0),
	_passCount(0),
	_passCountDay1(0),
	_passCountDay2(0),
	_qsoCount(0),
	_qsoCountDay1(0),
	_qsoCountDay2(0),
	_rawOverlayScore(0),
	_rawScore(0),
	_runCount(0),
	_runCountDay1(0),
	_runCountDay2(0),
	_score(0),
	_snpCount(0),
	_snpCountDay1(0),
	_snpCountDay2(0),
	_snpHitCount(0),
	_snpHitCountDay1(0),
	_snpHitCountDay2(0),
	_snpMultCount(0),
	_snpMultCountDay1(0),
	_snpMultCountDay2(0),
	_snpMultHitCount(0),
	_snpMultHitCountDay1(0),
	_snpMultHitCountDay2(0),
	_timeOffset(0),
	_unknownCount(0),
	_unknownCountDay1(0),
	_unknownCountDay2(0),
	_euFinish(-1),
	_usaFinish(-1),
	_worldFinish(-1),
	_lat(-90),
	_lon(-90),
	_px(0),
	_contest(gContest)
{}

void	Log::AppendErrMsgList			(QString msg, QString arg) { _errMsgList.append(QString("%1 %2").arg(msg).arg(arg)); }
void	Log::AppendBustCausedList		(QsoPtr q)	{ _lock.lock(); _bustedCallCausedByTimeMap	.insertMulti(q->DateTime(), q); _lock.unlock(); }
void	Log::AppendBustedXchCausedList	(QsoPtr q)	{ _lock.lock(); _bustedXchCausedByTimeMap	.insertMulti(q->DateTime(), q); _lock.unlock(); }
void	Log::AppendBcvList				(QsoPtr q)	{ _lock.lock(); _bcvByTimeMap				.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendBadRevLogList		(QsoPtr q)	{ _lock.lock(); _badRevLogByTimeMap			.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendBustedCallList		(QsoPtr q)	{ _lock.lock(); _bustedCallByTimeMap		.insertMulti(q->DateTime(), q); _lock.unlock(); }
void	Log::AppendBustedFccCallList	(QsoPtr q)	{ _lock.lock(); _bustedFccCallByTimeMap		.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendBustedXchList		(QsoPtr q)	{ _lock.lock(); _bustedXchByTimeMap			.insertMulti(q->DateTime(), q); _lock.unlock(); }
void	Log::AppendCrossBandList		(QsoPtr q)	{ _lock.lock(); _crossBandByTimeMap			.insertMulti(q->DateTime(), q); _lock.unlock(); }
void	Log::AppendDupeList				(QsoPtr q)	{ _lock.lock(); _dupeByTimeMap				.insertMulti(q->DateTime(), q); _lock.unlock(); }
void	Log::AppendInvalidCallList		(QsoPtr q)	{ _lock.lock(); _invalidCtyList				.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendGoodList				(QsoPtr q)	{ _lock.lock(); _goodQsoByTimeMap			.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendNilCausedList		(QsoPtr q)	{ _lock.lock(); _nilCausedByTimeMap			.insertMulti(q->DateTime(), q); _lock.unlock(); }
void	Log::AppendNilList				(QsoPtr q)	{ _lock.lock(); _nilByTimeMap				.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendNotCheckedList		(QsoPtr q)	{ _lock.lock(); _notCheckedByTimeMap		.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendOutOfBandList		(QsoPtr q)  { _lock.lock(); _outOfBandByTimeMap			.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendOutOfTimeList		(QsoPtr q)	{ _lock.lock(); _outOfTimeByTimeMap			.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendUniList				(QsoPtr q)	{ _lock.lock(); _uniByTimeMap				.insertMulti(q->DateTime(), q);	_lock.unlock(); }
void	Log::AppendRubberClockedList	(QsoPtr q)	{ _lock.lock(); _rubberClockedByBandMap		.insertMulti(q->DateTime(), q);	_lock.unlock(); }

void	Log::AppendLargeTimeOffsetList	(QString s)	{ _largeTimeOffsetList.append(s); }
void	Log::ClearBandInfoMap()						{ _bandInfoMap.clear();	_overlayBandInfoMap.clear(); }

QStringList		Log::BandNameList()				const { return ContestPtr()->BandNameList(); }
QsoList	const   Log::BandChangeViolationList()	const { return _bcvByTimeMap				.values(); }
QsoList	const   Log::BadRevLogList()			const { return _badRevLogByTimeMap			.values(); }
QsoList	const   Log::BustedCallList()			const { return _bustedCallByTimeMap			.values(); }
QsoList	const   Log::BustCausedList()			const { return _bustedCallCausedByTimeMap	.values(); }
QsoList	const	Log::BustedFccCallList()		const { return _bustedFccCallByTimeMap		.values(); }
QsoList	const   Log::BustedXchCausedList()		const { return _bustedXchCausedByTimeMap	.values(); }
QsoList	const   Log::BustedXchList()			const { return _bustedXchByTimeMap			.values(); }
QsoList	const   Log::CrossBandList()			const { return _crossBandByTimeMap			.values(); }
QsoList	const   Log::DupeList()					const { return _dupeByTimeMap				.values(); }
QsoList	const   Log::GoodList()					const { return _goodQsoByTimeMap			.values(); }
QsoList	const	Log::NoLogList()				const { return _noLogByTimeMap				.values(); }
QsoList	const   Log::NilCausedList()			const { return _nilCausedByTimeMap			.values(); }
QsoList	const   Log::NilList()					const { return _nilByTimeMap				.values(); }
QsoList	const   Log::NotCheckedList()			const { return _notCheckedByTimeMap			.values(); }
QsoList	const   Log::OutOfTimeList()			const { return _outOfTimeByTimeMap			.values(); }
QsoList	const   Log::RubberClockedByBandMap()	const { return _rubberClockedByBandMap		.values(); }
QsoList	const   Log::UniList()					const { return _uniByTimeMap				.values(); }

QStringList		Log::ErrorMsgList()				const { return _errMsgList;			}

QMap<QDateTime, Qso *> const Log::NilCausedMap()const { return _nilCausedByTimeMap;		}
QList <LostMult> const Log::LostMultList()		const { return _lostMultList;			}
std::vector<Qso *>	const &	Log::AllQso()		const { return _allQso;					} // by reference -- never copy!
QMutex		&	Log::Lock()							  { return _lock;					}

QsoListByBandMap	const Log::GetQsoListByBandMap()	const { return  _qsoListByBand;				}

QsoList			Log::GetQsoListByCallBand(QString call, QString band) const { return  _qsoListByCallBandMap.value(call).value(band); }
Contest const *	Log::ContestPtr()				const { return _contest;				}
Cty		const *	Log::CtyPtr()					const { return _cty;					}
Px		const *	Log::PxPtr()                	const { return _px;						}
QString	const	Log::Address()                 	const { return _addressStreet;			}
QString	const	Log::AddressCity()				const { return _addressCity;			}
QString	const	Log::AddressStateProvince()		const { return _addressStateProvince;	}
QString	const	Log::AddressPostalCode()		const { return _addressPostalCode;		}
QString	const	Log::AddressCountry()        	const { return _addressCountry;			}
QString	const	Log::AdminText()				const { return _contest->AdminText();	}
QString	const	Log::Call()						const { return _call;				}
QString	const	Log::Category()                	const { return _category;				}
QString	const	Log::CategoryAssisted()        	const { return _categoryAssisted;		}
QString	const	Log::CategoryBand()            	const { return _categoryBand;			}
QString	const	Log::CategoryMode()            	const { return _categoryMode;			}
QString	const	Log::CategoryOperator()        	const { return _categoryOperator;		}
QString	const	Log::CategoryOverlay()         	const { return _categoryOverlay;		}
QString	const	Log::CategoryPower()           	const { return _categoryPower;			}
QString	const	Log::CategoryStation()         	const { return _categoryStation;		}
QString	const	Log::CategoryTime()            	const { return _categoryTime;			}
QString	const	Log::CategoryTransmitter()     	const { return _categoryTransmitter;	}
QString	const	Log::ClaimedScore()            	const { return _claimedScore;			}
QString	const	Log::Club()                    	const { return _club;					}
QString	const	Log::Cont()                    	const { return _cont;					}
QString	const	Log::CreatedBy()				const { return _createdBy;				}
QString	const	Log::CtyName()                 	const { return _cty->Name();			}
QString	const	Log::StdPx()                 	const { return _px->StdPx();			}
QString	const	Log::District()                 const { return _district;				}
QString	const	Log::ExchSent()                	const { return _exchSent;				}
QString	const	Log::GuestOp()                 	const { return _guestOp;				}
QString	const	Log::HostCall()                 const { return _hostCall;				}
QString	const	Log::HqFrom()                  	const { return _hqFrom;					}
QString	const	Log::HqDateRcvd()              	const { return _hqDateRcvd;				}
QString	const	Log::Location()                	const { return _location;				}
QString	const	Log::Mode()						const { return _mode;					}
QString	const	Log::Name()                    	const { return _name;					}
QString	const	Log::Operators()               	const { return _operators;				}
QString	const	Log::PortDigit()				const { return  _portDigit;				}

QString	const	Log::RawCall()                 	const { return _rawCall;				}
QString	const	Log::SoapBox()                 	const { return _soapBox;				}
QString	const	Log::TypeStr()					const { return gSrcMap["ContestType"];	}

QStringList	Log::OpsList()						const { return _opsList;				}
QStringList	Log::BigTimeOffsetList()			const { return _largeTimeOffsetList;	}
QFileInfo 	Log::FileInfo()                		const { return _fileInfo;				}
bool      	Log::DoNotCheck()             		const { return _doNotCheck;				}
bool      	Log::Certificate()             		const { return _certificate;			}
bool      	Log::OverlayCertificate()      		const { return _overlayCertificate;		}
bool      	Log::DontBustCall()            		const { return _dontBustCall;			}
bool      	Log::DontBustNil()             		const { return _dontBustNil;			}
bool      	Log::DontBustXch()             		const { return _dontBustXch;			}
bool	  	Log::ExactQrgByBand(QString band)	const { return _exactQrgByBand.value(band); }
bool      	Log::DontFillRunMult()				const { return _dontFillRunMult;		}
bool      	Log::FillRunMult()             		const { return _fillRunMult;			}
bool		Log::IsBadRcvdInfo()				const { return _isBadRcvdInfo;			}
bool		Log::IsBadSentInfo()				const { return _isBadSentInfo;			}
bool		Log::IsFinalTm()					const { return _isFinalTm;				}
bool		Log::HasRedCard()					const { return _redCard;				}
bool		Log::HasYellowCard()				const { return _yellowCard;				}
bool		Log::Disqualified()					const { return _disqualified;			}
bool		Log::IsAdministrativeChecklog()		const { return _isAdministrativeChecklog; }
bool		Log::IsChecklog()					const { return _isChecklog; }
bool		Log::IsIncomplete()					const { return _incomplete;				}
bool      	Log::IsAssisted()           		const { return _assisted;				}
bool      	Log::IsMultiSingle()           		const { return _isMultiSingle;			}
bool      	Log::IsMultiTwo()              		const { return _isMultiTwo;				}
bool      	Log::IsMultiMulti()              	const { return _isMultiMulti;			}
bool      	Log::IsNoAward()	              	const { return _isNoAward;				}
bool		Log::IsSingleOp()					const { return _categoryOperator.startsWith("S"); }
bool		Log::IsMarMobile()					const { return _marMobile;				}
bool      	Log::IsPortable()	              	const { return _isPortable;				}
bool      	Log::IsWVe()                   		const { return _cty->IsWVe();			}
bool      	Log::IsW()                   		const { return _cty->IsW();				}
bool      	Log::IsInFccData()                  const { return _cty->IsInFccData();		}
bool      	Log::TimeNormalized()          		const { return _timeNormalized;			}
bool      	Log::SubmittedLate()          		const { return _submittedLate;			}
bool		Log::IsOverlayClassic()				const { return _categoryOverlayClassic; }
bool		Log::IsOverlayRookie()				const { return _categoryOverlayRookie;	}
bool		Log::IsOverlay()					const { return _categoryOverlayRookie || _categoryOverlayClassic; }

int			Log::OverlayNetQso(QString band)	const { return _overlayBandInfoMap.value(band).NetQso();	}
int			Log::OverlayNetPts(QString band)	const { return _overlayBandInfoMap.value(band).NetPts();	}
int			Log::OverlayNetMults(QString band)	const { return _overlayBandInfoMap.value(band).NetMults();	}
int			Log::OverlayNetMults2(QString band)	const { return _overlayBandInfoMap.value(band).NetMults2(); }
int			Log::OverlayNetMults3(QString band)	const { return _overlayBandInfoMap.value(band).NetMults3(); }

int			Log::OverlayNetQso()				const { return _overlayBandInfoMap.value("ALL").NetQso();	}
int			Log::OverlayNetPts()				const { return _overlayBandInfoMap.value("ALL").NetPts();	}
int			Log::OverlayNetMults()				const { return _overlayBandInfoMap.value("ALL").NetMults(); }
int			Log::OverlayNetMults2()				const { return _overlayBandInfoMap.value("ALL").NetMults2();}
int			Log::OverlayNetMults3()				const { return _overlayBandInfoMap.value("ALL").NetMults3();}
int			Log::CategoryNum()					const { return _categoryNum;								}
int			Log::Score()                   		const { return _score;										}
int			Log::RawScore()                		const { return _rawScore;									}
size_t		Log::LogSize()						const { return _allQso.size();								}
int			Log::OverlayScore()					const { return _overlayScore;								}

int			Log::NetQso()                  		const { return _bandInfoMap.value("ALL").NetQso();			}
int			Log::NetPts()                  		const { return _bandInfoMap.value("ALL").NetPts();			}
int			Log::NetMults()                		const { return _bandInfoMap.value("ALL").NetMults();		}
int			Log::NetMults2()               		const { return _bandInfoMap.value("ALL").NetMults2();		}
int			Log::NetMults3()               		const { return _bandInfoMap.value("ALL").NetMults3();		}

int			Log::RawQso()						const { return _bandInfoMap.value("ALL").RawQso();			}
int			Log::RawPts()						const { return _bandInfoMap.value("ALL").RawPts();			}
int			Log::RawMults()						const { return _bandInfoMap.value("ALL").RawMults();		}
int			Log::RawMults2()					const { return _bandInfoMap.value("ALL").RawMults2();		}
int			Log::RawMults3()					const { return _bandInfoMap.value("ALL").RawMults3();		}

int			Log::NetQso(QString band)			const { return _bandInfoMap.value(band).NetQso();			}
int			Log::NetPts(QString band)			const { return _bandInfoMap.value(band).NetPts();			}
int			Log::NetMults(QString band)			const { return _bandInfoMap.value(band).NetMults();			}
int			Log::NetMults2(QString band)		const { return _bandInfoMap.value(band).NetMults2();		}
int			Log::NetMults3(QString band)		const { return _bandInfoMap.value(band).NetMults3();		}

int			Log::RawQso(QString band)			const { return _bandInfoMap.value(band).RawQso();			}
int			Log::RawPts(QString band)			const { return _bandInfoMap.value(band).RawPts();			}
int			Log::RawMults(QString band)			const { return _bandInfoMap.value(band).RawMults();			}
int			Log::RawMults2(QString band)		const { return _bandInfoMap.value(band).RawMults2();		}
int			Log::RawMults3(QString band)		const { return _bandInfoMap.value(band).RawMults3();		}

size_t		Log::QsoCount()						const { return _allQso.size();			}
int			Log::RunCount()						const { return _runCount;				}
int			Log::PassCount()					const { return _passCount;				}
int			Log::UnknownCount()					const { return _unknownCount;			}
int			Log::MultCount()					const { return _multCount;				}
int			Log::HitCount()						const { return _hitCount;				}
int			Log::MultHitCount()					const { return _multHitCount;			}
int			Log::SnpCount()						const { return _snpCount;				}
int			Log::SnpHitCount()					const { return _snpHitCount;			}
int			Log::SnpMultCount()					const { return _snpMultCount;			}
int			Log::SnpMultHitCount()				const { return _snpMultHitCount;		}
int			Log::QsoCountDay1()					const { return _qsoCountDay1;			}
int			Log::RunCountDay1()					const { return _runCountDay1;			}
int		  	Log::PassCountDay1()				const { return _passCountDay1;			}
int		  	Log::UnknownCountDay1()				const { return _unknownCountDay1;		}
int		  	Log::MultCountDay1()				const { return _multCountDay1;			}
int		  	Log::HitCountDay1()					const { return _hitCountDay1;			}
int		  	Log::MultHitCountDay1()				const { return _multHitCountDay1;		}
int		  	Log::SnpCountDay1()					const { return _snpCountDay1;			}
int		  	Log::SnpHitCountDay1()				const { return _snpHitCountDay1;		}
int		  	Log::SnpMultCountDay1()				const { return _snpMultCountDay1;		}
int		  	Log::SnpMultHitCountDay1()			const { return _snpMultHitCountDay1;	}
int			Log::QsoCountDay2()					const { return _qsoCountDay2;			}
int			Log::RunCountDay2()					const { return _runCountDay2;			}
int		  	Log::PassCountDay2()				const { return _passCountDay2;			}
int		  	Log::UnknownCountDay2()				const { return _unknownCountDay2;		}
int		  	Log::MultCountDay2()				const { return _multCountDay2;			}
int		  	Log::HitCountDay2()					const { return _hitCountDay2;			}
int		  	Log::MultHitCountDay2()				const { return _multHitCountDay2;		}
int		  	Log::SnpCountDay2()					const { return _snpCountDay2;			}
int		  	Log::SnpHitCountDay2()				const { return _snpHitCountDay2;		}
int		  	Log::SnpMultCountDay2()				const { return _snpMultCountDay2;		}
int		  	Log::SnpMultHitCountDay2()			const { return _snpMultHitCountDay2;	}
int		  	Log::CheckableCount()				const { return _checkableCount;			}
int       	Log::GoodCount()               		const { return _goodQsoByTimeMap.size();}
int       	Log::BustedCallCount()         		const { return _bustedCallByTimeMap.size() + _badRevLogByTimeMap.size(); }
int       	Log::BadXchCount()             		const { return _bustedXchByTimeMap.size();	}
int       	Log::NilCount()                		const { return _nilByTimeMap.size();	}
int       	Log::BustedCallCausedCount()   		const { return _bustedCallCausedByTimeMap.size(); }
int       	Log::XchCausedCount()          		const { return _bustedXchCausedByTimeMap.size(); }
int       	Log::NilCausedCount()          		const { return _nilCausedByTimeMap.size();	}
int       	Log::UniCount()                		const { return _uniByTimeMap.size();	}
int       	Log::DupeCount()					const { return _dupeByTimeMap.size();	}
int       	Log::BandChangeViolationCount()		const { return _bcvByTimeMap.size();	}
int       	Log::TimeViolationCount()			const { return _outOfTimeByTimeMap.size();	}
int			Log::WorldFinish()					const { return _worldFinish;			}
int			Log::EuFinish()						const { return _euFinish;				}
int			Log::UsaFinish()					const { return _usaFinish;				}

double		Log::PercentBust()					const { return _checkableCount ? double(_bustedCallByTimeMap.size()	* 100.0) / _checkableCount : 0; }
double		Log::PercentNil()					const { return _checkableCount ? double(_nilByTimeMap.size()		* 100.0) / _checkableCount : 0; }
double		Log::PercentXch()					const { return _checkableCount ? double(_bustedXchByTimeMap.size()	* 100.0) / _checkableCount : 0; }

double		Log::PercentCheckedGood()			const { return _checkableCount ? double(_goodQsoByTimeMap.size()			* 100.0) / _checkableCount : 0; }
double		Log::PercentBustCaused()				const { return _checkableCount ? double(_bustedCallCausedByTimeMap.size()	* 100.0) / _checkableCount : 0; }
double		Log::PercentNilCaused()				const { return _checkableCount ? double(_nilCausedByTimeMap.size()			* 100.0) / _checkableCount : 0; }
double		Log::PercentXchCaused()				const { return _checkableCount ? double(_bustedXchCausedByTimeMap.size()	* 100.0) / _checkableCount : 0; }
double		Log::PercentCrossBand()				const { return _checkableCount ? double(_crossBandByTimeMap.size()			* 100.0) / _checkableCount : 0; }
double		Log::OperatingTime()				const { return _operatingTime / 3600.0; }
double		Log::Reduction()					const { return _rawScore > 0.0 ? double(_rawScore - _score) / _rawScore : 0.0; }
double		Log::ErrorRate()					const { return (1.0 - PercentCheckedGood()); }
time_t		Log::TimeAdjustment()				const {	return _timeOffset; }
QDateTime	Log::LastQsoTime()					const { return _finalQsoDateTime; }

void	Log::Call					(QString call)		{ _call = call;							}
void	Log::DontFillRunMult		(bool b)			{ _dontFillRunMult		= b;			}
void	Log::IsAdministrativeChecklog(bool b)			{ _isAdministrativeChecklog = b;		}
void	Log::IsNoAward				(bool b)			{ _isNoAward			= b;			}
void	Log::IsFinalTm				(bool b)			{ _isFinalTm			= b;			}
void	Log::FillRunMult			(bool fill)			{ _fillRunMult			= fill;			}
void	Log::IsIncomplete			(bool b)			{ _incomplete			= b;			}
void	Log::CategoryNum			(int num)			{ _categoryNum			= num;			}
void	Log::FinalScore				(int final)			{ _score				= final;		}
void	Log::District				(QString d)			{ _district				= d;			}
void	Log::Category				(QString cat)		{ _category				= cat;			}
void	Log::CategoryAssisted		(QString s)			{ _categoryAssisted		= s;			}
void	Log::CategoryBand			(QString s)			{ _categoryBand			= s;			}
void	Log::CategoryMode			(QString s)			{ _categoryMode			= s;			}
void	Log::CategoryOperator		(QString s)			{ _categoryOperator		= s;			}
void	Log::CategoryPower			(QString s)			{ _categoryPower		= s;			}
void	Log::CategoryStation		(QString s)			{ _categoryStation		= s;			}
void	Log::CategoryTime			(QString s)			{ _categoryTime			= s;			}
void	Log::CategoryTransmitter	(QString s)			{ _categoryTransmitter	= s;			}
void	Log::CategoryOverlay		(QString s)			{ _categoryOverlay		= s;			}
void	Log::ClaimedScore			(QString claimed)	{ _claimedScore			= claimed;		}
void	Log::Cont					(QString)			{ _cont					= _px->Cont();	}
void	Log::Club					(QString club)		{ _club					= club;			}
void	Log::ExchSent				(QString sent)		{ _exchSent				= sent;			}
void	Log::GuestOp				(QString op)		{ _guestOp				= op;			}
void	Log::Location				(QString loc)		{ _location				= loc;			}
void	Log::Mode					(QString mode)		{ _mode					= mode;			}
void	Log::Operators				(QString ops)		{ _operators			= ops;			}
void	Log::RawCall				(QString call)		{ _rawCall				= call;			}
void	Log::FileInfo				(QFileInfo info)	{ _fileInfo				= info;			}
void	Log::PxPtr					(Px const * pxPtr)	{ _px = pxPtr; _cty = _px->GetCty();	}
void	Log::Certificate			(bool b)			{ _certificate			= b;			}
void	Log::DoNotCheck				(bool b)			{ _doNotCheck			= b;			}
void	Log::DontBustCall			(bool b)			{ _dontBustCall			= b;			}
void	Log::DontBustXch			(bool b)			{ _dontBustXch			= b;			}
void	Log::DontBustNil			(bool b)			{ _dontBustNil			= b;			}
void	Log::IsBadRcvdInfo			(bool b)			{ _isBadRcvdInfo		= b;			}
void	Log::IsBadSentInfo			(bool b)			{ _isBadSentInfo		= b;			}
void	Log::IsAssisted				(bool b)			{ _assisted				= b;			}
void	Log::SetRedCard				(bool b)			{ _redCard				= b;			}
void	Log::SetYellowCard			(bool b)			{ _yellowCard			= b;			}
void	Log::SetMarMobile			(bool b)			{ _marMobile			= b;			}
void	Log::Disqualified			(bool b)			{ _disqualified			= b;			}
void	Log::OverlayCertificate		(bool b)			{ _overlayCertificate	= b;			}
void	Log::TimeNormalized			(bool b)			{ _timeNormalized		= b;			}
void	Log::SubmittedLate			(bool b)			{ _submittedLate		= b;			}

void	Log::NetQso					(int n)				{ _bandInfoMap["ALL"].NetQso(n);	}
void	Log::NetMults				(int n)				{ _bandInfoMap["ALL"].NetMults(n);	}
void	Log::NetMults2				(int n)				{ _bandInfoMap["ALL"].NetMults2(n); }
void	Log::SetNetQso				(int n)				{ _bandInfoMap["ALL"].NetQso(n);	}
void	Log::SetNetMults			(int n)				{ _bandInfoMap["ALL"].NetMults(n);	}
void	Log::SetNetMults2			(int n)				{ _bandInfoMap["ALL"].NetMults2(n); }
void	Log::SetNetMults3			(int n)				{ _bandInfoMap["ALL"].NetMults2(n); }
void	Log::Score					(int score)			{ _score = score;					}

void	Log::TimeAdjustment			(time_t t)			{ _timeOffset			+= t;		}
void	Log::WorldFinish			(int finish)		{ _worldFinish			= finish;	}
void	Log::EuFinish				(int finish)		{ _euFinish				= finish;	}
void	Log::UsaFinish				(int finish)		{ _usaFinish			= finish;	}

void	Log::InsertExactQrgByBand	(QString band, bool exact)	{ _exactQrgByBand.insert(band, exact); }
void	Log::WriteSprLine			(QTextStream &) { }
void	Log::SortAllQso() { std::stable_sort(_allQso.begin(), _allQso.end(), gQsoTimeLessThanQsoTime); }
void	Log::SetCty(Cty const * cty) { _cty = cty; }

std::map <QString, QString>	Log::_districtByStateMap = 
{
	{"CT"	, "1"},	{"MA"	, "1"},	{"EMA"	, "1"},	{"WMA"	, "1"},	{"ME"	, "1"},	{"NH"	, "1"},	{"RI"	, "1"},	{"VT"	, "1"},	{"NY"	, "2"},
	{"ENY"	, "2"},	{"NLI"	, "2"},	{"NNY"	, "2"},	{"WNY"	, "2"},	{"NJ"	, "2"},	{"NNJ"	, "2"},	{"SNJ"	, "2"},	{"DE"	, "3"},	{"DEL"	, "3"},
	{"PA"	, "3"},	{"EPA"	, "3"},	{"WPA"	, "3"},	{"MDC"	, "3"},	{"MD"	, "3"},	{"DC"	, "3"},	{"AL"	, "4"},	{"GA"	, "4"},	{"KY"	, "4"},
	{"NC"	, "4"},	{"FL"	, "4"},	{"FLA"	, "4"},	{"NFL"	, "4"},	{"SFL"	, "4"},	{"WCF"	, "4"},	{"SC"	, "4"},	{"TN"	, "4"},	{"VA"	, "4"},
	{"PR"	, "4"},	{"VI"	, "4"},	{"AR"	, "5"},	{"LA"	, "5"},	{"MS"	, "5"},	{"NM"	, "5"},	{"TX"	, "5"},	{"NTX"	, "5"},	{"STX"	, "5"},
	{"WTX"	, "5"},	{"OK"	, "5"},	{"CA"	, "6"},	{"EB"	, "6"},	{"LAX"	, "6"},	{"ORG"	, "6"},	{"SB"	, "6"},	{"SCV"	, "6"},	{"SDG"	, "6"},
	{"SF"	, "6"},	{"SJV"	, "6"},	{"SV"	, "6"},	{"PAC"	, "6"},	{"AK"	, "7"},	{"AZ"	, "7"},	{"WA"	, "7"},	{"EWA"	, "7"},	{"WWA"	, "7"},
	{"ID"	, "7"},	{"MT"	, "7"},	{"NV"	, "7"},	{"OR"	, "7"},	{"UT"	, "7"},	{"WY"	, "7"},	{"MI"	, "8"},	{"OH"	, "8"},	{"WV"	, "8"},
	{"WVA"	, "8"},	{"IL"	, "9"},	{"IN"	, "9"},	{"WI"	, "9"},	{"WS"	, "9"},	{"CO"	, "0"},	{"IA"	, "0"},	{"IO"	, "0"},	{"KS"	, "0"},
	{"MN"	, "0"},	{"MO"	, "0"},	{"NE"	, "0"},	{"ND"	, "0"},	{"SD"	, "0"},	{"MAR"	, "1"},	{"PEI"	, "1"},	{"NB"	, "1"},{"NL"	, "1"},
	{"NS"	, "1"},	{"QC"	, "2"},	{"ON"	, "3"},	{"MB"	, "4"},	{"SK"	, "5"},	{"AB"	, "6"},	{"BC"	, "7"},	{"NT"	, "8"}
};

void RevLog::Write()
{
	QString call = Call();
	call.replace("/", "-");

	QString fileName = gContest->RlDir().absoluteFilePath(call.toLower() + ".rl");
	QFile file(fileName);
	QTextStream strm(&file);
	file.open(QIODevice::WriteOnly | QIODevice::Text);

	foreach(QsoPtr q, _allQso)
		strm << q->ToFullReverseString() << '\n';
}

QString  RevLog::Call() const
{
	if (_allQso.size() == 0)
		return "";

	return (*_allQso.begin())->CallRcvd();
}
