#ifndef _PERBANDINFO
#define _PERBANDINFO

class PerBandInfo
{
public:
	PerBandInfo() :
	_rawQso(0),
	_netQso(0),
	_rawMults(0),
	_netMults(0),
	_rawMults2(0),
	_netMults2(0),
	_rawMults3(0),
	_netMults3(0),
	_roverGridsVisited(0),
	_rawPts(0),
	_netPts(0),
	_dupes(0),
	_badCall(0),
	_badXch(0),
	_nil(0),
	_badFormat(0)
	{}

	PerBandInfo(QString bandName) :
	_band(bandName),
	_rawQso(0),
	_netQso(0),
	_rawMults(0),
	_netMults(0),
	_rawMults2(0),
	_netMults2(0),
	_rawMults3(0),
	_netMults3(0),
	_roverGridsVisited(0),
	_rawPts(0),
	_netPts(0),
	_dupes(0),
	_badCall(0),
	_badXch(0),
	_nil(0),
	_badFormat(0)
	{}

	PerBandInfo(PerBandInfo const & pbi) :
	_band				(pbi._band),
	_rawQso				(pbi._rawQso),
	_netQso				(pbi._netQso),
	_rawMults			(pbi._rawMults),
	_netMults			(pbi._netMults),
	_rawMults2			(pbi._rawMults2),
	_netMults2			(pbi._netMults2),
	_rawMults3			(pbi._rawMults3),
	_netMults3			(pbi._netMults3),
	_roverGridsVisited	(pbi._roverGridsVisited),
	_rawPts				(pbi._rawPts),
	_netPts				(pbi._netPts),
	_dupes				(pbi._dupes),
	_badCall			(pbi._badCall),
	_badXch				(pbi._badXch),
	_nil				(pbi._nil),
	_badFormat			(pbi._badFormat)
	{}

	int				RawQso()					const	{ return _rawQso;				}
	int				NetQso()					const	{ return _netQso;				}
	int				RawMults()					const	{ return _rawMults;				}
	int				NetMults()					const	{ return _netMults;				}
	int				RawMults2()					const	{ return _rawMults2;			}
	int				NetMults2()					const	{ return _netMults2;			}
	int				RawMults3()					const	{ return _rawMults3;			}
	int				NetMults3()					const	{ return _netMults3;			}
	int				RoverGridsVisited()			const	{ return _roverGridsVisited;	}
	int				RawPts()					const	{ return _rawPts;				}
	int				NetPts()					const	{ return _netPts;				}
	int				Dupes()						const	{ return _dupes;				}
	int				BadCalls()					const	{ return _badCall;				}
	int				BadXch()					const	{ return _badXch;				}
	int				Nil()						const	{ return _nil;					}
	int				BadFormat()					const	{ return _badFormat;			}
	QString			Band()						const	{ return _band;					}

	void			Band				(QString band)	{ _band					= band;	}
	void			RawQso				(int cnt)		{ _rawQso				= cnt;	}
	void			NetQso				(int cnt)		{ _netQso				= cnt;	}
	void			RawPts				(int cnt)		{ _rawPts				= cnt;	}
	void			RawMults			(int cnt)		{ _rawMults				= cnt;	}
	void			RawMults2			(int cnt)		{ _rawMults2			= cnt;	}
	void			RawMults3			(int cnt)		{ _rawMults3			= cnt;	}
	void			NetMults			(int cnt)		{ _netMults				= cnt;	}
	void			NetMults2			(int cnt)		{ _netMults2			= cnt;	}
	void			NetMults3			(int cnt)		{ _netMults3			= cnt;	}
	void			RoverGridsVisited	(int cnt)		{ _roverGridsVisited	= cnt;	}
	void			NetPts				(int cnt)		{ _netPts				= cnt;	}
	void			Dupes				(int cnt)		{ _dupes				= cnt;	}
	void			BadCalls			(int cnt)		{ _badCall				= cnt;	}
	void			BadXch				(int cnt)		{ _badXch				= cnt;	}
	void			Nil					(int cnt)		{ _nil					= cnt;	}
	void			BadFormat			(int cnt)		{ _badFormat			= cnt;	}

	void			ClearQsos			()				{ _rawQso = _netQso	= _rawPts = _netPts	= 0; }
	void			ClearMults			()				{ _rawMults = _rawMults2 = _rawMults3 = _netMults = _netMults2 = _netMults3 = 0; }
	void			IncrRawQso()						{ _rawQso++; }
	void			IncrNetQso			()				{ _netQso++;					}
	void			IncrRawMults		()				{ _rawMults++;					}
	void			IncrRawMults2		()				{ _rawMults2++;					}
	void			IncrRawMults3		()				{ _rawMults3++;					}
	void			IncrNetMults		()				{ _netMults++;					}
	void			IncrNetMults2		()				{ _netMults2++;					}
	void			IncrNetMults3		()				{ _netMults3++;					}
	void			IncrRoverGridsVisited()				{ _roverGridsVisited++;			}
	void			IncrDupes			()				{ _dupes++;						}
	void			IncrBadCalls		()				{ _badCall++;					}
	void			IncrBadXch			()				{ _badXch++;					}
	void			IncrNil				()				{ _nil++;						}
	void			IncrBadFormat		()				{ _badFormat++;					}

	void			DecrRawQso			()				{ _rawQso--;					}
	void			DecrNetQso			()				{ _netQso--;					}
	void			DecrRawMults		()				{ _rawMults--;					}
	void			DecrRawMults2		()				{ _rawMults2--;					}
	void			DecrRawMults3		()				{ _rawMults3--;					}
	void			DecrNetMults		()				{ _netMults--;					}
	void			DecrNetMults2		()				{ _netMults2--;					}
	void			DecrNetMults3		()				{ _netMults3--;					}
	void			DecrRoverGridsVisited()				{ _roverGridsVisited--;			}
	void			DecrDupes			()				{ _dupes--;						}
	void			DecrBadCalls		()				{ _badCall--;					}
	void			DecrBadXch			()				{ _badXch--;					}
	void			DecrNil				()				{ _nil--;						}
	void			DecrBadFormat		()				{ _badFormat--;					}

	void			AddRawPts			(int pts)		{ _rawPts += pts;				}
	void			AddNetPts			(int pts)		{ _netPts += pts;				}
	void			AddRawMults			(int mults)		{ _rawMults += mults;			}
	void			AddRawMults2		(int mults)		{ _rawMults2 += mults;			}
	void			AddRawMults3		(int mults)		{ _rawMults3 += mults;			}
	void			AddNetMults			(int mults)		{ _netMults += mults;			}
	void			AddNetMults2		(int mults)		{ _netMults2 += mults;			}
	void			AddNetMults3		(int mults)		{ _netMults3 += mults;			}

private:
	QString	_band;
	int		_rawQso;
	int		_netQso;
	int		_rawMults;
	int		_netMults;
	int		_rawMults2;
	int		_netMults2;
	int		_rawMults3;
	int		_netMults3;
	int		_roverGridsVisited;
	int		_rawPts;
	int		_netPts;
	int		_dupes;
	int		_badCall;
	int		_badXch;
	int		_nil;
	int		_badFormat;
};

typedef QMap  <QString, PerBandInfo> BandInfoMap;

#endif // _PERBANDINFO
