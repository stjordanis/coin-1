/**************************************************************************\
 * 
 *  Copyright (C) 1998-1999 by Systems in Motion.  All rights reserved.
 *
 *  This file is part of the Coin library.
 *
 *  This file may be distributed under the terms of the Q Public License
 *  as defined by Troll Tech AS of Norway and appearing in the file
 *  LICENSE.QPL included in the packaging of this file.
 *
 *  If you want to use Coin in applications not covered by licenses
 *  compatible with the QPL, you can contact SIM to aquire a
 *  Professional Edition license for Coin.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef __SOPENDULUM_H__
#define __SOPENDULUM_H__

#include <Inventor/nodes/SoRotation.h>
#include <Inventor/fields/SoSFRotation.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFBool.h>

#if defined(COIN_EXCLUDE_SOPENDULUM)
#error Configuration settings disrespected -- do not include this file!
#endif // COIN_EXCLUDE_SOPENDULUM

// *************************************************************************

class SoPendulum : public SoRotation {
  typedef SoRotation inherited;

//$ BEGIN TEMPLATE NodeHeader(SoPendulum)
private:
  static SoType classTypeId;

public:
  static SoType getClassTypeId(void);
  virtual SoType getTypeId(void) const;
  static void * createInstance(void);
public:
  static void initClass(void);
  static void cleanClass(void);

  SoPendulum(void);
protected:
  virtual ~SoPendulum();
//$ END TEMPLATE NodeHeader

public:
  SoSFRotation rotation0;
  SoSFRotation rotation1;
  SoSFFloat speed;
  SoSFBool on;
};

#endif // !__SOPENDULUM_H__
