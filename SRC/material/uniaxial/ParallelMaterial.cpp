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
                                                                        
// $Revision: 1.12 $
// $Date: 2007-02-02 01:19:30 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/uniaxial/ParallelMaterial.cpp,v $
                                                                        
                                                                        
// File: ~/material/ParallelModel.C
//
// Written: fmk 
// Created: 07/98
// Revision: A
//
// Description: This file contains the class definition for 
// ParallelModel. ParallelModel is an aggregation
// of UniaxialMaterial objects all considered acting in parallel.
//
// What: "@(#) ParallelModel.C, revA"

#include <ParallelMaterial.h>
#include <ID.h>
#include <Vector.h>
#include <Channel.h>
#include <FEM_ObjectBroker.h>
#include <stdlib.h>
#include <string.h>
#include <MaterialResponse.h>
#ifdef _CSS
#include <math.h>
#endif // _CSS

#include <elementAPI.h>

void *
OPS_ParallelMaterial(void)
{
  // Pointer to a uniaxial material that will be returned
  UniaxialMaterial *theMaterial = 0;
  Vector *theFactors = 0;

  int argc = OPS_GetNumRemainingInputArgs();

  if (argc < 2) {
    opserr << "Invalid #args,  want: uniaxialMaterial Parallel $tag $tag1 $tag2 ... <-factors $fact1 $fact2 ...>" << endln;
    return 0;
  }

  // count the number of materials
  int numMats = -1;
  int gotFactors = 0;

  while (argc > 0)  {
    const char *argvLoc = OPS_GetString();
    if (strcmp(argvLoc, "-factors") == 0) {
      gotFactors = 1;
      break;
    }
    numMats++;
    argc = OPS_GetNumRemainingInputArgs();
  }
  
  // reset to read from beginning
  OPS_ResetCurrentInputArg(2);
  
  int numData = 1+numMats;
  int *iData = new int[numData];
  UniaxialMaterial **theMats = new UniaxialMaterial *[numMats];
  double *dData = 0;

  if (gotFactors) {
      dData = new double[numMats];
      theFactors = new Vector(dData, numMats);
  }
  
  if (OPS_GetIntInput(&numData, iData) != 0) {
    opserr << "WARNING invalid data for uniaxialMaterial Parallel" << endln;
    delete [] iData;
    delete [] theMats;
    if (dData != 0)
      delete [] dData;
    if (theFactors != 0)
      delete theFactors;    
    return 0;
  }

  for (int i=1; i<numMats+1; i++) {
    UniaxialMaterial *theMat = OPS_getUniaxialMaterial(iData[i]);
    if (theMat == 0) {
      opserr << "WARNING no existing material with tag " << iData[i] 
	     << " for uniaxialMaterial Parallel " << iData[0] << endln;
      delete [] iData;
      delete [] theMats;
      if (dData != 0)
	delete [] dData;
      if (theFactors != 0)
	delete theFactors;
      return 0;
    }
    theMats[i-1] = theMat;
  }
  
  if (gotFactors) {
    const char *argvLoc = OPS_GetString();
    if (OPS_GetDoubleInput(&numMats, dData) != 0) {
      opserr << "WARNING invalid factors for uniaxialMaterial Parallel" << endln;
      delete [] iData;
      delete [] theMats;
      if (dData != 0)
	delete [] dData;
      if (theFactors != 0)
	delete theFactors;      
      return 0;
    }    
  }

  // Parsing was successful, allocate the material
  theMaterial = new ParallelMaterial(iData[0], numMats, theMats, theFactors);
  if (theMaterial == 0) {
    opserr << "WARNING could not create uniaxialMaterial of type Parallel" << endln;
    return 0;
  }
  
  delete [] iData;
  delete [] theMats;
  if (dData != 0)
    delete [] dData;
  if (theFactors != 0)
    delete theFactors;
  
  return theMaterial;
}



ParallelMaterial::ParallelMaterial(
				 int tag, 
				 int num, 
				 UniaxialMaterial **theMaterialModels,
				 Vector *factors)
:UniaxialMaterial(tag, MAT_TAG_ParallelMaterial),
 trialStrain(0.0), trialStrainRate(0.0), numMaterials(num),
 theModels(0), theFactors(0)
{
    // create an array (theModels) to store copies of the MaterialModels
    theModels = new UniaxialMaterial *[num];

    if (theModels == 0) {
	opserr << "FATAL ParallelMaterial::ParallelMaterial() ";
	opserr << " ran out of memory for array of size: " << num << endln;
	exit(-1);
    }

    // into the newly created array store a pointer to a copy
    // of the UniaxialMaterial stored in theMaterialModels
    for (int i=0; i<num; i++) {
	theModels[i] = theMaterialModels[i]->getCopy();
    }

    // copy the factors
    if (factors != 0) {
      theFactors = new Vector(*factors);
    }
}



// this constructor is used for a ParallelMaterailModel object that
// needs to be constructed in a remote actor process. recvSelf() needs
// to be called on this object
ParallelMaterial::ParallelMaterial()
:UniaxialMaterial(0,MAT_TAG_ParallelMaterial),
 trialStrain(0.0), trialStrainRate(0.0), numMaterials(0),
 theModels(0), theFactors(0)
{

}


ParallelMaterial::~ParallelMaterial()
{
    // invoke the destructor on each MaterialModel object
    for (int i=0; i<numMaterials; i++)
	    delete theModels[i];

    // now we can delete the array
    if (theModels != 0) // just in case blank constructor called and no recvSelf()
	    delete [] theModels;

    // clean up the factors
    if (theFactors != 0)
        delete theFactors;
}



int 
ParallelMaterial::setTrialStrain(double strain, double strainRate)
{
    // set the trialStrain and the trialStrain in each of the
    // local MaterialModel objects 
    trialStrain = strain;
    trialStrainRate = strainRate;

    for (int i=0; i<numMaterials; i++)
      theModels[i]->setTrialStrain(strain, strainRate);

    return 0;
}


double 
ParallelMaterial::getStrain(void)
{
  return trialStrain;
}

double 
ParallelMaterial::getStrainRate(void)
{
    return trialStrainRate;
}

double 
ParallelMaterial::getStress(void)
{
    // get the stress = sum of stress in all local MaterialModel objects
    double stress = 0.0;
    if (theFactors == 0) {
        for (int i=0; i<numMaterials; i++)
            stress += theModels[i]->getStress();
    } else {
        for (int i=0; i<numMaterials; i++)
            stress += (*theFactors)(i) * theModels[i]->getStress();
    }

    return stress;
}



double 
ParallelMaterial::getTangent(void)
{
    // get the tangent = sum of tangents in all local MaterialModel objects    
    double E = 0.0;
    if (theFactors == 0) {
        for (int i=0; i<numMaterials; i++)
            E += theModels[i]->getTangent();
    } else {
        for (int i=0; i<numMaterials; i++)
            E += (*theFactors)(i) * theModels[i]->getTangent();
    }

    return E;
}

double 
ParallelMaterial::getInitialTangent(void)
{
    // get the tangent = sum of tangents in all local MaterialModel objects    
    double E = 0.0;
    if (theFactors == 0) {
        for (int i=0; i<numMaterials; i++)
            E += theModels[i]->getInitialTangent();
    } else {
        for (int i=0; i<numMaterials; i++)
            E += (*theFactors)(i) * theModels[i]->getInitialTangent();
    }

    return E;
}

double 
ParallelMaterial::getDampTangent(void)
{
    // get the damp tangent = sum of damp tangents in all local MaterialModel objects    
    double eta = 0.0;
    if (theFactors == 0) {
        for (int i=0; i<numMaterials; i++)
            eta += theModels[i]->getDampTangent();
    } else {
        for (int i=0; i<numMaterials; i++)
            eta += (*theFactors)(i) * theModels[i]->getDampTangent();
    }

    return eta;
}

int 
ParallelMaterial::commitState(void)
{
  // invoke commitState() on each of local MaterialModel objects
  for (int i=0; i<numMaterials; i++) {
    int res = theModels[i]->commitState();
    if (res < 0)
      return res;
  }
#ifdef _CSS
  energy = 0;
  for (int i = 0; i < numMaterials; i++)
	  energy += theModels[i]->getEnergy();
#endif // _CSS
  return 0;    
}

#ifdef _CSS
void ParallelMaterial::resetEnergy(void)
{
    // invoke commitState() on each of local MaterialModel objects
    for (int i = 0; i < numMaterials; i++)
        theModels[i]->resetEnergy();
}
#endif // _CSS

int 
ParallelMaterial::revertToLastCommit(void)
{
  // invoke commitState() on each of local MaterialModel objects
  for (int i=0; i<numMaterials; i++) {
    int res = theModels[i]->revertToLastCommit();
    if (res < 0)
      return res;
  }
  return 0;    
}


int 
ParallelMaterial::revertToStart(void)
{
    trialStrain = 0.0;
    trialStrainRate = 0.0;

    // invoke commitState() on each of local MaterialModel objects
    for (int i=0; i<numMaterials; i++) {
      int res = theModels[i]->revertToStart();
      if (res < 0)
	return res;
    }
    return 0;    
}



UniaxialMaterial *
ParallelMaterial::getCopy(void)
{
  ParallelMaterial *theCopy = new 
    ParallelMaterial(this->getTag(), numMaterials, theModels, theFactors);
  
  theCopy->trialStrain = trialStrain;
  theCopy->trialStrainRate = trialStrainRate;
  
  return theCopy;
}


int 
ParallelMaterial::sendSelf(int cTag, Channel &theChannel)
{
    static ID data(3);

    // send ID of size 3 so no possible conflict with classTags ID
    int dbTag = this->getDbTag();
    data(0) = this->getTag();
    data(1) = numMaterials;
    data(2) = 0;
    if (theFactors != 0)
        data(2) = 1;

    if (theChannel.sendID(dbTag, cTag, data) < 0) {
      opserr << "ParallelMaterial::sendSelf() - failed to send data" << endln;
      return -1;
    }

    // send the factors if not null
    if (theFactors != 0) {
      if (theChannel.sendVector(dbTag, cTag, *theFactors) < 0) {
	opserr << "ParallelMaterial::sendSelf() - failed to send factors" << endln;
	return -2;
      }
    }

    // now create an ID containing the class tags and dbTags of all
    // the MaterialModel objects in this ParallelMaterial
    // then send each of the MaterialModel objects
    ID classTags(numMaterials*2);
    for (int i=0; i<numMaterials; i++) {
	classTags(i) = theModels[i]->getClassTag();
	int matDbTag = theModels[i]->getDbTag();
	if (matDbTag == 0) {
	  matDbTag  = theChannel.getDbTag();
	  if (matDbTag != 0)
	    theModels[i]->setDbTag(matDbTag);
	}
	classTags(i+numMaterials) = matDbTag;
    }

    if (theChannel.sendID(dbTag, cTag, classTags) < 0) {
      opserr << "ParallelMaterial::sendSelf() - failed to send classTags" << endln;
      return -3;
    }

    for (int j=0; j<numMaterials; j++)
      if (theModels[j]->sendSelf(cTag, theChannel) < 0) {
	opserr << "ParallelMaterial::sendSelf() - failed to send material" << endln;
	return -4;
      }
    
    return 0;
}

int 
ParallelMaterial::recvSelf(int cTag, Channel &theChannel, 
				FEM_ObjectBroker &theBroker)
{
    static ID data(3);
    int dbTag = this->getDbTag();

    if (theChannel.recvID(dbTag, cTag, data) < 0) {
      opserr << "ParallelMaterial::recvSelf() - failed to receive data" << endln;
      return -1;
    }

    this->setTag(int(data(0)));
    int numMaterialsSent = int(data(1));
    if (numMaterials != numMaterialsSent) { 
      numMaterials = numMaterialsSent;
      if (theModels != 0) {
	for (int i=0; i<numMaterials; i++)
	  delete theModels[i];

	delete [] theModels;
      }

      theModels = new UniaxialMaterial *[numMaterials];      
      if (theModels == 0) {
	opserr << "FATAL ParallelMaterial::recvSelf() - ran out of memory";
	opserr << " for array of size: " << numMaterials << endln;
	return -2;
      }
      for (int i=0; i<numMaterials; i++)
	theModels[i] = 0;
    }

    if (data(2) == 1) {
        theFactors = new Vector(numMaterials);
        if (theChannel.recvVector(dbTag, cTag, *theFactors) < 0) {
	  opserr << "ParallelMaterial::recvSelf() - failed to receive factors" << endln;
        return -3;
        }
    }

    // create and receive an ID for the classTags and dbTags of the local 
    // MaterialModel objects
    ID classTags(numMaterials*2);
    if (theChannel.recvID(dbTag, cTag, classTags) < 0) {
      opserr << "ParallelMaterial::recvSelf() - failed to receive classTags" << endln;
      return -4;
    }

    // now for each of the MaterialModel objects, create a new object
    // and invoke recvSelf() on it
    for (int i=0; i<numMaterials; i++) {
      int matClassTag = classTags(i);
      if (theModels[i] == 0 || theModels[i]->getClassTag() != matClassTag) {
	if (theModels[i] != 0)
	  delete theModels[i];
	UniaxialMaterial *theMaterialModel = 
	    theBroker.getNewUniaxialMaterial(matClassTag);
	if (theMaterialModel != 0) {
	    theModels[i] = theMaterialModel;
	    theMaterialModel->setDbTag(classTags(i+numMaterials));
	}
	else {
	    opserr << "FATAL ParallelMaterial::recvSelf() ";
	    opserr << " could not get a UniaxialMaterial" << endln;
	    return -5;
	}    	    
      }
      if (theModels[i]->recvSelf(cTag, theChannel, theBroker) < 0) {
	opserr << "ParallelMaterial::recvSelf - failed to receive material" << endln;
	return -6;
      }
    }
    return 0;
}

void 
ParallelMaterial::Print(OPS_Stream &s, int flag)
{
    if (flag == OPS_PRINT_PRINTMODEL_MATERIAL) {
        s << "ParallelMaterial tag: " << this->getTag() << endln;
        for (int i = 0; i < numMaterials; i++) {
            s << " ";
            theModels[i]->Print(s, flag);
        }
        if (theFactors != 0)
            opserr << " Factors: " << *theFactors;
    }
    
    if (flag == OPS_PRINT_PRINTMODEL_JSON) {
        s << "\t\t\t{";
        s << "\"name\": \"" << this->getTag() << "\", ";
        s << "\"type\": \"ParallelMaterial\", ";
        s << "\"materials\": [";
        for (int i = 0; i < numMaterials-1; i++)
            s << "\"" << theModels[i]->getTag() << "\", ";
        s << "\"" << theModels[numMaterials-1]->getTag() << "\"]}";
        if (theFactors != 0) {
            s << "\"factors\": [";
            for (int i = 0; i < numMaterials-1; i++)
                s << (*theFactors)(i) << ", ";
            s << (*theFactors)(numMaterials-1) << "]}";
        }
    }
}

Response*
ParallelMaterial::setResponse(const char **argv, int argc, OPS_Stream &theOutput)
{
  Response *theResponse = 0;
#ifdef _CSS
  if (strcmp(argv[0], "stresses") == 0) {
      for (int i = 0; i < numMaterials; i++) {
          theOutput.tag("UniaxialMaterialOutput");
          theOutput.attr("matType", this->getClassType());
          theOutput.attr("matTag", this->getTag());
          theOutput.tag("ResponseType", "sigma11");
          theOutput.endTag();
      }
      theResponse = new MaterialResponse(this, 100, Vector(numMaterials));
  }
  else if (strcmp(argv[0], "material") == 0 ||
	  strcmp(argv[0], "component") == 0) {
	  if (argc > 1) {
		  int matNum = atoi(argv[1]) - 1;
		  if (matNum >= 0 && matNum < numMaterials)
			  theResponse = theModels[matNum]->setResponse(&argv[2], argc - 2, theOutput);
	  }
  }
  else {
      theResponse = UniaxialMaterial::setResponse(argv, argc, theOutput);
  }
#else
  theOutput.tag("UniaxialMaterialOutput");
  theOutput.attr("matType", this->getClassType());
  theOutput.attr("matTag", this->getTag());

  // stress
  if (strcmp(argv[0],"stress") == 0) {
    theOutput.tag("ResponseType", "sigma11");
    theResponse =  new MaterialResponse(this, 1, this->getStress());
  }  
  // tangent
  else if (strcmp(argv[0],"tangent") == 0) {
    theOutput.tag("ResponseType", "C11");
    theResponse =  new MaterialResponse(this, 2, this->getTangent());
  }

  // strain
  else if (strcmp(argv[0],"strain") == 0) {
    theOutput.tag("ResponseType", "eps11");
    theResponse =  new MaterialResponse(this, 3, this->getStrain());
  }

  // strain
  else if ((strcmp(argv[0],"stressStrain") == 0) || 
	   (strcmp(argv[0],"stressANDstrain") == 0)) {
    theOutput.tag("ResponseType", "sig11");
    theOutput.tag("ResponseType", "eps11");
    theResponse =  new MaterialResponse(this, 4, Vector(2));
  }

  else if (strcmp(argv[0],"stresses") == 0) {
    for (int i=0; i<numMaterials; i++) {
      theOutput.tag("UniaxialMaterialOutput");
      theOutput.attr("matType", this->getClassType());
      theOutput.attr("matTag", this->getTag());
      theOutput.tag("ResponseType", "sigma11");
      theOutput.endTag();
    }
    theResponse = new MaterialResponse(this, 100, Vector(numMaterials));
  }

  else if (strcmp(argv[0],"material") == 0 ||
	   strcmp(argv[0],"component") == 0) {
    if (argc > 1) {
      int matNum = atoi(argv[1]) - 1;
      if (matNum >= 0 && matNum < numMaterials)
	theResponse = theModels[matNum]->setResponse(&argv[2], argc-2, theOutput);
    }
  }
#endif // _CSS

  theOutput.endTag();
  return theResponse;
}

int
ParallelMaterial::getResponse(int responseID, Information &info)
{
  Vector stresses(numMaterials);

  switch (responseID) {
  case 100:
    for (int i = 0; i < numMaterials; i++)
      stresses(i) = theModels[i]->getStress();
    return info.setVector(stresses);

  default:
    return this->UniaxialMaterial::getResponse(responseID, info);
  }
}

#ifdef _CSS
double ParallelMaterial::getInitYieldStrain()
{
    double res = 0;
    for (int i = 0; i < numMaterials; i++) {
        double d = fabs(theModels[i]->getInitYieldStrain());
        if (res == 0 || d < res)
            res = d;
    }

    return res;
}
double ParallelMaterial::getEnergy()
{
    return energy;
}
#endif // _CSS
int
ParallelMaterial::setParameter(const char **argv, int argc, Parameter &param)
{
  if (argc < 1)
    return -1;
  
  if (strcmp(argv[0],"material") == 0 || strcmp(argv[0],"component") == 0) {

    if (argc < 3)
      return -1;

    int materialTag = atoi(argv[1]);

    for (int i = 0; i < numMaterials; i++) {
      if (materialTag == theModels[i]->getTag())
	theModels[i]->setParameter(&argv[2], argc-2, param);
    }
  }

  else { // Send to everything
    for (int i = 0; i < numMaterials; i++)
      theModels[i]->setParameter(argv, argc, param);
  }

  return 0;
}

double
ParallelMaterial::getStressSensitivity(int gradIndex, bool conditional)
{
  double dsdh = 0.0;
  for (int i = 0; i < numMaterials; i++)
    dsdh += theModels[i]->getStressSensitivity(gradIndex, conditional);

  return dsdh;
}

double
ParallelMaterial::getTangentSensitivity(int gradIndex)
{
  double dEdh = 0.0;
  for (int i = 0; i < numMaterials; i++)
    dEdh += theModels[i]->getTangentSensitivity(gradIndex);

  return dEdh;
}

double
ParallelMaterial::getInitialTangentSensitivity(int gradIndex)
{
  double dEdh = 0.0;
  for (int i = 0; i < numMaterials; i++)
    dEdh += theModels[i]->getInitialTangentSensitivity(gradIndex);

  return dEdh;
}

double
ParallelMaterial::getDampTangentSensitivity(int gradIndex)
{
  double dEdh = 0.0;
  for (int i = 0; i < numMaterials; i++)
    dEdh += theModels[i]->getDampTangentSensitivity(gradIndex);

  return dEdh;
}

double
ParallelMaterial::getRhoSensitivity(int gradIndex)
{
  double dpdh = 0.0;
  for (int i = 0; i < numMaterials; i++)
    dpdh += theModels[i]->getRhoSensitivity(gradIndex);

  return dpdh;
}

int
ParallelMaterial::commitSensitivity(double dedh, int gradIndex, int numGrads)
{
  int ok = 0;
  for (int i = 0; i < numMaterials; i++)
    ok += theModels[i]->commitSensitivity(dedh, gradIndex, numGrads);

  return ok;
}
