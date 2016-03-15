
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#define BBCDEBUG_LEVEL 1
#include <bbcat-base/EnhancedFile.h>

#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

const std::string ADMData::tempidsuffix = "_T";
bool              ADMData::defaultpuremode = false;

ADMData::ADMData() : puremode(defaultpuremode)
{
}

ADMData::ADMData(const ADMData& obj) : puremode(defaultpuremode)
{
  Copy(obj);
}

ADMData::~ADMData()
{
  Delete();
}

/*--------------------------------------------------------------------------------*/
/** Copy from another ADM
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Copy(const ADMData& obj)
{
  ADMOBJECTS_MAP::const_iterator it;

  Delete();
  
  uniqueids = obj.uniqueids;
  nonadmxml = obj.nonadmxml;
  puremode  = obj.puremode;
 
  // 1st pass: copy all objects
  for (it = obj.admobjects.begin(); it != obj.admobjects.end(); ++it)
  {
    const ADMObject    *oldobj = it->second;
    const std::string& type    = oldobj->GetType();
    ADMObject *newobj = NULL;
    
    if      (type == ADMAudioProgramme::Type)     newobj = new ADMAudioProgramme(*this, dynamic_cast<const ADMAudioProgramme *>(oldobj));
    else if (type == ADMAudioContent::Type)       newobj = new ADMAudioContent(*this, dynamic_cast<const ADMAudioContent *>(oldobj));
    else if (type == ADMAudioObject::Type)        newobj = new ADMAudioObject(*this, dynamic_cast<const ADMAudioObject *>(oldobj));
    else if (type == ADMAudioPackFormat::Type)    newobj = new ADMAudioPackFormat(*this, dynamic_cast<const ADMAudioPackFormat *>(oldobj));
    else if (type == ADMAudioChannelFormat::Type) newobj = new ADMAudioChannelFormat(*this, dynamic_cast<const ADMAudioChannelFormat *>(oldobj));
    else if (type == ADMAudioStreamFormat::Type)  newobj = new ADMAudioStreamFormat(*this, dynamic_cast<const ADMAudioStreamFormat *>(oldobj));
    else if (type == ADMAudioTrackFormat::Type)   newobj = new ADMAudioTrackFormat(*this, dynamic_cast<const ADMAudioTrackFormat *>(oldobj));
    else if (type == ADMAudioTrack::Type)         newobj = new ADMAudioTrack(*this, dynamic_cast<const ADMAudioTrack *>(oldobj));

    if (newobj)
    {
      BBCDEBUG3(("Copied object %s successfully to %s", oldobj->ToString().c_str(), newobj->ToString().c_str()));
    }
  }

  // 2nd pass: connect all references
  for (it = obj.admobjects.begin(); it != obj.admobjects.end(); ++it)
  {
    ADMOBJECTS_MAP::iterator it2;

    // use key for original list of objects to find equivalent object in new list
    if ((it2 = admobjects.find(it->first)) != admobjects.end())
    {
      // duplicate references from original object (requires all objects to exist in new list)
      it2->second->CopyReferences(it->second);
    }
    else BBCERROR("Failed to find copied object '%s' in new list", it->first.c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Delete all objects within this ADM
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Delete()
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    delete it->second;
  }

  audioprogrammes.clear();
  audiocontent.clear();
  audioobjects.clear();
  
  admobjects.clear();
  tracklist.clear();
  uniqueids.clear();
  nonadmxml.clear();
}

/*--------------------------------------------------------------------------------*/
/** Finalise ADM
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Finalise()
{
  std::vector<ADMObject *> channelformats;
  uint_t i;

  BBCDEBUG1(("Sorting tracks..."));
  SortTracks();

  // sort channel formats in time order
  BBCDEBUG1(("Sorting block formats in all channel formats..."));
  GetWritableObjects(ADMAudioChannelFormat::Type, channelformats);
  for (i = 0; i < channelformats.size(); i++)
  {
    ADMAudioChannelFormat *cf = dynamic_cast<ADMAudioChannelFormat *>(channelformats[i]);
    if (cf) cf->SortBlockFormats();
  }
  
  BBCDEBUG2(("Connecting references..."));
  ConnectReferences();

  BBCDEBUG3(("Changing temporary IDs..."));
  ChangeTemporaryIDs();
}

/*--------------------------------------------------------------------------------*/
/** Register an ADM sub-object with this ADM
 *
 * @param obj ptr to ADM object
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Register(ADMObject *obj)
{
  admobjects[obj->GetMapEntryID()] = obj;

  // add object to specialised lists
  AddToList<ADMAudioTrack>(tracklist, obj);
  AddToList<ADMAudioProgramme>(audioprogrammes, obj);
  AddToList<ADMAudioContent>(audiocontent, obj);
  AddToList<ADMAudioObject>(audioobjects, obj);

  obj->SetReferences();
}

/*--------------------------------------------------------------------------------*/
/** Return whether the specified type is a valid object type
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::ValidType(const std::string& type) const
{
  return ((type == ADMAudioProgramme::Type) ||
          (type == ADMAudioContent::Type) ||
          (type == ADMAudioObject::Type) ||
          (type == ADMAudioPackFormat::Type) ||
          (type == ADMAudioChannelFormat::Type) ||
          (type == ADMAudioStreamFormat::Type) ||
          (type == ADMAudioTrackFormat::Type) ||
          (type == ADMAudioTrack::Type));
}

/*--------------------------------------------------------------------------------*/
/** Create an ADM sub-object within this ADM object
 *
 * @param type object type - should always be the static 'Type' member of the object to be created (e.g. ADMAudioProgramme::Type)
 * @param id unique ID for the object (or empty string to create one using CreateID())
 * @param name human-readable name of the object
 *
 * @return ptr to object or NULL if type unrecognized or the object already exists
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::Create(const std::string& type, const std::string& id, const std::string& name)
{
  ADMObject *obj = NULL;

  if (ValidType(type))
  {
    ADMOBJECTS_CIT it;
    // if id is empty, create one
    std::string id1  = (!id.empty() ? id : CreateID(type));
    std::string uuid = type + "/" + id1;

    // ensure the id doesn't already exist
    if ((it = admobjects.find(uuid)) == admobjects.end())
    {
      BBCDEBUG3(("Creating %s ID %s Name %s UUID %s", type.c_str(), id1.c_str(), name.c_str(), uuid.c_str())); 

      if      (type == ADMAudioProgramme::Type)     obj = new ADMAudioProgramme(*this, id1, name);
      else if (type == ADMAudioContent::Type)       obj = new ADMAudioContent(*this, id1, name);
      else if (type == ADMAudioObject::Type)        obj = new ADMAudioObject(*this, id1, name);
      else if (type == ADMAudioPackFormat::Type)    obj = new ADMAudioPackFormat(*this, id1, name);
      else if (type == ADMAudioChannelFormat::Type) obj = new ADMAudioChannelFormat(*this, id1, name);
      else if (type == ADMAudioStreamFormat::Type)  obj = new ADMAudioStreamFormat(*this, id1, name);
      else if (type == ADMAudioTrackFormat::Type)   obj = new ADMAudioTrackFormat(*this, id1, name);
      else if (type == ADMAudioTrack::Type)
      {
        // create track and assign next available track number to it
        ADMAudioTrack *track;
        uint_t tn = GetNextTrackNum();
        
        if ((track = new ADMAudioTrack(*this, id1)) != NULL)
        {
          track->SetTrackNum(tn);
          BBCDEBUG3(("Created audioTrack ID '%s' track %2u", track->GetID().c_str(), track->GetTrackNum()));
        }
        obj = track;
      }
      else
      {
        BBCERROR("Cannot create type '%s'", type.c_str());
      }
    }
    else obj = it->second;
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Find an unique ID given the specified format string
 *
 * @param type object type
 * @param format C-style format string
 * @param start starting index
 *
 * @return unique ID
 */
/*--------------------------------------------------------------------------------*/
std::string ADMData::FindUniqueID(const std::string& type, const std::string& format, uint_t start)
{
  ADMOBJECTS_CIT it;
  std::string id;
  uint_t n = start;

  // increment test value until ID is unique
  while (true)
  {
    std::string testid;

    Printf(testid, format.c_str(), ++n);

    // test this ID
    if ((it = admobjects.find(type + "/" + testid)) == admobjects.end())
    {
      BBCDEBUG4(("ID '%s' is unique", (type + "/" + testid).c_str()));
      // ID not already in list -> must be unique
      id = testid;
      // save ID number
      uniqueids[type] = n;
      break;
    }
    else BBCDEBUG4(("ID '%s' is *not* unique", (type + "/" + testid).c_str()));
  }

  return id;
}

/*--------------------------------------------------------------------------------*/
/** Create an unique ID (temporary) for the specified object
 *
 * @param type object type string
 *
 * @return unique ID
 *
 * @note in some cases, this ID is TEMPORARY and will be updated by the object itself
 */
/*--------------------------------------------------------------------------------*/
std::string ADMData::CreateID(const std::string& type)
{
  std::string id;

  if (ValidType(type))
  {
    std::string format;
    uint_t start = 0;

    if      (type == ADMAudioProgramme::Type)     {format = ADMAudioProgramme::IDPrefix     + "%04x"; start = 0x1000;}
    else if (type == ADMAudioContent::Type)       {format = ADMAudioContent::IDPrefix       + "%04x"; start = 0x1000;}
    else if (type == ADMAudioObject::Type)        {format = ADMAudioObject::IDPrefix        + "%04x"; start = 0x1000;}
    else if (type == ADMAudioPackFormat::Type)    {format = ADMAudioPackFormat::IDPrefix    + "%08x" + tempidsuffix; start = uniqueids[type];}    // temporary
    else if (type == ADMAudioBlockFormat::Type)   {format = ADMAudioBlockFormat::IDPrefix   + "%08x" + tempidsuffix; start = uniqueids[type];}    // temporary
    else if (type == ADMAudioChannelFormat::Type) {format = ADMAudioChannelFormat::IDPrefix + "%08x" + tempidsuffix; start = uniqueids[type];}    // temporary
    else if (type == ADMAudioStreamFormat::Type)  {format = ADMAudioStreamFormat::IDPrefix  + "%08x" + tempidsuffix; start = uniqueids[type];}    // temporary
    else if (type == ADMAudioTrackFormat::Type)   {format = ADMAudioTrackFormat::IDPrefix   + "%08x" + tempidsuffix; start = uniqueids[type];}    // temporary
    else if (type == ADMAudioTrack::Type)         format = ADMAudioTrack::IDPrefix          + "%08x";

    if ((type == ADMAudioBlockFormat::Type) && start)
    {
      // for block formats (after the initial one has been found)
      // simply provide an incrementing number
      Printf(id, format.c_str(), ++start);
      uniqueids[type] = start;
    }
    else id = FindUniqueID(type, format, start);
  }

  BBCDEBUG3(("Created ID %s for type %s", id.c_str(), type.c_str())); 

  return id;
}

/*--------------------------------------------------------------------------------*/
/** Change the ID of the specified object
 *
 * @param obj ADMObject to change ID of
 * @param id new ID
 * @param start starting index used for search
 */
/*--------------------------------------------------------------------------------*/
void ADMData::ChangeID(ADMObject *obj, const std::string& id, uint_t start)
{
  // detect format characters in id
  bool format = (id.find("%") != std::string::npos);

  // if ID is to be updated, remove existing object from map
  // and re-enter with its new ID
  if (format || (id != obj->GetID()))
  {
    ADMOBJECTS_IT it;
    std::string   newid;

    // find object in map and delete it
    if ((it = admobjects.find(obj->GetMapEntryID())) != admobjects.end())
    {
      admobjects.erase(it);
    }

    // if id is a format string, find unique ID
    if (format) newid = FindUniqueID(obj->GetType(), id, start);
    // test to ensure explicit ID is not already used
    else if (admobjects.find(obj->GetType() + "/" + id) != admobjects.end())
    {
      BBCDEBUG1(("ID '%s' already exists for type '%s'!", id.c_str(), obj->GetType().c_str()));
      newid = FindUniqueID(obj->GetType(), id + "_%02x", 0);
    }
    // else just update object's ID
    else newid = id;

    BBCDEBUG3(("Change object<%s>'s ID from '%s' to '%s' (from '%s') (name '%s')", StringFrom(obj).c_str(), obj->GetID().c_str(), newid.c_str(), id.c_str(), obj->GetName().c_str()));

    obj->SetUpdatedID(newid);

    // put object back into map with new ID
    admobjects[obj->GetMapEntryID()] = obj;
  }
}

/*--------------------------------------------------------------------------------*/
/** Change temporary ID of object and all its referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMData::ChangeTemporaryID(ADMObject *obj, std::map<ADMObject *,bool>& map)
{
  // only process objects once!
  if (map.find(obj) == map.end())
  {
    std::vector<ADMObject::REFERENCEDOBJECT> objects;
    XMLValues values;
    const std::string& id = obj->GetID();
    uint_t i;

    // mark this object is generated
    map[obj] = true;

    if ((id.length() >= tempidsuffix.length()) && (id.substr(id.length() - tempidsuffix.length()) == tempidsuffix))
    {
      // this object has a temporary ID -> change it
      obj->UpdateID();
    }

    // get list of referenced objects and recurse
    obj->GetValuesAndReferences(values, objects, true);

    // don't care about values
    (void)values;

    for (i = 0; i < objects.size(); i++)
    {
      ChangeTemporaryID(objects[i].obj, map);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Change temporary IDs to full valid ones based on a set of rules
 */
/*--------------------------------------------------------------------------------*/
void ADMData::ChangeTemporaryIDs()
{
  // list of types to start changing ID's from
  // NOTE: all programme types are processed, THEN all content, THEN all objects, etc.
  static const std::string types[] =
  {
    ADMAudioProgramme::Type,
    ADMAudioContent::Type,
    ADMAudioObject::Type,
    ADMAudioPackFormat::Type,
  };
  std::map<ADMObject *,bool> map;
  ADMOBJECTS_IT it;
  uint_t i;
  
  // cycle through each of the types above
  for (i = 0; i < NUMBEROF(types); i++)
  {
    for (it = admobjects.begin(); it != admobjects.end(); ++it)
    {
      ADMObject *obj = it->second;

      if (obj->GetType() == types[i])
      {
        // change ID then move down hierarchy
        ChangeTemporaryID(obj, map);
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Create audioProgramme object
 *
 * @param name name of object
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioProgramme object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioProgramme *ADMData::CreateProgramme(const std::string& name)
{
  return new ADMAudioProgramme(*this, CreateID(ADMAudioProgramme::Type), name);
}

/*--------------------------------------------------------------------------------*/
/** Create audioContent object
 *
 * @param name name of object
 * @param programme audioProgramme object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioContent object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioContent *ADMData::CreateContent(const std::string& name, ADMAudioProgramme *programme)
{
  ADMAudioContent *content;

  if ((content = new ADMAudioContent(*this, CreateID(ADMAudioContent::Type), name)) != NULL)
  {
    if (programme) programme->Add(content);
  }

  return content;
}

/*--------------------------------------------------------------------------------*/
/** Create audioObject object
 *
 * @param name name of object
 * @param content audioContent object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioObject object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObject *ADMData::CreateObject(const std::string& name, ADMAudioContent *content)
{
  ADMAudioObject *object;

  if ((object = new ADMAudioObject(*this, CreateID(ADMAudioObject::Type), name)) != NULL)
  {
    if (content) content->Add(object);
  }

  return object;
}

/*--------------------------------------------------------------------------------*/
/** Create audioPackFormat object
 *
 * @param name name of object
 * @param object audioObject object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioPackFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioPackFormat *ADMData::CreatePackFormat(const std::string& name, ADMAudioObject *object)
{
  ADMAudioPackFormat *packFormat;

  if ((packFormat = new ADMAudioPackFormat(*this, CreateID(ADMAudioPackFormat::Type), name)) != NULL)
  {
    if (object) object->Add(packFormat);
  }

  return packFormat;
}

/*--------------------------------------------------------------------------------*/
/** Return next available track number
 */
/*--------------------------------------------------------------------------------*/
uint_t ADMData::GetNextTrackNum() const
{
  uint_t i, trackNum = 0;

  for (i = 0; i < tracklist.size(); i++)
  {
    trackNum = std::max(trackNum, tracklist[i]->GetTrackNum() + 1);
  }

  BBCDEBUG3(("GetNextTrackNum with tracklist of %u items is %u", (uint_t)tracklist.size(), trackNum));
  
  return trackNum;
}

/*--------------------------------------------------------------------------------*/
/** Create audioTrack object
 *
 * @param trackNum track number (or Track_Auto to automatically choose track)
 * @param object audioObject object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioTrack object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioTrack *ADMData::CreateTrack(uint_t trackNum, ADMAudioObject *object)
{
  ADMAudioTrack *track;

  // pick next track number
  if (trackNum == Track_Auto) trackNum = GetNextTrackNum();

  if ((track = new ADMAudioTrack(*this, CreateID(ADMAudioTrack::Type))) != NULL)
  {
    track->SetTrackNum(trackNum);
    
    if (object) object->Add(track);
  }

  return track;
}

/*--------------------------------------------------------------------------------*/
/** Create audioChannelFormat object
 *
 * @param name name of object
 * @param packFormat audioPackFormat object to attach this object to or NULL
 * @param streamFormat audioStreamFormat object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioChannelFormat *ADMData::CreateChannelFormat(const std::string& name, ADMAudioPackFormat *packFormat, ADMAudioStreamFormat *streamFormat)
{
  ADMAudioChannelFormat *channelFormat;

  if ((channelFormat = new ADMAudioChannelFormat(*this, CreateID(ADMAudioChannelFormat::Type), name)) != NULL)
  {
    if (packFormat) packFormat->Add(channelFormat);
    if (streamFormat) streamFormat->Add(channelFormat);
  }

  return channelFormat;
}

/*--------------------------------------------------------------------------------*/
/** Create audioBlockFormat object
 *
 * @param channelformat optional channel format to add the new block to
 *
 * @return ADMAudioBlockFormat object
 *
 * @note audioBlockFormat object MUST be added to an audioChannelFormat to be useful (but can be added after this call)!
 */
/*--------------------------------------------------------------------------------*/
ADMAudioBlockFormat *ADMData::CreateBlockFormat(ADMAudioChannelFormat *channelFormat)
{
  ADMAudioBlockFormat *blockFormat;

  if ((blockFormat = new ADMAudioBlockFormat) != NULL)
  {
    if (channelFormat) channelFormat->Add(blockFormat);
  }

  return blockFormat;
}

/*--------------------------------------------------------------------------------*/
/** Create audioTrackFormat object
 *
 * @param name name of object
 * @param streamFormat audioStreamFormat object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioTrackFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioTrackFormat *ADMData::CreateTrackFormat(const std::string& name, ADMAudioStreamFormat *streamFormat)
{
  ADMAudioTrackFormat *trackFormat;

  if ((trackFormat = new ADMAudioTrackFormat(*this, CreateID(ADMAudioTrackFormat::Type), name)) != NULL)
  {
    if (streamFormat)
    {
      streamFormat->Add(trackFormat);
      trackFormat->Add(streamFormat);
    }
  }

  return trackFormat;
}

/*--------------------------------------------------------------------------------*/
/** Create audioStreamFormat object
 *
 * @param name name of object
 * @param trackFormat audioTrackFormat object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioStreamFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioStreamFormat *ADMData::CreateStreamFormat(const std::string& name, ADMAudioTrackFormat *trackFormat)
{
  ADMAudioStreamFormat *streamFormat;

  if ((streamFormat = new ADMAudioStreamFormat(*this, CreateID(ADMAudioStreamFormat::Type), name)) != NULL)
  {
    if (trackFormat)
    {
      trackFormat->Add(streamFormat);
      streamFormat->Add(trackFormat);
    }
  }

  return streamFormat;
}

/*--------------------------------------------------------------------------------*/
/** Return the object associated with the specified reference
 *
 * @param value a name/value pair specifying object type and name
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::GetReference(const XMLValue& value)
{
  ADMObject *obj = NULL;
  ADMOBJECTS_CIT it;
  std::string uuid = value.name, cmp;

  cmp = "UIDRef";
  if ((uuid.size() >= cmp.size()) && (uuid.compare(uuid.size() - cmp.size(), cmp.size(), cmp) == 0))
  {
    uuid = uuid.substr(0, uuid.size() - 3);
  }
  else
  {
    cmp = "IDRef";
    if ((uuid.size() >= cmp.size()) && (uuid.compare(uuid.size() - cmp.size(), cmp.size(), cmp) == 0))
    {
      uuid = uuid.substr(0, uuid.size() - cmp.size());
    }
  }

  uuid += "/" + value.value;

  if ((it = admobjects.find(uuid)) != admobjects.end()) obj = it->second;
  else
  {
#if BBCDEBUG_LEVEL >= 4
    BBCDEBUG1(("Failed to find reference '%s', object list:", uuid.c_str()));
    for (it = admobjects.begin(); it != admobjects.end(); ++it)
    {
      BBCDEBUG1(("\t%s / %s (%u / %u)%s", it->first.c_str(), uuid.c_str(), (uint_t)it->first.size(), (uint_t)uuid.size(), (it->first == uuid) ? " *" : ""));
    }
#endif
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Sort tracks into numerical order
 */
/*--------------------------------------------------------------------------------*/
void ADMData::SortTracks()
{
  sort(tracklist.begin(), tracklist.end(), ADMAudioTrack::Compare);

#if BBCDEBUG_LEVEL >= 4
  std::vector<const ADMAudioTrack *>::const_iterator it;

  BBCDEBUG1(("%lu tracks:", tracklist.size()));
  for (it = tracklist.begin(); it != tracklist.end(); ++it)
  {
    BBCDEBUG1(("%u: %s", (*it)->GetTrackNum(), (*it)->ToString().c_str()));
  }
#endif
}

/*--------------------------------------------------------------------------------*/
/** Connect references between objects
 */
/*--------------------------------------------------------------------------------*/
void ADMData::ConnectReferences()
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    it->second->SetReferences();
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a mapping from objects of type <type1> to/from objects of type <type2>
 *
 * @param refmap map to be populated
 * @param type1 first type (ADMAudioXXX:TYPE>
 * @param type2 second type (ADMAudioXXX:TYPE>
 * @param reversed false for type1 -> type2, true for type2 -> type1
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GenerateReferenceMap(ADMREFERENCEMAP& refmap, const std::string& type1, const std::string& type2, bool reversed) const
{
  ADMOBJECTS_CIT it;
  uint_t i;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj1 = it->second;

    if (obj1->GetType() == type1)
    {
      std::vector<const ADMObject *> list1;

      list1.push_back(obj1);

      // find all other objects it references
      GetReferencedObjects(list1);

      // for each object in list1 of type <type2>, add it to a list for obj1
      for (i = 0; i < list1.size(); i++)
      {
        const ADMObject *obj2 = list1[i];

        if (obj1->GetType() == type2)
        {
          const ADMObject *objA = reversed ? obj2 : obj1;
          const ADMObject *objB = reversed ? obj1 : obj2;
          std::vector<const ADMObject *>& list2 = refmap[objA];

          // if obj2 is not in list2, add it
          if (std::find(list2.begin(), list2.end(), objB) == list2.end()) list2.push_back(objB);
        }
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Update audio object limits
 */
/*--------------------------------------------------------------------------------*/
void ADMData::UpdateAudioObjectLimits()
{
  ADMREFERENCEMAP channelformats;
  ADMOBJECTS_IT it;
  uint_t i;

  // generate mapping from channel formats -> audio objects
  GenerateReferenceMap(channelformats, ADMAudioObject::Type, ADMAudioChannelFormat::Type, true);

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    ADMAudioObject *audioobj;
    const ADMObject *obj = it->second;

    // for each audio object
    if (((audioobj = const_cast<ADMAudioObject *>(dynamic_cast<const ADMAudioObject *>(obj))) != NULL) &&
        !(audioobj->StartTimeSet() || audioobj->DurationSet())) // don't update audio object limits if they have been explicitly set
    {
      std::vector<const ADMObject *> list;
      const uint64_t originalstart = audioobj->GetStartTime();
      uint64_t start = originalstart;
      uint64_t end   = start;

      list.push_back(obj);

      // find all other objects it references
      GetReferencedObjects(list);

      // scan for block formats and find their limits
      bool first   = true;
      bool canmove = true;
      for (i = 0; i < list.size(); i++)
      {
        const ADMObject *obj = list[i];
        const ADMAudioBlockFormat *blockformat;

        if ((blockformat = dynamic_cast<const ADMAudioBlockFormat *>(obj)) != NULL)
        {
          // block limits are relative to audio object
          uint64_t bstart = blockformat->GetStartTime(audioobj);
          uint64_t bend   = blockformat->GetEndTime(audioobj);

          // update start and end times
          start = first ? bstart : std::min(start, bstart);
          end   = first ? bend   : std::max(end, bend);

          first = false;
        }

        if ((obj->GetType() == ADMAudioChannelFormat::Type) &&
            (channelformats.find(obj) != channelformats.end()) &&
            (channelformats[obj].size() > 1))
        {
          // unable to move this audio object because a channel format used by this audio object
          // is used by other audio objects
          BBCDEBUG2(("Cannot move object '%s', channel format '%s' is used by %u audio objects",
                  audioobj->GetName().c_str(),
                  obj->GetName().c_str(),
                  (uint_t)channelformats[obj].size()));
          canmove = false;
        }
      }

      // if unable to move audio object, set the new start as the original start
      if (!canmove) start = originalstart;

      BBCDEBUG2(("Updating object '%s' start %s -> %s end %s -> %s (canmove: %s)",
              audioobj->GetName().c_str(),
              StringFrom(audioobj->GetStartTime()).c_str(), StringFrom(start).c_str(),
              StringFrom(audioobj->GetEndTime()).c_str(),   StringFrom(end).c_str(),
              canmove ? "yes" : "no"));

      audioobj->SetStartTime(start);
      audioobj->SetEndTime(end);

      if (start > originalstart)
      {
        uint64_t diff = start - originalstart;

        BBCDEBUG2(("Shifting all block formats by %s", StringFrom(diff).c_str()));

        // modify every block format so that the first one starts at zero
        for (i = 0; i < list.size(); i++)
        {
          ADMAudioBlockFormat *blockformat;

          if ((blockformat = const_cast<ADMAudioBlockFormat *>(dynamic_cast<const ADMAudioBlockFormat *>(list[i]))) != NULL)
          {
            // move all block formats *back* by the same amount the audio object moves *forward*
            blockformat->SetRTime(blockformat->GetRTime() - diff);
          }
        }
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Get list of objects of specified type
 *
 * @param type audioXXX object type
 * @param list list to be populated
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GetObjects(const std::string& type, std::vector<const ADMObject *>& list) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (obj->GetType() == type)
    {
      list.push_back(obj);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Get list of writable objects of specified type
 *
 * @param type audioXXX object type
 * @param list list to be populated
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GetWritableObjects(const std::string& type, std::vector<ADMObject *>& list) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    ADMObject *obj = it->second;

    if (obj->GetType() == type)
    {
      list.push_back(obj);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Get ADM object by ID (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
const ADMObject *ADMData::GetObjectByID(const std::string& id, const std::string& type) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetID() == id)) return obj;
  }

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Get ADM object by Name (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
const ADMObject *ADMData::GetObjectByName(const std::string& name, const std::string& type) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetName() == name)) return obj;
  }

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Get writable ADM object by ID (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::GetWritableObjectByID(const std::string& id, const std::string& type)
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetID() == id)) return obj;
  }

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Get writable ADM object by Name (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::GetWritableObjectByName(const std::string& name, const std::string& type)
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetName() == name)) return obj;
  }

  return NULL;
}


/*--------------------------------------------------------------------------------*/
/** Return a list of all ADM Audio Objects
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GetAudioObjectList(std::vector<const ADMAudioObject *>& list) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMAudioObject *obj;

    if ((obj = dynamic_cast<const ADMAudioObject *>(it->second)) != NULL) list.push_back(obj);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return non-ADM XML for a particular node that needs to be preserved
 */
/*--------------------------------------------------------------------------------*/
const XMLValues *ADMData::GetNonADMXML(const std::string& node) const
{
  std::map<std::string,XMLValues>::const_iterator it;
  const XMLValues *values = NULL;

  if ((it = nonadmxml.find(node)) != nonadmxml.end()) values = &it->second;
  
  return values;
}

/*--------------------------------------------------------------------------------*/
/** Create Non ADM XML
 *
 * @param node name of XML node to hang additional XML off (or empty for root node)
 *
 * @return a XMLValues structure to add additional XML to
 *
 * @note if node already exists, it will be returned without any creation
 */
/*--------------------------------------------------------------------------------*/
XMLValues *ADMData::CreateNonADMXML(const std::string& node)
{
  std::map<std::string,XMLValues>::iterator it;
  XMLValues *values = NULL;

  // find or create
  if ((it = nonadmxml.find(node)) != nonadmxml.end()) values = &it->second;
  else                                                values = &nonadmxml[node];
  
  return values;
}

/*--------------------------------------------------------------------------------*/
/** Dump ADM or part of ADM as textual description
 *
 * @param str std::string to be modified with description
 * @param obj ADM object or NULL to dump the entire ADM
 * @param indent indentation for each level of objects
 * @param eol end-of-line string
 * @param level initial indentation level
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Dump(std::string& str, const ADMObject *obj, const std::string& indent, const std::string& eol, uint_t level) const
{
  std::map<const ADMObject *,bool> map;
  DUMPCONTEXT    context;
  ADMOBJECTS_CIT it;

  // initialise context
  context.str       = str;
  context.indent    = indent;
  context.eol       = eol;
  context.ind_level = level;

  // if explicit object specified, dump it
  if (obj) Dump(obj, map, context);
  else
  {
    // otherwise find Programme object and start with it
    for (it = admobjects.begin(); it != admobjects.end(); ++it)
    {
      const ADMObject *obj = it->second;

      if (obj->GetType() == ADMAudioProgramme::Type)
      {
        Dump(obj, map, context);
      }
    }
  }

  // return generated string
  str = context.str;
}

/*--------------------------------------------------------------------------------*/
/** Generate textual description of ADM object (recursive
 *
 * @param obj ADM object
 * @param map map of objects already stored (bool is a dummy)
 * @param context DUMPCONTEXT for tracking indentation, etc
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Dump(const ADMObject *obj, std::map<const ADMObject *,bool>& map, DUMPCONTEXT& context) const
{
  std::vector<ADMObject::REFERENCEDOBJECT> objects;
  XMLValues values;
  std::string& str = context.str;
  std::string  indent;
  uint_t i;

  // mark this object is generated
  map[obj] = true;

  obj->GetValuesAndReferences(values, objects, true);

  indent = CreateIndent(context.indent, context.ind_level++);

  Printf(str, "%s%s: %s", indent.c_str(), obj->GetType().c_str(), obj->GetID().c_str());

  if (obj->GetName() != "")
  {
    Printf(str, " / %s", obj->GetName().c_str());
  }

  str    += context.eol;
  indent += context.indent;

  // output attributes
  for (i = 0; i < values.size(); i++)
  {
    const XMLValue& value = values[i];

    if (value.attr)
    {
      Printf(str, "%s%s: %s%s", indent.c_str(), value.name.c_str(), value.value.c_str(), context.eol.c_str());
    }
  }

  // output values
  for (i = 0; i < values.size(); i++)
  {
    const XMLValue& value = values[i];

    if (!value.attr)
    {
      XMLValue::ATTRS::const_iterator it;

      Printf(str, "%s%s", indent.c_str(), value.name.c_str());

      if (value.attrs.end() != value.attrs.begin())
      {
        Printf(str, " (");
        for (it = value.attrs.begin(); it != value.attrs.end(); ++it)
        {
          Printf(str, "%s=%s", it->first.c_str(), it->second.c_str());
        }
        Printf(str, ")");
      }

      Printf(str, ": %s%s", value.value.c_str(), context.eol.c_str());
    }
  }

  // output references
  for (i = 0; i < objects.size(); i++)
  {
    const ADMObject::REFERENCEDOBJECT& object = objects[i];
    bool dumpthisobject = (map.find(object.obj) == map.end());

    if (object.genref)
    {
      // output reference to object.obj
      Printf(str, "%s%s: %s", indent.c_str(), object.obj->GetReference().c_str(), object.obj->GetID().c_str());

      if (dumpthisobject)
      {
        str += context.eol;
        context.ind_level++;
        Dump(object.obj, map, context);
        context.ind_level--;
      }
      else Printf(str, " (see above)%s", context.eol.c_str());
    }
    else if (dumpthisobject) Dump(object.obj, map, context);
  }

  context.ind_level--;
}

/*--------------------------------------------------------------------------------*/
/** From a list of objects, find all objects that are referenced
 *
 * @param list initial list of objects - will be EXPANDED with more objects
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GetReferencedObjects(std::vector<const ADMObject *>& list) const
{
  uint_t i, j;

  // for loop with expanding list which collects referenced objects as it goes
  for (i = 0; i < list.size(); i++)
  {
    std::vector<ADMObject::REFERENCEDOBJECT> objects;
    const ADMObject *obj = list[i];
    XMLValues values;

    // collect values and references for this object
    obj->GetValuesAndReferences(values, objects);

    // for each object, add it to the list if it has not already in the map
    for (j = 0; j < objects.size(); j++)
    {
      const ADMObject::REFERENCEDOBJECT& object = objects[j];

      if (std::find(list.begin(), list.end(), object.obj) == list.end())
      {
        // add it to the list to be processed
        list.push_back(object.obj);
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references
 *
 * @param str string to be modified
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GenerateReferenceList(std::string& str) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    obj->GenerateReferenceList(str);
  }
}

/*--------------------------------------------------------------------------------*/
/** Create/link ADM objects
 *
 * @param data OBJECTNAMES structure populated with names of objects to link/create (empty names are not created/linked)
 *
 * @return true if successful
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::CreateObjects(OBJECTNAMES& names)
{
  bool success = true;

  // clear object pointers
  memset(&names.objects, 0, sizeof(names.objects));

  // look up objects and create any that need creating
  if ((names.programmeName != "") && ((names.objects.programme = dynamic_cast<ADMAudioProgramme *>(GetWritableObjectByName(names.programmeName, ADMAudioProgramme::Type))) == NULL))
  {
    if ((names.objects.programme = CreateProgramme(names.programmeName)) != NULL)
    {
      BBCDEBUG2(("Created programme '%s'", names.programmeName.c_str()));
    }
    else
    {
      BBCERROR("Failed to create programme '%s'", names.programmeName.c_str());
      success = false;
    }
  }

  if ((names.contentName != "") && ((names.objects.content = dynamic_cast<ADMAudioContent *>(GetWritableObjectByName(names.contentName, ADMAudioContent::Type))) == NULL))
  {
    if ((names.objects.content = CreateContent(names.contentName)) != NULL)
    {
      BBCDEBUG2(("Created content '%s'", names.contentName.c_str()));
    }
    else
    {
      BBCERROR("Failed to create content '%s'", names.contentName.c_str());
      success = false;
    }
  }

  if ((names.objectName != "") && ((names.objects.object = dynamic_cast<ADMAudioObject *>(GetWritableObjectByName(names.objectName, ADMAudioObject::Type))) == NULL))
  {
    if ((names.objects.object = CreateObject(names.objectName)) != NULL)
    {
      BBCDEBUG2(("Created object '%s'", names.objectName.c_str()));
    }
    else
    {
      BBCERROR("Failed to create object '%s'", names.objectName.c_str());
      success = false;
    }
  }

  if ((names.packFormatName != "") && ((names.objects.packFormat = dynamic_cast<ADMAudioPackFormat *>(GetWritableObjectByName(names.packFormatName, ADMAudioPackFormat::Type))) == NULL))
  {
    if ((names.objects.packFormat = CreatePackFormat(names.packFormatName)) != NULL)
    {
      BBCDEBUG2(("Created pack format '%s'", names.packFormatName.c_str()));
    }
    else
    {
      BBCERROR("Failed to create packFormat '%s'", names.packFormatName.c_str());
      success = false;
    }
  }

  if ((names.channelFormatName != "") && ((names.objects.channelFormat = dynamic_cast<ADMAudioChannelFormat *>(GetWritableObjectByName(names.channelFormatName, ADMAudioChannelFormat::Type))) == NULL))
  {
    if ((names.objects.channelFormat = CreateChannelFormat(names.channelFormatName)) != NULL)
    {
      BBCDEBUG2(("Created channel format '%s'", names.channelFormatName.c_str()));
    }
    else
    {
      BBCERROR("Failed to create channelFormat '%s'", names.channelFormatName.c_str());
      success = false;
    }
  }

  if ((names.streamFormatName != "") && ((names.objects.streamFormat = dynamic_cast<ADMAudioStreamFormat *>(GetWritableObjectByName(names.streamFormatName, ADMAudioStreamFormat::Type))) == NULL))
  {
    if ((names.objects.streamFormat = CreateStreamFormat(names.streamFormatName)) != NULL)
    {
      // set stream type (PCM)
      BBCDEBUG2(("Created stream format '%s'", names.streamFormatName.c_str()));
      names.objects.streamFormat->SetFormatLabel(1);
      names.objects.streamFormat->SetFormatDefinition("PCM");
    }
    else
    {
      BBCERROR("Failed to create streamFormat '%s'", names.streamFormatName.c_str());
      success = false;
    }
  }

  if ((names.trackFormatName != "") && ((names.objects.trackFormat = dynamic_cast<ADMAudioTrackFormat *>(GetWritableObjectByName(names.trackFormatName, ADMAudioTrackFormat::Type))) == NULL))
  {
    if ((names.objects.trackFormat = CreateTrackFormat(names.trackFormatName)) != NULL)
    {
      // set track type (PCM)
      BBCDEBUG2(("Created track format '%s'", names.trackFormatName.c_str()));
      names.objects.trackFormat->SetFormatLabel(1);
      names.objects.trackFormat->SetFormatDefinition("PCM");
    }
    else
    {
      BBCERROR("Failed to create trackFormat '%s'", names.trackFormatName.c_str());
      success = false;
    }
  }

  if (names.trackNumber < tracklist.size())
  {
    if ((names.objects.audioTrack = const_cast<ADMAudioTrack *>(tracklist[names.trackNumber])) != NULL)
    {
      BBCDEBUG2(("Found track number %u (%u tracks)", names.trackNumber, (uint_t)tracklist.size()));
    }
    else
    {
      BBCERROR("Failed to find track number %u (%u tracks)", names.trackNumber, (uint_t)tracklist.size());
    }
  }

#define LINK(master,slave)                                  \
  if (names.objects.master && names.objects.slave)          \
  {                                                         \
    if (names.objects.master->Add(names.objects.slave))     \
    {                                                       \
      BBCDEBUG2(("Connected %s '%s' to %s '%s'",               \
              names.objects.slave->GetType().c_str(),       \
              names.objects.slave->GetName().c_str(),       \
              names.objects.master->GetType().c_str(),      \
              names.objects.master->GetName().c_str()));    \
    }                                                       \
    else                                                    \
    {                                                       \
      BBCERROR("Failed to connect %s '%s' to %s '%s'",         \
            names.objects.slave->GetType().c_str(),         \
            names.objects.slave->GetName().c_str(),         \
            names.objects.master->GetType().c_str(),        \
            names.objects.master->GetName().c_str());       \
      success = false;                                      \
    }                                                       \
  }

  // link objects
  LINK(packFormat, channelFormat);
  LINK(trackFormat, streamFormat);
  LINK(streamFormat, trackFormat);
  LINK(streamFormat, packFormat);
  LINK(streamFormat, channelFormat);
  LINK(audioTrack, trackFormat);
  LINK(audioTrack, packFormat);

  // add the object to the content
  LINK(content, object);
  // add the pack to the object
  LINK(object, packFormat);
  // add the track to the object
  LINK(object, audioTrack);

  LINK(content, object);

  LINK(programme, content);

  if (names.typeLabel != ADMObject::TypeLabel_Unknown)
  {
  // set typeLabel for objects that have not had their typeLabel set
#define SETTYPELABEL(obj) \
    if (names.objects.obj && (names.objects.obj->GetTypeLabel() == ADMObject::TypeLabel_Unknown)) names.objects.obj->SetTypeLabel(names.typeLabel);
    SETTYPELABEL(packFormat);
    SETTYPELABEL(trackFormat);
    SETTYPELABEL(streamFormat);
    SETTYPELABEL(channelFormat);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Create ADM from a simple text file
 *
 * @param filename file describing the basic ADM layout
 *
 * The file MUST be of the following format with each entry on its own line:
 * <ADM programme name>[:<ADM content name>]
 *
 * then for each track:
 * <track>:<trackname>:<objectname>
 *
 * Where <track> is 1..number of tracks available within ADM
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::CreateFromFile(const char *filename)
{
  EnhancedFile fp;
  bool success = false;

  if (fp.fopen(filename, "r"))
  {
    static char line[1024];
    ADMData::OBJECTNAMES names;
    uint_t ln = 1;

    success = true;

    while (fp.readline(line, sizeof(line) - 1) != EOF)
    {
      if (ln == 1)
      {
        // line 1: <programme-name>[:<content-name>]
        const char *p;

        // set names for programme and content objects
        // find optionally different content name
        if ((p = strchr(line, ':')) != NULL)
        {
          names.programmeName.assign(line, p - line);
          names.contentName.assign(p + 1);
        }
        else names.programmeName = names.contentName = line;
      }
      else
      {
        const char *p1, *p2;
        uint_t tr;

        if (sscanf(line, "%u", &tr) == 1)
        {
          if ((tr > 0) && (tr <= tracklist.size()))
          {
            if (((p1 = strchr(line, ':')) != NULL) && ((p2 = strchr(p1 + 1, ':')) != NULL))
            {
              std::string trackname;

              p1++;

              // set track number
              names.trackNumber = tr - 1;

              // set track name
              trackname.assign(p1, p2 - p1);

              // derive channel and stream names from track name
              names.channelFormatName = trackname;
              names.streamFormatName  = trackname;
              names.trackFormatName   = trackname;

              // set object name
              names.objectName.assign(p2 + 1);

              // set pack name from object name
              names.packFormatName = names.objectName;

              // set typeLabel to be used
              names.typeLabel = ADMObject::TypeLabel_Objects;

              // create / connect objects
              CreateObjects(names);
            }
            else
            {
              BBCERROR("Failed to decode <tr>:<trackname>:<objectname> line");
              success = false;
            }
          }
          else
          {
            BBCERROR("Track %u out of range 1-%u", tr, (uint_t)tracklist.size());
            success = false;
          }
        }
        else
        {
          BBCERROR("Failed to extract track number from '%s'", line);
          success = false;
        }
      }

      ln++;
    }

    fp.fclose();
  }

  return success;
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Return ADM as JSON
 */
/*--------------------------------------------------------------------------------*/
json_spirit::mObject ADMData::ToJSON() const
{
  json_spirit::mObject obj;
  json_spirit::mArray  array;

  // get list of ADMAudioObjects
  std::vector<const ADMObject *> list;
  uint_t i;

  GetObjects(ADMAudioObject::Type, list);
  for (i = 0; i < list.size(); i++)
  {
    const ADMAudioObject               *object = static_cast<const ADMAudioObject *>(list[i]);
    const std::vector<ADMAudioTrack *>& tracks = object->GetTrackRefs();
    uint_t t;

    for (t = 0; t < tracks.size(); t++)
    {
      uint_t trackNum = tracks[t]->GetTrackNum();
      const std::vector<ADMAudioBlockFormat *> *blockformats = object->GetBlockFormatList(trackNum);

      if (blockformats)
      {
        uint_t bid;

        for (bid = 0; bid < blockformats->size(); bid++)
        {
          json_spirit::mObject obj;

          obj["object"]   = object->GetID();
          obj["reltrack"] = (sint_t)trackNum - (sint_t)object->GetStartChannel();
          (*blockformats)[bid]->ToJSON(obj);

          array.push_back(obj);
        }
      }
    }
  }

  obj["channels"] = array;

  return obj;
}
#endif

BBC_AUDIOTOOLBOX_END
