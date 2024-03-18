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
                                                                        
// $Revision: 1.6 $
// $Date: 2003-02-14 23:01:38 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/uniaxial/ElasticPPLagMaterial.cpp,v $
                                                                        
// Written: fmk 
// Created: 07/98
// Revision: A
//
// Description: This file contains the class implementation for 
// ElasticMaterial. 
//
// What: "@(#) ElasticPPLagMaterial.C, revA"


#include <ElasticPPLagMaterial.h>
#include <Vector.h>
#include <Channel.h>
#include <Parameter.h>
#include <math.h>
#include <float.h>

#include <elementAPI.h>

void *
OPS_ElasticPPLagMaterial(void)
{
  // Pointer to a uniaxial material that will be returned
  UniaxialMaterial *theMaterial = 0;

  int numArgs = OPS_GetNumRemainingInputArgs();
  if (numArgs < 3 || numArgs > 5) {
    opserr << "Invalid #args,  want: uniaxialMaterial ElasticPPLag $tag $E $epsP <$epsN $eps0>\n";
    return 0;
  }
  
  int iData[1];
  double dData[4];
  dData[3] = 0.0; // setting default eps0 to 0.

  int numData = 1;
  if (OPS_GetIntInput(numData, iData) != 0) {
    opserr << "WARNING invalid tag for uniaxialMaterial ElasticPPLag" << endln;
    return 0;
  }

  numData = numArgs-1;
  if (OPS_GetDoubleInput(numData, dData) != 0) {
    opserr << "Invalid data for uniaxial ElasticPPLag " << iData[0] << endln;
    return 0;	
  }

  if (numData == 2) 
    dData[2] = -dData[1];

  // Parsing was successful, allocate the material
    theMaterial = new ElasticPPLagMaterial(iData[0], dData[0], dData[1], dData[2], dData[3]);
  if (theMaterial == 0) {
    opserr << "WARNING could not create uniaxialMaterial of type ElasticPPLag\n";
    return 0;
  }

  return theMaterial;
}


ElasticPPLagMaterial::ElasticPPLagMaterial(int tag, double e, double eyp)
:UniaxialMaterial(tag,MAT_TAG_ElasticPPLagMaterial),
 ezero(0.0), E(e), ep(0.0),
 trialStrain(0.0), trialStress(0.0), trialTangent(E),
 commitStrain(0.0), commitStress(0.0), commitTangent(E)
{
	EnergyP = 0;	//by SAJalali
	fyp = E*eyp;
  fyn = -fyp;
  in = inP = 0;
  eRev = eRevP = 0;
}

ElasticPPLagMaterial::ElasticPPLagMaterial(int tag, double e, double eyp,
				     double eyn, double ez )
:UniaxialMaterial(tag,MAT_TAG_ElasticPPLagMaterial),
 ezero(ez), E(e), ep(0.0),
 trialStrain(0.0), trialStress(0.0), trialTangent(E),
 commitStrain(0.0), commitStress(0.0), commitTangent(E)
{
    if (eyp < 0) {
	opserr << "ElasticPPLagMaterial::ElasticPPLagMaterial() - eyp < 0, setting > 0\n";
	eyp *= -1.;
    }
    if (eyn > 0) {
	opserr << "ElasticPPLagMaterial::ElasticPPLagMaterial() - eyn > 0, setting < 0\n";
	eyn *= -1.;
    }    
	EnergyP = 0;	//by SAJalali

    fyp = E*eyp;
    fyn = E*eyn;
	in = inP = 0;
	eRev = eRevP = 0;
}

ElasticPPLagMaterial::ElasticPPLagMaterial()
:UniaxialMaterial(0,MAT_TAG_ElasticPPLagMaterial),
 fyp(0.0), fyn(0.0), ezero(0.0), E(0.0), ep(0.0), 
 trialStrain(0.0), trialStress(0.0), trialTangent(0.0),
 commitStrain(0.0), commitStress(0.0), commitTangent(0.0)
{
	EnergyP = 0;	//by SAJalali

}

ElasticPPLagMaterial::~ElasticPPLagMaterial()
{
  // does nothing
}

int 
ElasticPPLagMaterial::setTrialStrain(double strain, double strainRate)
{
	/*
	if (fabs(eps - strain) < DBL_EPSILON)
	return 0;
	*/
	//double eo = eyp;
	double ey = fyp / E;
	double db = ey;
	double dbn = -db;
	//double B = E;
	trialStrain = strain;
	double eps = strain;
	double deps = strain - commitStrain;
	//trialTangent = commitTangent;
	//trialStress = commitStress;
	in = inP;
	double lag = 0.05;
	//id = idp;
	//ee = ep;

	if (in == 0)
	{
		if (fabs(eps) <= lag)
		{
			trialStress = 1.0e-15 * E * eps;
			trialTangent = 1.0e-15 * E;
			//trialStress = E * eps;
			//trialTangent = E;
		}
		else if (eps > lag)
		{
			trialStress = fyp;
			trialTangent = 1.0e-15 * E;
			in = 21;
		}
		else if (eps < -lag)
		{
			trialStress = fyn;
			trialTangent = 1.0e-15 * E;
			in = 22;
		}
	}
	if (in == 21)
	{
		if (deps < 0.0)
		{
			eRev = commitStrain;
			if (eps < commitStrain - lag)
			{
				trialStress = fyn;
				trialTangent = 1.0e-15 * E;
				in = 22;
			}
			else {
				trialStress = 1.0e-15 * E * deps;
				trialTangent = 1.0e-15 * E;
				in = 13;
				return 0;
			}
		}
		else
		{
			trialStress = fyp;
			trialTangent = 1.0e-15 * E;
		}
	}
	if (in == 13)
	{
		if (fabs(eps - eRevP + lag / 2.) < lag / 2.)
		{
			trialStress = 1.0e-15 * E * deps;
			trialTangent = 1.0e-15 * E;
		}
		else if (eps > eRevP)
		{
			trialStress = fyp;
			trialTangent = 1.0e-15 * E;
			in = 21;
		}
		else {
			trialStress = fyn;
			trialTangent = 1.0e-15 * E;
			in = 22;
			return 0;
		}

	}
	if (in == 22)
	{
		if (deps > 0.0)
		{
			eRev = commitStrain;
			if (eps > commitStrain + lag)
			{
				trialStress = fyp;
				trialTangent = 1.0e-15 * E;
				in = 21;
			}
			else {
				trialStress = 1.0e-15 * E * deps;
				trialTangent = 1.0e-15 * E;
				in = 14;
				return 0;
			}
		}
		else
		{
			trialStress = fyn;
			trialTangent = 1.0e-15 * E;
		}
	}
	if (in == 14)
	{
		if (fabs(eps - eRevP - lag / 2.) < lag / 2.)
		{
			trialStress = 1.0e-15 * E * deps;
			trialTangent = 1.0e-15 * E;
		}
		else if (eps < eRevP)
		{
			trialStress = fyn;
			trialTangent = 1.0e-15 * E;
			in = 22;
		}
		else {
			trialStress = fyp;
			trialTangent = 1.0e-15 * E;
			in = 21;
		}
	}
	return 0;
}


double 
ElasticPPLagMaterial::getStrain(void)
{
  return trialStrain;
}

double 
ElasticPPLagMaterial::getStress(void)
{
  return trialStress;
}


double 
ElasticPPLagMaterial::getTangent(void)
{
  return trialTangent;
}

int 
ElasticPPLagMaterial::commitState(void)
{
    double sigtrial;	// trial stress
    double f;		// yield function

    // compute trial stress
    sigtrial = E * ( trialStrain - ezero - ep );

    // evaluate yield function
    if ( sigtrial >= 0.0 )
	f =  sigtrial - fyp;
    else
	f = -sigtrial + fyn;

    double fYieldSurface = - E * DBL_EPSILON;
    if ( f > fYieldSurface ) {
      // plastic
      if ( sigtrial > 0.0 ) {
	ep += f / E;
      } else {
	ep -= f / E;
      }
    }

	//added by SAJalali for energy recorder
	EnergyP += 0.5*(commitStress + trialStress)*(trialStrain - commitStrain);

    commitStrain = trialStrain;
    commitTangent=trialTangent;
    commitStress = trialStress;
	inP = in;
	eRevP = eRev;
    return 0;
}	


int 
ElasticPPLagMaterial::revertToLastCommit(void)
{
  trialStrain = commitStrain;
  trialTangent = commitTangent;
  trialStress = commitStress;
  in = inP;
  eRev = eRevP;
  return 0;
}


int 
ElasticPPLagMaterial::revertToStart(void)
{
  trialStrain = commitStrain = 0.0;
  trialTangent = commitTangent = E;
  trialStress = commitStress = 0.0;

  ep = 0.0;
  EnergyP = 0;	//by SAJalali
  in = inP = 0;
  eRev = eRevP = 0;
  return 0;
}


UniaxialMaterial *
ElasticPPLagMaterial::getCopy(void)
{
  ElasticPPLagMaterial *theCopy =
    new ElasticPPLagMaterial(this->getTag(),E,fyp/E,fyn/E,ezero);
  theCopy->ep = this->ep;
  theCopy->inP = this->inP;
  
  return theCopy;
}


int 
ElasticPPLagMaterial::sendSelf(int cTag, Channel &theChannel)
{
  int res = 0;
  static Vector data(11);//editted by SAJalali for EnergyP
  data(0) = this->getTag();
  data(1) = ep;
  data(2) = E;
  data(3) = ezero;
  data(4) = fyp;
  data(5) = fyn;
  data(6) = commitStrain;
  data(7) = commitStress;
  data(8) = EnergyP;//SAJalali
  data(9) = inP;
  data(10) = eRevP;
  res = theChannel.sendVector(this->getDbTag(), cTag, data);
  if (res < 0) 
    opserr << "ElasticPPLagMaterial::sendSelf() - failed to send data\n";

  return res;
}

int 
ElasticPPLagMaterial::recvSelf(int cTag, Channel &theChannel, 
				 FEM_ObjectBroker &theBroker)
{
  int res = 0;
  static Vector data(11);//editted by SAJalali for EnergyP
  res = theChannel.recvVector(this->getDbTag(), cTag, data);
  if (res < 0) 
    opserr << "ElasticPPLagMaterial::recvSelf() - failed to recv data\n";
  else {
    this->setTag(int(data(0)));
    ep    = data(1);
    E     = data(2);
    ezero = data(3);
    fyp   = data(4);
    fyn   = data(5);  
    commitStrain=data(6);
    commitStress=data(7);
	EnergyP = data(8);//SAJalali
	inP = data(9);
	eRevP = data(10);
    trialStrain = commitStrain;
    trialTangent = commitTangent;
    trialStress = commitStress;
  }

  return res;
}

void 
ElasticPPLagMaterial::Print(OPS_Stream &s, int flag)
{
	if (flag == OPS_PRINT_PRINTMODEL_MATERIAL) {
		s << "ElasticPPLagMaterial tag: " << this->getTag() << endln;
		s << "  E: " << E << endln;
		s << "  ep: " << ep << endln;
		s << "  stress: " << trialStress << " tangent: " << trialTangent << endln;
	}
    
	if (flag == OPS_PRINT_PRINTMODEL_JSON) {
		s << "\t\t\t{";
		s << "\"name\": \"" << this->getTag() << "\", ";
		s << "\"type\": \"ElasticPPLagMaterial\", ";
		s << "\"E\": " << E << ", ";
		s << "\"epsyp\": " << fyp/E << ", ";
		s << "\"epsyn\": " << fyn/E << ", ";
		s << "\"eps0\": " << ezero << "}";
	}
}


int
ElasticPPLagMaterial::setParameter(const char **argv, int argc, Parameter &param)
{
  if (strcmp(argv[0],"sigmaY") == 0 || strcmp(argv[0],"fy") == 0 || strcmp(argv[0],"Fy") == 0) {
    param.setValue(fyp);
    return param.addObject(1, this);
  }
  if (strcmp(argv[0],"E") == 0) {
    param.setValue(E);
    return param.addObject(2, this);
  }
  if (strcmp(argv[0],"epsP") == 0 || strcmp(argv[0],"ep") == 0) {
    param.setValue(ep);
    return param.addObject(3, this);
  }

  return -1;
}

int
ElasticPPLagMaterial::updateParameter(int parameterID, Information &info)
{
  switch (parameterID) {
  case -1:
    return -1;
  case 1:
    this->fyp = info.theDouble;
    this->fyn = -fyp;
    break;
  case 2:
    this->E = info.theDouble;
    trialTangent = E;
    break;
  case 3:
    this->ep = info.theDouble;
    break;
  default:
    return -1;
  }
  
  return 0;
}
