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

#ifndef __SOGLENVIRONMENTELEMENT_H__
#define __SOGLENVIRONMENTELEMENT_H__

#include <Inventor/elements/SoEnvironmentElement.h>

#if defined(COIN_EXCLUDE_SOGLENVIRONMENTELEMENT)
#error "Configuration settings disrespected -- do not include this file!"
#endif /// defined(COIN_EXCLUDE_SOGLENVIRONMENTELEMENT)


class SoGLEnvironmentElement : public SoEnvironmentElement {
  typedef SoEnvironmentElement inherited;

//$ BEGIN TEMPLATE ElementHeader(SoGLEnvironmentElement)
public:
  static SoType classTypeId;
  static SoType getClassTypeId(void);
  static void * createInstance(void);
public:
  static int getClassStackIndex(void);
  virtual ~SoGLEnvironmentElement(void);

  static void initClass(void);
  static void cleanClass(void);

protected:
  SoGLEnvironmentElement(void);
  static int classStackIndex;
//$ END TEMPLATE ElementHeader

public:
  virtual void init(SoState * state);
  virtual void pop(SoState * state,
		   const SoElement * prevTopElement);

  virtual SbBool matches(const SoElement * element) const;
  virtual SoElement * copyMatchInfo() const;

protected:

  virtual void setElt(SoState * const state,
		      const float ambientIntensity, 
		      const SbColor & ambientColor,
		      const SbVec3f & attenuation, 
		      const int32_t fogType,
		      const SbColor & fogColor, 
		      const float fogVisibility);
private:
  void updategl(SoState * const state);
};

#endif // !__SOGLENVIRONMENTELEMENT_H__
