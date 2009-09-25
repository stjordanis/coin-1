/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2008 by Kongsberg SIM.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Kongsberg SIM about acquiring
 *  a Coin Professional Edition License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg SIM, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

#include <Inventor/scxml/ScXMLEventTarget.h>

/*!
  \class ScXMLEventTarget ScXMLEventTarget.h Inventor/scxml/ScXMLEventTarget.h
  \brief base class for event targets for inter-system event communication

  This class manages inter-communication between SCXML systems and systems
  intended for communicating with such systems.  It also manages the external
  event queue for such systems (systems may also have internal event queues).

  \ingroup scxml
  \since Coin 3.1
*/

#include <cassert>
#include <map>
#include <list>
#include <algorithm>

#include <boost/scoped_ptr.hpp>

#include <Inventor/SbName.h>
#include <Inventor/C/threads/mutex.h>
#include <Inventor/sensors/SoAlarmSensor.h>
#include <Inventor/scxml/ScXMLEvent.h>
#include <Inventor/scxml/ScXMLEventElt.h>
#include <Inventor/scxml/ScXMLSendElt.h>
#include <Inventor/scxml/ScXMLStateMachine.h>
#include <Inventor/errors/SoDebugError.h>
#include "scxml/ScXMLCommonP.h"
#include "scxml/ScXMLP.h"

// *************************************************************************

class DelayEventData {
  // FIXME: these objects should per spec die with the invoking eventtarget.
public:
  DelayEventData(void)
  : targettype(NULL), targetname(NULL), delay(0.0f), alarm(NULL), event(NULL)
  {
  }
  ~DelayEventData(void)
  {
    delete [] this->targettype; this->targettype = NULL;
    delete [] this->targetname; this->targetname = NULL;
    if (event) { delete this->event; this->event = NULL; }
  }

  char * targettype;
  char * targetname;
  float delay;
  boost::scoped_ptr<SoAlarmSensor> alarm;
  const ScXMLEvent * event;
};

// *************************************************************************

typedef std::map<const char *, ScXMLEventTarget *> SessionMap;
typedef std::pair<const char *, ScXMLEventTarget *> SessionMapEntry;

typedef std::map<const char *, SessionMap *> TargetMap;
typedef std::pair<const char *, SessionMap *> TargetMapEntry;

typedef std::map<const char *, TargetMap *> TargetTypeMap;
typedef std::pair<const char *, TargetMap *> TargetTypeMapEntry;

class ScXMLEventTarget::PImpl {
public:
  PImpl(void) : targetsessionid(NULL) { }
  ~PImpl(void)
  {
    delete [] this->targetsessionid; this->targetsessionid = NULL;
    // FIX?ME: clean out queues
  }

  static TargetTypeMap * targettypes;

  std::list<const ScXMLEvent *> externaleventqueue;
  std::list<const ScXMLEvent *> internaleventqueue;
  std::list<const ScXMLEvent *> delayeventqueue;
  std::list<ScXMLEventTarget *> targetqueue;

  char * targetsessionid; // needed in destructor - can't fetch virtually

  static void delay_event_alarm_cb(void * userdata, SoSensor * sensor);

};

void
ScXMLEventTarget::PImpl::delay_event_alarm_cb(void * userdata, SoSensor * sensor)
{
  assert(userdata);
  DelayEventData * eventdata = static_cast<DelayEventData *>(userdata);
  assert(eventdata->targettype);
  assert(eventdata->targetname);
  assert(eventdata->event);

  ScXMLEventTarget * target =
    ScXMLEventTarget::getEventTarget(eventdata->targettype,
                                     eventdata->targetname);

  if (unlikely(!target)) {
    SoDebugError::post("delay_event_alarm_cb",
                       "delay-event %s: no target found",
                       eventdata->event->getEventName().getString());
  }
  else {
    target->queueEvent(eventdata->event);
    target->processEventQueue();
  }

  delete eventdata;
}

#define PRIVATE(obj) ((obj)->pimpl)

TargetTypeMap * ScXMLEventTarget::PImpl::targettypes = NULL;

SCXML_OBJECT_ABSTRACT_SOURCE(ScXMLEventTarget);

void
ScXMLEventTarget::initClass(void)
{
  SCXML_OBJECT_INIT_ABSTRACT_CLASS(ScXMLEventTarget, ScXMLObject, "ScXMLObject");

  ScXMLP::lock();
  assert(ScXMLEventTarget::PImpl::targettypes == NULL);
  ScXMLEventTarget::PImpl::targettypes = new TargetTypeMap;
  ScXMLP::unlock();
}

void
ScXMLEventTarget::cleanClass(void)
{
  // FIXME: clean up map
  ScXMLP::lock();
  assert(ScXMLEventTarget::PImpl::targettypes != NULL);
  delete ScXMLEventTarget::PImpl::targettypes;
  ScXMLEventTarget::PImpl::targettypes = NULL;
  ScXMLP::unlock();
  ScXMLEventTarget::classTypeId = SoType::badType();
}


/*!
*/
void
ScXMLEventTarget::registerEventTarget(ScXMLEventTarget * target, const char * sessionid)
{
  const SbName targettypename(target->getEventTargetType());
  const SbName targetnamename(target->getEventTargetName());
  SbName targetsessionid(SbName::empty());
  if (sessionid) {
    targetsessionid = SbName(sessionid);
  }

  assert(ScXMLEventTarget::PImpl::targettypes);
  ScXMLP::lock();
  TargetTypeMap::iterator typeit = ScXMLEventTarget::PImpl::targettypes->find(targettypename.getString());
  TargetMap * targetmap = NULL;
  if (typeit != ScXMLEventTarget::PImpl::targettypes->end()) {
    targetmap = typeit->second;
  } else {
    targetmap = new TargetMap;
    TargetTypeMapEntry entry(targettypename.getString(), targetmap);
    ScXMLEventTarget::PImpl::targettypes->insert(entry);
  }

  TargetMap::iterator targetit = targetmap->find(targetnamename.getString());
  SessionMap * sessionmap = NULL;
  if (targetit != targetmap->end()) {
    sessionmap = targetit->second;
  } else {
    sessionmap = new SessionMap;
    TargetMapEntry entry(targetnamename.getString(), sessionmap);
    targetmap->insert(entry);
  }

  SessionMap::iterator sessionit = sessionmap->find(targetsessionid.getString());
  if (sessionit != sessionmap->end()) {
    if (target == sessionit->second) {
      // already registered
      SoDebugError::post("ScXMLEventTarget::registerEventTarget",
                         "type/name/session touple already registered for this target");
    } else {
      SoDebugError::post("ScXMLEventTarget::registerEventTarget",
                         "type/name/session touple already registered for another target");
    }
  } else {
    SessionMapEntry entry(targetsessionid.getString(), target);
    sessionmap->insert(entry);
  }

  ScXMLP::unlock();
}

/*!
*/
void
ScXMLEventTarget::unregisterEventTarget(ScXMLEventTarget * target, const char * sessionid)
{
  const SbName targettypename(target->getEventTargetType());
  const SbName targetnamename(target->getEventTargetName());
  SbName targetsessionid(SbName::empty());
  if (sessionid) {
    targetsessionid = SbName(sessionid);
  }

  assert(ScXMLEventTarget::PImpl::targettypes);
  ScXMLP::lock();
  TargetTypeMap::iterator typeit = ScXMLEventTarget::PImpl::targettypes->find(targettypename.getString());
  if (unlikely(typeit == ScXMLEventTarget::PImpl::targettypes->end())) {
    SoDebugError::post("ScXMLEventTarget::unregisterEventTarget",
                       "given target type ('%s') not registered",
                       targettypename.getString());
    ScXMLP::unlock();
    return;
  }
  TargetMap * targetmap = typeit->second;

  TargetMap::iterator targetit = targetmap->find(targetnamename.getString());
  if unlikely (targetit == targetmap->end()) {
    SoDebugError::post("ScXMLEventTarget::unregisterEventTarget",
                       "given target name not registered");
    ScXMLP::unlock();
    return;
  }

  SessionMap * sessionmap = targetit->second;

  SessionMap::iterator sessionit = sessionmap->find(targetsessionid.getString());
  if unlikely (sessionit == sessionmap->end()) {
    if (target == sessionit->second) {
      sessionmap->erase(sessionit);
    } else {
      SoDebugError::post("ScXMLEventTarget::unregisterEventTarget",
                         "given target session not registered");
    }
  }
  ScXMLP::unlock();
}

/*!
*/
ScXMLEventTarget *
ScXMLEventTarget::getEventTarget(const char * targettype, const char * targetname, const char * sessionid)
{
  const SbName targettypename(targettype);
  const SbName targetnamename(targetname);
  SbName targetsessionid(SbName::empty());
  if (sessionid != NULL) {
    targetsessionid = SbName(sessionid);
  }

  assert(ScXMLEventTarget::PImpl::targettypes);
  ScXMLP::lock();
  TargetTypeMap::iterator typeit = ScXMLEventTarget::PImpl::targettypes->find(targettypename.getString());
  if unlikely (typeit == ScXMLEventTarget::PImpl::targettypes->end()) {
    // SoDebugError::post("ScXMLEventTarget::getEventTarget",
    //                    "given target type ('%s') not registered 1",
    //                    targettypename.getString());
    ScXMLP::unlock();
    return NULL;
  }
  TargetMap * targetmap = typeit->second;

  TargetMap::iterator targetit = targetmap->find(targetnamename.getString());
  if unlikely (targetit == targetmap->end()) {
    // SoDebugError::post("ScXMLEventTarget::getEventTarget",
    //                    "given target (type '%s' '%s') not registered",
    //                    targettypename.getString(), targetnamename.getString());
    ScXMLP::unlock();
    return NULL;
  }
  SessionMap * sessionmap = targetit->second;

  SessionMap::iterator sessionit = sessionmap->find(targetsessionid.getString());
  if unlikely (targetit == targetmap->end()) {
    // SoDebugError::post("ScXMLEventTarget::getEventTarget",
    //                    "given target (type '%s' '%s') with session '%s' not registered",
    //                    targettypename.getString(), targetnamename.getString(), targetsessionid.getString());
    ScXMLP::unlock();
    return NULL;
  }

  ScXMLEventTarget * target = sessionit->second;
  ScXMLP::unlock();
  return target;
}


/*!
*/
ScXMLEventTarget::ScXMLEventTarget(void)
: targetname(NULL),
  targettype(NULL),
  currentevent(NULL),
  isprocessingqueue(FALSE)
{
}

/*!
*/
ScXMLEventTarget::~ScXMLEventTarget(void)
{
  if (this->targetname && this->targettype) {
    ScXMLEventTarget::unregisterEventTarget(this, PRIVATE(this)->targetsessionid);
  }
  delete [] this->targetname;
  this->targetname = NULL;
  delete [] this->targettype;
  this->targettype = NULL;
}

/*!
*/
void
ScXMLEventTarget::setEventTargetType(const char * targettypestr)
{
  if (this->targetname && this->targettype) {
    ScXMLEventTarget::unregisterEventTarget(this, PRIVATE(this)->targetsessionid);
  }

  delete [] this->targettype;
  this->targettype = NULL;

  if (targettypestr) {
    this->targettype = new char [strlen(targettypestr) + 1];
    assert(this->targettype);
    strcpy(this->targettype, targettypestr);

    if (this->targetname && this->targettype) {
      if (this->isOfType(ScXMLStateMachine::getClassTypeId())) {
        SbName sessionid = static_cast<ScXMLStateMachine *>(this)->getSessionId();
        delete [] PRIVATE(this)->targetsessionid;
        PRIVATE(this)->targetsessionid = new char [strlen(sessionid.getString()) + 1];
        strcpy(PRIVATE(this)->targetsessionid, sessionid.getString());
      }
      ScXMLEventTarget::registerEventTarget(this, PRIVATE(this)->targetsessionid);
    }
  }
}

/*!
  \fn const char * ScXMLEventTarget::getEventTargetType(void) const
*/

/*!
*/
void
ScXMLEventTarget::setEventTargetName(const char * targetnamestr)
{
  if (this->targetname && this->targettype) {
    ScXMLEventTarget::unregisterEventTarget(this, PRIVATE(this)->targetsessionid);
  }

  delete [] this->targetname;
  this->targetname = NULL;

  if (targetnamestr) {
    this->targetname = new char [strlen(targetnamestr) + 1];
    assert(this->targetname);
    strcpy(this->targetname, targetnamestr);

    if (this->targetname && this->targettype) {
      if (this->isOfType(ScXMLStateMachine::getClassTypeId())) {
        SbName sessionid = static_cast<ScXMLStateMachine *>(this)->getSessionId();
        delete [] PRIVATE(this)->targetsessionid;
        PRIVATE(this)->targetsessionid = new char [strlen(sessionid.getString()) + 1];
        strcpy(PRIVATE(this)->targetsessionid, sessionid.getString());
      }
      ScXMLEventTarget::registerEventTarget(this, PRIVATE(this)->targetsessionid);
    }
  }
}

/*!
  \fn const char * ScXMLEventTarget::getEventTargetName(void) const
*/


/*!
  Queues an ScXMLEvent event.
  The event is copied, and can be deleted as soon as the function return.
*/
void
ScXMLEventTarget::queueEvent(const ScXMLEvent * event)
{
  ScXMLP::lock();
  PRIVATE(this)->externaleventqueue.push_back(event->clone());
  ScXMLP::unlock();
}

/*!
  Queues an ScXMLEvent event that has the given \a eventid as its event name.
*/
void
ScXMLEventTarget::queueEvent(const SbName & eventid)
{
  ScXMLEvent * event = new ScXMLEvent;
  event->setEventName(eventid);
  ScXMLP::lock();
  PRIVATE(this)->externaleventqueue.push_back(event);
  ScXMLP::unlock();
}

/*!
*/
void
ScXMLEventTarget::queueInternalEvent(const SbName & eventid)
{
  ScXMLEvent * event = new ScXMLEvent;
  event->setEventName(eventid);
  ScXMLP::lock();
  PRIVATE(this)->internaleventqueue.push_back(event);
  ScXMLP::unlock();
}

/*!
*/
void
ScXMLEventTarget::queueInternalEvent(const ScXMLEvent * event)
{
  ScXMLP::lock();
  PRIVATE(this)->internaleventqueue.push_back(event->clone());
  ScXMLP::unlock();
}

/*!
*/
SbBool
ScXMLEventTarget::processEventQueue(void)
{
  if (this->isprocessingqueue) {
    // to avoid recursive event-processing - everything must go through the event
    // queues in a strict serialized fashion...
    return FALSE;
  }

  this->isprocessingqueue = TRUE;

  SbBool eventsuccess = FALSE;
  SbBool communicating = TRUE;
  while (communicating) {
    const ScXMLEvent * event = NULL;
    while ((event = this->getNextEvent())) {
      if (this->processOneEvent(event)) {
        eventsuccess = TRUE;
      }
      delete event;
    }

    // have we communicated with other event targets, and need to roll
    // their event queue as well?
    if (PRIVATE(this)->targetqueue.empty()) {
      communicating = FALSE;
    } else {
      while (!PRIVATE(this)->targetqueue.empty()) {
        // FIXME: process external event targets we've communicated with,
        ScXMLP::lock();
        ScXMLEventTarget * target = PRIVATE(this)->targetqueue.front();
        PRIVATE(this)->targetqueue.pop_front();
        ScXMLP::unlock();
        target->processEventQueue();
      }
    }
  }

  this->isprocessingqueue = FALSE;
  return eventsuccess;
}

/*!
*/
const ScXMLEvent *
ScXMLEventTarget::getNextEvent(void)
{
  const ScXMLEvent * nextevent = NULL;
  // internal events have first priority
  nextevent = this->getNextInternalEvent();
  if (!nextevent) {
    nextevent = this->getNextExternalEvent();
  }
  return nextevent;
}

/*!
*/
const ScXMLEvent *
ScXMLEventTarget::getNextInternalEvent(void)
{
  ScXMLP::lock();
  const ScXMLEvent * nextevent = NULL;
  if (!PRIVATE(this)->internaleventqueue.empty()) {
    nextevent = PRIVATE(this)->internaleventqueue.front();
    PRIVATE(this)->internaleventqueue.pop_front();
  }
  ScXMLP::unlock();
  return nextevent;
}

/*!
*/
const ScXMLEvent *
ScXMLEventTarget::getNextExternalEvent(void)
{
  ScXMLP::lock();
  const ScXMLEvent * nextevent = NULL;
  if (!PRIVATE(this)->externaleventqueue.empty()) {
    nextevent = PRIVATE(this)->externaleventqueue.front();
    PRIVATE(this)->externaleventqueue.pop_front();
  }
  ScXMLP::unlock();
  return nextevent;
}

/*!
  This function processes one event.  The base class implementation does nothing.
*/
SbBool
ScXMLEventTarget::processOneEvent(const ScXMLEvent * event)
{
  return FALSE;
}

/*!
  Sets a pointer for the event that is 'current' during event processing.
  This is an internal method, and the updating of the current event should
  be handled automatically.
*/
void
ScXMLEventTarget::setCurrentEvent(const ScXMLEvent * event)
{
  this->currentevent = event;
}

/*!
  \fn const ScXMLEvent * ScXMLEventTarget::getCurrentEvent(void) const

  This method returns the current event during event processing, and NULL
  when not processing events.

  Event processing is in special cases done with NULL as the current event,
  as for instance during state machine initialization.
*/

// inter-system communication
/*!
*/
SbBool
ScXMLEventTarget::sendExternalEvent(const ScXMLSendElt * sendelt)
{
  // SCXML errors produced:
  //   internal: error.send.TargetTypeInvalid
  //   - event "targettype" specified is not supported
  //   internal: error.send.TargetUnavailable
  //   - value of the "target" attribute is not supported, invalid or unreachable
  //   internal: error.Fetch
  //   - event and namelist or inline content requirement error
  //   4.1.1:
  //   send.failed.targettypenotsupported.stateID.sendID
  //   send.failed.targetnotfound.stateID.sendID

  const char * sendtargettypeattr = sendelt->getTargetTypeAttribute();
  const char * sendtargetattr = sendelt->getTargetAttribute();

  if (!sendtargettypeattr && !sendtargetattr) { // FIXME: necessary by spec?
    sendtargettypeattr = this->getEventTargetType();
    sendtargetattr = this->getEventTargetName();
  }

  if (!sendtargettypeattr) {
    this->queueInternalEvent(SbName("error.send.TargetTypeInvalid"));
    return FALSE;
  }

  if (strcmp(sendtargettypeattr, "scxml") == 0) {
  }
  else if (strcmp(sendtargettypeattr, "x-coin-navigation") == 0) {
  }
  else {
    this->queueInternalEvent(SbName("error.send.TargetTypeInvalid"));
    // FIXME: is this something we know? should we perhaps unwind the map to see that
    // no such target type is registered at all first before instead flagging
    // targetunavailable?
    return FALSE;
  }

  float delay = 0.0f;
  const char * delaystr = sendelt->getDelayAttribute();
  if (delaystr) {
    // format: "<num>s" or "<num>ms"
    int len = strlen(delaystr);
    if (len > 1) {
      if (delaystr[len-1] != 's') {
        SoDebugError::post("ScXMLEventTarget::sendExternalEvent",
                           "delay attribute format error (was: %s)", delaystr);
        return FALSE;
      }
      if (delaystr[len-2] == 'm') {
        delay = atof(delaystr) / 1000.0f;
      } else {
        delay = atof(delaystr);
      }
    } else {
      SoDebugError::post("ScXMLEventTarget::sendExternalEvent",
                         "delay attribute format error (was: %s)", delaystr);
      return FALSE;
    }
  }

  ScXMLEventTarget * target =
    ScXMLEventTarget::getEventTarget(sendtargettypeattr, sendtargetattr);
  if (unlikely(!target)) {
    this->queueInternalEvent(SbName("error.send.TargetUnavailable"));
    return FALSE;
  }

  const ScXMLEvent * event = sendelt->createEvent(this);
  if (!event) {
    return FALSE;
  }

  if (delay > 0.0f) {
    DelayEventData * eventdata = new DelayEventData;
    eventdata->targettype = new char [strlen(sendtargettypeattr) + 1];
    strcpy(eventdata->targettype, sendtargettypeattr);
    eventdata->targetname = new char [strlen(sendtargetattr) + 1];
    strcpy(eventdata->targetname, sendtargetattr);
    eventdata->event = event;
    eventdata->alarm.reset(new SoAlarmSensor(PImpl::delay_event_alarm_cb,
                                             eventdata));
    eventdata->alarm->setTimeFromNow(SbTime(double(delay)));
    eventdata->alarm->schedule();
  } else {
    target->queueEvent(event);
    delete event;
    std::list<ScXMLEventTarget *>::iterator findit =
      std::find(PRIVATE(this)->targetqueue.begin(),
                PRIVATE(this)->targetqueue.end(),
                target);
    if (findit == PRIVATE(this)->targetqueue.end()) {
      ScXMLP::lock();
      PRIVATE(this)->targetqueue.push_back(target);
      ScXMLP::unlock();
    }
  }

  return TRUE;
}

/*!
*/
SbBool
ScXMLEventTarget::sendInternalEvent(const ScXMLEventElt * eventelt)
{
  assert(eventelt);
  ScXMLEvent * event = eventelt->createEvent(this);
  this->queueInternalEvent(event);
  delete event;
  return TRUE;
}

#undef PRIVATE