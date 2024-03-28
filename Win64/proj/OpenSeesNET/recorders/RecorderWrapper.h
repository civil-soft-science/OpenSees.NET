#pragma once
#include <response\Response.h>
//#include <recorder\DamageRecorder.h>
#include <recorder\NodeRecorder.h>
#include <recorder\DriftRecorder.h>
#include <recorder\ElementRecorder.h>
#include <recorder\EnvelopeDriftRecorder.h>
#include <recorder\EnvelopeElementRecorder.h>
#include <recorder\EnvelopeNodeRecorder.h>
#include <recorder\EnvelopeDriftRecorder.h>
#include <recorder\ResidDriftRecorder.h>
#include <recorder\ResidElementRecorder.h>
#include <recorder\ResidNodeRecorder.h>
#include <recorder\ResidDriftRecorder.h>
#include <recorder\ConditionalDriftRecorder.h>
#include <recorder\ConditionalElementRecorder.h>
#include <recorder\ConditionalNodeRecorder.h>
#include <recorder\ConditionalDriftRecorder.h>
//#include <recorder\MaxNodeDispRecorder.h>
//#include <recorder\NormElementRecorder.h>
//#include <recorder\NormEnvelopeElementRecorder.h>
//#include <recorder\PatternRecorder.h>


#include "../taggeds/TaggedObjectWrapper.h"
#include "../actors/IMovableObjectWrapper.h"
#include "../matrix/IDWrapper.h"
#include "../domains/domain/BaseDomainWrapper.h"
#include "../domains/timeSeries/TimeSeriesWrapper.h"
#include "../damage/DamageModelWrapper.h"
#include "../handlers/HandlerWrapper.h"


using namespace System;
using namespace OpenSees;
using namespace OpenSees::DamageModels;
using namespace OpenSees::Components;
using namespace OpenSees::Components::Timeseries;
using namespace OpenSees::Handlers;

namespace OpenSees {
	namespace Recorders {
		

		public ref class RecorderWrapper : TaggedObjectWrapper, IMovableObjectWrapper
		{
		public:
			RecorderWrapper() {};
			~RecorderWrapper() {};
			String^ GetStreamHeader() {
				if(_Recorder == 0)
					return nullptr;
				else
				{
					OPS_Stream* opsstrptr = _Recorder->getOutputHandler();
					if(opsstrptr == 0) return nullptr;

					OPS_StreamWrapper^ opsstr = gcnew OPS_StreamWrapper(opsstrptr);
					return opsstr->GetStreamHeader();
				}
			}
			String^ GetClassType() {
				const char* type = _Recorder->getClassType();
				return gcnew String(type);
			}

			int GetClassTag() {
				return _Recorder->getClassTag();
			}

			int CloseOutputStreamHandler() {
				OPS_Stream* opsstrptr = _Recorder->getOutputHandler();
				if (opsstrptr == 0) return -1;
				OPS_StreamWrapper^ opsstr = gcnew OPS_StreamWrapper(opsstrptr);
				return opsstr->CloseStreamHeader();
			}

			String^ GetFilename() {
				const char* filename = _Recorder->getOutputHandlerFilename();
				if (filename == 0) return nullptr;
				return gcnew String(filename);
			}
		internal:
			RecorderWrapper(Recorder* recorder) {
				this->_Recorder = recorder;
				this->_TaggedObject = recorder;
			};
			Recorder * _Recorder;
		private:

		};

		public ref class NodeRecorderWrapper : RecorderWrapper
		{
		public:
			NodeRecorderWrapper(IDWrapper^ theDof,
				IDWrapper^ theNodes, int sensitivity, String^ dataToStore,
				BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int procDataMethod, int procGrpNum, double deltaT,
				bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries);
			~NodeRecorderWrapper() {};

		};

		public ref class DriftRecorderWrapper : RecorderWrapper
		{
		public:
			DriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler, int procDataMethod, int procGrpNum, bool echoTime, double deltaT);

			DriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int procDataMethod, int procGrpNum, bool echoTime, double deltaT);

			~DriftRecorderWrapper() {};

			

		};

		public ref class ElementRecorderWrapper : RecorderWrapper
		{
		public:
			ElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				bool echoTime,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theOutputHandler,
				int procMethod, int procGrpNum, double deltaT,
				IDWrapper^ dof);
			~ElementRecorderWrapper() {};

		};

		public ref class EnvelopeDriftRecorderWrapper : RecorderWrapper
		{
		public:
			EnvelopeDriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int procDataMethod, int procGrpNum, bool echoTime);
			EnvelopeDriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int procDataMethod, int procGrpNum, bool echoTime);
			~EnvelopeDriftRecorderWrapper() {};
		};

		public ref class EnvelopeElementRecorderWrapper : RecorderWrapper
		{
		public:
			EnvelopeElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int procMethod, int procGrpNum,
				bool echoTimeFlag,
				IDWrapper^ dof);

			~EnvelopeElementRecorderWrapper() {};
		};

		public ref class EnvelopeNodeRecorderWrapper : RecorderWrapper
		{
		public:
			EnvelopeNodeRecorderWrapper(IDWrapper^ theDof,
				IDWrapper^ theNodes, String^ dataToStore,
				BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int procDataMethod, int procGrpNum,
				bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries);
			~EnvelopeNodeRecorderWrapper() {};
		};

		public ref class ResidDriftRecorderWrapper : RecorderWrapper
		{
		public:
			ResidDriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int procDataMethod, int procGrpNum, bool echoTime);
			ResidDriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int procDataMethod, int procGrpNum, bool echoTime);
			~ResidDriftRecorderWrapper() {};
		};

		public ref class ResidElementRecorderWrapper : RecorderWrapper
		{
		public:
			ResidElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int procMethod, int procGrpNum,
				bool echoTimeFlag,
				IDWrapper^ dof);

			~ResidElementRecorderWrapper() {};
		};

		public ref class ResidNodeRecorderWrapper : RecorderWrapper
		{
		public:
			ResidNodeRecorderWrapper(IDWrapper^ theDof,
				IDWrapper^ theNodes, String^ dataToStore,
				BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int procDataMethod, int procGrpNum,
				bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries);
			~ResidNodeRecorderWrapper() {};
		};

		public ref class ConditionalDriftRecorderWrapper : RecorderWrapper
		{
		public:
			ConditionalDriftRecorderWrapper(int ndI, int ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int rcrdrTag, int procDataMethod, int procGrpNum, bool echoTime);
			ConditionalDriftRecorderWrapper(IDWrapper^ ndI, IDWrapper^ ndJ, int dof, int perpDirn,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int rcrdrTag, int procDataMethod, int procGrpNum, bool echoTime);
			~ConditionalDriftRecorderWrapper() {};
		};

		public ref class ConditionalElementRecorderWrapper : RecorderWrapper
		{
		public:
			ConditionalElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				int rcrdrTag, int procMethod, int procGrpNum,
				bool echoTimeFlag,
				IDWrapper^ dof);

			~ConditionalElementRecorderWrapper() {};
		};

		public ref class ConditionalNodeRecorderWrapper : RecorderWrapper
		{
		public:
			ConditionalNodeRecorderWrapper(IDWrapper^ theDof,
				IDWrapper^ theNodes, String^ dataToStore,
				BaseDomainWrapper^ theDomain, OPS_StreamWrapper^ theOutputHandler, int rcrdrTag, int procDataMethod, int procGrpNum,
				bool echoTimeFlag, array<TimeSeriesWrapper^>^ timeSeries);
			~ConditionalNodeRecorderWrapper() {};
		};

		/*
		public ref class DamageRecorderWrapper : RecorderWrapper
		{
		public:
			DamageRecorderWrapper(int elemid, IDWrapper^ secIDs, int dofid, DamageModelWrapper^ dmgPtr,
				BaseDomainWrapper^ theDomainPtr,
				bool echotimeflag, double deltat, OPS_StreamWrapper^ theOutputStream);
			~DamageRecorderWrapper() {};
		};

public ref class MaxNodeDispRecorderWrapper : RecorderWrapper
		{
		public:
			MaxNodeDispRecorderWrapper(int dof, IDWrapper^ theNodes, BaseDomainWrapper^ theDomain);
			~MaxNodeDispRecorderWrapper() {};
		};

		public ref class NormElementRecorderWrapper : RecorderWrapper
		{
		public:
			NormElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				bool echoTimeFlag,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				double deltaT,
				IDWrapper^ dof);

			NormElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				bool echoTimeFlag,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler);

			~NormElementRecorderWrapper() {};
		};

		public ref class NormEnvelopeElementRecorderWrapper : RecorderWrapper
		{
		public:
			NormEnvelopeElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler,
				double deltaT,
				bool echoTimeFlag,
				IDWrapper^ dof);

			NormEnvelopeElementRecorderWrapper(IDWrapper^ eleID,
				array<String^>^ argv,
				BaseDomainWrapper^ theDomain,
				OPS_StreamWrapper^ theHandler);

			~NormEnvelopeElementRecorderWrapper() {};
		};

		public ref class PatternRecorderWrapper : RecorderWrapper
		{
		public:
			PatternRecorderWrapper(int thePattern,
				BaseDomainWrapper^ theDomain,
				String^ argv,
				double deltaT,
				int startFlag);

			~PatternRecorderWrapper() {};
		};*/
	}
}