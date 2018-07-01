#include "StdAfx.h"

void VhfContest::MarkNilBust()
{
	Contest::MarkNilBust();

	foreach(Log * log, _logsByCall)				// record grid visited
	{
		foreach(QsoPtr q, log->AllQso())
		{
			QMap < QString, QsoPtr> & gridsSentMap = ((VhfLog *)log)->GetGridsSentMap();
			if (!gridsSentMap.contains(q->XchSent()))
				gridsSentMap[q->XchSent()] = q;
		}
	}
}

void VhfContest::WriteTsvHeader(QTextStream & strm)
{
	strm << "cabrillo_id"	<< '\t'
		<< "year"			<< '\t'
		<< "callsign"		<< '\t'
		<< "host_callsign"	<< '\t'
		<< "operators"		<< '\t'
		<< "category_id"	<< '\t'
		<< "section_id"		<< '\t'
		<< "dxcc_prefix"	<< '\t'
		<< "score"			<< '\t'
		<< "q_total"		<< '\t'
		<< "m_total"		<< '\t'
		<< "late_log_flag"	<< '\t'
		<< "clb_abbr"		<< '\t'
		<< "minutes_on"		<< '\t';

	foreach(QString band, BandNameList())
		strm << "q_" + band << '\t' << "pts_" + band << '\t' << "m_" + band << '\t';

	strm << "band_activated" << '\t' << "grid_activated" << '\t';
	strm << endl;
}


void VhfContest::WriteTabHeader(QTextStream & strm)
{
	strm <<	"Call"						<<	'\t'
		<<	"Loc"						<<	'\t'
		<<	"Div"						<<	'\t'
		<<	"Reg"						<<	'\t'
		<<	"Dist"						<<	'\t'
		<<  "Cat"						<<  '\t'
		<<  "HqCat"						<<  '\t'
		<<	"Cert"						<<	'\t'
		<<	"Claimed"					<<	'\t'
		<<	"Raw Q"						<<	'\t'
		<<	"Raw M"						<<	'\t'
		<<	"Raw Scr"					<<	'\t'
		<<	"Final Q"					<<	'\t'
		<<	"Final M"					<<	'\t'
		<<	"Grids Activated (rovers)"	<<	'\t'
		<<	"Final Scr"					<<	'\t'
		<<	"Reduct"					<<	'\t'
		<<	"Dupes"						<<	'\t'
		<<	"B"							<<	'\t'
		<<	"NIL"						<<	'\t'
		<<	"X"							<<	'\t'
		<<	"Uni"						<<	'\t'
		<<	"C B"						<<	'\t'
		<<	"C NIL"						<<	'\t'
		<<	"C X"						<<	'\t';

	foreach(QString band, _bandNameList)
	{
		strm <<  QString("%1\t%2\t")
			.arg(band + " Q")
			.arg(band + " M");
	 }

	strm <<  "Bands Operated"	<<	'\t'
	     <<  "CatOp"        	<<	'\t'
		 <<  "CatTx"			<<	'\t'
		 <<  "CatTime"			<<	'\t'
		 <<  "CatOverlay"		<<	'\t'
		 <<  "CatBand"			<<	'\t'
		 <<  "CatPwr"			<<	'\t'
		 <<  "CatMode"			<<	'\t'
		 <<  "CatAssisted"		<<	'\t'
		 <<  "CatStn"			<<	'\t'
		 <<  "Club"				<<	'\t'
		 <<  "Operators"		<<	'\t'
		 <<  "Name"				<<	'\t'
		 <<  "Email"			<<	'\t'
		 <<  "HQ-FROM"			<<	'\t'
		 <<  "HQ-CLUB-ABBR"		<<	'\t'
		 <<  "Soap Box"			<<	'\t'
		 <<  "Created By"		<<	'\t'
		 <<  "Op Hrs"			<<	'\t'
		 <<  "City"				<<	'\t'
		 <<  "State"			<<	'\t'
		 <<  "Postcode"			<<	'\t'
		 <<  "Country"			<<	'\t'
		 <<  "Addr 1"			<<	'\t'
		 <<  "Addr 2"			<<	'\t'
		 <<  "Addr 3"			<<	'\t'
		 <<  "Addr 4"			<<	'\t'
		 <<  "Addr 5"			<<	'\t'
		 <<  "Addr 6"			<<	endl;
}

void VhfContest::Init()
{
	_mode = "ALL";
	_numberOfFields	= 8;
	gSpt = gRbn = false;

	_bandNameList
		<< B6.Name()
		<< B2.Name()
		<< B222.Name()
		<< B432.Name()
		<< B903.Name()
		<< B1296.Name()
		<< B2304.Name()
		<< B3456.Name()
		<< B5760.Name()
		<< B10G.Name()
		<< B24G.Name()
		<< B47G.Name()
		<< B75G.Name()
		<< B119G.Name()
		<< B142G.Name()
		<< B241G.Name()
		<< BLight.Name();

	_sentRstFldNum  = 0;		  // not used
	_rcvdRstFldNum  = 0;		  // not used
	_sentXchFldNum  = 5;
	_rcvdCallFldNum = 6;
	_rcvdXchFldNum  = 7;
	_hqCategoryMap.insert( "Single Operator, VHF 3-band"			, "3B");
	_hqCategoryMap.insert( "Single Operator, 3-band"				, "3B");
	_hqCategoryMap.insert( "Single Operator, 3 Band"				, "3B");
	_hqCategoryMap.insert( "Single Operator, Low Power"				, "A" );
	_hqCategoryMap.insert( "Single Operator, High Power"			, "B" );
	_hqCategoryMap.insert( "Single Operator, VHF FM-only"			, "FM");
	_hqCategoryMap.insert( "Single Operator, FM Only"				, "FM");
	_hqCategoryMap.insert( "Limited Multioperator"					, "L" );
	_hqCategoryMap.insert( "Multioperator (Unlimited)"				, "M");
	_hqCategoryMap.insert( "Unlimited Multioperator"				, "M");
	_hqCategoryMap.insert( "Single Operator, Portable"				, "Q" );
	_hqCategoryMap.insert( "Rover"									, "R" );
	_hqCategoryMap.insert( "Classic Rover"							, "R" );
	_hqCategoryMap.insert( "Limited Rover"							, "RL");
	_hqCategoryMap.insert( "Unlimited Rover"						, "RU");
	_hqCategoryMap.insert( "Checklog"								, "CK");

	//SO3B	Single Operator, 3 Band
	//SOLP	Single Operator, Low Power
	//SOHP	Single Operator, High Power
	//SOFM	Single Operator, FM Only
	//LM		Limited Multioperator
	//UM		Unlimited Multioperator
	//SOP		Single Operator, Portable
	//R		Classic Rover
	//RL		Limited Rover
	//RU		Unlimited Rover


	foreach(QString key, _hqCategoryMap.keys())
		_revHqCategoryMap.insert(_hqCategoryMap.value(key), key);
}

void VhfContest::WriteTxtHeader(QTextStream & strm)
{
	strm << "CALL         RAW Q   D   N   B   X   Pts Mults    Final      Raw  Claimed   Delta  Hrs Cat" << endl;
}

VhfContest::VhfContest() : ArrlContest(){}
