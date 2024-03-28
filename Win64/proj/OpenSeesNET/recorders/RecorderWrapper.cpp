#include "stdafx.h"

#include "RecorderWrapper.h"
#include "../domains/timeSeries/TimeSeriesWrapper.h"

using namespace System::Runtime::InteropServices;
using namespace System;
using namespace OpenSees;
using namespace OpenSees::DamageModels;
using namespace OpenSees::Recorders;
using namespace OpenSees::Components;
using namespace OpenSees::Handlers;


NodeRecorderWrapper::NodeRecorderWrapper(IDWrapper^ theDof,
	IDWrapper^ theNodes, int sensitivity, String^ dataToStore,
	BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int procDataMethod, int procGrpNum, double deltaT,
	bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries) {
	TimeSeries** theSeries = new TimeSeries * [timeSeries->Length];
	for (int i = 0; i < timeSeries->Length; i++)
	{
		theSeries[i] = timeSeries[i]->_TimeSeries;
	}

	_Recorder = new NodeRecorder(*theDof->_ID, theNodes->_ID, sensitivity, (char*)(void*)Marshal::StringToHGlobalAnsi(dataToStore),
		*theDomain->_Domain, theOutputHandler->_OPS_StreamPtr, procDataMethod, procGrpNum, deltaT, echoTimeFlag, theSeries);
}

DriftRecorderWrapper::DriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theHandler,
	int procDataMethod, int procGrpNum, bool echoTime, double deltaT) {
	_Recorder = new DriftRecorder(ndI, ndJ, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr,
		procDataMethod, procGrpNum, echoTime, deltaT);
}


DriftRecorderWrapper::DriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theHandler,
	int procDataMethod, int procGrpNum, bool echoTime, double deltaT) {
	_Recorder = new

		DriftRecorder(*ndI->_ID, *ndJ->_ID, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr,
			procDataMethod, procGrpNum, echoTime, deltaT);
}


ElementRecorderWrapper::ElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	bool echoTime,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theOutputHandler,
	int procMethod, int procGrpNum, double deltaT,
	IDWrapper^ dof) {


	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}

	_Recorder = new ElementRecorder(eleID->_ID, cargv,
		argv->Length, echoTime,
		*theDomain->_Domain, theOutputHandler->_OPS_StreamPtr, procMethod, procGrpNum, deltaT, dof->_ID);
}

EnvelopeDriftRecorderWrapper::EnvelopeDriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int procDataMethod, int procGrpNum, bool echoTime) {
	_Recorder = new EnvelopeDriftRecorder(ndI, ndJ, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr, procDataMethod, procGrpNum, echoTime);
}

EnvelopeDriftRecorderWrapper::
EnvelopeDriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int procDataMethod, int procGrpNum, bool echoTime) {
	_Recorder = new
		EnvelopeDriftRecorder(*ndI->_ID, *ndJ->_ID, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr, procDataMethod, procGrpNum, echoTime);
}

EnvelopeElementRecorderWrapper::EnvelopeElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int procMethod, int procGrpNum,	bool echoTimeFlag, IDWrapper^ dof) {
	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}
	_Recorder = new EnvelopeElementRecorder(eleID->_ID, cargv, argv->Length, *theDomain->_Domain, theHandler->_OPS_StreamPtr, procMethod, procGrpNum, echoTimeFlag, dof->_ID);
}


EnvelopeNodeRecorderWrapper::EnvelopeNodeRecorderWrapper(IDWrapper^ theDof,
	IDWrapper^ theNodes, String^ dataToStore,
	BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int procDataMethod, int procGrpNum,
	bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries) {
	TimeSeries** theSeries = new TimeSeries * [timeSeries->Length];
	for (int i = 0; i < timeSeries->Length; i++)
	{
		theSeries[i] = timeSeries[i]->_TimeSeries;
	}

	_Recorder = new EnvelopeNodeRecorder(*theDof->_ID, theNodes->_ID, (char*)(void*)Marshal::StringToHGlobalAnsi(dataToStore),
		*theDomain->_Domain, theOutputHandler->_OPS_StreamPtr, procDataMethod, procGrpNum, echoTimeFlag, theSeries);
}

ResidDriftRecorderWrapper::ResidDriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int procDataMethod, int procGrpNum, bool echoTime) {
	_Recorder = new ResidDriftRecorder(ndI, ndJ, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr, procDataMethod, procGrpNum, echoTime);
}

ResidDriftRecorderWrapper::
ResidDriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int procDataMethod, int procGrpNum, bool echoTime) {
	_Recorder = new
		ResidDriftRecorder(*ndI->_ID, *ndJ->_ID, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr, procDataMethod, procGrpNum, echoTime);
}

ResidElementRecorderWrapper::ResidElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int procMethod, int procGrpNum,	bool echoTimeFlag, IDWrapper^ dof) {
	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}
	_Recorder = new ResidElementRecorder(eleID->_ID, cargv, argv->Length, *theDomain->_Domain, theHandler->_OPS_StreamPtr, procMethod, procGrpNum, echoTimeFlag, dof->_ID);
}


ResidNodeRecorderWrapper::ResidNodeRecorderWrapper(IDWrapper^ theDof,
	IDWrapper^ theNodes, String^ dataToStore,
	BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int procDataMethod, int procGrpNum,
	bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries) {
	TimeSeries** theSeries = new TimeSeries * [timeSeries->Length];
	for (int i = 0; i < timeSeries->Length; i++)
	{
		theSeries[i] = timeSeries[i]->_TimeSeries;
	}

	_Recorder = new ResidNodeRecorder(*theDof->_ID, theNodes->_ID, (char*)(void*)Marshal::StringToHGlobalAnsi(dataToStore),
		*theDomain->_Domain, theOutputHandler->_OPS_StreamPtr, procDataMethod, procGrpNum, echoTimeFlag, theSeries);
}

ConditionalDriftRecorderWrapper::ConditionalDriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int rcrdrTag, int procDataMethod, int procGrpNum, bool echoTime) {
	_Recorder = new ConditionalDriftRecorder(ndI, ndJ, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr, rcrdrTag, procDataMethod, procGrpNum, echoTime);
}

ConditionalDriftRecorderWrapper::
ConditionalDriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int rcrdrTag, int procDataMethod, int procGrpNum, bool echoTime) {
	_Recorder = new
		ConditionalDriftRecorder(*ndI->_ID, *ndJ->_ID, dof, perpDirn, *theDomain->_Domain, theHandler->_OPS_StreamPtr, rcrdrTag, procDataMethod, procGrpNum, echoTime);
}

ConditionalElementRecorderWrapper::ConditionalElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	int rcrdrTag, int procMethod, int procGrpNum,	bool echoTimeFlag, IDWrapper^ dof) {
	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}
	_Recorder = new ConditionalElementRecorder(eleID->_ID, cargv, argv->Length, *theDomain->_Domain, theHandler->_OPS_StreamPtr, rcrdrTag, procMethod, procGrpNum, echoTimeFlag, dof->_ID);
}


ConditionalNodeRecorderWrapper::ConditionalNodeRecorderWrapper(IDWrapper^ theDof,
	IDWrapper^ theNodes, String^ dataToStore,
	BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int rcrdrTag, int procDataMethod, int procGrpNum,
	bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries) {
	TimeSeries** theSeries = new TimeSeries * [timeSeries->Length];
	for (int i = 0; i < timeSeries->Length; i++)
	{
		theSeries[i] = timeSeries[i]->_TimeSeries;
	}

	_Recorder = new ConditionalNodeRecorder(*theDof->_ID, theNodes->_ID, (char*)(void*)Marshal::StringToHGlobalAnsi(dataToStore),
		*theDomain->_Domain, theOutputHandler->_OPS_StreamPtr, rcrdrTag, procDataMethod, procGrpNum, echoTimeFlag, theSeries);
}

/*
DamageRecorderWrapper::DamageRecorderWrapper(int elemid, IDWrapper^ secIDs, int dofid, DamageModelWrapper^ dmgPtr,
	BaseDomainWrapper^ theDomainPtr,
	bool echotimeflag, double deltat, OPS_StreamWrapper^ theOutputStream) {
	_Recorder = new DamageRecorder(elemid, *secIDs->_ID, dofid, dmgPtr->_DamageModel, *theDomainPtr->_Domain, echotimeflag, deltat, *theOutputStream->_OPS_StreamPtr);
}

MaxNodeDispRecorderWrapper::MaxNodeDispRecorderWrapper(int dof, IDWrapper^ theNodes, BaseDomainWrapper^ theDomain) {
	_Recorder = new MaxNodeDispRecorder(dof, *theNodes->_ID, *theDomain->_Domain);
}

NormElementRecorderWrapper::NormElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	bool echoTimeFlag,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	double deltaT,
	IDWrapper^ dof) {

	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}

	_Recorder = new NormElementRecorder(eleID->_ID, cargv, argv->Length, echoTimeFlag, *theDomain->_Domain, *theHandler->_OPS_StreamPtr, deltaT, dof->_ID);
}


NormElementRecorderWrapper::NormElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	bool echoTimeFlag,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler) {

	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}

	_Recorder = new		NormElementRecorder(eleID->_ID, cargv, argv->Length, echoTimeFlag, *theDomain->_Domain, *theHandler->_OPS_StreamPtr);
}


NormEnvelopeElementRecorderWrapper::NormEnvelopeElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler,
	double deltaT,
	bool echoTimeFlag,
	IDWrapper^ dof) {

	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}

	_Recorder = new NormEnvelopeElementRecorder(eleID->_ID, cargv, argv->Length, *theDomain->_Domain, *theHandler->_OPS_StreamPtr, deltaT, echoTimeFlag, dof->_ID);
}


NormEnvelopeElementRecorderWrapper::NormEnvelopeElementRecorderWrapper(IDWrapper^ eleID,
	array<String^>^ argv,
	BaseDomainWrapper^ theDomain,
	OPS_StreamWrapper^ theHandler) {

	const char** cargv = new const char*[argv->Length];
	for (int i = 0; i < argv->Length; i++)
	{
		cargv[i] = (char*)(void*)Marshal::StringToHGlobalAnsi(argv[i]);
	}
	_Recorder = new		NormEnvelopeElementRecorder(eleID->_ID, cargv, argv->Length, *theDomain->_Domain, *theHandler->_OPS_StreamPtr);
}

PatternRecorderWrapper::PatternRecorderWrapper(int thePattern,
	BaseDomainWrapper^ theDomain,
	String^ argv,
	double deltaT,
	int startFlag) {
	_Recorder = new PatternRecorder(thePattern, *theDomain->_Domain, (char*)(void*)Marshal::StringToHGlobalAnsi(argv), deltaT, startFlag);
}*/








