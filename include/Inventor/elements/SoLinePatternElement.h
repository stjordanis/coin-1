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

#ifndef __SOLINEPATTERNELEMENT_H__
#define __SOLINEPATTERNELEMENT_H__

#include <Inventor/confdep.h>
#if defined(COIN_EXCLUDE_SOLINEPATTERNELEMENT)
#error "Configuration settings disable this class!"
#endif // COIN_EXCLUDE_SOLINEPATTERNELEMENT

#include <Inventor/elements/SoInt32Element.h>

class SoLinePatternElement : public SoInt32Element {
  typedef SoInt32Element inherited;

//$ BEGIN TEMPLATE ElementHeader(SoLinePatternElement)
public:
  static SoType classTypeId;
  static SoType getClassTypeId(void);
  static void * createInstance(void);
public:
  static int getClassStackIndex(void);
  virtual ~SoLinePatternElement(void);

  static void initClass(void);
  static void cleanClass(void);

protected:
  SoLinePatternElement(void);
  static int classStackIndex;
//$ END TEMPLATE ElementHeader

public:
  
  enum Patterns {
    CONTINUOUS = 0xFFFF
  };

  virtual void init(SoState * state);

  static  void set(SoState * const state, SoNode * const node,
		   const int32_t pattern);
  static  void set(SoState * const state, const int32_t pattern);
  static  int32_t get(SoState * const state);
  static  int32_t getDefault();
  
};

#endif // !__SOLINEPATTERNELEMENT_H__
