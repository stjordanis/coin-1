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

#ifndef __SOSFENUM_H__
#define __SOSFENUM_H__

#include <Inventor/fields/SoSField.h>
#include <Inventor/SbString.h>

#if defined(COIN_EXCLUDE_SOSFENUM)
#error "Configuration settings not respected, should not include this file!"
#endif // COIN_EXCLUDE_SOSFENUM


class SoSFEnum : public SoSField {
  typedef SoSField inherited;

//$ BEGIN TEMPLATE SField(SoSFEnum, int, int)
private:
  static SoType classTypeId;

public:
  static void * createInstance(void);
  static SoType getClassTypeId(void);
  virtual SoType getTypeId(void) const;

  static void initClass(void);
  static void cleanClass(void);

  virtual void copyFrom(const SoField & field);
  const SoSFEnum & operator = (const SoSFEnum & field);
  virtual SbBool isSame(const SoField & field) const;
public:
  SoSFEnum(void);
  virtual ~SoSFEnum(void);
private:
  virtual SbBool readValue(SoInput * in);
  virtual void writeValue(SoOutput * out) const;
public:
  /*! Returns this field's value. */
  int getValue(void) const
    { this->evaluate(); return this->value; }
  void setValue(int newvalue);
  /*! Copy value from \a newvalue into this field. */
  int operator = (int newvalue)
    { this->setValue(newvalue); return this->value; }

  int operator == (const SoSFEnum & field) const;
  /*! Returns \a TRUE if this field is not equal to \a field. */
  int operator != (const SoSFEnum & field) const
    { return ! operator == (field); }

protected:
  /*! The value contained in the field. */
  int value;
//$ END TEMPLATE SField

public:
  void setValue(const SbName name);
  void setEnums(const int num, const int * vals, const SbName * names);

protected:
  virtual SbBool findEnumValue(const SbName & name, int & val);
  virtual SbBool findEnumName(int value, const SbName * & name) const;

  int numEnums;
  int * enumValues;
  SbName * enumNames;
  SbBool legalValuesSet;

private:
  virtual void convertTo(SoField * dest) const;

  friend class SoMFEnum; // For readValue() & writeValue().
};


// This macro is only called from node classes which uses SoSFEnum
// instances. By putting this macro in this headerfile instead of
// nodes/SoSubNode.h, we avoid making node classes including
// SoSubNode.h (which is basically all of them) dependent on
// SoSFEnum.h.

#define SO_NODE_SET_SF_ENUM_TYPE(_fieldname_, _enumtype_) \
  do { \
    int _enum_num; \
    const int * _enum_vals; \
    const SbName * _enum_names; \
    this->fieldData.getEnumData(SO__QUOTE(_enumtype_), \
                                _enum_num, _enum_vals, _enum_names); \
    this->_fieldname_.setEnums(_enum_num, _enum_vals, _enum_names); \
  } while (FALSE)

#endif // !__SOSFENUM_H__
