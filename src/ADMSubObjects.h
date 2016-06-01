#ifndef __ADM_SUB_OBJECTS__
#define __ADM_SUB_OBJECTS__

#include "XMLValue.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Class representing loudness metadata held within the ADM
 */
/*--------------------------------------------------------------------------------*/
class ADMLoudnessMetadata
{
public:
  ADMLoudnessMetadata() {}
  ADMLoudnessMetadata(const ADMLoudnessMetadata& obj) {operator = (obj);}

  /*--------------------------------------------------------------------------------*/
  /** Return name of XML element to use
   */
  /*--------------------------------------------------------------------------------*/
  static std::string GetElementName() {return "loudnessMetadata";}
  
  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  ADMLoudnessMetadata& operator = (const ADMLoudnessMetadata& obj);

  /*--------------------------------------------------------------------------------*/
  /** Read values from XMLValue (representing 'loudnessMetadata' element)
   */
  /*--------------------------------------------------------------------------------*/
  ADMLoudnessMetadata& SetValues(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Write values to XMLValue (creating 'loudnessMetadata' element)
   */
  /*--------------------------------------------------------------------------------*/
  bool GetValues(XMLValue& value, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Allow access to extra values not interpreted
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValue& GetExtraValues() const   {return extravalues;}
  XMLValue&       GetExtraValuesWritable() {return extravalues;}

  // use .IsSet() to tell whether value has been set
  // use .Get() or normal casting to get value
  // use .Set() or normal assign to set value
  NAMEDPARAMETER(std::string,  loudnessMethod);
  NAMEDPARAMETER(std::string,  loudnessRecType);
  NAMEDPARAMETER(std::string,  loudnessCorrectionType);
  NAMEDPARAMETER(float,        integratedLoudness);
  NAMEDPARAMETER(float,        loudnessRange);
  NAMEDPARAMETER(float,        maxTruePeak);
  NAMEDPARAMETER(float,        maxMomentary);
  NAMEDPARAMETER(float,        maxShortTerm);
  NAMEDPARAMETER(float,        dialogLoudness);

protected:
  XMLValue extravalues;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Class representing audioProgrammeReferenceScreen
 */
/*--------------------------------------------------------------------------------*/
class ADMAudioProgrammeReferenceScreen
{
public:
  ADMAudioProgrammeReferenceScreen() {}
  ADMAudioProgrammeReferenceScreen(const ADMAudioProgrammeReferenceScreen& obj) {operator = (obj);}

  /*--------------------------------------------------------------------------------*/
  /** Return name of XML element to use
   */
  /*--------------------------------------------------------------------------------*/
  static std::string GetElementName() {return "audioProgrammeReferenceScreen";}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioProgrammeReferenceScreen& operator = (const ADMAudioProgrammeReferenceScreen& obj);

  /*--------------------------------------------------------------------------------*/
  /** Read values from XMLValue (representing 'audioProgrammeReferenceScreen' element)
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioProgrammeReferenceScreen& SetValues(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Write values to XMLValue (creating 'audioProgrammeReferenceScreen' element)
   */
  /*--------------------------------------------------------------------------------*/
  bool GetValues(XMLValue& value, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Allow access to extra values not interpreted
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValue& GetExtraValues() const   {return extravalues;}
  XMLValue&       GetExtraValuesWritable() {return extravalues;}

  // use .IsSet() to tell whether value has been set
  // use .Get() or normal casting to get value
  // use .Set() or normal assign to set value
  NAMEDPARAMETER(float, aspectRatio);
  struct
  {
    NAMEDPARAMETER(float, azimuth);
    NAMEDPARAMETER(float, elevation);
    NAMEDPARAMETER(float, distance);
    NAMEDPARAMETER(float, X);
    NAMEDPARAMETER(float, Y);
    NAMEDPARAMETER(float, Z);
  } screenCentrePosition;
  struct
  {
    NAMEDPARAMETER(float, azimuth);
    NAMEDPARAMETER(float, X);
  } screenWidth;

protected:
  XMLValue extravalues;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Class representing dialogue
 */
/*--------------------------------------------------------------------------------*/
class ADMAudioContentDialogue
{
public:
  ADMAudioContentDialogue() {}
  ADMAudioContentDialogue(const ADMAudioContentDialogue& obj) {operator = (obj);}

  /*--------------------------------------------------------------------------------*/
  /** Return name of XML element to use
   */
  /*--------------------------------------------------------------------------------*/
  static std::string GetElementName() {return "dialogue";}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioContentDialogue& operator = (const ADMAudioContentDialogue& obj);

  /*--------------------------------------------------------------------------------*/
  /** Read values from XMLValue (representing 'dialogue' element)
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioContentDialogue& SetValues(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Write values to XMLValue (creating 'dialogue' element)
   */
  /*--------------------------------------------------------------------------------*/
  bool GetValues(XMLValue& value, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Allow access to extra values not interpreted
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValue& GetExtraValues() const   {return extravalues;}
  XMLValue&       GetExtraValuesWritable() {return extravalues;}

  // use .IsSet() to tell whether value has been set
  // use .Get() or normal casting to get value
  // use .Set() or normal assign to set value
  NAMEDPARAMETER(uint_t, dialogue);
  NAMEDPARAMETER(uint_t, nonDialogueContentKind);
  NAMEDPARAMETER(uint_t, dialogueContentKind);
  NAMEDPARAMETER(uint_t, mixedContentKind);

protected:
  XMLValue extravalues;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Class representing frequency and type definition
 */
/*--------------------------------------------------------------------------------*/
class ADMFrequency
{
public:
  ADMFrequency() {}
  ADMFrequency(const ADMFrequency& obj) {operator = (obj);}

  /*--------------------------------------------------------------------------------*/
  /** Return name of XML element to use
   */
  /*--------------------------------------------------------------------------------*/
  static std::string GetElementName() {return "frequency";}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  ADMFrequency& operator = (const ADMFrequency& obj);

  /*--------------------------------------------------------------------------------*/
  /** Read values from XMLValue (representing 'frequency' element)
   */
  /*--------------------------------------------------------------------------------*/
  ADMFrequency& SetValues(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Write values to XMLValue (creating 'frequency' element)
   */
  /*--------------------------------------------------------------------------------*/
  bool GetValues(XMLValue& value, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Allow access to extra values not interpreted
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValue& GetExtraValues() const   {return extravalues;}
  XMLValue&       GetExtraValuesWritable() {return extravalues;}

  // use .IsSet() to tell whether value has been set
  // use .Get() or normal casting to get value
  // use .Set() or normal assign to set value
  NAMEDPARAMETER(uint_t,      frequency);
  NAMEDPARAMETER(std::string, typeDefinition);

protected:
  XMLValue extravalues;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Class represent audioObjectInteraction
 *
 */
/*--------------------------------------------------------------------------------*/
class ADMAudioObjectInteraction
{
public:
  ADMAudioObjectInteraction() {}
  ADMAudioObjectInteraction(const ADMAudioObjectInteraction& obj) {operator = (obj);}

  /*--------------------------------------------------------------------------------*/
  /** Return name of XML element to use
   */
  /*--------------------------------------------------------------------------------*/
  static std::string GetElementName() {return "audioObjectInteraction";}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioObjectInteraction& operator = (const ADMAudioObjectInteraction& obj);

  /*--------------------------------------------------------------------------------*/
  /** Read values from XMLValue (representing 'audioObjectInteraction' element)
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioObjectInteraction& SetValues(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Write values to XMLValue (creating 'audioObjectInteraction' element)
   */
  /*--------------------------------------------------------------------------------*/
  bool GetValues(XMLValue& value, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Allow access to extra values not interpreted
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValue& GetExtraValues() const   {return extravalues;}
  XMLValue&       GetExtraValuesWritable() {return extravalues;}

  // use .IsSet() to tell whether value has been set
  // use .Get() or normal casting to get value
  // use .Set() or normal assign to set value
  NAMEDPARAMETER(bool, onOffInteract);
  NAMEDPARAMETER(bool, gainInteract);
  NAMEDPARAMETER(bool, positionInteract);

  /*--------------------------------------------------------------------------------*/
  /** Access to gainInteractionRange is direct, i.e. gainInteractionRange.min or gainInteractionRange.max
   */
  /*--------------------------------------------------------------------------------*/
  struct {
    NAMEDPARAMETER(float, min);
    NAMEDPARAMETER(float, max);
  } gainInteractionRange;
  
  /*--------------------------------------------------------------------------------*/
  /** Access to positionInteractionRange is via map with an entry per co-ordinate
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    NAMEDPARAMETER(float,       min);
    NAMEDPARAMETER(float,       max);
    NAMEDPARAMETER(std::string, coordinate);
  } PositionInteractionRange;
  typedef std::map<std::string,PositionInteractionRange> PositionInteractionRanges;

  /*--------------------------------------------------------------------------------*/
  /** Return readonly version of position interaction ranges
   *
   * PositionInteractionRanges is a map with the coordinate as the key
   */
  /*--------------------------------------------------------------------------------*/
  const PositionInteractionRanges& GetPositionInteractionRanges() const {return positionInteractionRanges;}

  /*--------------------------------------------------------------------------------*/
  /** Return readonly positionInteractionRange for specified coordinate, if it exists
   */
  /*--------------------------------------------------------------------------------*/
  const PositionInteractionRange *GetPositionInteractionRange(const std::string& coordinate) const;

  /*--------------------------------------------------------------------------------*/
  /** Return writable positionInteractionRange for specified coordinate, if it exists or creation is allowed
   */
  /*--------------------------------------------------------------------------------*/
  PositionInteractionRange *GetPositionInteractionRangeWritable(const std::string& coordinate, bool create = false);
  
protected:
  PositionInteractionRanges positionInteractionRanges;
  XMLValue extravalues;
};

BBC_AUDIOTOOLBOX_END

#endif
