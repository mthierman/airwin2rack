/* ========================================
 *  BitShiftGain - BitShiftGain.h
 *  Copyright (c) 2016 airwindows, Airwindows uses the MIT license
 * ======================================== */

#ifndef __BitShiftGain_H
#include "BitShiftGain.h"
#endif
#include <cmath>
#include <algorithm>
namespace airwinconsolidated::BitShiftGain {

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {return new BitShiftGain(audioMaster);}

BitShiftGain::BitShiftGain(audioMasterCallback audioMaster) :
    AudioEffectX(audioMaster, kNumPrograms, kNumParameters)
{
	A = 0.5;
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

BitShiftGain::~BitShiftGain() {}
VstInt32 BitShiftGain::getVendorVersion () {return 1000;}
void BitShiftGain::setProgramName(char *name) {vst_strncpy (_programName, name, kVstMaxProgNameLen);}
void BitShiftGain::getProgramName(char *name) {vst_strncpy (name, _programName, kVstMaxProgNameLen);}
//airwindows likes to ignore this stuff. Make your own programs, and make a different plugin rather than
//trying to do versioning and preventing people from using older versions. Maybe they like the old one!

static float pinParameter(float data)
{
	if (data < 0.0f) return 0.0f;
	if (data > 1.0f) return 1.0f;
	return data;
}

void BitShiftGain::setParameter(VstInt32 index, float value) {
    switch (index) {
        case kParamA: A = value; break;
        default: break; // unknown parameter, shouldn't happen!
    }
}

float BitShiftGain::getParameter(VstInt32 index) {
    switch (index) {
        case kParamA: return A; break;
        default: break; // unknown parameter, shouldn't happen!
    } return 0.0; //we only need to update the relevant name, this is simple to manage
}

void BitShiftGain::getParameterName(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "BitShift", kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
    } //this is our labels for displaying in the VST host
}

void BitShiftGain::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: int2string ((VstInt32)((A * 32)-16), text, kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
	} //this displays the values and handles 'popups' where it's discrete choices
}

void BitShiftGain::getParameterLabel(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "bits", kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
    }
}

VstInt32 BitShiftGain::canDo(char *text) 
{ return (_canDo.find(text) == _canDo.end()) ? -1: 1; } // 1 = yes, -1 = no, 0 = don't know

bool BitShiftGain::getEffectName(char* name) {
    vst_strncpy(name, "BitShiftGain", kVstMaxProductStrLen); return true;
}

VstPlugCategory BitShiftGain::getPlugCategory() {return kPlugCategEffect;}

bool BitShiftGain::getProductString(char* text) {
  	vst_strncpy (text, "airwindows BitShiftGain", kVstMaxProductStrLen); return true;
}

bool BitShiftGain::getVendorString(char* text) {
  	vst_strncpy (text, "airwindows", kVstMaxVendorStrLen); return true;
}
bool BitShiftGain::parameterTextToValue(VstInt32 index, const char *text, float &value) {
    switch(index) {
    case kParamA: { auto b = string2float(text, value); if (b) { value = std::clamp( (std::round(value) + 0.1 - (-16))/32, 0., 1. ); } return b; break; }

    }
    return false;
}
bool BitShiftGain::canConvertParameterTextToValue(VstInt32 index) {
    switch(index) {
        case kParamA: return true;

    }
    return false;
}
} // end namespace
