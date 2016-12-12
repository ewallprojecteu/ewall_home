#include "DGWdefs.h"
#include <sstream>

inline int getSensorNameUnitMote(int metavalue, string &name, string &unit, string &mote)
{
	switch (metavalue)
	{
		case _TEMPERATURE:
			name = "temperature"; unit = "°C"; mote = "";
			return 0; //nonCA device
		case _HUMIDITY:
			name = "humidity"; unit = "%"; mote = "";
			return 0;
		case _LUMINOCITY:
			name = "illuminance"; unit = "lux"; mote = "";
			return 0;
		case _ACTIVITY:
			name = "movement"; unit = "bool"; mote = "";
			return 0;
		case _GASES:
			name = "gas"; unit = ""; mote = "";
			return 0;
		case _LPG:
			name = "LPG"; unit = ""; mote = "";
			return 0;
		case _NG:
			name = "NG"; unit = ""; mote = "";
			return 0;
		case _CO:
			name = "CO"; unit = ""; mote = "";
			return 0;
		case _DOOR:
			name = "door_open"; unit = "bool"; mote = "";
			return 0;
		case _BPM:
			name = "BPM"; unit = ""; mote = "";
			return 0;
		case _SPO:
			name = "SPO"; unit = ""; mote = "";
			return 0;

		case _MDC_PULS_OXIM_SAT_O2: //19384
			name = "SPO2"; unit = "%"; mote = "pulse_oximeter";
			return 1; //CA device
		case _MDC_PULS_OXIM_PULS_RATE: //18458
			name = "HR"; unit = "bpm"; mote = "pulse_oximeter";
			return 1;
		case _MDC_PULS_RATE_NON_INV: //18474
			name = "noninvBPPR"; unit = "bpm"; mote = "blood_pressure";
			return 1;
		case _MDC_PRESS_BLD_NONINV_SYS: //18949
			name = "systolicBP"; unit = "mmHg"; mote = "blood_pressure";
			return 1;
		case _MDC_PRESS_BLD_NONINV_DIA: //18950
			name = "diastolicBP"; unit = "mmHg"; mote = "blood_pressure";
			return 1;
		case _MDC_PRESS_BLD_NONINV_MEAN: //18951
			name = "meanABP"; unit = "mmHg"; mote = "blood_pressure";
			return 1;
		case _MDC_CONC_GLU_CAPILLARY_WHOLEBLOOD: //29112
			name = "glucose_concentration_capillary"; unit = "mg dL-1"; mote = "glucometer";
			return 1;
		case _MDC_LEN_BODY_ACTUAL: //57668
			name = "length_body_actual"; unit = "cm"; mote = "weighing_scale";
			return 1;
		case _MDC_RATIO_MASS_BODY_LEN_SQ: //57680
			name = "ratio_mass_body_length"; unit = ""; mote = "weighing_scale";
			return 1;
		case _MDC_MASS_BODY_ACTUAL: 
			name = "mass_body_actual"; unit = "kg"; mote = "weighing_scale";
			return 1;
		default:
			name = ""; unit = ""; mote = "";
			return -1;
	}
}

inline int getSensorValueType(int metavalue, float inputValue, string &outValue)
{
	std::stringstream strs;
	switch (metavalue)
	{
		case _ACTIVITY:
		case _DOOR:
			if (inputValue == 0)
			{
				strs << "false\0";
				outValue = strs.str();
			}
			else
			{
				strs << "true\0";
				outValue = strs.str();
			}
			return 1;
		case _GASES:
		case _LPG:
		case _NG:
		case _CO:
			strs << max(min((int)(inputValue), 10), 0) << "\0";
			outValue = strs.str();
			return 1;
		case _TEMPERATURE:
		case _HUMIDITY:
		case _LUMINOCITY:		
		case _BPM:
		case _SPO:
		case _MDC_PULS_OXIM_SAT_O2: //19384
		case _MDC_PULS_OXIM_PULS_RATE: //18458
		case _MDC_PULS_RATE_NON_INV: //18474
		case _MDC_PRESS_BLD_NONINV_SYS: //18949
		case _MDC_PRESS_BLD_NONINV_DIA: //18950
		case _MDC_PRESS_BLD_NONINV_MEAN: //18951
		case _MDC_CONC_GLU_CAPILLARY_WHOLEBLOOD: //29112
		case _MDC_LEN_BODY_ACTUAL: //57668
		case _MDC_RATIO_MASS_BODY_LEN_SQ: //57680
		case _MDC_MASS_BODY_ACTUAL:
			strs << inputValue << "\0";
			outValue = strs.str();
			return 1;
		default:
			strs << "\0";
			outValue = strs.str();
			return -1;
	}
	return 1;
}
