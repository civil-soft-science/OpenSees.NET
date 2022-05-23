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

// Source: /usr/local/cvs/OpenSees/SRC/material/uniaxial/ConfinedConcrete02.cpp,v

// Written: SA Jalali (Civil Soft Science)
// Created: 02/1401
//

//#include <stdlib.h>
//#include <string.h>
//#include <math.h>

#include <Concrete02.h>
//#include <OPS_Globals.h>
//#include <float.h>
//#include <Channel.h>
//#include <Information.h>

#include <elementAPI.h>
#include <OPS_Globals.h>

const double rats[16] = {0.3, 0.28, 0.26, 0.24, 0.22, 0.2, 0.18, 0.16, 0.14, 0.12, 0.1, 0.08, 0.06, 0.04, 0.02, 0};
const double facs[16][16] = {
	{2.30, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{2.26, 2.24, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{2.23, 2.20, 2.18, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{2.19, 2.17, 2.14, 2.11, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{2.16, 2.13, 2.10, 2.07, 2.04, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{2.11, 2.09, 2.06, 2.03, 2.00, 1.98, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{2.06, 2.04, 2.01, 1.99, 1.96, 1.94, 1.91, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{2.01, 1.99, 1.97, 1.95, 1.93, 1.90, 1.87, 1.83, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{1.95, 1.93, 1.91, 1.89, 1.87, 1.85, 1.82, 1.79, 1.75, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{1.89, 1.88, 1.86, 1.84, 1.82, 1.79, 1.77, 1.74, 1.70, 1.67, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
	{1.81, 1.80, 1.78, 1.77, 1.75, 1.73, 1.71, 1.68, 1.65, 1.62, 1.57, 0.00, 0.00, 0.00, 0.00, 0.00},
	{1.73, 1.72, 1.71, 1.69, 1.68, 1.66, 1.64, 1.61, 1.58, 1.55, 1.51, 1.47, 0.00, 0.00, 0.00, 0.00},
	{1.65, 1.64, 1.62, 1.61, 1.59, 1.57, 1.56, 1.53, 1.50, 1.48, 1.45, 1.42, 1.37, 0.00, 0.00, 0.00},
	{1.55, 1.54, 1.53, 1.51, 1.50, 1.48, 1.47, 1.45, 1.43, 1.41, 1.38, 1.35, 1.31, 1.26, 0.00, 0.00},
	{1.44, 1.43, 1.42, 1.40, 1.39, 1.38, 1.37, 1.35, 1.33, 1.30, 1.28, 1.26, 1.23, 1.19, 1.13, 0.00},
	{1.30, 1.30, 1.29, 1.28, 1.27, 1.26, 1.25, 1.23, 1.21, 1.20, 1.17, 1.15, 1.12, 1.09, 1.05, 1.00}};
void *
OPS_ConfinedConcrete02()
{
	// Pointer to a uniaxial material that will be returned
	UniaxialMaterial *theMaterial = 0;

	int iData[1];
	double dData[21];
	int numData = 1;

	if (OPS_GetIntInput(numData, iData) != 0)
	{
		opserr << "WARNING invalid uniaxialMaterial ConfinedConcrete02 tag" << endln;
		return 0;
	}

	numData = OPS_GetNumRemainingInputArgs();

	if (numData != 21)
	{
		opserr << "Invalid #args for ConfinedConcrete02 with tag:" << iData[0] << "; want:\n";
		opserr << "uniaxialMaterial ConfinedConcrete02 tag fc0 epsc0 fcu double epscu rat ft Ets \n";
		opserr << "    B H cover fyh nBarTop dBarTop nBarBot dBarBot nBarInt dBarInt nBarTransH nBarTransB dBarTrans sStirrup\n";
		return 0;
	}

	if (OPS_GetDoubleInput(numData, dData) != 0)
	{
		opserr << "Invalid #args for ConfinedConcrete02 with tag:" << iData[0] << "; want:\n";
		opserr << "uniaxialMaterial ConfinedConcrete02 tag fc0 epsc0 fcu double epscu rat ft Ets \n";
		opserr << "    B H cover fyh nBarTop dBarTop nBarBot dBarBot nBarInt dBarInt nBarTransH nBarTransB dBarTrans sStirrup\n";
		return 0;
	}

	int &tag = iData[0];
	int i = 0;
	double &fc0 = dData[i++];
	double &epsc0 = dData[i++];
	double &fcu = dData[i++];
	double &epscu = dData[i++];
	double &rat = dData[i++];
	double &ft = dData[i++];
	double &Ets = dData[i++];

	//section props (to compute Mander's coeff.s):
	double &B = dData[i++];
	double &H = dData[i++];
	double &cover = dData[i++];
	double &fyh = dData[i++];

	double &nBarTop = dData[i++];
	double &dBarTop = dData[i++];
	double &nBarBot = dData[i++];
	double &dBarBot = dData[i++];
	double &nBarInt = dData[i++];
	double &dBarInt = dData[i++];
	double &nBarTH = dData[i++];
	double &nBarTB = dData[i++];
	double &dBarT = dData[i++];
	double &sStirrup = dData[i++];

	// calculate total plan area of ineffectually confined core(area of all parabolas)
	double dc = B - 2 * cover - dBarT;
	double bc = H - 2 * cover - dBarT;

	double wiTop = (dc - dBarT - nBarTop * dBarTop) / (nBarTop - 1);
	double wiBot = (dc - dBarT - nBarBot * dBarBot) / (nBarBot - 1);

	// number of longitudinal bars in H direction is 2 more than
	// nBarInt because thete are top and bot bars
	double nBarLH = nBarInt + 2;
	double wiInt = (bc - dBarT - dBarBot - dBarTop - nBarInt * dBarInt) / (nBarLH - 1);
	double zigmaWi2 = (nBarTop - 1) * pow(wiTop, 2) + (nBarBot - 1) * pow(wiBot, 2) + 2 * (nBarLH - 1) * pow(wiInt, 2);

	double ABarTop = 3.1415 * pow(dBarTop, 2) / 4.;
	double ABarBot = 3.1415 * pow(dBarBot, 2) / 4.;
	double ABarInt = 3.1415 * pow(dBarInt, 2) / 4.;
	double ABarLTot = nBarTop * ABarTop + nBarBot * ABarBot + 2 * nBarInt * ABarInt;
	double rhoCC = ABarLTot / (bc * dc);

	double sStirrupPrime = sStirrup - dBarT;
	double term1 = 1 - zigmaWi2 / (6. * bc * dc);
	double term2 = 1 - sStirrupPrime / (2 * bc);
	double term3 = 1 - sStirrupPrime / (2 * dc);
	double term4 = 1 - rhoCC;
	double ke = term1 * term2 * term3 / term4;

	double ABarT = 3.1415 * pow(dBarT, 2) / 4.;
	double Asx = nBarTH * ABarT;
	double Asy = nBarTB * ABarT;

	double rhoX = Asx / (sStirrup * dc);
	double rhoY = Asy / (sStirrup * bc);

	double f_lx = ke * rhoX * fyh;
	double f_ly = ke * rhoY * fyh;

	// calculate KMander
	//rat1 > rat2
	double rat1 = f_lx > f_ly ? f_lx : f_ly;
	double rat2 = f_lx > f_ly ? f_ly : f_lx;
	rat1 /= fc0;
	rat2 /= fc0;

	//interpolate factor
	int i2, j2, i1, j1;
	for (i2 = 0; i2 < 16; i2++)
		if (rat1 > rats[i2])
			break;
	for (j2 = 0; j2 < 16; j2++)
		if (rat2 > rats[j2])
			break;
	i1 = i2 - 1;
	j1 = j2 - 1;
	double a = (rat1 - rats[i1]) / (rats[i2] - rats[i1]);
	double b = (rat2 - rats[j1]) / (rats[j2] - rats[j1]);
	double f1 = facs[i1][j1] + a * (facs[i2][j1] - facs[i1][j1]);
	double f2 = facs[i1][j2] + a * (facs[i2][j2] - facs[i1][j2]);
	double k1 = f1 + b * (f2 - f1);
	double fcc = fc0*k1;
	double ecc = (1+5*(k1-1))*epsc0; 	;	//Mander's Eq.
	double ec85 = 260*0.015*ecc+0.0038; 	//Mander's Eq.
	epscu = (ec85-ecc)*(0.85/fcu*fc0)+ecc; //Mander's Eq.
	fcu *= fcc/fc0;

	theMaterial = new Concrete02(tag, fcc, ecc, fcu, epscu, rat, ft, Ets);
	if (theMaterial == 0)
	{
		opserr << "WARNING could not create uniaxialMaterial of type ConfinedConcrete02 Material\n";
		return 0;
	}

	return theMaterial;
}