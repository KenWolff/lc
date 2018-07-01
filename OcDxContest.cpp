#include "StdAfx.h"

void OcDxContest::WriteTxtHeader(QTextStream & strm)
{
	strm << "Call           Raw   D   N   B   X   BC  U   Q    M    Final    Raw      Claimed     Delta     Hrs   Cat\n";
}

void OcDxContest::Init()
{
	if (_mode == "PH")
		_mode = "SSB";
	
	if (_mode != "CW" && _mode != "SSB" && _mode != "RTTY")
		AppendErrMsgList("MODE:", _mode + "must be CW or SSB");

	_bandNameList << B160.Name() << B80.Name() << B40.Name() << B20.Name() << B15.Name() << B10.Name();

	enum { SO, QRP, ASSISTED, MS, M2, MM, CK };

	MagOpDesc SOMag	("SINGLE OPERATOR#",						SO_MAG_ORDER);
	MagOpDesc QRPMag("QRP#",									QRP_MAG_ORDER);
	MagOpDesc MSMag	("MULTI-OPERATOR#\nSINGLE-TRANSMITTER#",	MS_MAG_ORDER);
	MagOpDesc M2Mag	("MULTI-OPERATOR#\nTWO-TRANSMITTER#",		M2_MAG_ORDER);
	MagOpDesc MMMag	("MULTI-OPERATOR#\nMULTI-TRANSMITTER#",		MM_MAG_ORDER);
	MagOpDesc CKMag	("CHECK LOGS#",								CK_MAG_ORDER);

	_magOpDescByMagOrderNumMap.insert(SOMag	.MagOrder(),	SOMag);
	_magOpDescByMagOrderNumMap.insert(QRPMag.MagOrder(),	QRPMag);
	_magOpDescByMagOrderNumMap.insert(MSMag	.MagOrder(),	MSMag);
	_magOpDescByMagOrderNumMap.insert(M2Mag	.MagOrder(),	M2Mag);
	_magOpDescByMagOrderNumMap.insert(MMMag	.MagOrder(),	MMMag);

	CatDesc	SOABHDesc("ALL_H_U",		"A",		"Single Op All Band High Power",	H, 0, SOMag);
	CatDesc SO10HDesc("10M_H_U",	"28",		"Single Op 28 MHz High Power",		H, 1, SOMag);
	CatDesc SO15HDesc("15M_H_U",	"21",		"Single Op 21 MHz High Power",		H, 2, SOMag);
	CatDesc SO20HDesc("20M_H_U",	"14",		"Single Op 14 MHz High Power",		H, 3, SOMag);
	CatDesc SO40HDesc("40M_H_U",	"7",		"Single Op 7 MHz High Power",		H, 4, SOMag);
	CatDesc SO80HDesc("80M_H_U",	"3.5",		"Single Op 3.5 MHz High Power",		H, 6, SOMag);
	CatDesc SO160HDesc("160M_H_U",	"1.8",		"Single Op 1.8 MHz High Power",		H, 6, SOMag);

	if (_mode == "SSB")
		SO80HDesc = CatDesc("80M_H_U", "3.7", "Single Op 3.7 MHz High Power",		H, 6, SOMag);

	CatDesc	SOABLDesc("ALL_L_U",		"A",		"Single Op All Band Low Power",		L, 0, SOMag);
	CatDesc SO10LDesc("10M_L_U",	"28",		"Single Op 28 MHz Low Power",		L, 1, SOMag);
	CatDesc SO15LDesc("15M_L_U",	"21",		"Single Op 21 MHz Low Power",		L, 2, SOMag);
	CatDesc SO20LDesc("20M_L_U",	"14",		"Single Op 14 MHz Low Power",		L, 3, SOMag);
	CatDesc SO40LDesc("40M_L_U",	"7",		"Single Op 7 MHz Low Power",		L, 4, SOMag);
	CatDesc SO80LDesc("80M_L_U",	"3.5",		"Single Op 3.5 MHz Low Power",		L, 6, SOMag);
	CatDesc SO160LDesc("160M_L_U",	"1.8",		"Single Op 1.8 MHz Band Low Power", L, 7, SOMag);

	if (_mode == "SSB")
		SO80LDesc = CatDesc("80M_L_U", "3.7",	"Single Op 3.7 MHz Low Power",		L, 6, SOMag);

	CatDesc	SOABQDesc("ALL_Q_U",		"A",		"Single Op All Band QRP",			Q, 0, QRPMag);
	CatDesc SO10QDesc("10M_Q_U",	"28",		"Single Op 28 MHz QRP",				Q, 1, QRPMag);
	CatDesc SO15QDesc("15M_Q_U",	"21",		"Single Op 21 MHz QRP",				Q, 2, QRPMag);
	CatDesc SO20QDesc("20M_Q_U",	"14",		"Single Op 14 MHz QRP",				Q, 3, QRPMag);
	CatDesc SO40QDesc("40M_Q_U",	"7",		"Single Op 7 MHz QRP",				Q, 4, QRPMag);
	CatDesc SO80QDesc("80M_Q_U",	"3.5",		"Single Op 3.5 MHz QRP",			Q, 6, QRPMag);
	CatDesc SO160QDesc("160M_Q_U",	"1.8",		"Single Op 1.8 MHz QRP",			Q, 7, QRPMag);

	if (_mode == "SSB")
		SO80QDesc = CatDesc("80M_Q_U", "3.7",	"Single Op 3.7 MHz QRP",			Q, 6, QRPMag);

	CatDesc	MSDesc("MS", "MS", "Multi-Single High", H, 0, MSMag);
	CatDesc	M2Desc("M2", "M2", "Multi-Two",			H, 0, M2Mag);
	CatDesc	MMDesc("MM", "MM", "Multi-Multi",		H, 0, MMMag);

	CatDesc CKDesc("CK", "CK", "Checklog",			H, 0, CKMag);

	_catDescByNum.insert(100, SOABHDesc);
	_catDescByNum.insert(200, SO10HDesc);
	_catDescByNum.insert(300, SO15HDesc);
	_catDescByNum.insert(400, SO20HDesc);
	_catDescByNum.insert(500, SO40HDesc);
	_catDescByNum.insert(600, SO80HDesc);
	_catDescByNum.insert(700, SO160HDesc);

	_catDescByNum.insert(110, SOABLDesc);
	_catDescByNum.insert(210, SO10LDesc);
	_catDescByNum.insert(310, SO15LDesc);
	_catDescByNum.insert(410, SO20LDesc);
	_catDescByNum.insert(510, SO40LDesc);
	_catDescByNum.insert(610, SO80LDesc);
	_catDescByNum.insert(710, SO160LDesc);

	_catDescByNum.insert(120, SOABQDesc);
	_catDescByNum.insert(220, SO10QDesc);
	_catDescByNum.insert(320, SO15QDesc);
	_catDescByNum.insert(420, SO20QDesc);
	_catDescByNum.insert(520, SO40QDesc);
	_catDescByNum.insert(620, SO80QDesc);
	_catDescByNum.insert(720, SO160QDesc);

	_catDescByNum.insert(1000, MSDesc);
	_catDescByNum.insert(2000, M2Desc);
	_catDescByNum.insert(3000, MMDesc);

	_catDescByNum.insert(10000, CKDesc);

	foreach(int num, _catDescByNum.keys())
		_catNumByInternalDesc.insert(_catDescByNum.value(num).Internal(), num);
}

void OcDxContest::WriteTabHeader(QTextStream & strm)
{
	strm << "Call"			<<	'\t'
		<<  "Cty Px"		<<	'\t'
		<<  "Cty Name"		<<	'\t'
		<<  "Cont"			<<	'\t'
		<<  "Loc"			<<	'\t'
		<<  "Dist"			<<	'\t'
		<<  "Cat"			<<	'\t'
		<<  "Mode"			<<	'\t'
		<<  "Cert"			<<	'\t'
		<<  "Claimed"		<<	'\t'
		<<  "Raw Scr"		<<	'\t'
		<<  "Final Scr"		<<	'\t'
		<<  "Reduct"		<<	'\t'
		<<  "Raw Q"			<<	'\t'
		<<  "Dupes"			<<	'\t'
		<<  "B"				<<	'\t'
		<<  "NIL"			<<	'\t'
		<<  "X"				<<	'\t'
		<<  "BCV"			<<	'\t'
		<<  "TV"			<<	'\t'
		<<  "Uni"			<<	'\t'
		<<  "C B"			<<	'\t'
		<<  "C NIL"			<<	'\t'
		<<  "C X"			<<	'\t'
		<<  "Mult"			<<	'\t'
		<<  "Qso"			<<	'\t'
		<<  "160M"			<<	'\t'
		<<  "80M"			<<	'\t'
		<<  "40M"			<<	'\t'
		<<  "20M"			<<	'\t'
		<<  "15M"			<<	'\t'
		<<  "10M"			<<	'\t'
		<<  "CatOp"			<<	'\t'
		<<  "CatTx"			<<	'\t'
		<<  "CatTime"		<<	'\t'
		<<  "CatOverlay"	<<	'\t'
		<<  "CatBand"		<<	'\t'
		<<  "CatPwr"		<<	'\t'
		<<  "CatMode"		<<	'\t'
		<<  "CatAssisted"	<<	'\t'
		<<  "CatStn"		<<	'\t'
		<<  "Club"			<<	'\t'
		<<  "Operators"		<<	'\t'
		<<  "Name"			<<	'\t'
		<<  "Email"			<<	'\t'
		<<  "HQ From"		<<	'\t'
		<<  "Soap Box"		<<	'\t'
		<<  "Created By"	<<	'\t'
		<<  "Op Hrs"		<<	'\t'
		<<  "City"			<<	'\t'
		<<  "State"			<<	'\t'
		<<  "Postcode"		<<	'\t'
		<<  "Country"		<<	'\t'
		<<  "Addr 1"		<<	'\t'
		<<  "Addr 2"		<<	'\t'
		<<  "Addr 3"		<<	'\t'
		<<  "Addr 4"		<<	'\t'
		<<  "Addr 5"		<<	'\t'
		<<  "Addr 6"		<<	'\t'
		<< endl;
}
