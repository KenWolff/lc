#include "StdAfx.h"

// all this is initialized in the Contest contsructor -- not goog

QMap <QString, int>			gDistrictBySectionMap;
QMap <QString, QString>		gStateAbbreviationMap;
QMap <QString, QString>		gStateByFccCallMap;		// initialized by reading fcc data
QMap <QString, QString>		gLicenseByFccCallMap;		// initialized by reading fcc data
QMap <QString, int>			gZoneByStateMap;
