/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */

// $Revision: 1.16 $
// $Date: 2007-11-30 19:24:52 $
// $Source: /usr/local/cvs/OpenSees/SRC/recorder/DriftRecorder.cpp,v $

// Written: MHS
// Created: Oct 2001
//
// Description: This file contains the class definition for DriftRecorder.

#include <BinaryFileStream.h>
#include <Channel.h>
#include <DataFileStream.h>
#include <Domain.h>
#include <DriftRecorder.h>
#include <FEM_ObjectBroker.h>
#include <ID.h>
#include <Matrix.h>
#include <Node.h>
#include <StandardStream.h>
#include <Vector.h>
#include <XmlFileStream.h>
#include <elementAPI.h>
#include <math.h>
#include <string.h>

enum outputMode  {STANDARD_STREAM, DATA_STREAM, XML_STREAM, DATABASE_STREAM, BINARY_STREAM, DATA_STREAM_CSV, TCP_STREAM, DATA_STREAM_ADD};

DriftRecorder::DriftRecorder()
	 :Recorder(RECORDER_TAGS_DriftRecorder),
	 ndI(0), ndJ(0), dof(0), perpDirn(0), oneOverL(0), data(0),
	 theDomain(0), theOutputHandler(0),
	 initializationDone(false), numNodes(0), echoTimeFlag(false)
{

}


DriftRecorder::DriftRecorder(int ni,
	 int nj,
	 int df,
	 int dirn,
	 Domain& theDom,
	 OPS_Stream* theDataOutputHandler,
	 int procMethod, int procGrpN,
	 bool timeFlag,
	 double dT)
	 :Recorder(RECORDER_TAGS_DriftRecorder),
	 ndI(0), ndJ(0), theNodes(0), dof(df), perpDirn(dirn), oneOverL(0), data(0),
	 theDomain(&theDom), theOutputHandler(theDataOutputHandler),
	 initializationDone(false), numNodes(0), echoTimeFlag(timeFlag), deltaT(dT),
	 nextTimeStampToRecord(0.0)
	 , procDataMethod(procMethod), procGrpNum(procGrpN)
{
	 ndI = new ID(1);
	 ndJ = new ID(1);

	 if (ndI != 0 && ndJ != 0) {
		  (*ndI)(0) = ni;
		  (*ndJ)(0) = nj;
	 }
}


DriftRecorder::DriftRecorder(const ID& nI,
	 const ID& nJ,
	 int df,
	 int dirn,
	 Domain& theDom,
	 OPS_Stream* theDataOutputHandler,
	 int procMethod, int procGrpN,
	 bool timeFlag,
	 double dT)
	 :Recorder(RECORDER_TAGS_DriftRecorder),
	 ndI(0), ndJ(0), theNodes(0), dof(df), perpDirn(dirn), oneOverL(0), data(0),
	 theDomain(&theDom), theOutputHandler(theDataOutputHandler),
	 initializationDone(false), numNodes(0), echoTimeFlag(timeFlag), deltaT(dT),
	 procDataMethod(procMethod), procGrpNum(procGrpN)
{
	 ndI = new ID(nI);
	 ndJ = new ID(nJ);
}

DriftRecorder::~DriftRecorder()
{
	 if (ndI != 0)
		  delete ndI;

	 if (ndJ != 0)
		  delete ndJ;

	 if (oneOverL != 0)
		  delete oneOverL;

	 if (data != 0)
		  delete data;

	 if (theNodes != 0)
		  delete[] theNodes;
	 if (theOutputHandler != 0)
	 {
		  theOutputHandler->endTag(); // Data
		  theOutputHandler->endTag(); // OpenSeesOutput
		  delete theOutputHandler;
	 }
}

int
DriftRecorder::record(int commitTag, double timeStamp)
{

	 if (theDomain == 0 || ndI == 0 || ndJ == 0) {
		  return 0;
	 }


	 if (initializationDone == false)
		  if (this->initialize() != 0) {
				opserr << "DriftRecorder::record() - failed in initialize()\n";
				return -1;
		  }

	 if (numNodes == 0 || data == 0)
		  return 0;

	 bool doRec = true;
	 if (deltaT != 0.0)
	 {
		  if (timeStamp < nextTimeStampToRecord - deltaT)
		  {
				nextTimeStampToRecord = 0;
		  }
		  doRec = (timeStamp - nextTimeStampToRecord >= -deltaT * relDeltaTTol);
		  if (doRec)
				nextTimeStampToRecord = timeStamp + deltaT;
	 }
	 if (!doRec)
		  return 0;

	 int timeOffset = 0;
	 if (echoTimeFlag == true) {
		  (*data)(0) = theDomain->getCurrentTime();
		  timeOffset = 1;
	 }
#ifdef _CSS
	 if (procDataMethod)
	 {
		  int nProcOuts;
		  int nVals = numNodes;
		  if (procGrpNum == -1)
				if (procDataMethod != 0)
					 nProcOuts = 1;
				else
					 nProcOuts = nVals;
		  else {
				nProcOuts = nVals / procGrpNum;
				if (nProcOuts * procGrpNum < nVals)
					 nProcOuts++;
		  }
		  double* vals = 0, * val, val1 = 0;
		  vals = new double[nProcOuts];
		  for (int i = 0; i < nProcOuts; i++)
				vals[i] = 0;
		  int iGrpN = 0;
		  int nextGrpN = procGrpNum;
		  val = &vals[iGrpN];
		  int loc = timeOffset;
		  for (int i = 0; i < numNodes; i++) {
				Node* nodeI = theNodes[2 * i];
				Node* nodeJ = theNodes[2 * i + 1];

				if ((*oneOverL)(i) != 0.0) {
					 const Vector& dispI = nodeI->getTrialDisp();
					 const Vector& dispJ = nodeJ->getTrialDisp();

					 double dx = dispJ(dof) - dispI(dof);

					 val1 = dx * (*oneOverL)(i);

				}
				else
					 val1 = 0.0;

				if (procGrpNum != -1 && i == nextGrpN)
				{
					 iGrpN++;
					 nextGrpN += procGrpNum;
					 val = &vals[iGrpN];
				}
				if (i == 0 && procDataMethod != 1)
					 *val = fabs(val1);
				if (procDataMethod == 1)
					 *val += val1;
				else if (procDataMethod == 2 && val1 > *val)
					 *val = val1;
				else if (procDataMethod == 3 && val1 < *val)
					 *val = val1;
				else if (procDataMethod == 4 && fabs(val1) > *val)
					 *val = fabs(val1);
				else if (procDataMethod == 5 && fabs(val1) < *val)
					 *val = fabs(val1);
		  }
		  for (int i = 0; i < nProcOuts; i++)
		  {
				val = &vals[i];
				(*data)(loc++) = *val;
		  }
		  delete[] vals;
	 }
	 else
#endif // _CSS

		  for (int i = 0; i < numNodes; i++) {
				Node* nodeI = theNodes[2 * i];
				Node* nodeJ = theNodes[2 * i + 1];

				if ((*oneOverL)(i) != 0.0) {
					 const Vector& dispI = nodeI->getTrialDisp();
					 const Vector& dispJ = nodeJ->getTrialDisp();

					 double dx = dispJ(dof) - dispI(dof);

					 (*data)(i + timeOffset) = dx * (*oneOverL)(i);

				}
				else
					 (*data)(i + timeOffset) = 0.0;
		  }
	 if (theOutputHandler != 0)
		  theOutputHandler->write(*data);


	 return 0;
}

int
DriftRecorder::restart(void)
{
	 return 0;
}

int
DriftRecorder::setDomain(Domain& theDom)
{
	 theDomain = &theDom;
	 initializationDone = false;
	 return 0;
}

int
DriftRecorder::sendSelf(int commitTag, Channel& theChannel)
{
	 opserr << "DriftRecorder::sendSelf - should not be used in OpenSeesSP\n";
	 return 0;


	 static ID idData(7);
	 idData.Zero();
	 if (ndI != 0 && ndI->Size() != 0)
		  idData(0) = ndI->Size();
	 if (ndJ != 0 && ndJ->Size() != 0)
		  idData(1) = ndJ->Size();
	 idData(2) = dof;
	 idData(3) = perpDirn;
	 if (theOutputHandler != 0) {
		  idData(4) = theOutputHandler->getClassTag();
	 }
	 if (echoTimeFlag == true)
		  idData(5) = 0;
	 else
		  idData(5) = 1;

	 idData(6) = this->getTag();

	 if (theChannel.sendID(0, commitTag, idData) < 0) {
		  opserr << "DriftRecorder::sendSelf() - failed to send idData\n";
		  return -1;
	 }

	 Vector dt(1);
	 dt(0) = deltaT;
	 if (theChannel.sendVector(0, commitTag, dt) < 0) {
		  opserr << "DriftRecorder::sendSelf() - failed to send dt\n";
		  return -1;
	 }

	 if (ndI != 0)
		  if (theChannel.sendID(0, commitTag, *ndI) < 0) {
				opserr << "DriftRecorder::sendSelf() - failed to send dof id's\n";
				return -1;
		  }

	 if (ndJ != 0)
		  if (theChannel.sendID(0, commitTag, *ndJ) < 0) {
				opserr << "DriftRecorder::sendSelf() - failed to send dof id's\n";
				return -1;
		  }



	 static Vector dData(3);
	 dData(0) = deltaT;
	 dData(1) = nextTimeStampToRecord;
	 dData(2) = relDeltaTTol;
	 if (theChannel.sendVector(0, commitTag, dData) < 0) {
		  opserr << "ElementRecorder::sendSelf() - failed to send dData\n";
		  return -1;
	 }
	 if (theOutputHandler != 0)
		  if (theOutputHandler->sendSelf(commitTag, theChannel) < 0) {
				opserr << "DriftRecorder::sendSelf() - failed to send the DataOutputHandler\n";
				return -1;
		  }

	 return 0;
}

int
DriftRecorder::recvSelf(int commitTag, Channel& theChannel,
	 FEM_ObjectBroker& theBroker)
{
	 opserr << "DriftRecorder::recvSelf - should not be used in OpenSeesSP\n";
	 return 0;

	 static ID idData(7);
	 if (theChannel.recvID(0, commitTag, idData) < 0) {
		  opserr << "DriftRecorder::sendSelf() - failed to send idData\n";
		  return -1;
	 }

	 if (idData(0) != 0) {
		  ndI = new ID(idData(0));
		  if (ndI == 0) {
				opserr << "DriftRecorder::sendSelf() - out of memory\n";
				return -1;
		  }
		  if (theChannel.recvID(0, commitTag, *ndI) < 0) {
				opserr << "DriftRecorder::sendSelf() - failed to recv dof id's\n";
				return -1;
		  }
	 }

	 if (idData(1) != 0) {

		  ndJ = new ID(idData(1));
		  if (ndJ == 0) {
				opserr << "DriftRecorder::sendSelf() - out of memory\n";
				return -1;
		  }
		  if (theChannel.recvID(0, commitTag, *ndJ) < 0) {
				opserr << "DriftRecorder::sendSelf() - failed to recv dof id's\n";
				return -1;
		  }
	 }

	 dof = idData(2);
	 perpDirn = idData(3);

	 if (idData(5) == 0)
		  echoTimeFlag = true;
	 else
		  echoTimeFlag = false;

	 this->setTag(idData(6));

	 static Vector dData(1);
	 if (theChannel.recvVector(0, commitTag, dData) < 0) {
		  opserr << "DriftRecorder::recieveSelf() - failed to recieve deltaT\n";
		  return -1;
	 }
	 deltaT = dData(0);
	 nextTimeStampToRecord = dData(1);

	 if (theOutputHandler != 0)
		  delete theOutputHandler;

	 theOutputHandler = theBroker.getPtrNewStream(idData(4));
	 if (theOutputHandler == 0) {
		  opserr << "DriftRecorder::sendSelf() - failed to get a data output handler\n";
		  return -1;
	 }

	 if (theOutputHandler->recvSelf(commitTag, theChannel, theBroker) < 0) {
		  delete theOutputHandler;
		  theOutputHandler = 0;
	 }

	 return 0;
}


int
DriftRecorder::initialize(void)
{
	 if (theOutputHandler != 0)
	 {
		  theOutputHandler->tag("OpenSeesOutput");

		  if (echoTimeFlag == true) {
				theOutputHandler->tag("TimeOutput");
				theOutputHandler->tag("ResponseType", "time");
				theOutputHandler->endTag();
		  }
	 }
	 initializationDone = true; // still might fail but don't want back in again

	 //
	 // clean up old memory
	 //

	 if (theNodes != 0) {
		  delete[] theNodes;
		  theNodes = 0;
	 }
	 if (data != 0) {
		  delete data;
		  data = 0;
	 }
	 if (oneOverL != 0) {
		  delete oneOverL;
		  oneOverL = 0;
	 }

	 //
	 // check valid node ID's
	 //

	 if (ndI == 0 || ndJ == 0) {
		  opserr << "DriftRecorder::initialize() - no nodal id's set\n";
		  return -1;
	 }

	 int ndIsize = ndI->Size();
	 int ndJsize = ndJ->Size();

	 if (ndIsize == 0) {
		  opserr << "DriftRecorder::initialize() - no nodal id's set\n";
		  return -1;
	 }

	 if (ndIsize != ndJsize) {
		  opserr << "DriftRecorder::initialize() - error node arrays differ in size\n";
		  return -2;
	 }

	 //
	 // lets loop through & determine number of valid nodes
	 //


	 numNodes = 0;

	 for (int i = 0; i < ndIsize; i++) {
		  int ni = (*ndI)(i);
		  int nj = (*ndJ)(i);

		  Node* nodeI = theDomain->getNode(ni);
		  Node* nodeJ = theDomain->getNode(nj);

		  if (nodeI != 0 && nodeJ != 0) {
				const Vector& crdI = nodeI->getCrds();
				const Vector& crdJ = nodeJ->getCrds();

				if (crdI.Size() > perpDirn && crdJ.Size() > perpDirn)
					 if (crdI(perpDirn) != crdJ(perpDirn))
						  numNodes++;
		  }
	 }

	 if (numNodes == 0) {
		  opserr << "DriftRecorder::initialize() - no valid nodes or perpendicular direction\n";
		  return 0;
	 }

	 //
	 // allocate memory
	 //

	 int timeOffset = 0;
	 if (echoTimeFlag == true)
		  timeOffset = 1;

	 theNodes = new Node * [2 * numNodes];
	 oneOverL = new Vector(numNodes);
#ifdef _CSS
	 int nProcOuts;
	 int nVals = numNodes;
	 if (procGrpNum == -1)
		  if (procDataMethod != 0)
				nProcOuts = 1;
		  else
				nProcOuts = nVals;
	 else {
		  nProcOuts = nVals / procGrpNum;
		  if (nProcOuts * procGrpNum < nVals)
				nProcOuts++;
	 }
	 data = new Vector(nProcOuts + timeOffset); // data(0) allocated for time
#else
	 data = new Vector(numNodes + timeOffset); // data(0) allocated for time
#endif // _CSS
	 if (theNodes == 0 || oneOverL == 0 || data == 0) {
		  opserr << "DriftRecorder::initialize() - out of memory\n";
		  return -3;
	 }

	 //
	 // set node pointers and determine one over L
	 //

	 int counter = 0;
	 int counterI = 0;
	 int counterJ = 1;
	 for (int j = 0; j < ndIsize; j++) {
		  int ni = (*ndI)(j);
		  int nj = (*ndJ)(j);

		  Node* nodeI = theDomain->getNode(ni);
		  Node* nodeJ = theDomain->getNode(nj);

		  if (nodeI != 0 && nodeJ != 0) {
				const Vector& crdI = nodeI->getCrds();
				const Vector& crdJ = nodeJ->getCrds();

				if (crdI.Size() > perpDirn && crdJ.Size() > perpDirn)
					 if (crdI(perpDirn) != crdJ(perpDirn)) {

						  if (theOutputHandler != 0)
						  {
								theOutputHandler->tag("DriftOutput");
								theOutputHandler->attr("node1", ni);
								theOutputHandler->attr("node2", nj);
								theOutputHandler->attr("perpDirn", perpDirn);
								theOutputHandler->attr("lengthPerpDirn", fabs(crdJ(perpDirn) - crdI(perpDirn)));
								theOutputHandler->tag("ResponseType", "drift");
								theOutputHandler->endTag();  // DriftOutput
						  }
						  (*oneOverL)(counter) = 1.0 / fabs(crdJ(perpDirn) - crdI(perpDirn));
						  theNodes[counterI] = nodeI;
						  theNodes[counterJ] = nodeJ;
						  counterI += 2;
						  counterJ += 2;
						  counter++;
					 }
		  }
	 }
	 if (theOutputHandler != 0)
		  theOutputHandler->tag("Data");


	 //
	 // mark as having been done & return
	 //

	 return 0;
}
//added by SAJalali
double DriftRecorder::getRecordedValue(int clmnId, int rowOffset, bool reset)
{
	 double res = 0;
	 if (!initializationDone)
		  return res;
	 if (clmnId >= data->Size())
		  return res;
	 res = (*data)(clmnId);
	 return res;
}

int DriftRecorder::flush(void) {
  if (theOutputHandler != 0) {
    return theOutputHandler->flush();
  }
  return 0;
}