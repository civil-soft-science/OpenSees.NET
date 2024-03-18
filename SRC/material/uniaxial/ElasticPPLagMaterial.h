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
// $Date: 2006-08-03 23:42:19 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/uniaxial/ElasticPPLagMaterial.h,v $
                                                                        
#ifndef ElasticPPLagMaterial_h
#define ElasticPPLagMaterial_h


#include <UniaxialMaterial.h>

class ElasticPPLagMaterial : public UniaxialMaterial
{
  public:
    ElasticPPLagMaterial(int tag, double E, double eyp);    
    ElasticPPLagMaterial(int tag, double E, double eyp, double eyn, double ezero);    
    ElasticPPLagMaterial();    

    ~ElasticPPLagMaterial();

    const char *getClassType(void) const {return "ElasticPPLagMaterial";};

    int setTrialStrain(double strain, double strainRate = 0.0); 
    double getStrain(void);          
    double getStress(void);
    double getTangent(void);

    double getInitialTangent(void) {return E;};

    int commitState(void);
    int revertToLastCommit(void);    
    int revertToStart(void);    

    UniaxialMaterial *getCopy(void);
    
    int sendSelf(int commitTag, Channel &theChannel);  
    int recvSelf(int commitTag, Channel &theChannel, 
		 FEM_ObjectBroker &theBroker);    
    
    void Print(OPS_Stream &s, int flag =0);

    int setParameter(const char **argv, int argc, Parameter &param);
    int updateParameter(int parameterID, Information &info);
	//by SAJalali
	virtual double getEnergy() { return EnergyP; };
#ifdef _CSS
	virtual double getInitYieldStrain() { return fyp / E; }//by SAJalali
#endif // _CSS

  protected:
    
  private:
      int in, inP;
      double eRev, eRevP;
    double fyp, fyn;	// positive and negative yield stress
    double ezero;	// initial strain
    double E;		// elastic modulus
    double ep;		// plastic strain at last commit
    double trialStrain;	     // current trial strain
    double trialStress;      // current trial stress
    double trialTangent;     // current trial tangent
    double commitStrain;     // last committed strain
    double commitStress;     // last committed stress
    double commitTangent;    // last committed  tangent

	double EnergyP; //by SAJalali
};


#endif



