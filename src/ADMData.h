#ifndef __ADM_DATA__
#define __ADM_DATA__

#include <string>
#include <vector>
#include <map>

#include <bbcat-base/misc.h>

#include "ADMObjects.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** ADM data class
 *
 * It CANNOT, by itself decode the XML from axml chunks, that must be done by a derived class.
 */
/*--------------------------------------------------------------------------------*/
class ADMData
{
public:
  ADMData();
  ADMData(const ADMData& obj);
  virtual ~ADMData();

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  virtual ADMData& operator = (const ADMData& obj) {Copy(obj); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Copy from another ADM
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Copy(const ADMData& obj);

  /*--------------------------------------------------------------------------------*/
  /** Delete all objects within this ADM
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Delete();

  /*--------------------------------------------------------------------------------*/
  /** Set default 'pure' mode value
   */
  /*--------------------------------------------------------------------------------*/
  static void SetDefaultPureMode(bool enable = true) {defaultpuremode = enable;}

  /*--------------------------------------------------------------------------------*/
  /** Enable/disable 'pure' mode
   *
   * @param enable true to enable pure mode
   *
   * @note in pure mode, implied links and settings will *not* be applied
   * @note (e.g. setting type labels of referenced objects)
   */
  /*--------------------------------------------------------------------------------*/
  void EnablePureMode(bool enable = true) {puremode = enable;}
  bool InPureMode() const {return puremode;}

  /*--------------------------------------------------------------------------------*/
  /** Finalise ADM
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Finalise();

  /*--------------------------------------------------------------------------------*/
  /** Create an ADM sub-object within this ADM object
   *
   * @param type object type - should always be the static 'Type' member of the object to be created (e.g. ADMAudioProgramme::Type)
   * @param id unique ID for the object (or empty string to have one created)
   * @param name human-readable name of the object
   *
   * @return ptr to object or NULL if type unrecognized or the object already exists
   */
  /*--------------------------------------------------------------------------------*/
  virtual ADMObject *Create(const std::string& type, const std::string& id, const std::string& name);

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
  std::string CreateID(const std::string& type);

  /*--------------------------------------------------------------------------------*/
  /** Change the ID of the specified object
   *
   * @param obj ADMObject to change ID of
   * @param id new ID
   * @param start starting index used for search
   */
  /*--------------------------------------------------------------------------------*/
  void ChangeID(ADMObject *obj, const std::string& id, uint_t start = 0);

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
  ADMAudioProgramme *CreateProgramme(const std::string& name);
  const ADMAudioProgramme::LIST& GetAudioProgrammeList() const {return audioprogrammes;}
  
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
  ADMAudioContent *CreateContent(const std::string& name, ADMAudioProgramme *programme = NULL);
  const ADMAudioContent::LIST& GetAudioContentList() const {return audiocontent;}

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
  ADMAudioObject *CreateObject(const std::string& name, ADMAudioContent *content = NULL);
  const ADMAudioObject::LIST& GetAudioObjectList() const {return audioobjects;}

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
  ADMAudioPackFormat *CreatePackFormat(const std::string& name, ADMAudioObject *object = NULL);

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
  enum {Track_Auto = 0x10000};
  ADMAudioTrack *CreateTrack(uint_t trackNum = Track_Auto, ADMAudioObject *object = NULL);

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
  ADMAudioChannelFormat *CreateChannelFormat(const std::string& name, ADMAudioPackFormat *packFormat = NULL, ADMAudioStreamFormat *streamFormat = NULL);

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
  ADMAudioBlockFormat *CreateBlockFormat(ADMAudioChannelFormat *channelFormat);

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
  ADMAudioTrackFormat *CreateTrackFormat(const std::string& name, ADMAudioStreamFormat *streamFormat = NULL);

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
  ADMAudioStreamFormat *CreateStreamFormat(const std::string& name, ADMAudioTrackFormat *trackFormat = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Register an ADM sub-object with this ADM
   *
   * @param obj ptr to ADM object
   *
   */
  /*--------------------------------------------------------------------------------*/
  void Register(ADMObject *obj);

  /*--------------------------------------------------------------------------------*/
  /** Return the object associated with the specified reference
   *
   * @param value a name/value pair specifying object type and name
   */
  /*--------------------------------------------------------------------------------*/
  ADMObject *GetReference(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Get list of objects of specified type
   *
   * @param type audioXXX object type (ADMAudioXXX::Type)
   * @param list list to be populated
   */
  /*--------------------------------------------------------------------------------*/
  void GetObjects(const std::string& type, std::vector<const ADMObject *>& list) const;

  /*--------------------------------------------------------------------------------*/
  /** Get list of writable objects of specified type
   *
   * @param type audioXXX object type (ADMAudioXXX::Type)
   * @param list list to be populated
   */
  /*--------------------------------------------------------------------------------*/
  void GetWritableObjects(const std::string& type, std::vector<ADMObject *>& list) const;

  /*--------------------------------------------------------------------------------*/
  /** Get ADM object by ID (with optional object type specified)
   *
   * @param id object ID
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const ADMObject *GetObjectByID(const std::string& id, const std::string& type = "") const;

  /*--------------------------------------------------------------------------------*/
  /** Get ADM object by Name (with optional object type specified)
   *
   * @param name object name
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const ADMObject *GetObjectByName(const std::string& name, const std::string& type = "") const;

  /*--------------------------------------------------------------------------------*/
  /** Get writable ADM object by ID (with optional object type specified)
   *
   * @param id object ID
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  ADMObject *GetWritableObjectByID(const std::string& id, const std::string& type = "");

  /*--------------------------------------------------------------------------------*/
  /** Get writable ADM object by Name (with optional object type specified)
   *
   * @param name object name
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  ADMObject *GetWritableObjectByName(const std::string& name, const std::string& type = "");

  /*--------------------------------------------------------------------------------*/
  /** Return a list of all ADM Audio Objects
   */
  /*--------------------------------------------------------------------------------*/
  void GetAudioObjectList(std::vector<const ADMAudioObject *>& list) const;

  /*--------------------------------------------------------------------------------*/
  /** Return track list (list of ADMAudioTrack objects)
   */
  /*--------------------------------------------------------------------------------*/
  typedef std::vector<const ADMAudioTrack *> TRACKLIST;
  const TRACKLIST& GetTrackList() const {return tracklist;}

  /*--------------------------------------------------------------------------------*/
  /** Return non-ADM XML for a particular node that needs to be preserved
   */
  /*--------------------------------------------------------------------------------*/
  virtual const XMLValues *GetNonADMXML(const std::string& node) const;

  /*--------------------------------------------------------------------------------*/
  /** Create Non ADM XML
   *
   * @param node name of XML node to hang additional XML off (or empty for the root ADM node)
   *
   * @return a XMLValues structure to add additional XML to
   *
   * @note node *must* describe an existing ADM XML node (or use empty for the root ADM node)
   * @note if node already exists, it will be returned without any creation
   */
  /*--------------------------------------------------------------------------------*/
  virtual XMLValues *CreateNonADMXML(const std::string& node);
  
  /*--------------------------------------------------------------------------------*/
  /** Structure containing names of ADM objects to create and connect together using CreateObjects()
   *
   * If the required object of the specified name does not exist, it is created
   *
   * Objects are connected according to the ADM rules, as many objects in the structure
   * are connected together as possible
   *
   * Any empty names will be ignored and objects not created or connected
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct
  {
    uint_t      trackNumber;            ///< physical track number to use (1- based) (NOTE: tracks are never created by CreateObjects())
    std::string programmeName;          ///< programme title
    std::string contentName;            ///< content title
    std::string objectName;             ///< object name
    std::string packFormatName;         ///< pack format name
    std::string channelFormatName;      ///< channel format name
    std::string streamFormatName;       ///< stream format name
    std::string trackFormatName;        ///< track format name
    uint_t      typeLabel;              ///< default typeLabel for packs, channels, streams and tracks
    struct {
      ADMAudioProgramme     *programme;
      ADMAudioContent       *content;
      ADMAudioObject        *object;
      ADMAudioPackFormat    *packFormat;
      ADMAudioChannelFormat *channelFormat;
      ADMAudioStreamFormat  *streamFormat;
      ADMAudioTrackFormat   *trackFormat;
      ADMAudioTrack         *audioTrack;
    } objects;
  } OBJECTNAMES;

  /*--------------------------------------------------------------------------------*/
  /** Create/link ADM objects
   *
   * @param data OBJECTNAMES structure populated with names of objects to link/create (see above)
   *
   * @return true if successful
   */
  /*--------------------------------------------------------------------------------*/
  bool CreateObjects(OBJECTNAMES& names);

  /*--------------------------------------------------------------------------------*/
  /** Generate a mapping from objects of type <type1> to/from objects of type <type2>
   *
   * @param refmap map to be populated
   * @param type1 first type (ADMAudioXXX:TYPE>
   * @param type2 second type (ADMAudioXXX:TYPE>
   * @param reversed false for type1 -> type2, true for type2 -> type1
   */
  /*--------------------------------------------------------------------------------*/
  typedef std::map<const ADMObject *,std::vector<const ADMObject *> > ADMREFERENCEMAP;
  void GenerateReferenceMap(ADMREFERENCEMAP& refmap, const std::string& type1, const std::string& type2, bool reversed = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Update audio object limits
   */
  /*--------------------------------------------------------------------------------*/
  void UpdateAudioObjectLimits();

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
  virtual void Dump(std::string& str, const ADMObject *obj = NULL, const std::string& indent = "  ", const std::string& eol = "\n", uint_t level = 0) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  /*--------------------------------------------------------------------------------*/
  /** From a list of objects, find all objects that are referenced
   *
   * @param list initial list of objects - will be EXPANDED with more objects
   */
  /*--------------------------------------------------------------------------------*/
  void GetReferencedObjects(std::vector<const ADMObject *>& list) const;

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
  bool CreateFromFile(const char *filename);

protected:
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
  std::string FindUniqueID(const std::string& type, const std::string& format, uint_t start);

  /*--------------------------------------------------------------------------------*/
  /** Return next available track number
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetNextTrackNum() const;

  /*--------------------------------------------------------------------------------*/
  /** Return whether the specified type is a valid object type
   */
  /*--------------------------------------------------------------------------------*/
  bool ValidType(const std::string& type) const;

  /*--------------------------------------------------------------------------------*/
  /** Context structure for generating textual dump of ADM
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    std::string str;                    ///< string into which text is generated
    std::string indent;                 ///< indent string for each level
    std::string eol;                    ///< end-of-line string
    uint_t      ind_level;              ///< current indentation level
  } DUMPCONTEXT;
  /*--------------------------------------------------------------------------------*/
  /** Generate textual description of ADM object (recursive
   *
   * @param obj ADM object
   * @param map map of objects already stored (bool is a dummy)
   * @param context DUMPCONTEXT for tracking indentation, etc
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Dump(const ADMObject *obj, std::map<const ADMObject *,bool>& map, DUMPCONTEXT& context) const;

  /*--------------------------------------------------------------------------------*/
  /** Connect XML references once all objects have been read
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ConnectReferences();

  /*--------------------------------------------------------------------------------*/
  /** Sort tracks into numerical order
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SortTracks();

  /*--------------------------------------------------------------------------------*/
  /** Change temporary ID of object and all its referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ChangeTemporaryID(ADMObject *obj, std::map<ADMObject *,bool>& map);

  /*--------------------------------------------------------------------------------*/
  /** Change temporary IDs to full valid ones based on a set of rules
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ChangeTemporaryIDs();

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Return ADM as JSON
   */
  /*--------------------------------------------------------------------------------*/
  virtual json_spirit::mObject ToJSON() const;
#endif

  typedef std::map<std::string,ADMObject*> ADMOBJECTS_MAP;
  typedef ADMOBJECTS_MAP::iterator         ADMOBJECTS_IT;
  typedef ADMOBJECTS_MAP::const_iterator   ADMOBJECTS_CIT;

  /*--------------------------------------------------------------------------------*/
  /** Try to add object to list by checking type using dynamic casting
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  void AddToList(std::vector<T*>& list, ADMObject *obj)
  {
    T *p;
    if ((p = dynamic_cast<T *>(obj)) != NULL) list.push_back(p);
  }

  /*--------------------------------------------------------------------------------*/
  /** Try to add object to list by checking type using dynamic casting
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  void AddToList(std::vector<const T*>& list, const ADMObject *obj)
  {
    const T *p;
    if ((p = dynamic_cast<const T *>(obj)) != NULL) list.push_back(p);
  }

protected:
  ADMAudioProgramme::LIST         audioprogrammes;
  ADMAudioContent::LIST           audiocontent;
  ADMAudioObject::LIST            audioobjects;
  ADMOBJECTS_MAP                  admobjects;
  TRACKLIST                       tracklist;
  std::map<std::string,uint_t>    uniqueids;
  std::map<std::string,XMLValues> nonadmxml;
  bool                            puremode;

  static const std::string        tempidsuffix;
  static bool                     defaultpuremode;
};

BBC_AUDIOTOOLBOX_END

#endif
