
#include <stdio.h>
#include <math.h>

#include <map>
#include <algorithm>

#define BBCDEBUG_LEVEL 1
#include "ADMObjects.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

std::map<uint_t,std::string> ADMObject::typeLabelMap;
std::map<uint_t,std::string> ADMObject::formatLabelMap;

// absolute maximum time
const uint64_t ADMObject::MaxTime = (uint64_t)-1;

/*--------------------------------------------------------------------------------*/
/** Base constructor for all objects
 *
 * @param _owner an instance of ADMData that this object should belong to
 * @param _id unique ID for this object (specified as part of the ADM)
 * @param _name optional human-friendly name of the object
 *
 */
/*--------------------------------------------------------------------------------*/
ADMObject::ADMObject(ADMData& _owner, const std::string& _id, const std::string& _name) : owner(_owner),
                                                                                          id(_id),
                                                                                          name(_name),
                                                                                          standarddef(false)
{
  if (typeLabelMap.size() == 0)
  {
    // populate typeLabel map
    SetTypeDefinition(TypeLabel_DirectSpeakers, "DirectSpeakers");
    SetTypeDefinition(TypeLabel_Matrix,         "Matrix");
    SetTypeDefinition(TypeLabel_Objects,        "Objects");
    SetTypeDefinition(TypeLabel_HOA,            "HOA");
    SetTypeDefinition(TypeLabel_Binaural,       "Binaural");
  }

  // reserve first entry for attributes only
  AddValue(XMLValue());
}

ADMObject::ADMObject(ADMData& _owner, const ADMObject *obj) : owner(_owner),
                                                              id(obj->GetID()),
                                                              name(obj->GetName()),
                                                              typeLabel(obj->GetTypeLabel()),
                                                              typeDefinition(obj->GetTypeDefinition()),
                                                              values(obj->values),
                                                              standarddef(obj->standarddef)
{
  if (typeLabelMap.size() == 0)
  {
    // populate typeLabel map
    SetTypeDefinition(TypeLabel_DirectSpeakers, "DirectSpeakers");
    SetTypeDefinition(TypeLabel_Matrix,         "Matrix");
    SetTypeDefinition(TypeLabel_Objects,        "Objects");
    SetTypeDefinition(TypeLabel_HOA,            "HOA");
    SetTypeDefinition(TypeLabel_Binaural,       "Binaural");
  }

  // reserve first entry for attributes only
  AddValue(XMLValue());
}

ADMObject::~ADMObject()
{
}

/*--------------------------------------------------------------------------------*/
/** Set and Get object ID
 *
 * @param id new ID
 * @param start start index to find an ID from
 *
 * @note setting the ID updates the map held within the ADMData object
 * @note some types of ID start numbering at 0x1000, some at 0x0000
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetID(const std::string& _id, uint_t start)
{
  // get owner to change it and update its map of objects in the process
  owner.ChangeID(this, _id, start);
}

/*--------------------------------------------------------------------------------*/
/** Register this object with the owner
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Register()
{
  owner.Register(this);
}

/*--------------------------------------------------------------------------------*/
/** Log an error internally and output to global error system
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::LogError(const char *fmt, ...) const
{
  va_list ap;

  va_start(ap, fmt);
  LogErrorV(fmt, ap);
  va_end(ap);
}

void ADMObject::LogErrorV(const char *fmt, va_list ap) const
{
  ADMData *adm = const_cast<ADMData *>(&owner);
  if (adm) adm->LogErrorV(fmt, ap);
}

/*--------------------------------------------------------------------------------*/
/** Set object typeLabel
 *
 * @param type typeLabel index
 *
 * @note if type is a recognized typeLabel, typeDefinition will automatically be set!
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetTypeLabel(uint_t type)
{
  if (type != typeLabel)
  {
    BBCDEBUG2(("%s(%s,%s): Change typeLabel from %s to %04x", GetType().c_str(), GetName().c_str(), GetID().c_str(), typeLabel.ToString().c_str(), type));

    typeLabel = type;

    // only update referenced objects if *not* in pure mode
    if (!owner.InPureMode())
    {
      // update typeDefinition if possible
      if (typeLabelMap.find(typeLabel) != typeLabelMap.end()) SetTypeDefinition(typeLabelMap[typeLabel]);

      // update typeLabel's of referenced objects
      UpdateRefTypeLabels();

      // in some cases, setting the typeLabel causes a change in ID
      UpdateID();
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Set and Get object typeDefinition
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetTypeDefinition(const std::string& str)
{
  if (str != typeDefinition)
  {
    BBCDEBUG2(("%s(%s,%s): Change typeDefinition from '%s' to '%s'", GetType().c_str(), GetName().c_str(), GetID().c_str(), typeDefinition.ToString().c_str(), str.c_str()));

    typeDefinition = str;

    // only update referenced objects if *not* in pure mode
    if (!owner.InPureMode())
    {
      // update typeDefinitions's of referenced objects
      UpdateRefTypeDefinitions();
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Set typeLabel and typeDefinition (if valid and ADM is not in pure mode) in supplied object
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetTypeInfoInObject(ADMObject *obj) const
{
  // only update object if *not* in pure mode and object is NOT a standard definition object
  if (!owner.InPureMode() && !obj->IsStandardDefinition())
  {
    if (typeDefinition.IsSet())  obj->SetTypeDefinition(typeDefinition);
    if (typeLabel.IsSet())       obj->SetTypeLabel(typeLabel);
  }
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::UpdateID()
{
  // do nothing
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetValues()
{
  if (values.size() && values[0].name.empty())
  {
    values[0].GetAttributeAndErase(typeLabel);
    values[0].GetAttributeAndErase(typeDefinition);
  }
}

/*--------------------------------------------------------------------------------*/
/** Try to connect references after all objects have been set up
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetReferences()
{
  XMLValues::iterator it;

  // cycle through values looking for references to the specified object type
  for (it = values.begin(); (it != values.end());)
  {
    const ADMObject *obj  = NULL; // for neater response handling
    const XMLValue& value = *it;
    bool  refrejected = false;

#if BBCDEBUG_LEVEL >= 4
    if (!value.name.empty()) BBCDEBUG("%s<%s>: handling reference '%s' / '%s'", GetMapEntryID().c_str(), StringFrom(this).c_str(), value.name.c_str(), value.value.c_str());
#endif
    
    // the value name is reference name of object type
    if (value.name == ADMAudioContent::Reference)
    {
      ADMAudioContent *ref;

      // look up reference using owner ADMData object, try to cast it to an object of the correct type
      if ((ref = dynamic_cast<ADMAudioContent *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioObject::Reference)
    {
      ADMAudioObject *ref;

      if ((ref = dynamic_cast<ADMAudioObject *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioTrack::Reference)
    {
      ADMAudioTrack *ref;

      if ((ref = dynamic_cast<ADMAudioTrack *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioPackFormat::Reference)
    {
      ADMAudioPackFormat *ref;

      if ((ref = dynamic_cast<ADMAudioPackFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioStreamFormat::Reference)
    {
      ADMAudioStreamFormat *ref;

      if ((ref = dynamic_cast<ADMAudioStreamFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioTrackFormat::Reference)
    {
      ADMAudioTrackFormat *ref;

      if ((ref = dynamic_cast<ADMAudioTrackFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioChannelFormat::Reference)
    {
      ADMAudioChannelFormat *ref;

      if ((ref = dynamic_cast<ADMAudioChannelFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioObject::ComplementaryObjectReference)
    {
      ADMAudioObject *ref;

      if ((ref = dynamic_cast<ADMAudioObject *>(owner.GetReference(value, &ADMAudioObject::Type))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !AddComplementary(ref);
      }
    }
    else
    {
      ++it;
      // note continue to avoid removing non-reference values
      continue;
    }

    if (obj)
    {
      if (refrejected)
      {
        LogError("Reference %s as reference '%s' for %s REJECTED",
              obj->ToString().c_str(),
              value.value.c_str(),
              ToString().c_str());
      }
      else
      {
        BBCDEBUG3(("Found %s as reference '%s' for %s",
                obj->ToString().c_str(),
                value.value.c_str(),
                ToString().c_str()));
      }
    }
    else
    {
      LogError("Cannot find %s reference '%s' for %s",
            value.name.c_str(), value.value.c_str(),
            ToString().c_str());
    }

    // REMOVE value (the reference) from the list
    it = values.erase(it);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  XMLValues::const_iterator it;

  UNUSED_PARAMETER(objects);

  // add attributes value (no name)
  {
    XMLValue value;

    if (GetID() != "")
    {
      if (GetType() == ADMAudioTrack::Type) value.SetAttribute("UID",            GetID());
      else                                  value.SetAttribute(GetType() + "ID", GetID());
    }

    if (GetName() != "")
    {
      value.SetAttribute(GetType() + "Name", GetName());
    }

    if (full || typeLabel.IsSet())
    {
      value.SetAttribute(typeLabel);
    }
    if (full || typeDefinition.IsSet())
    {
      value.SetAttribute(typeDefinition);
    }

    objvalues.AddValue(value);
  }

  // add existing list of values/attributes to list
  for (it = values.begin(); it != values.end(); ++it)
  {
    // output anything that has at least a name
    if (!it->name.empty())
    {
      objvalues.AddValue(*it);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Copy a list of references from one ADM to this ADM
 */
/*--------------------------------------------------------------------------------*/
template<typename T>
void ADMObject::CopyReferencesEx(std::vector<T *>& dst, const std::vector<T *>& src)
{
  uint_t i;
  for (i = 0; i < src.size(); i++)
  {
    ADMObject *obj;

    if ((obj = owner.GetWritableObjectByID(src[i]->GetID(), src[i]->GetType())) != NULL)
    {
      T *obj2;
      if ((obj2 = dynamic_cast<T *>(obj)) != NULL)
      {
        typename std::vector<T *>::const_iterator it;

        // ensure object is not already in the list
        if ((it = find(dst.begin(), dst.end(), obj2)) == dst.end())
        {
          dst.push_back(obj2);
          // add a reference back to this object
          obj->AddBackReference(this);
        }
      }
      else LogError("Object '%s' is not of the correct type!", src[i]->ToString().c_str());
    }
    else LogError("Failed to find object '%s' is new ADM!", src[i]->ToString().c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Add reference *back* to an object that references this one
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::AddBackReference(ADMObject *obj)
{
  BBCDEBUG3(("%s<%s>: Adding %s<%s> to back references",
             GetMapEntryID().c_str(), StringFrom(this).c_str(),
             obj->GetMapEntryID().c_str(), StringFrom(obj).c_str()));
  backrefs.push_back(obj);
}

/*--------------------------------------------------------------------------------*/
/** Remove back reference (added above)
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::RemoveBackReference(ADMObject *obj)
{
  std::vector<ADMObject *>::iterator it;

  if (backrefs.size() && ((it = std::find(backrefs.begin(), backrefs.end(), obj)) != backrefs.end()))
  {
    BBCDEBUG3(("%s<%s>: Removing %s<%s> from back references",
               GetMapEntryID().c_str(), StringFrom(this).c_str(),
               obj->GetMapEntryID().c_str(), StringFrom(obj).c_str()));
    backrefs.erase(it);
  }
}

/*--------------------------------------------------------------------------------*/
/** This provide a mechanism to lock this object for any ADM manipulation operations
 */
/*--------------------------------------------------------------------------------*/
ADMObject::operator const ThreadLockObject& () const
{
  return owner.operator const ThreadLockObject& ();
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioProgramme::Type      = "audioProgramme";
const std::string ADMAudioProgramme::Reference = Type + "IDRef";
const std::string ADMAudioProgramme::IDPrefix  = "APR_";

ADMAudioProgramme::~ADMAudioProgramme()
{
  RemoveReferences(this, contentrefs);
  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::SetValues()
{
  ADMObject::SetValues();

  if (values.size() && values[0].name.empty())
  {
    values[0].GetAttributeAndErase(language);
    values[0].GetAttributeAndErase(maxDuckingDepth);
    values[0].GetAttributeAndErase(audioProgrammeLanguage);
    values[0].GetAttributeAndErase(formatLabel);
    values[0].GetAttributeAndErase(formatDefinition);
  }

  // read loudnessMetadata sub value
  const XMLValue *subvalue;
  if ((subvalue = values.GetValue(ADMLoudnessMetadata::GetElementName())) != NULL)
  {
    loudnessMetadata.SetValues(*subvalue);
    values.EraseValue(subvalue);
  }
  if ((subvalue = values.GetValue(ADMAudioProgrammeReferenceScreen::GetElementName())) != NULL)
  {
    audioProgrammeReferenceScreen.SetValues(*subvalue);
    values.EraseValue(subvalue);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // update value containing attributes
  objvalues[0].SetAttribute(language, full);
  objvalues[0].SetAttribute(maxDuckingDepth, full);
  objvalues[0].SetAttribute(audioProgrammeLanguage, full);
  objvalues[0].SetAttribute(formatLabel, full);
  objvalues[0].SetAttribute(formatDefinition, full);

  {
    XMLValue value;
    if (loudnessMetadata.GetValues(value, full))
    {
      objvalues.AddValue(value);
    }
  }

  {
    XMLValue value;
    if (audioProgrammeReferenceScreen.GetValues(value, full))
    {
      objvalues.AddValue(value);
    }
  }

  // references only
  object.genref  = true;

  for (i = 0; i < contentrefs.size(); i++)
  {
    object.obj = contentrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < contentrefs.size(); i++) GenerateReference(str, contentrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Copy references from another object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::CopyReferences(const ADMObject *_obj)
{
  ADMObject::CopyReferences(_obj);

  const ADMAudioProgramme *obj = dynamic_cast<const ADMAudioProgramme *>(_obj);
  if (obj)
  {
    CopyReferencesEx<>(contentrefs, obj->contentrefs);
  }
  else LogError("Cannot copy references from '%s', type is '%s' not '%s'", _obj->ToString().c_str(), _obj->GetType().c_str(), GetType().c_str());
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioContent::Type      = "audioContent";
const std::string ADMAudioContent::Reference = Type + "IDRef";
const std::string ADMAudioContent::IDPrefix  = "ACO_";

ADMAudioContent::~ADMAudioContent()
{
  RemoveReferences(this, objectrefs);
  RemoveReferences(this);
  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::SetValues()
{
  ADMObject::SetValues();

  if (values.size() && values[0].name.empty())
  {
    values[0].GetAttributeAndErase(language);
  }

  // read loudnessMetadata sub value
  const XMLValue *subvalue;
  if ((subvalue = values.GetValue(ADMLoudnessMetadata::GetElementName())) != NULL)
  {
    loudnessMetadata.SetValues(*subvalue);
    values.EraseValue(subvalue);
  }
  if ((subvalue = values.GetValue(ADMAudioContentDialogue::GetElementName())) != NULL)
  {
    dialogue.SetValues(*subvalue);
    values.EraseValue(subvalue);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // update value containing attributes
  objvalues[0].SetAttribute(language, full);

  {
    XMLValue value;
    if (loudnessMetadata.GetValues(value, full))
    {
      objvalues.AddValue(value);
    }
  }

  {
    XMLValue value;
    if (dialogue.GetValues(value, full))
    {
      objvalues.AddValue(value);
    }
  }

  // references only
  object.genref  = true;

  for (i = 0; i < objectrefs.size(); i++)
  {
    object.obj = objectrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < objectrefs.size(); i++) GenerateReference(str, objectrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Copy references from another object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::CopyReferences(const ADMObject *_obj)
{
  ADMObject::CopyReferences(_obj);

  const ADMAudioContent *obj = dynamic_cast<const ADMAudioContent *>(_obj);
  if (obj)
  {
    CopyReferencesEx<>(objectrefs, obj->objectrefs);
  }
  else LogError("Cannot copy references from '%s', type is '%s' not '%s'", _obj->ToString().c_str(), _obj->GetType().c_str(), GetType().c_str());
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioObject::Type                         = "audioObject";
const std::string ADMAudioObject::Reference                    = Type + "IDRef";
const std::string ADMAudioObject::IDPrefix                     = "AO_";
const std::string ADMAudioObject::ComplementaryObjectReference = "audioComplementaryObjectIDRef";

/*--------------------------------------------------------------------------------*/
/** ADM AudioObject object
 *
 * @param _owner an instance of ADMData that this object should belong to
 * @param _id unique ID for this object (specified as part of the ADM)
 * @param _name optional human-friendly name of the object
 *
 * @note type passed to base constructor is fixed by static member variable Type
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObject::ADMAudioObject(ADMData& _owner, const std::string& _id, const std::string& _name) :
  ADMObject(_owner, _id, _name),
  starttrack(0),
  trackcount(0)
{
  Register();
}

ADMAudioObject::ADMAudioObject(ADMData& _owner, const ADMAudioObject *obj) :
  ADMObject(_owner, obj),
  starttrack(obj->starttrack),
  trackcount(obj->trackcount),
  startTime(obj->startTime),
  duration(obj->duration),
  dialogue(obj->dialogue),
  importance(obj->importance),
  interact(obj->interact),
  disableDucking(obj->disableDucking)
{
  Register();
}

ADMAudioObject::~ADMAudioObject()
{
  RemoveReferences(this, objectrefs);
  RemoveReferences(this, packformatrefs);
  RemoveReferences(this, trackrefs);
  RemoveReferences(this, complementaryobjectrefs);
  RemoveReferences(this);

  // remove this object from any cursors
  uint_t i;
  for (i = 0; i < cursorrefs.size(); i++) cursorrefs[i]->Remove(this);

  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::SetValues()
{
  ADMObject::SetValues();

  if (values.size() && values[0].name.empty())
  {
    values[0].GetAttributeAndErase(startTime);
    values[0].GetAttributeAndErase(duration);
  }

  values.GetValueAndErase(dialogue);
  values.GetValueAndErase(importance);
  values.GetValueAndErase(interact);
  values.GetValueAndErase(disableDucking);

  XMLValue *value;
  while ((value = values.GetValueWritable(ADMAudioObjectInteraction::GetElementName())) != NULL)
  {
    ADMAudioObjectInteraction audioObjectInteraction;

    audioObjectInteraction.SetValues(*value);
    audioObjectInteractionList.push_back(audioObjectInteraction);
      
    values.EraseValue(value);
  }
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioObject object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioObject::Add(ADMAudioTrack *obj)
{
  AddIfNotExists(trackrefs, obj);
  SortTracks();

  return true;
}

/*--------------------------------------------------------------------------------*/
/** Add track cursor reference
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioObject::Add(ADMTrackCursor *cursor)
{
  if (std::find(cursorrefs.begin(), cursorrefs.end(), cursor) == cursorrefs.end())
  {
    cursorrefs.push_back(cursor);
  }

  return true;
}

/*--------------------------------------------------------------------------------*/
/** Add track cursor reference
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::Remove(ADMTrackCursor *cursor)
{
  std::vector<ADMTrackCursor *>::iterator it;

  if ((it = std::find(cursorrefs.begin(), cursorrefs.end(), cursor)) != cursorrefs.end())
  {
    BBCDEBUG2(("Removing link between %s and ADMTrackCursor<%s>", ToString().c_str(), StringFrom(*it).c_str()));
    cursorrefs.erase(it);
  }
}

/*--------------------------------------------------------------------------------*/
/** Sort tracks
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::SortTracks()
{
  std::sort(trackrefs.begin(), trackrefs.end(), &ADMAudioTrack::Compare);
  starttrack = trackrefs[0]->GetTrackNum();
  trackcount = (uint_t)trackrefs.size();
}

/*--------------------------------------------------------------------------------*/
/** Process object parameters for rendering
 *
 * @param channel channel number (within object)
 * @param dst object parameters object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::ProcessObjectParameters(uint_t channel, AudioObjectParameters& dst)
{
  UNUSED_PARAMETER(channel);

  // use parameters from audio objects (if they are not the default values) to set parameters within AudioObjectParameters objects (if they are *not* already set)
  if (!dst.IsObjectImportanceSet() && importance.IsSet())     dst.SetObjectImportance(importance);
  if (!dst.IsInteractSet()         && interact.IsSet())       dst.SetInteract(interact);
  if (!dst.IsDialogueSet()         && dialogue.IsSet())       dst.SetDialogue(dialogue);
  if (!dst.IsDisableDuckingSet()   && disableDucking.IsSet()) dst.SetDisableDucking(disableDucking);
}

/*--------------------------------------------------------------------------------*/
/** Use object parameters to set parameters within audio object
 *
 * @param src audio object parameters object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::UpdateAudioObject(const AudioObjectParameters& src)
{
  // update parameters within *this* object with those from src *if* they are set
  if (src.IsObjectImportanceSet()) importance     = src.GetObjectImportance();
  if (src.IsInteractSet())         interact       = src.GetInteract();
  if (src.IsDialogueSet())         dialogue       = src.GetDialogue();
  if (src.IsDisableDuckingSet())   disableDucking = src.GetDisableDucking();
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // update value containing attributes
  objvalues[0].SetAttribute(startTime, full);
  objvalues[0].SetAttribute(duration, full);
  objvalues[0].SetAttribute(dialogue, full);
  objvalues[0].SetAttribute(importance, full);
  objvalues[0].SetAttribute(interact, full);
  objvalues[0].SetAttribute(disableDucking, full);

  // output list of interaction structures
  for (i = 0; i < (uint_t)audioObjectInteractionList.size(); i++)
  {
    XMLValue value;
    audioObjectInteractionList[i].GetValues(value, full);
    objvalues.AddValue(value);
  }

  // output list of references for complementary objects
  for (i = 0; i < (uint_t)complementaryobjectrefs.size(); i++)
  {
    XMLValue value;
    value.SetValue(ComplementaryObjectReference, complementaryobjectrefs[i]->GetID());
    objvalues.AddValue(value);
  }

  // references only
  object.genref  = true;

  for (i = 0; i < (uint_t)objectrefs.size(); i++)
  {
    object.obj = objectrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < (uint_t)packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < (uint_t)trackrefs.size(); i++)
  {
    object.obj = trackrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < objectrefs.size(); i++) GenerateReference(str, objectrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
  for (i = 0; i < trackrefs.size(); i++) GenerateReference(str, trackrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Get audioChannelFormat for a particular track
 */
/*--------------------------------------------------------------------------------*/
ADMAudioChannelFormat *ADMAudioObject::GetChannelFormat(uint_t track) const
{
  std::map<uint_t,const ADMAudioTrack *>::const_iterator it;
  ADMAudioChannelFormat *channelFormat = NULL;
  uint_t i;

  for (i = 0; i < trackrefs.size(); i++)
  {
    BBCDEBUG2(("ADMAudioObject<%s>: %s ('%s') trackrefs %u/%u ADMAudioTrack<%s>: tracknum %u/%u", StringFrom(this).c_str(), GetName().c_str(), GetID().c_str(), i, (uint_t)trackrefs.size(), StringFrom(trackrefs[i]).c_str(), trackrefs[i]->GetTrackNum(), track));

    if (trackrefs[i]->GetTrackNum() == track)
    {
      const ADMAudioTrack&                      audiotrack      = *trackrefs[i];
      const std::vector<ADMAudioTrackFormat *>& trackformatrefs = audiotrack.GetTrackFormatRefs();

      BBCDEBUG2(("Found %s<%s>: '%s' ('%s')", trackrefs[i]->GetType().c_str(), StringFrom(trackrefs[i]).c_str(), trackrefs[i]->GetName().c_str(), trackrefs[i]->GetID().c_str()));

      if (trackformatrefs.size() == 1)
      {
        const std::vector<ADMAudioStreamFormat *>& streamformatrefs = trackformatrefs[0]->GetStreamFormatRefs();

        BBCDEBUG2(("Found %s<%s>: '%s' ('%s')", trackformatrefs[0]->GetType().c_str(), StringFrom(trackformatrefs[0]).c_str(), trackformatrefs[0]->GetName().c_str(), trackformatrefs[0]->GetID().c_str()));

        if (streamformatrefs.size() == 1)
        {
          const std::vector<ADMAudioChannelFormat *>& channelformatrefs = streamformatrefs[0]->GetChannelFormatRefs();

          BBCDEBUG2(("Found %s<%s>: '%s' ('%s')", streamformatrefs[0]->GetType().c_str(), StringFrom(streamformatrefs[0]).c_str(), streamformatrefs[0]->GetName().c_str(), streamformatrefs[0]->GetID().c_str()));

          if (channelformatrefs.size() == 1)
          {
            channelFormat = channelformatrefs[0];
            BBCDEBUG2(("Found %s<%s>: '%s' ('%s')", channelFormat->GetType().c_str(), StringFrom(channelFormat).c_str(), channelFormat->GetName().c_str(), channelFormat->GetID().c_str()));
          }
          else LogError("Incorrect channelformatrefs in '%s' (%u)!", streamformatrefs[0]->ToString().c_str(), (uint_t)channelformatrefs.size());
        }
        else LogError("Incorrect streamformatrefs in '%s' (%u)!", trackformatrefs[0]->ToString().c_str(), (uint_t)streamformatrefs.size());
      }
      else LogError("Incorrect trackformatrefs in '%s' (%u)!", audiotrack.ToString().c_str(), (uint_t)trackformatrefs.size());
      break;
    }
  }

  return channelFormat;
}

/*--------------------------------------------------------------------------------*/
/** Get list of audioBlockFormats for a particular track
 */
/*--------------------------------------------------------------------------------*/
const std::vector<ADMAudioBlockFormat *> *ADMAudioObject::GetBlockFormatList(uint_t track) const
{
  const ADMAudioChannelFormat              *channelformat;
  const std::vector<ADMAudioBlockFormat *> *blockformats = NULL;

  if ((channelformat = GetChannelFormat(track)) != NULL)
  {
    blockformats = &channelformat->GetBlockFormatRefs();
  }

  return blockformats;
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Convert parameters to a JSON object
 *
 * ADM audio objects contain extra information for the JSON representation
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::ToJSON(JSONValue& obj) const
{
  // use ToJSON() functions to avoid issues with older versions of the JSON library with 64-bit ints
  json::ToJSON(GetID(),           obj["id"]);
  json::ToJSON(GetName(),         obj["name"]);
  json::ToJSON(GetStartTime(),    obj["starttime"]);
  json::ToJSON(GetDuration(),     obj["duration"]);
  json::ToJSON(GetStartChannel(), obj["startchannel"]);
  json::ToJSON(GetChannelCount(), obj["channelcount"]);
  
  if (GetDialogue()       != AudioObjectParameters::GetDialogueDefault())         json::ToJSON(GetDialogue(),       obj["dialogue"]);
  if (GetImportance()     != AudioObjectParameters::GetObjectImportanceDefault()) json::ToJSON(GetImportance(),     obj["importance"]);
  if (GetInteract()       != AudioObjectParameters::GetInteractDefault())         json::ToJSON(GetInteract(),       obj["interact"]);
  if (GetDisableDucking() != AudioObjectParameters::GetDisableDuckingDefault())   json::ToJSON(GetDisableDucking(), obj["disableducking"]);

  if (objectrefs.size())
  {
    JSONValue array;
    uint_t i;
    
    for (i = 0; i < objectrefs.size(); i++)
    {
      const ADMAudioObject& object = *objectrefs[i];

      array.append(object.GetID());
    }

    obj["objectrefs"] = array;
  }
}

/*--------------------------------------------------------------------------------*/
/** Set parameters from a JSON object
 *
 * ADM audio objects contain extra information for the JSON representation
 *
 * @note NOT IMPLEMENTED
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioObject::FromJSON(const JSONValue& obj)
{
  bool success = false;

  UNUSED_PARAMETER(obj);
  
  return success;
}
#endif

/*--------------------------------------------------------------------------------*/
/** Copy references from another object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::CopyReferences(const ADMObject *_obj)
{
  ADMObject::CopyReferences(_obj);

  const ADMAudioObject *obj = dynamic_cast<const ADMAudioObject *>(_obj);
  if (obj)
  {
    CopyReferencesEx<>(objectrefs, obj->objectrefs);
    CopyReferencesEx<>(packformatrefs, obj->packformatrefs);
    CopyReferencesEx<>(trackrefs, obj->trackrefs);
  }
  else LogError("Cannot copy references from '%s', type is '%s' not '%s'", _obj->ToString().c_str(), _obj->GetType().c_str(), GetType().c_str());
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioTrack::Type      = "audioTrackUID";
const std::string ADMAudioTrack::Reference = Type + "Ref";
const std::string ADMAudioTrack::IDPrefix  = "ATU_";

ADMAudioTrack::~ADMAudioTrack()
{
  RemoveReferences(this, trackformatrefs);
  RemoveReferences(this, packformatrefs);
  RemoveReferences(this);
  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::SetValues()
{
  ADMObject::SetValues();

  if (values.size() && values[0].name.empty())
  {
    if (values[0].GetAttributeAndErase(trackNum))
    {
      // trackNum is 1- based in XML, 0- based in object
      if (trackNum.IsSet()) trackNum = limited::subz(trackNum.Get(), 1u);
    }
    
    values[0].GetAttributeAndErase(sampleRate);
    values[0].GetAttributeAndErase(bitDepth);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // update value containing attributes
  // trackNum is NOT output to XML (where full == false)
  if (full) objvalues[0].SetAttribute("trackNum", trackNum.Get() + 1);
  objvalues[0].SetAttribute(sampleRate, true);
  objvalues[0].SetAttribute(bitDepth, true);

  // output references to objects
  object.genref = true;

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    object.obj = trackformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < trackformatrefs.size(); i++) GenerateReference(str, trackformatrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Copy references from another object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::CopyReferences(const ADMObject *_obj)
{
  ADMObject::CopyReferences(_obj);

  const ADMAudioTrack *obj = dynamic_cast<const ADMAudioTrack *>(_obj);
  if (obj)
  {
    CopyReferencesEx<>(trackformatrefs, obj->trackformatrefs);
    CopyReferencesEx<>(packformatrefs, obj->packformatrefs);
  }
  else LogError("Cannot copy references from '%s', type is '%s' not '%s'", _obj->ToString().c_str(), _obj->GetType().c_str(), GetType().c_str());
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioPackFormat::Type      = "audioPackFormat";
const std::string ADMAudioPackFormat::Reference = Type + "IDRef";
const std::string ADMAudioPackFormat::IDPrefix  = "AP_";

ADMAudioPackFormat::~ADMAudioPackFormat()
{
  RemoveReferences(this, channelformatrefs);
  RemoveReferences(this, packformatrefs);
  RemoveReferences(this);
  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::SetValues()
{
  ADMObject::SetValues();

  values[0].GetAttributeAndErase(absoluteDistance);
  values[0].GetAttributeAndErase(importance);
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::UpdateID()
{
  if (!standarddef)
  {
    // call SetID() with new ID
    std::string _id;

    Printf(_id, "%04x%%04x", typeLabel.Get());

    // custom pack formats start indexing at 0x1000
    SetID(GetIDPrefix() + _id, 0x1000);
  }
  else
  {
    LogError("Cannot change ID of standard definitions object '%s'", ToString().c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue         value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  objvalues[0].SetAttribute(absoluteDistance);
  objvalues[0].SetAttribute(importance);

  // generate references only
  object.genref  = true;

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    object.obj = channelformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioPackFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioPackFormat::Add(ADMAudioPackFormat *obj)
{
  if (std::find(packformatrefs.begin(), packformatrefs.end(), obj) == packformatrefs.end())
  {
    SetTypeInfoInObject(obj);
    packformatrefs.push_back(obj);
    obj->AddBackReference(this);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < channelformatrefs.size(); i++) GenerateReference(str, channelformatrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Copy references from another object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::CopyReferences(const ADMObject *_obj)
{
  ADMObject::CopyReferences(_obj);

  const ADMAudioPackFormat *obj = dynamic_cast<const ADMAudioPackFormat *>(_obj);
  if (obj)
  {
    CopyReferencesEx<>(channelformatrefs, obj->channelformatrefs);
    CopyReferencesEx<>(packformatrefs, obj->packformatrefs);
  }
  else LogError("Cannot copy references from '%s', type is '%s' not '%s'", _obj->ToString().c_str(), _obj->GetType().c_str(), GetType().c_str());
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioStreamFormat::Type      = "audioStreamFormat";
const std::string ADMAudioStreamFormat::Reference = Type + "IDRef";
const std::string ADMAudioStreamFormat::IDPrefix  = "AS_";

ADMAudioStreamFormat::~ADMAudioStreamFormat()
{
  RemoveReferences(this, channelformatrefs);
  RemoveReferences(this, trackformatrefs);
  RemoveReferences(this, packformatrefs);
  RemoveReferences(this);
  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::SetValues()
{
  ADMObject::SetValues();

  if (values.size() && values[0].name.empty())
  {
    values[0].GetAttributeAndErase(formatLabel);
    values[0].GetAttributeAndErase(formatDefinition);
  }
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::UpdateID()
{
  if (!standarddef)
  {
    // call SetID() with new ID
    std::string _id;
    uint_t i;

    Printf(_id, "%04x%%04x", typeLabel.Get());

    // custom stream formats start indexing at 0x1000
    SetID(GetIDPrefix() + _id, 0x1000);

    // set referenced trackformats' IDs

    // form ID using streamformat's ID with track number suffixed
    _id = "";
    Printf(_id, "%s_%%02x", GetID().substr(GetIDPrefix().length()).c_str());
    for (i = 0; i < trackformatrefs.size(); i++)
    {
      trackformatrefs[i]->SetID(trackformatrefs[i]->GetIDPrefix() + _id);
    }
  }
  else
  {
    LogError("Cannot change ID of standard definitions object '%s'", ToString().c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioChannelFormat *obj)
{
  bool success = false;
  
  if (packformatrefs.size() == 0)
  {
    if (channelformatrefs.size() == 0)
    {
      SetTypeInfoInObject(obj);
      channelformatrefs.push_back(obj);
      obj->AddBackReference(this);
      success = true;
    }
    else
    {
      // only a single reference allowed -> overwrite existing
      channelformatrefs[0]->RemoveBackReference(this);  // remove reference back from current value before overwriting
      channelformatrefs[0] = obj;
      obj->AddBackReference(this);
      success = true;
    }
  }
  else LogError("Cannot add audioChannelFormat reference '%s' to audioStreamFormat '%s' when it references an audioPackFormat", obj->ToString().c_str(), ToString().c_str());

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioTrackFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioTrackFormat *obj)
{
  if (std::find(trackformatrefs.begin(), trackformatrefs.end(), obj) == trackformatrefs.end())
  {
    SetTypeInfoInObject(obj);
    trackformatrefs.push_back(obj);
    obj->AddBackReference(this);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioPackFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioPackFormat *obj)
{
  bool success = false;
  
  if (packformatrefs.size() == 0)
  {
    if (packformatrefs.size() == 0)
    {
      SetTypeInfoInObject(obj);
      packformatrefs.push_back(obj);
      obj->AddBackReference(this);
      success = true;
    }
    else
    {
      // only a single reference allowed -> overwrite existing
      packformatrefs[0]->RemoveBackReference(this);  // remove reference back from current value before overwriting
      packformatrefs[0] = obj;
      obj->AddBackReference(this);
      success = true;
    }
  }
  else LogError("Cannot add audioPackFormat reference '%s' to audioStreamFormat '%s' when it references an audioChannelFormat", obj->ToString().c_str(), ToString().c_str());

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // update value containing attributes
  objvalues[0].SetAttribute(formatLabel, full);
  objvalues[0].SetAttribute(formatDefinition, full);

  // generate references only
  object.genref  = true;

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    object.obj = channelformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    object.obj = trackformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < channelformatrefs.size(); i++) GenerateReference(str, channelformatrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
  for (i = 0; i < trackformatrefs.size(); i++) GenerateReference(str, trackformatrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Copy references from another object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::CopyReferences(const ADMObject *_obj)
{
  ADMObject::CopyReferences(_obj);

  const ADMAudioStreamFormat *obj = dynamic_cast<const ADMAudioStreamFormat *>(_obj);
  if (obj)
  {
    CopyReferencesEx<>(channelformatrefs, obj->channelformatrefs);
    CopyReferencesEx<>(packformatrefs, obj->packformatrefs);
    CopyReferencesEx<>(trackformatrefs, obj->trackformatrefs);
  }
  else LogError("Cannot copy references from '%s', type is '%s' not '%s'", _obj->ToString().c_str(), _obj->GetType().c_str(), GetType().c_str());
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioTrackFormat::Type      = "audioTrackFormat";
const std::string ADMAudioTrackFormat::Reference = Type + "IDRef";
const std::string ADMAudioTrackFormat::IDPrefix  = "AT_";

ADMAudioTrackFormat::~ADMAudioTrackFormat()
{
  RemoveReferences(this, streamformatrefs);
  RemoveReferences(this);
  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::SetValues()
{
  ADMObject::SetValues();

  if (values.size() && values[0].name.empty())
  {
    values[0].GetAttributeAndErase(formatLabel);
    values[0].GetAttributeAndErase(formatDefinition);
  }
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::UpdateID()
{
  if (!standarddef)
  {
    // ONLY update this trackformat's ID if there are no streamformat references
    // (otherwise the streamformat will sort this trackformat out)

    if (!streamformatrefs.size())
    {
      // call SetID() with new ID
      std::string _id;

      Printf(_id, "%04x1000_%%02x", typeLabel.Get());

      // custom track formats start indexing at 0x1000
      SetID(GetIDPrefix() + _id);
    }
  }
  else
  {
    LogError("Cannot change ID of standard definitions object '%s'", ToString().c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // update value containing attributes
  objvalues[0].SetAttribute(formatLabel, full);
  objvalues[0].SetAttribute(formatDefinition, full);

  // generate references only
  object.genref  = true;

  for (i = 0; i < streamformatrefs.size(); i++)
  {
    object.obj = streamformatrefs[i];
    objects.push_back(object);
  }
}

bool ADMAudioTrackFormat::Add(ADMAudioStreamFormat *obj)
{
  if (streamformatrefs.size() == 0)
  {
    SetTypeInfoInObject(obj);
    streamformatrefs.push_back(obj);
    obj->AddBackReference(this);
    return true;
  }

  // only a single reference allowed -> overwrite existing
  streamformatrefs[0]->RemoveBackReference(this);  // remove reference back from current value before overwriting
  streamformatrefs[0] = obj;
  obj->AddBackReference(this);
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < streamformatrefs.size(); i++) GenerateReference(str, streamformatrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Copy references from another object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::CopyReferences(const ADMObject *_obj)
{
  ADMObject::CopyReferences(_obj);

  const ADMAudioTrackFormat *obj = dynamic_cast<const ADMAudioTrackFormat *>(_obj);
  if (obj)
  {
    CopyReferencesEx<>(streamformatrefs, obj->streamformatrefs);
  }
  else LogError("Cannot copy references from '%s', type is '%s' not '%s'", _obj->ToString().c_str(), _obj->GetType().c_str(), GetType().c_str());
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioChannelFormat::Type      = "audioChannelFormat";
const std::string ADMAudioChannelFormat::Reference = Type + "IDRef";
const std::string ADMAudioChannelFormat::IDPrefix  = "AC_";

ADMAudioChannelFormat::ADMAudioChannelFormat(ADMData& _owner, const ADMAudioChannelFormat *obj) : ADMObject(_owner, obj),
                                                                                                  frequencylist(obj->frequencylist)
{
  Register();

  // copy all blockformats from obj
  const std::vector<ADMAudioBlockFormat *>& oldblockformatrefs = obj->GetBlockFormatRefs();
  uint_t i;
  for (i = 0; i < oldblockformatrefs.size(); i++)
  {
    blockformatrefs.push_back(new ADMAudioBlockFormat(oldblockformatrefs[i]));
  }
}

ADMAudioChannelFormat::~ADMAudioChannelFormat()
{
  RemoveReferences(this);

  // remove this object from any cursors
  uint_t i;
  for (i = 0; i < cursorrefs.size(); i++) cursorrefs[i]->Remove(this);

  // delete all block formats
  for (i = 0; i < blockformatrefs.size(); i++) delete blockformatrefs[i];

  owner.DeRegister(this);
}

/*--------------------------------------------------------------------------------*/
/** Remove reference to an AudioBlockFormat object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::Remove(ADMAudioBlockFormat *obj)
{
  std::vector<ADMAudioBlockFormat *>::iterator it;

  if ((it = std::find(blockformatrefs.begin(), blockformatrefs.end(), obj)) != blockformatrefs.end())
  {
    // if block format exists, remove it from the list
    blockformatrefs.erase(it);
    // delete it
    delete obj;

    // re-seek on each cursor to avoid pointing to invalid blockformat
    uint_t i;
    for (i = 0; i < cursorrefs.size(); i++) cursorrefs[i]->Reseek();
  }
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::SetValues()
{
  XMLValue *value;

  ADMObject::SetValues();

  while ((value = values.GetValueWritable(ADMFrequency::GetElementName())) != NULL)
  {
    ADMFrequency freq;

    freq.SetValues(*value);
    frequencylist.push_back(freq);

    values.EraseValue(value);
  }
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::UpdateID()
{
  if (!standarddef)
  {
    // call SetID() with new ID
    std::string _id;

    Printf(_id, "%04x%%04x", typeLabel.Get());

    // custom channel formats start indexing at 0x1000
    SetID(GetIDPrefix() + _id, 0x1000);
  }
  else
  {
    LogError("Cannot change ID of standard definitions object '%s'", ToString().c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Sort block formats in time order
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::SortBlockFormats()
{
  std::sort(blockformatrefs.begin(), blockformatrefs.end(), ADMAudioBlockFormat::Compare);
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioBlockFormat object and ensures blocks are sorted by time
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioChannelFormat::Add(ADMAudioBlockFormat *obj)
{
  uint64_t t = obj->GetStartTime();
  uint_t   n = (uint_t)blockformatrefs.size();

  // insert obj into list ordered by time

  // allocate memory in large chunks to reduce re-allocation time
  if (n == blockformatrefs.capacity()) blockformatrefs.reserve(blockformatrefs.capacity() + 1024);

  // if the list is empty just append
  if (!n)
  {
    BBCDEBUG3(("blockformat list is empty: new item appended"));
    blockformatrefs.push_back(obj);
  }
  // the most likely place for the new item is on the end so check this first
  else if (t >= blockformatrefs[n - 1]->GetStartTime())
  {
    // ensure the new object is not already in the list
    if (obj != blockformatrefs[n - 1])
    {
      // new object just needs to be appended to list
      BBCDEBUG3(("New item is beyond last item (%llu >= %llu): new item appended", t, blockformatrefs[n - 1]->GetStartTime()));
      blockformatrefs.push_back(obj);
    }
  }
  // if the list has only one item or the new object is before the first item, new item must be inserted at the start
  else if ((n == 1) || (t <= blockformatrefs[0]->GetStartTime()))
  {
    // ensure the new object is not already in the list
    if (obj != blockformatrefs[0])
    {
      // new object just needs inserted at the start of the list
      BBCDEBUG3(("New item is before first item (%llu <= %llu): new item inserted at start", t, blockformatrefs[0]->GetStartTime()));
      blockformatrefs.insert(blockformatrefs.begin(), obj);
    }
  }
  // object should be placed somewhere in the list but not at the end (checked above)
  // list has at least two entries
  else
  {
    // start at the middle of the list and use a binary search
    uint_t inc = (n + 1) >> 1;      // round up division
    uint_t pos = std::min(inc, n - 2);                                 // start in the middle but ensure the last position is never checked (no point)
    uint_t count = 0;

    // break out when the new object is between the current and next item
    while (!((t >= blockformatrefs[pos]->GetStartTime()) &&
             (t <  blockformatrefs[pos + 1]->GetStartTime())))
    {
      // half increment (rounding up to ensure it is always non-zero)
      inc = (inc + 1) >> 1;

      // if the new item is before the current one, move back by the increment
      if (t < blockformatrefs[pos]->GetStartTime())
      {
        if (pos >= inc) pos -= inc;
        else            pos  = 0;
      }
      // else move forward by the increment, limiting to the end but one item
      else pos = std::min(pos + inc, n - 2);

      count++;
    }

    // check that the item isn't already in the list
    if (obj != blockformatrefs[pos])
    {
      (void)count;

      BBCDEBUG3(("New item is between indexes %u and %u (%llu <= %llu < %llu) (%u iterations): new item inserted between them", pos, pos + 1, blockformatrefs[pos]->GetStartTime(), t, blockformatrefs[pos + 1]->GetStartTime(), count));

      // the new item is between the pos'th and pos+1'th item
      // but insert works by inserting *before* the given position
      // therefore the position needs to be incremented by one
      pos++;

      blockformatrefs.insert(blockformatrefs.begin() + pos, obj);
    }
  }

  return true;
}

/*--------------------------------------------------------------------------------*/
/** Add track cursor reference
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioChannelFormat::Add(ADMTrackCursor *cursor)
{
  if (std::find(cursorrefs.begin(), cursorrefs.end(), cursor) == cursorrefs.end())
  {
    cursorrefs.push_back(cursor);
  }

  return true;
}

/*--------------------------------------------------------------------------------*/
/** Add track cursor reference
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::Remove(ADMTrackCursor *cursor)
{
  std::vector<ADMTrackCursor *>::iterator it;

  if ((it = std::find(cursorrefs.begin(), cursorrefs.end(), cursor)) != cursorrefs.end())
  {
    BBCDEBUG2(("Removing link between %s and ADMTrackCursor<%s>", ToString().c_str(), StringFrom(*it).c_str()));
    cursorrefs.erase(it);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  uint_t i;
  for (i = 0; i < (uint_t)frequencylist.size(); i++)
  {
    const ADMFrequency& frequency = frequencylist[i];
    XMLValue value;

    if (frequency.GetValues(value, full))
    {
      objvalues.AddValue(value);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::GenerateReferenceList(std::string& str) const
{
  UNUSED_PARAMETER(str);
}

/*--------------------------------------------------------------------------------*/
/** Provide a way of accessing contained items without knowing what they are
 * (used for block formats)
 *
 * @param n object index
 * @param object structure to be filled
 *
 * @return true if object valid
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioChannelFormat::GetContainedObject(uint_t n, CONTAINEDOBJECT& object) const
{
  bool success = false;

  if (n < blockformatrefs.size())
  {
    const ADMAudioBlockFormat *block = blockformatrefs[n];
    std::string id;

    object.type = block->GetType();
    object.values.clear();

    // allocate blank value for attributes
    object.values.AddValue(XMLValue());

    Printf(id, "%s%s_%08x", block->GetIDPrefix().c_str(), GetID().substr(GetIDPrefix().length()).c_str(), n + 1);
    object.values[0].SetAttribute(block->GetType() + "ID", id);

    block->GetValues(object.values);

    success = true;
  }

  return success;
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioBlockFormat::Type      = "audioBlockFormat";
const std::string ADMAudioBlockFormat::Reference = Type + "IDRef";
const std::string ADMAudioBlockFormat::IDPrefix  = "AB_";

/*--------------------------------------------------------------------------------*/
/** ADM AudioBlockFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioBlockFormat::ADMAudioBlockFormat() :
  rtime(0),
  duration(0),
  rtimeSet(false),
  durationSet(false)
{
}

ADMAudioBlockFormat::ADMAudioBlockFormat(const ADMAudioBlockFormat *obj) :
  objparameters(obj->objparameters),
  rtime(obj->rtime),
  duration(obj->duration),
  rtimeSet(obj->rtimeSet),
  durationSet(obj->durationSet)
{
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::SetValues(XMLValues& values, ADMObject *owner)
{
  XMLValues::iterator it;
  ParameterSet othervalues;
  Position position, minposition, maxposition;
  bool     positionset = false, minpositionset = false, maxpositionset = false;    // used to detect which position(s) have been set and detect changes in co-ordinate system

  if (values.size() && values[0].name.empty())
  {
    const std::string *attr;
    uint64_t _time;

    if ((attr = values[0].GetAttribute("rtime")) != NULL)
    {
      if (CalcTime(_time, *attr)) SetStartTime(_time);
      values[0].EraseAttribute("rtime");
    }

    if ((attr = values[0].GetAttribute("duration")) != NULL)
    {
      if (CalcTime(_time, *attr)) SetDuration(_time);
      values[0].EraseAttribute("duration");
      objparameters.SetInterpolationTime(_time);
    }
  }
  else owner->LogError("No attributes for audioBlockFormat!");

  for (it = values.begin(); it != values.end();)
  {
    const XMLValue& value = *it;

    // ignore unnamed entries
    if (value.name.empty())
    {
      ++it;
    }
    else if (value.name == "cartesian")
    {
      bool val;

      if (Evaluate(value.value, val)) objparameters.SetCartesian(val);

      it = values.erase(it);
    }
    else if (value.name == "position")
    {
      double val;

      if (Evaluate(value.value, val))
      {
        const std::string *coord, *bound, *scrlock;
        Position *pos = &position;      // default is 'normal' position
        bool     *set = &positionset;

        // determine *which* position is being set
        if ((bound = value.GetAttribute("bound")) != NULL)
        {
          if (*bound == "min")
          {
            pos = &minposition;
            set = &minpositionset;
          }
          else if (*bound == "max")
          {
            pos = &maxposition;
            set = &maxpositionset;
          }
          else owner->LogError("Illegal bound value '%s' for position", bound->c_str());
        }

        scrlock = value.GetAttribute("screenEdgeLock");

        if ((coord = value.GetAttribute("coordinate")) != NULL)
        {
          BBCDEBUG4(("Position type %s value %0.6lf", coord->c_str(), val));

          // prevent changing of co-ordinate system for a particular position instance
          if      ((!*set ||  pos->polar) && (*coord == "azimuth"))                {*set = true; pos->polar = true;  pos->pos.az = val; if (scrlock) objparameters.SetScreenEdgeLock(*coord, *scrlock);}
          else if ((!*set ||  pos->polar) && (*coord == "elevation"))              {*set = true; pos->polar = true;  pos->pos.el = val; if (scrlock) objparameters.SetScreenEdgeLock(*coord, *scrlock);}
          else if ((!*set ||  pos->polar) && (*coord == "distance"))               {*set = true; pos->polar = true;  pos->pos.d  = val;}
          else if ((!*set || !pos->polar) && ((*coord == "x") || (*coord == "X"))) {*set = true; pos->polar = false; pos->pos.x  = val;}
          else if ((!*set || !pos->polar) && ((*coord == "y") || (*coord == "Y"))) {*set = true; pos->polar = false; pos->pos.y  = val;}
          else if ((!*set || !pos->polar) && ((*coord == "z") || (*coord == "Z"))) {*set = true; pos->polar = false; pos->pos.z  = val;}
          else owner->LogError("Illegal co-ordinate '%s' specified (bound '%s', co-ordinate system '%s')", coord->c_str(), bound ? bound->c_str() : "", pos->polar ? "spherical" : "cartesian");
        }
      }
      else owner->LogError("Failed to evaluate '%s' as floating point number for position", value.value.c_str());

      it = values.erase(it);
    }
    else if (value.name == "width")
    {
      float val;

      if (Evaluate(value.value, val)) objparameters.SetWidth(val);

      it = values.erase(it);
    }
    else if (value.name == "depth")
    {
      float val;

      if (Evaluate(value.value, val)) objparameters.SetDepth(val);

      it = values.erase(it);
    }
    else if (value.name == "height")
    {
      float val;

      if (Evaluate(value.value, val)) objparameters.SetHeight(val);

      it = values.erase(it);
    }
    else if (value.name == "gain")
    {
      float val;

      if (Evaluate(value.value, val)) objparameters.SetGain(val);

      it = values.erase(it);
    }
    else if (value.name == "diffuse")
    {
      float val;

      if (Evaluate(value.value, val)) objparameters.SetDiffuseness(val);

      it = values.erase(it);
    }
    else if (value.name == "jumpPosition")
    {
      bool bval;

      if (Evaluate(value.value, bval))
      {
        XMLValue::ATTRS::const_iterator it2;
        double fval = 0.0;

        // read interpolationLength, if it exists
        if (bval)
        {
          // if jumpPosition is set, read interpolationLength and use it for the interpolationtime
          if ((it2 = value.attrs.find("interpolationLength")) != value.attrs.end()) Evaluate(it2->second, fval);
        }

        objparameters.SetJumpPosition(bval, fval);
      }

      it = values.erase(it);
    }
    else if (value.name == "channelLock")
    {
      bool val;

      if (Evaluate(value.value, val))
      {
        XMLValue::ATTRS::const_iterator it2;
        float fval;

        objparameters.SetChannelLock(val);

        // read maxDistance, if it exists
        if (((it2 = value.attrs.find("maxDistance")) != value.attrs.end()) && Evaluate(it2->second, fval)) objparameters.SetChannelLockMaxDistance(fval);
      }

      it = values.erase(it);
    }
    else if (value.name == "objectDivergence")
    {
      float fval;

      if (Evaluate(value.value, fval))
      {
        XMLValue::ATTRS::const_iterator it2;

        objparameters.SetDivergenceBalance(fval);

        // read maxDistance, if it exists
        if (((it2 = value.attrs.find("azimuthRange")) != value.attrs.end()) && Evaluate(it2->second, fval)) objparameters.SetDivergenceAzimuth(fval);
      }

      it = values.erase(it);
    }
    else if (value.name == "screenRef")
    {
      bool val;

      if (Evaluate(value.value, val)) objparameters.SetOnScreen(val);

      it = values.erase(it);
    }
    else if (value.name == "importance")
    {
      uint_t val;

      if (Evaluate(value.value, val)) objparameters.SetChannelImportance(val);

      it = values.erase(it);
    }
    else if (value.name == "zoneExclusion")
    {
      const XMLValues *_subvalues = value.GetSubValues();

      // does the zoneExclusion value have subvalues?
      if (_subvalues)
      {
        const XMLValues& subvalues = *_subvalues;
        uint_t i;

        // iterate through each subvalue of zoneExclusion
        for (i = 0; i < subvalues.size(); i++)
        {
          const XMLValue& value2 = subvalues[i];

          // for zone subvalues, extract the bounds of the excluded zone and add it to the list in the object parameters
          if (value2.name =="zone")
          {
            XMLValue::ATTRS::const_iterator it2;
            float minx, miny, minz, maxx, maxy, maxz;

            // extract bounds of zone
            if (((it2 = value2.attrs.find("minX")) != value.attrs.end()) && Evaluate(it2->second, minx) &&
                ((it2 = value2.attrs.find("minY")) != value.attrs.end()) && Evaluate(it2->second, miny) &&
                ((it2 = value2.attrs.find("minZ")) != value.attrs.end()) && Evaluate(it2->second, minz) &&
                ((it2 = value2.attrs.find("maxX")) != value.attrs.end()) && Evaluate(it2->second, maxx) &&
                ((it2 = value2.attrs.find("maxY")) != value.attrs.end()) && Evaluate(it2->second, maxy) &&
                ((it2 = value2.attrs.find("maxZ")) != value.attrs.end()) && Evaluate(it2->second, maxz))
            {
              // add zone to list in object parameters
              objparameters.AddExcludedZone(value2.value, minx, miny, minz, maxx, maxy, maxz);
            }
            else owner->LogError("Sub-value %u of zoneExclusion invalid", i);
          }
          else BBCDEBUG1(("Unrecognized sub-value %u ('%s') of zoneExclusion", i, value2.name.c_str()));
        }
      }
      else BBCDEBUG1(("zoneExclusion value has no sub-values!"));

      it = values.erase(it);
    }
    else // any other parameters -> assume they are part of the supplement information
    {
      objparameters.SetOtherValue(value.name, value.value);

      it = values.erase(it);
    }
  }

  // set position(s) within the audio object parameters object
  if (positionset)    objparameters.SetPosition(position);
  if (minpositionset) objparameters.SetMinPosition(minposition);
  if (maxpositionset) objparameters.SetMaxPosition(maxposition);
}

/*--------------------------------------------------------------------------------*/
/** Add position information to list of XMLValues
 *
 * @param objvalues list of XMLValues to be appended to
 * @param position position to encode
 * @param bound bound attribute for position or NULL
 *
 * @note screenEdgeLock is encoded ONLY if bound == NULL
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::GetPositionValues(XMLValues& objvalues, const Position& position, const char *bound) const
{
  std::string str;

  if (position.polar)
  {
    {
      XMLValue value;
      value.SetValue("position", position.pos.az);
      value.SetAttribute("coordinate", "azimuth");
      if (bound) value.SetAttribute("bound", bound);
      if (!bound && objparameters.GetScreenEdgeLock("azimuth", str)) value.SetAttribute("screenEdgeLock", str);
      objvalues.AddValue(value);
    }

    {
      XMLValue value;
      value.SetValue("position", position.pos.el);
      value.SetAttribute("coordinate", "elevation");
      if (bound) value.SetAttribute("bound", bound);
      if (!bound && objparameters.GetScreenEdgeLock("elevation", str)) value.SetAttribute("screenEdgeLock", str);
      objvalues.AddValue(value);
    }

    {
      XMLValue value;
      value.SetValue("position", position.pos.d);
      value.SetAttribute("coordinate", "distance");
      if (bound) value.SetAttribute("bound", bound);
      objvalues.AddValue(value);
    }
  }
  else
  {
    {
      XMLValue value;
      value.SetValue("position", position.pos.x, "%0.10lf");
      value.SetAttribute("coordinate", "X");
      if (bound) value.SetAttribute("bound", bound);
      objvalues.AddValue(value);
    }

    {
      XMLValue value;
      value.SetValue("position", position.pos.y, "%0.10lf");
      value.SetAttribute("coordinate", "Y");
      if (bound) value.SetAttribute("bound", bound);
      objvalues.AddValue(value);
    }

    {
      XMLValue value;
      value.SetValue("position", position.pos.z, "%0.10lf");
      value.SetAttribute("coordinate", "Z");
      if (bound) value.SetAttribute("bound", bound);
      objvalues.AddValue(value);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 *
 * @param objvalues list to be populated with XMLValue's holding object values
 * @param full true to generate complete list including values that do not appear in the XML
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::GetValues(XMLValues& objvalues, bool full) const
{
  std::string str;
  double      dval;
  float       fval;
  uint_t      uval;
  bool        bval;

  if (!objvalues.size()) objvalues.AddValue(XMLValue());

  // add values/attributes not held in 'values' to list
  if (full || RTimeSet())
  {
    objvalues[0].SetTimeAttribute("rtime", rtime);
  }
  if (full || DurationSet())
  {
    objvalues[0].SetTimeAttribute("duration", duration);
  }

  if (objparameters.IsCartesianSet()) {
    XMLValue value;
    value.SetValue("cartesian", objparameters.GetCartesian());
    objvalues.AddValue(value);
  }

  if (objparameters.IsPositionSet())    GetPositionValues(objvalues, objparameters.GetPosition());
  if (objparameters.IsMinPositionSet()) GetPositionValues(objvalues, objparameters.GetMinPosition(), "min");
  if (objparameters.IsMaxPositionSet()) GetPositionValues(objvalues, objparameters.GetMaxPosition(), "max");

  if (objparameters.GetGain(dval))
  {
    XMLValue value;
    value.SetValue("gain", dval, "%0.10lf");
    objvalues.AddValue(value);
  }

  if (objparameters.GetWidth(fval))
  {
    XMLValue value;
    value.SetValue("width", fval, "%0.10f");
    objvalues.AddValue(value);
  }

  if (objparameters.GetDepth(fval))
  {
    XMLValue value;
    value.SetValue("depth", fval, "%0.10f");
    objvalues.AddValue(value);
  }

  if (objparameters.GetHeight(fval))
  {
    XMLValue value;
    value.SetValue("height", fval, "%0.10f");
    objvalues.AddValue(value);
  }

  if (objparameters.GetDiffuseness(fval))
  {
    XMLValue value;
    value.SetValue("diffuse", fval);
    objvalues.AddValue(value);
  }

  if (objparameters.GetJumpPosition(bval, &dval))
  {
    XMLValue value;

    value.SetValue("jumpPosition", bval);

    // set interpolationLength
    if (bval) value.SetAttribute("interpolationLength", dval);

    objvalues.AddValue(value);
  }

  if (objparameters.GetDivergenceBalance(fval))
  {
    XMLValue value;

    value.SetValue("objectDivergence", fval);

    if (objparameters.GetDivergenceAzimuth(fval)) value.SetAttribute("azimuthRange", fval);

    objvalues.AddValue(value);
  }

  if (objparameters.GetChannelLock(bval))
  {
    XMLValue value;
    value.SetValue("channelLock", bval);

    if (objparameters.GetChannelLockMaxDistance(fval))
    {
      value.SetAttribute("maxDistance", fval);
    }

    objvalues.AddValue(value);
  }

  if (objparameters.GetChannelImportance(uval))
  {
    XMLValue value;
    value.SetValue("importance", uval);
    objvalues.AddValue(value);
  }

  // output all excluded zones
  const AudioObjectParameters::ExcludedZone *zone = objparameters.GetFirstExcludedZone();
  if (zone)
  {
    XMLValue value;

    value.name = "zoneExclusion";

    while (zone)
    {
      XMLValue subvalue;
      Position c1 = zone->GetMinCorner();
      Position c2 = zone->GetMaxCorner();

      subvalue.name  = "zone";
      subvalue.value = zone->GetName();

      subvalue.SetAttribute("minX", c1.pos.x);
      subvalue.SetAttribute("minY", c1.pos.y);
      subvalue.SetAttribute("minZ", c1.pos.z);
      subvalue.SetAttribute("maxX", c2.pos.x);
      subvalue.SetAttribute("maxY", c2.pos.y);
      subvalue.SetAttribute("maxZ", c2.pos.z);

      value.AddSubValue(subvalue);

      zone = zone->GetNext();
    }

    objvalues.AddValue(value);
  }

  // add all parameters from the supplement information
  ParameterSet::Iterator it;
  for (it = objparameters.GetOtherValuesBegin(); it != objparameters.GetOtherValuesEnd(); ++it)
  {
    // EXCLUDE screenedgelock parameters
    if (!AudioObjectParameters::IsScreenEdgeLockValue(it->first))
    {
      XMLValue value;
      value.SetValue(it->first, it->second);
      objvalues.AddValue(value);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::GenerateReferenceList(std::string& str) const
{
  UNUSED_PARAMETER(str);
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Convert parameters to a JSON object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::ToJSON(JSONValue& obj) const
{
  // use ToJSON() functions to avoid issues with older versions of the JSON library with 64-bit ints
  json::ToJSON(GetStartTime(), obj["startTime"]);
  json::ToJSON(GetDuration(),  obj["duration"]);
  objparameters.ToJSON(obj["parameters"]);
}

/*--------------------------------------------------------------------------------*/
/** Set parameters from a JSON object
 *
 * @note NOT IMPLEMENTED
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioBlockFormat::FromJSON(const JSONValue& obj)
{
  bool success = false;

  UNUSED_PARAMETER(obj);
  
  return success;
}
#endif

/*----------------------------------------------------------------------------------------------------*/

ADMTrackCursor::ADMTrackCursor(uint_t _channel) : AudioObjectCursor(),
                                                  channel(_channel),
                                                  objectindex(0),
                                                  blockindex(0),
                                                  currenttime(0),
                                                  blockformatstarted(false),
                                                  objparametersvalid(false)
{
}

ADMTrackCursor::ADMTrackCursor(const ADMTrackCursor& obj) : AudioObjectCursor(),
                                                            channel(obj.channel),
                                                            objectindex(0),
                                                            blockindex(0),
                                                            currenttime(0),
                                                            blockformatstarted(false),
                                                            objparametersvalid(false)
{
  uint_t i;

  for (i = 0; i < obj.objectlist.size(); i++) Add(obj.objectlist[i].audioobject, false);

  Sort();
}

ADMTrackCursor::~ADMTrackCursor()
{
}

/*--------------------------------------------------------------------------------*/
/** Add audio object to this object
 *
 * @return true if object added, false if object ignored
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const ADMAudioObject *object, bool sort)
{
  uint_t i;
  bool   added = false;

  // look for this object in the list
  for (i = 0; i < objectlist.size(); i++)
  {
    if (objectlist[i].audioobject == object) return false;
  }

  AUDIOOBJECT obj =
  {
    object,
    object->GetChannelFormat(channel),
  };
  if (obj.channelformat)
  {
    objectlist.push_back(obj);

    BBCDEBUG3(("Cursor<%s:%u>: Added object '%s', channel format '%s', %u blocks, start %s for %s",
               StringFrom(this).c_str(),
               channel,
               object->ToString().c_str(),
               obj.channelformat->ToString().c_str(),
               (uint_t)obj.channelformat->GetBlockFormatRefs().size(),
               obj.channelformat->GetBlockFormatRefs().size() ? GenerateTime(obj.channelformat->GetBlockFormatRefs()[0]->GetStartTime()).c_str() : "0",
               obj.channelformat->GetBlockFormatRefs().size() ? GenerateTime(obj.channelformat->GetBlockFormatRefs()[obj.channelformat->GetBlockFormatRefs().size() - 1]->GetEndTime()).c_str() : "0"));

    added = true;

    // add all referenced audio objects contained within supplied audio object
    // (do not sort objects by default, it will be done below)
    Add(object->GetObjectRefs(), false);
  }

  if (added && sort) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Add audio objects to this object
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const ADMAudioObject *objects[], uint_t n, bool sort)
{
  bool   added = false;
  uint_t i;

  for (i = 0; i < n; i++) added |= Add(objects[i], false);

  if (added && sort) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Add audio objects to this object
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const std::vector<const ADMAudioObject *>& objects, bool sort)
{
  bool   added = false;
  uint_t i;

  for (i = 0; i < objects.size(); i++) added |= Add(objects[i], false);

  if (added && sort) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Add audio objects to this object
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const std::vector<ADMAudioObject *>& objects, bool sort)
{
  bool   added = false;
  uint_t i;

  for (i = 0; i < objects.size(); i++) added |= Add(objects[i], false);

  if (added && sort) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Add audio objects to this object
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const std::vector<const ADMObject *>& objects, bool sort)
{
  bool   added = false;
  uint_t i;

  for (i = 0; i < objects.size(); i++)
  {
    const ADMAudioObject *obj;

    if ((obj = dynamic_cast<const ADMAudioObject *>(objects[i])) != NULL)
    {
      added |= Add(obj, false);
    }
  }

  if (added && sort) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Sort list of objects into time order
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::Sort()
{
  std::sort(objectlist.begin(), objectlist.end(), &Compare);

  Seek(currenttime);
}

/*--------------------------------------------------------------------------------*/
/** Remove audio object from list
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::Remove(const ADMAudioObject *obj)
{
  uint_t i;

  BBCDEBUG2(("Removing link between %s and ADMTrackCursor<%s>", obj->ToString().c_str(), StringFrom(this).c_str()));

  // find use of specified object and delete any entries
  for (i = 0; i < objectlist.size();)
  {
    if (objectlist[i].audioobject == obj)
    {
      objectlist.erase(objectlist.begin() + i);
    }
    else i++;
  }

  // re-seek to ensure pointers are still valid
  Seek(currenttime);
}

/*--------------------------------------------------------------------------------*/
/** Remove channelformat from list
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::Remove(ADMAudioChannelFormat *obj)
{
  uint_t i;

  BBCDEBUG2(("Removing link between %s and ADMTrackCursor<%s>", obj->ToString().c_str(), StringFrom(this).c_str()));

  // find use of specified object and delete any entries
  for (i = 0; i < objectlist.size();)
  {
    if (objectlist[i].channelformat == obj)
    {
      objectlist.erase(objectlist.begin() + i);
    }
    else i++;
  }

  // re-seek to ensure pointers are still valid
  Seek(currenttime);
}

/*--------------------------------------------------------------------------------*/
/** Return cursor start time in ns
 */
/*--------------------------------------------------------------------------------*/
uint64_t ADMTrackCursor::GetStartTime() const
{
  uint64_t t = 0;

  if (objectlist.size() > 0)
  {
    const AUDIOOBJECT&                        objectdata   = objectlist[0];
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectdata.channelformat->GetBlockFormatRefs();

    if (blockformats.size() > 0)
    {
      t = blockformats[0]->GetStartTime(objectdata.audioobject);

      BBCDEBUG3(("Cursor<%s:%u>: Object %u/%u start %s BlockFormat %u/%u start %s start %s",
                 StringFrom(this).c_str(), channel,
                 0, (uint_t)objectlist.size(),   StringFrom(objectdata.audioobject->GetStartTime()).c_str(),
                 0, (uint_t)blockformats.size(), StringFrom(blockformats[0]->GetStartTime()).c_str(),
                 StringFrom(t).c_str()));
    }
  }

  return t;
}

/*--------------------------------------------------------------------------------*/
/** Return cursor end time in ns
 */
/*--------------------------------------------------------------------------------*/
uint64_t ADMTrackCursor::GetEndTime() const
{
  uint64_t t = 0;

  if (objectlist.size() > 0)
  {
    const AUDIOOBJECT&                        objectdata   = objectlist[objectlist.size() - 1];
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectdata.channelformat->GetBlockFormatRefs();

    if (blockformats.size() > 0)
    {
      t = blockformats[blockformats.size() - 1]->GetEndTime(objectdata.audioobject);

      BBCDEBUG3(("Cursor<%s:%u>: Object %u/%u start %s BlockFormat %u/%u start %s duration %s end %s",
                 StringFrom(this).c_str(), channel,
                 (uint_t)(objectlist.size() - 1),   (uint_t)objectlist.size(),   StringFrom(objectdata.audioobject->GetStartTime()).c_str(),
                 (uint_t)(blockformats.size() - 1), (uint_t)blockformats.size(), StringFrom(blockformats[0]->GetStartTime()).c_str(), StringFrom(blockformats[0]->GetDuration()).c_str(),
                 StringFrom(t).c_str()));

    }
  }

  return t;
}

/*--------------------------------------------------------------------------------*/
/** Return audio object parameters at current time
 */
/*--------------------------------------------------------------------------------*/
const ADMAudioBlockFormat *ADMTrackCursor::GetBlockFormat() const
{
  const ADMAudioBlockFormat *blockformat = NULL;

  if (objectindex < objectlist.size())
  {
    const AUDIOOBJECT&                        objectdata   = objectlist[objectindex];
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectdata.channelformat->GetBlockFormatRefs();

    blockformat = blockformats[blockindex];
  }

  return blockformat;
}

/*--------------------------------------------------------------------------------*/
/** Get current audio object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObject *ADMTrackCursor::GetAudioObject() const
{
  ADMAudioObject *obj = NULL;

  if ((objectindex < objectlist.size()) && limited::inrange(currenttime, objectlist[objectindex].audioobject->GetStartTime(), objectlist[objectindex].audioobject->GetEndTime()))
  {
    obj = const_cast<ADMAudioObject *>(objectlist[objectindex].audioobject);
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Return audio object parameters at current time
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::GetObjectParameters(AudioObjectParameters& currentparameters) const
{
  // only set parameters if they are valid
  if (objparametersvalid)
  {
    currentparameters = objparameters;
    return true;
  }

  return false;
}

/*--------------------------------------------------------------------------------*/
/** Start a blockformat at t
 */
/*--------------------------------------------------------------------------------*/
ADMAudioBlockFormat *ADMTrackCursor::StartBlockFormat(uint64_t t)
{
  AUDIOOBJECT&        objectdata   = objectlist[objectindex];
  ADMAudioBlockFormat *blockformat;

  if ((blockformat = new ADMAudioBlockFormat) != NULL)
  {
    blockformat->SetStartTime(t, objectdata.audioobject);
    objectdata.channelformat->Add(blockformat);

    blockindex         = (uint_t)(objectdata.channelformat->GetBlockFormatRefs().size() - 1);
    blockformatstarted = true;

    BBCDEBUG3(("Cursor<%s:%u>: Created new blockformat %u at %0.3lfs for object '%s', channelformat '%s'", StringFrom(this).c_str(), channel, blockindex, (double)t * 1.0e-9, objectdata.audioobject->ToString().c_str(), objectdata.channelformat->ToString().c_str()));
  }

  return blockformat;
}

/*--------------------------------------------------------------------------------*/
/** End a blockformat at t that has previously been start
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::EndBlockFormat(uint64_t t)
{
  if (blockformatstarted)
  {
    AUDIOOBJECT&        objectdata   = objectlist[objectindex];
    ADMAudioBlockFormat *blockformat = objectdata.channelformat->GetBlockFormatRefs()[blockindex];

    blockformat->SetEndTime(t, objectdata.audioobject);

    BBCDEBUG3(("Cursor<%s:%u>: Completed blockformat %u at %0.3lfs (duration %0.3lfs) for object '%s', channelformat '%s'", StringFrom(this).c_str(), channel, blockindex, (double)t * 1.0e-9, (double)blockformat->GetDuration() * 1.0e-9, objectdata.audioobject->ToString().c_str(), objectdata.channelformat->ToString().c_str()));

    blockformatstarted = false;
  }
}

/*--------------------------------------------------------------------------------*/
/** Set audio object parameters for current time
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::SetObjectParameters(const AudioObjectParameters& newparameters)
{
  if ((objectindex < objectlist.size()) && (currenttime >= objectlist[objectindex].audioobject->GetStartTime()))
  {
    AUDIOOBJECT&                        objectdata   = objectlist[objectindex];
    std::vector<ADMAudioBlockFormat *>& blockformats = objectdata.channelformat->GetBlockFormatRefs();
    ADMAudioBlockFormat                 *blockformat;
    ADMAudioObject                      *audioobject;
    uint64_t                            relativetime = currenttime - objectdata.audioobject->GetStartTime();

    // update internal parameters
    objparameters      = newparameters;
    objparametersvalid = true;

    if ((blockindex < blockformats.size()) && (blockformats[blockindex]->GetStartTime() == relativetime))
    {
      // new position at same time as original -> just update the parameters
      blockformats[blockindex]->GetObjectParameters() = objparameters;
      BBCDEBUG2(("Updating channel %u to {'%s'}", channel, blockformats[blockindex]->GetObjectParameters().ToString().c_str()));
    }
    else
    {
      // new position requires new block format
      EndBlockFormat(currenttime);

      if ((blockformat = StartBlockFormat(currenttime)) != NULL)
      {
        blockformat->GetObjectParameters() = objparameters;
        BBCDEBUG2(("Updating channel %u to {'%s'}", channel, blockformats[blockindex]->GetObjectParameters().ToString().c_str()));
      }
    }

    // update parameters of ADMAudioObject from AudioObjectParameters object
    if (objectdata.audioobject && ((audioobject = const_cast<ADMAudioObject *>(objectdata.audioobject)) != NULL))
    {
      audioobject->UpdateAudioObject(objparameters);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** End position updates by marking the end of the last block
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::EndChanges()
{
  if ((objectindex < objectlist.size()) &&
      (currenttime >= objectlist[objectindex].audioobject->GetStartTime()) &&
      (objectlist[objectindex].channelformat->GetBlockFormatRefs().size() == 0))
  {
    // no blockformats for the current object, create one from start of object
    StartBlockFormat(objectlist[objectindex].audioobject->GetStartTime());
  }

  // close last blockformat off by setting end time
  EndBlockFormat(currenttime);
}

/*--------------------------------------------------------------------------------*/
/** Get position at specified time (ns)
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Seek(uint64_t t)
{
  uint_t oldobjectindex = objectindex;
  uint_t oldblockindex  = blockindex;

  // keep index in range
  if (objectindex >= (uint_t)objectlist.size()) objectindex = objectlist.size() ? (objectlist.size() - 1) : 0;

  if (objectindex < objectlist.size())
  {
    // find right object in list
    while ((objectindex > 0) && (t < objectlist[objectindex].audioobject->GetStartTime()))
    {
      // close last blockformat of this object off by setting end time
      EndBlockFormat(currenttime);

      // move back
      objectindex--;

      // if the blockformat list is not empty, set the bockindex to the last one
      const std::vector<ADMAudioBlockFormat *>& blockformats = objectlist[objectindex].channelformat->GetBlockFormatRefs();
      blockindex = blockformats.size() ? (uint_t)(blockformats.size() - 1) : 0;
    }
    while (((objectindex + 1) < objectlist.size()) && (t >= objectlist[objectindex + 1].audioobject->GetStartTime()))
    {
      // close last blockformat of this object off by setting end time at start of next object
      EndBlockFormat(objectlist[objectindex + 1].audioobject->GetStartTime());

      // move forward
      objectindex++;

      // set block index to first block
      blockindex = 0;
    }

    // move blockindex as needed
    const AUDIOOBJECT&                        objectdata   = objectlist[objectindex];
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectdata.channelformat->GetBlockFormatRefs();

    // keep block index in range
    if (blockindex >= (uint_t)blockformats.size()) blockindex = blockformats.size() ? (uint_t)(blockformats.size() - 1) : 0;

    if ((t >= objectdata.audioobject->GetStartTime()) && (blockindex < blockformats.size()))
    {
      // find right blockformat within object
      while ((blockindex       > 0)                   && (t <  blockformats[blockindex]->GetStartTime(objectdata.audioobject)))     blockindex--;
      while (((blockindex + 1) < blockformats.size()) && (t >= blockformats[blockindex + 1]->GetStartTime(objectdata.audioobject))) blockindex++;

      // ensure time is within chosen block (it may be outside if there isn't a valid block representing the time)
      if ((t >= blockformats[blockindex]->GetStartTime(objectdata.audioobject)) && (t < blockformats[blockindex]->GetEndTime(objectdata.audioobject)))
      {
        objparameters = blockformats[blockindex]->GetObjectParameters();
        objparametersvalid = true;
      }
      else objparametersvalid = false;
    }
    else objparametersvalid = false;

    if ((objectindex != oldobjectindex) || (blockindex != oldblockindex))
    {
      BBCDEBUG4(("Cursor<%s:%u>: Moved to object %u/%u, block %u/%u at %s (parameters '%s')", StringFrom(this).c_str(), channel, objectindex, (uint_t)objectlist.size(), blockindex, (uint_t)objectlist[objectindex].channelformat->GetBlockFormatRefs().size(), GenerateTime(t).c_str(), objectlist[objectindex].channelformat->GetBlockFormatRefs()[blockindex]->GetObjectParameters().ToString().c_str()));
    }
  }
  else objparametersvalid = false;

#if BBCDEBUG_LEVEL >= 2
  if (objectindex < objectlist.size()) BBCDEBUG("Cursor<%s:%u>: at %s object %u/%u, block %u/%u", StringFrom(this).c_str(), channel, GenerateTime(t).c_str(), objectindex, (uint_t)objectlist.size(), blockindex, (uint_t)objectlist[objectindex].channelformat->GetBlockFormatRefs().size());
  else                                 BBCDEBUG("Cursor<%s:%u>: at %s object %u/%u", StringFrom(this).c_str(), channel, GenerateTime(t).c_str(), objectindex, (uint_t)objectlist.size());
#endif

  currenttime = t;

  return ((objectindex != oldobjectindex) || (blockindex != oldblockindex));
}

BBC_AUDIOTOOLBOX_END
