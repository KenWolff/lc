#ifndef _CTY
#define _CTY

class Px;

class Cty
{
public:
	Cty(int, QString const name, QString const std_px, int const zone, int const iaru_zone, QString const cont,
		double const timeOffset, double const lat, double const lon, bool) :
		_name(name), _std_px(std_px), _cont(cont), _zone(zone),	_iaruZone(iaru_zone), _pts (0), _timeZoneOffset(timeOffset), _lat(lat),	_lon(lon), _isInFccData(false)
		{}

	virtual ~Cty();


private:
	static int CtyCount;
	Cty() {};
public:
        // ACCESSORS

	QString	const	Name()			const;
	QString	const	StdPx()			const;
	QString	const	Cont()			const;
	int				Zone()			const;
	int				IaruZone()		const;
	int				Pts()				const;
	double			TimeAdjustment()	const;
	double			Lat()				const;
	double			Lon()				const;
	bool			IsKG4()			const;
	bool			IsInFccData()		const;
	bool			IsUsaPossession()	const;
	bool			IsWVe()			const;
	bool			IsW()				const;

	QList <Px *> const PxList()	const;

	void			AppendPx	(Px * pxPtr);
	void			IsInFccData		(bool b)	;

private:
	QString      _name;
	QString      _std_px;
	QString      _cont;
	int          _zone;
	int          _iaruZone;
	int          _pts;
	double       _timeZoneOffset;
	double       _lat;
	double       _lon;
	bool         _isInFccData;
	QList <Px *> _pxList;
};

extern QSet <Cty const *>	gUsPossessionSet;
extern QStringList          gCtyNames;
extern QMap <int, QString>  gCtyNameByNum;
extern QMap <QString, int>  gCtyNumByName;
extern Cty const *W_id, *KH0_id, *KH1_id, *KH2_id, *KH3_id, *KH4_id, *KH5_id, *KH6_id, *KH8_id, *KH9_id, *KC4_id, *BY_id, *KG4_id, *KC6_id, *KL7_id, *KP1_id, *KP2_id, *KP4_id, *KP5_id, *VE_id, *CY9_id, *CY0_id, *UA_id, *UA0_id, *CE_id, *EA_id, *VK_id, *SP_id, *PA_id, *JA_id, *LU_id, *PY_id, *ZS_id, *ZL_id;
#endif // _CTY
