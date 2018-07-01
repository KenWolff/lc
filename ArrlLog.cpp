#include "StdAfx.h"
#include "ArrlLog.h"

std::map <QString, QString>	ArrlLog::_secToRegMap =
{
	{"AB",  "West Coast"},{"AK",  "West Coast"},{"AL",  "Southeast" },{"AR",  "Southeast" },{"AZ",  "West Coast"},{"BC",  "West Coast"},
	{"CO",  "Midwest"   },{"CT",  "Northeast" },{"DE",  "Northeast" },{"EB",  "West Coast"},{"EMA", "Northeast" },{"ENY", "Northeast" },
	{"EPA", "Northeast" },{"EWA", "West Coast"},{"GA",  "Southeast" },{"IA",  "Midwest"   },{"ID",  "West Coast"},{"IL",  "Central"   },
	{"IN",  "Central"   },{"KS",  "Midwest"   },{"KY",  "Central"   },{"LA",  "Southeast" },{"LAX", "West Coast"},{"MAR", "Northeast" },
	{"MB",  "Midwest"   },{"MDC", "Northeast" },{"ME",  "Northeast" },{"MI",  "Central"   },{"MN",  "Midwest"   },{"MO",  "Midwest"   },
	{"MS",  "Southeast" },{"MT",  "West Coast"},{"NC",  "Southeast" },{"ND",  "Midwest"   },{"NE",  "Midwest"   },{"NFL", "Southeast" },
	{"NH",  "Northeast" },{"NL",  "Northeast" },{"NLI", "Northeast "},{"NM",  "Midwest"   },{"NNJ", "Northeast" },{"NNY", "Northeast" },
	{"NT",  "West Coast"},{"NTX", "Midwest"   },{"NV",  "West Coast"},{"OH",  "Central"   },{"OK",  "Midwest"   },{"ON",  "Central"   },
	{"ONN", "Central"   },{"ONE", "Central"   },{"ONS", "Central"   },{"GTA", "Central"   },{"OR",  "West Coast"},{"ORG", "West Coast"},
	{"PAC", "West Coast"},{"PR",  "Southeast" },{"QC",  "Northeast" },{"RI",  "Northeast" },{"SB",  "West Coast"},{"SC",  "Southeast "},
	{"SCV", "West Coast"},{"SD",  "Midwest"   },{"SDG", "West Coast"},{"SF",  "West Coast"},{"SFL", "Southeast" },{"SJV", "West Coast"},
	{"SK",  "Midwest"   },{"SNJ", "Northeast" },{"STX", "Midwest"   },{"SV",  "West Coast"},{"TN" , "Southeast" },{"UT",  "Midwest"   },
	{"VA",  "Southeast" },{"VI",  "Southeast" },{"VT",  "Northeast" },{"WCF", "Southeast" },{"WI",  "Central"   },{"WMA", "Northeast" },
	{"WNY", "Northeast" },{"WPA", "Northeast" },{"WTX", "Midwest"   },{"WV",  "Southeast" },{"WWA", "West Coast"},{"WY",  "Midwest"   }
};

std::map <QString, QString>	ArrlLog::_secToDivMap =
{
	{"AB",  "Canada"		},{"AK", "Northwestern"		},{"AL", "Southeastern"	},{"AR", "Delta"			},{"AZ", "Southwestern"		},
	{"BC",  "Canada"		},{"CO", "Rocky Mountain"	},{"CT", "New England"	},{"DE", "Atlantic"			},{"EB",  "Pacific"			},
	{"EMA", "New England"	},{"ENY", "Hudson"			},{"EPA", "Atlantic"	},{"EWA", "Northwestern"	},{"GA",  "Southeastern"	},
	{"IA",  "Midwest"		},{"ID",  "Northwestern"	},{"IL", "Central"		},{ "IN",  "Central"		},{ "KS",  "Midwest"		},
	{"KY",  "Great Lakes"	},{"LA",  "Delta"			},{"LAX", "Southwestern"},{"MAR", "Canada"			},{"MB",  "Canada"			},
	{"MDC", "Atlantic"		},{"ME",  "New England"		},{"MI",  "Great Lakes"	},{"MN",  "Dakota"			},{"MO",  "Midwest"			},
	{"MS",  "Delta"			},{"MT",  "Northwestern"	},{"NC",  "Roanoke"		},{"ND",  "Dakota"			},{"NE",  "Midwest"			},
	{"NFL", "Southeastern"	},{"NH",  "New England"		},{"NL",  "Canada"		},{"NLI", "Hudson"			},{"NM",  "Rocky Mountain"	},
	{"NNJ", "Hudson"		},{"NNY", "Atlantic"		},{"NT",  "Canada"		},{"NTX", "West Gulf"		},{"NV",  "Pacific"			},
	{"OH",  "Great Lakes"	},{"OK",  "West Gulf"		},{"ON",  "Canada"		},{"ONE", "Canada"			},{"ONN", "Canada"			},
	{"ONS", "Canada"		},{"GTA", "Canada"			},{"OR",  "Northwestern"},{"ORG", "Southwestern"	},{"PAC", "Pacific"			},
	{"PR",  "Southeastern"	},{"QC",  "Canada"			},{"RI",  "New England"	},{"SB",  "Southwestern"	},{"SC",  "Roanoke"			},
	{"SCV", "Pacific"		},{"SD",  "Dakota"			},{"SDG", "Southwestern"},{"SF",  "Pacific"			},{"SFL", "Southeastern"	},
	{"SJV", "Pacific"		},{"SK",  "Canada"			},{"SNJ", "Atlantic"	},{"STX", "West Gulf"		},{"SV",  "Pacific"			},
	{"TN",  "Delta"			},{"UT",  "Rocky Mountain"	},{"VA",  "Roanoke"		},{"VI",  "Southeastern"	},{"VT",  "New England"		},
	{"WCF", "Southeastern"	},{"WI",  "Central"			},{"WMA", "New England"	},{"WNY", "Atlantic"		},{"WPA", "Atlantic"		},
	{"WTX", "West Gulf"		},{"WV",  "Roanoke"			},{"WWA", "Northwestern"},{"WY",  "Rocky Mountain"	}
};

ArrlLog::ArrlLog() : Log() {}
ArrlLog::~ArrlLog() {}
