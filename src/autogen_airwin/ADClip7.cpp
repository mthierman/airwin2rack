/* ========================================
 *  ADClip7 - ADClip7.h
 *  Copyright (c) 2016 airwindows, Airwindows uses the MIT license
 * ======================================== */

#ifndef __ADClip7_H
#include "ADClip7.h"
#endif
#include <cmath>
#include <algorithm>
namespace airwinconsolidated::ADClip7 {

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {return new ADClip7(audioMaster);}

ADClip7::ADClip7(audioMasterCallback audioMaster) :
    AudioEffectX(audioMaster, kNumPrograms, kNumParameters)
{
	A = 0.0;
	B = 0.5;
	C = 0.5;
	D = 0.0;

	lastSampleL = 0.0;
	lastSampleR = 0.0;
	for(int count = 0; count < 22199; count++) {bL[count] = 0; bR[count] = 0;}
	gcount = 0;
	lowsL = 0;
	lowsR = 0;
	refclipL = 0.99;
	refclipR = 0.99;
	iirLowsAL = 0.0;
	iirLowsAR = 0.0;
	iirLowsBL = 0.0;	
	iirLowsBR = 0.0;	
	
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	//this is reset: values being initialized only once. Startup values, whatever they are.
	
    _canDo.insert("plugAsChannelInsert"); // plug-in can be used as a channel insert effect.
    _canDo.insert("plugAsSend"); // plug-in can be used as a send effect.
    _canDo.insert("x2in2out"); 
    setNumInputs(kNumInputs);
    setNumOutputs(kNumOutputs);
    setUniqueID(kUniqueId);
    canProcessReplacing();     // supports output replacing
    canDoubleReplacing();      // supports double precision processing
	programsAreChunks(true);
    vst_strncpy (_programName, "Default", kVstMaxProgNameLen); // default program name
}

ADClip7::~ADClip7() {}
VstInt32 ADClip7::getVendorVersion () {return 1000;}
void ADClip7::setProgramName(char *name) {vst_strncpy (_programName, name, kVstMaxProgNameLen);}
void ADClip7::getProgramName(char *name) {vst_strncpy (name, _programName, kVstMaxProgNameLen);}
//airwindows likes to ignore this stuff. Make your own programs, and make a different plugin rather than
//trying to do versioning and preventing people from using older versions. Maybe they like the old one!

static float pinParameter(float data)
{
	if (data < 0.0f) return 0.0f;
	if (data > 1.0f) return 1.0f;
	return data;
}

void ADClip7::setParameter(VstInt32 index, float value) {
    switch (index) {
        case kParamA: A = value; break;
        case kParamB: B = value; break;
        case kParamC: C = value; break;
        case kParamD: D = value; break;
        default: break; // unknown parameter, shouldn't happen!
    }
}

float ADClip7::getParameter(VstInt32 index) {
    switch (index) {
        case kParamA: return A; break;
        case kParamB: return B; break;
        case kParamC: return C; break;
        case kParamD: return D; break;
        default: break; // unknown parameter, shouldn't happen!
    } return 0.0; //we only need to update the relevant name, this is simple to manage
}

void ADClip7::getParameterName(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "Boost", kVstMaxParamStrLen); break;
		case kParamB: vst_strncpy (text, "Soften", kVstMaxParamStrLen); break;
		case kParamC: vst_strncpy (text, "Enhance", kVstMaxParamStrLen); break;
		case kParamD: vst_strncpy (text, "Mode", kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
    } //this is our labels for displaying in the VST host
}

void ADClip7::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: float2string (A*18.0, text, kVstMaxParamStrLen); break;
        case kParamB: float2string (B, text, kVstMaxParamStrLen); break;
        case kParamC: float2string (C, text, kVstMaxParamStrLen); break;
        case kParamD: switch((VstInt32)( D * 2.999 )) //0 to almost edge of # of params
		{case 0: vst_strncpy (text, "Normal", kVstMaxParamStrLen); break;
		 case 1: vst_strncpy (text, "Atten", kVstMaxParamStrLen); break;
		 case 2: vst_strncpy (text, "Clips", kVstMaxParamStrLen); break;
		 default: break; // unknown parameter, shouldn't happen!
		} break;
        default: break; // unknown parameter, shouldn't happen!
	} //this displays the values and handles 'popups' where it's discrete choices
}

void ADClip7::getParameterLabel(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "dB", kVstMaxParamStrLen); break;
        case kParamB: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamC: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamD: vst_strncpy (text, "", kVstMaxParamStrLen); break;
		default: break; // unknown parameter, shouldn't happen!
    }
}

VstInt32 ADClip7::canDo(char *text) 
{ return (_canDo.find(text) == _canDo.end()) ? -1: 1; } // 1 = yes, -1 = no, 0 = don't know

bool ADClip7::getEffectName(char* name) {
    vst_strncpy(name, "ADClip7", kVstMaxProductStrLen); return true;
}

VstPlugCategory ADClip7::getPlugCategory() {return kPlugCategEffect;}

bool ADClip7::getProductString(char* text) {
  	vst_strncpy (text, "airwindows ADClip7", kVstMaxProductStrLen); return true;
}

bool ADClip7::getVendorString(char* text) {
  	vst_strncpy (text, "airwindows", kVstMaxVendorStrLen); return true;
}
bool ADClip7::parameterTextToValue(VstInt32 index, const char *text, float &value) {
    switch(index) {
    case kParamA: { auto b = string2float(text, value); if (b) { value = value / (18.0); } return b; break; }
    case kParamB: { auto b = string2float(text, value); return b; break; }
    case kParamC: { auto b = string2float(text, value); return b; break; }

    }
    return false;
}
bool ADClip7::canConvertParameterTextToValue(VstInt32 index) {
    switch(index) {
        case kParamA: return true;
        case kParamB: return true;
        case kParamC: return true;

    }
    return false;
}
} // end namespace
