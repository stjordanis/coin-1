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

#ifndef __SOINTERPOLATEVEC4F_H__
#define __SOINTERPOLATEVEC4F_H__

#include <Inventor/confdep.h>
#if defined(COIN_EXCLUDE_SOINTERPOLATEVEC4F)
#error "Configuration settings disrespected -- do not include this file!"
#endif // COIN_EXCLUDE_SOINTERPOLATEVEC4F

//$ BEGIN TEMPLATE InterpolateHeader(SoInterpolateVec4f, SoMFVec4f)
#include <Inventor/engines/SoInterpolateAbs.h>
#include <Inventor/fields/SoMFVec4f.h>

class SoInterpolateVec4f : public SoInterpolate {
  SO_INTERPOLATE_HEADER(SoInterpolateVec4f);

public:
  SoMFVec4f input0;
  SoMFVec4f input1;
};
//$ END TEMPLATE InterpolateHeader

#endif // !__SOINTERPOLATEVEC4F_H__
