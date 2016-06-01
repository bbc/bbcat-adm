
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BBCDEBUG_LEVEL 2
#include "ADMSubObjects.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
ADMLoudnessMetadata& ADMLoudnessMetadata::operator = (const ADMLoudnessMetadata& obj)
{
  loudnessMethod         = obj.loudnessMethod;
  loudnessRecType        = obj.loudnessRecType;
  loudnessCorrectionType = obj.loudnessCorrectionType;
  integratedLoudness     = obj.integratedLoudness;
  loudnessRange          = obj.loudnessRange;
  maxTruePeak            = obj.maxTruePeak;
  maxMomentary           = obj.maxMomentary;
  maxShortTerm           = obj.maxShortTerm;
  dialogLoudness         = obj.dialogLoudness;
  extravalues            = obj.extravalues;

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Read values from XMLValue (representing 'loudnessMetadata' element)
 */
/*--------------------------------------------------------------------------------*/
ADMLoudnessMetadata& ADMLoudnessMetadata::SetValues(const XMLValue& value)
{
  XMLValues *subvalues;

  // take a copy of values so that known values can be removed from it
  extravalues = value;

  // get attributes
  extravalues.GetAttributeAndErase(loudnessMethod);
  extravalues.GetAttributeAndErase(loudnessRecType);
  extravalues.GetAttributeAndErase(loudnessCorrectionType);

  if ((subvalues = extravalues.GetSubValuesWritable()) != NULL)
  {
    subvalues->GetValueAndErase(integratedLoudness);
    subvalues->GetValueAndErase(loudnessRange);
    subvalues->GetValueAndErase(maxTruePeak);
    subvalues->GetValueAndErase(maxMomentary);
    subvalues->GetValueAndErase(maxShortTerm);
    subvalues->GetValueAndErase(dialogLoudness);
    if (!subvalues->size()) extravalues.SetSubValues(NULL);
  }

  if (!extravalues.Empty()) BBCDEBUG2(("Warning: elements not decoded in loudnessMetadata element"));

  // remainder in extravalues is those attributes and values that were unknown

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Write values to XMLValue (creating 'loudnessMetadata' element)
 */
/*--------------------------------------------------------------------------------*/
bool ADMLoudnessMetadata::GetValues(XMLValue& value, bool full) const
{
  // copy existing unknown values and attributes
  value = extravalues;
  value.name = GetElementName();

  // set any attributes that are set
  value.SetAttribute(loudnessMethod, full);
  value.SetAttribute(loudnessRecType, full);
  value.SetAttribute(loudnessCorrectionType, full);

  // add any values that are set
  value.AddSubValue(integratedLoudness, full);
  value.AddSubValue(loudnessRange, full);
  value.AddSubValue(maxTruePeak, full);
  value.AddSubValue(maxMomentary, full);
  value.AddSubValue(maxShortTerm, full);
  value.AddSubValue(dialogLoudness, full);

  // return whether resultant value is non-empty
  return !value.Empty();
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
ADMAudioProgrammeReferenceScreen& ADMAudioProgrammeReferenceScreen::operator = (const ADMAudioProgrammeReferenceScreen& obj)
{
  aspectRatio = obj.aspectRatio;
  screenCentrePosition = obj.screenCentrePosition;
  screenWidth = obj.screenWidth;
  extravalues = obj.extravalues;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Read values from XMLValue (representing 'audioProgrammeReferenceScreen' element)
 */
/*--------------------------------------------------------------------------------*/
ADMAudioProgrammeReferenceScreen& ADMAudioProgrammeReferenceScreen::SetValues(const XMLValue& value)
{
  // take a copy of values so that known values can be removed from it
  extravalues = value;

  XMLValues *subvalues = extravalues.GetSubValuesWritable();
  if (subvalues)
  {
    XMLValue *subvalue;

    subvalues->GetValueAndErase(aspectRatio);

    if ((subvalue = subvalues->GetValueWritable("screenCentrePosition")) != NULL)
    {
      subvalue->GetAttributeAndErase(screenCentrePosition.azimuth);
      subvalue->GetAttributeAndErase(screenCentrePosition.elevation);
      subvalue->GetAttributeAndErase(screenCentrePosition.distance);
      subvalue->GetAttributeAndErase(screenCentrePosition.X);
      subvalue->GetAttributeAndErase(screenCentrePosition.Y);
      subvalue->GetAttributeAndErase(screenCentrePosition.Z);
      // if all attributes have been removed, delete subvalue
      if (!subvalue->HasAttributes()) subvalues->EraseValue(subvalue);
    }
    if ((subvalue = subvalues->GetValueWritable("screenWidth")) != NULL)
    {
      subvalue->GetAttributeAndErase(screenWidth.azimuth);
      subvalue->GetAttributeAndErase(screenWidth.X);
      // if all attributes have been removed, delete subvalue
      if (!subvalue->HasAttributes()) subvalues->EraseValue(subvalue);
    }
    // if subvalues list is now empty, delete it
    if (!subvalues->size()) extravalues.SetSubValues(NULL);
  }

  if (!extravalues.Empty()) BBCDEBUG2(("Warning: elements not decoded in %s element", GetElementName().c_str()));

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Write values to XMLValue (creating 'audioProgrammeReferenceScreen' element)
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioProgrammeReferenceScreen::GetValues(XMLValue& value, bool full) const
{
  // copy existing unknown values and attributes
  value = extravalues;
  value.name = GetElementName();

  value.AddSubValue(aspectRatio);

  {
    XMLValue subvalue;
    subvalue.name = "screenCentrePosition";
    subvalue.SetAttribute(screenCentrePosition.azimuth, full);
    subvalue.SetAttribute(screenCentrePosition.elevation, full);
    subvalue.SetAttribute(screenCentrePosition.distance, full);
    subvalue.SetAttribute(screenCentrePosition.X, full);
    subvalue.SetAttribute(screenCentrePosition.Y, full);
    subvalue.SetAttribute(screenCentrePosition.Z, full);
    if (subvalue.HasAttributes()) value.AddSubValue(subvalue);
  }
  {
    XMLValue subvalue;
    subvalue.name = "screenWidth";
    subvalue.SetAttribute(screenWidth.azimuth, full);
    subvalue.SetAttribute(screenWidth.X, full);
    if (subvalue.HasAttributes()) value.AddSubValue(subvalue);
  }

  // return whether resultant value is non-empty
  return !value.Empty();
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
ADMAudioContentDialogue& ADMAudioContentDialogue::operator = (const ADMAudioContentDialogue& obj)
{
  nonDialogueContentKind = obj.nonDialogueContentKind;
  dialogueContentKind    = obj.dialogueContentKind;
  mixedContentKind       = obj.mixedContentKind;
  extravalues            = obj.extravalues;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Read values from XMLValue (representing 'dialogue' element)
 */
/*--------------------------------------------------------------------------------*/
ADMAudioContentDialogue& ADMAudioContentDialogue::SetValues(const XMLValue& value)
{
  // take a copy of values so that known values can be removed from it
  extravalues = value;

  // read and then erase dialogue value
  dialogue.FromString(extravalues.value); extravalues.value = "";
  extravalues.GetAttributeAndErase(nonDialogueContentKind);
  extravalues.GetAttributeAndErase(dialogueContentKind);
  extravalues.GetAttributeAndErase(mixedContentKind);

  if (!extravalues.Empty()) BBCDEBUG2(("Warning: elements not decoded in %s element", GetElementName().c_str()));

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Write values to XMLValue (creating 'dialogue' element)
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioContentDialogue::GetValues(XMLValue& value, bool full) const
{
  // copy existing unknown values and attributes
  value = extravalues;
  value.name = GetElementName();

  if (full || dialogue.IsSet())
  {
    value.value = dialogue.ToString();
    value.SetAttribute(nonDialogueContentKind, full);
    value.SetAttribute(dialogueContentKind, full);
    value.SetAttribute(mixedContentKind, full);
  }
  
  // return whether resultant value is non-empty
  return !value.Empty();
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
ADMFrequency& ADMFrequency::operator = (const ADMFrequency& obj)
{
  frequency      = obj.frequency;
  typeDefinition = obj.typeDefinition;
  extravalues    = obj.extravalues;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Read values from XMLValue (representing 'frequency' element)
 */
/*--------------------------------------------------------------------------------*/
ADMFrequency& ADMFrequency::SetValues(const XMLValue& value)
{
  // take a copy of values so that known values can be removed from it
  extravalues = value;

  // read and then erase frequency value
  frequency.FromString(extravalues.value); extravalues.value = "";
  extravalues.GetAttributeAndErase(typeDefinition);

  if (!extravalues.Empty()) BBCDEBUG2(("Warning: elements not decoded in %s element", GetElementName().c_str()));

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Write values to XMLValue (creating 'frequency' element)
 */
/*--------------------------------------------------------------------------------*/
bool ADMFrequency::GetValues(XMLValue& value, bool full) const
{
  // copy existing unknown values and attributes
  value = extravalues;
  value.name = GetElementName();

  if (full || frequency.IsSet())
  {
    value.value = frequency.ToString();
    value.SetAttribute(typeDefinition, full);
  }
  
  // return whether resultant value is non-empty
  return !value.Empty();
}

/*----------------------------------------------------------------------------------------------------*/

static const std::string GainInteractionRangeName     = "gainInteractionRange";
static const std::string PositionInteractionRangeName = "positionInteractionRange";
static const std::string bound_name                   = "bound";
static const std::string bound_min                    = "min";
static const std::string bound_max                    = "max";
static const std::string coordinate_name              = "coordinate";

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObjectInteraction& ADMAudioObjectInteraction::operator = (const ADMAudioObjectInteraction& obj)
{
  onOffInteract             = obj.onOffInteract;
  gainInteract              = obj.gainInteract;
  positionInteract          = obj.positionInteract;
  gainInteractionRange      = obj.gainInteractionRange;
  positionInteractionRanges = obj.positionInteractionRanges;
  extravalues               = obj.extravalues;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Read values from XMLValue (representing 'audioObjectInteraction' element)
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObjectInteraction& ADMAudioObjectInteraction::SetValues(const XMLValue& value)
{
  XMLValues *subvalues;
  
  // take a copy of values so that known values can be removed from it
  extravalues = value;

  extravalues.GetAttributeAndErase(onOffInteract);
  extravalues.GetAttributeAndErase(gainInteract);
  extravalues.GetAttributeAndErase(positionInteract);

  // access list of subvalues
  if ((subvalues = extravalues.GetSubValuesWritable()) != NULL)
  {
    XMLValue *subvalue;
    
    // find and process all gainInteractionRange entries
    while ((subvalue = subvalues->GetValueWritable(GainInteractionRangeName)) != NULL)
    {
      // only process non-empty values
      if (!subvalue->Empty())
      {
        const std::string *bound;
        
        if ((bound = subvalue->GetAttribute(bound_name)) != NULL)
        {
          // check for valid bound value
          if      (*bound == bound_min) gainInteractionRange.min.FromString(subvalue->value);
          else if (*bound == bound_max) gainInteractionRange.max.FromString(subvalue->value);
          else BBCERROR("Unrecognized bound for gainInteractionRange of '%s'", bound->c_str());
        }
        else BBCERROR("No bound specified for gainInteractionRange");
      }
    
      // erase value from main list
      subvalues->EraseValue(subvalue);
    }

    // find and process all positionInteractionRange entries
    while ((subvalue = subvalues->GetValueWritable(PositionInteractionRangeName)) != NULL)
    {
      // only process non-empty values
      if (!subvalue->Empty())
      {
        const std::string *bound;
        const std::string *coordinate;
        
        if (((bound      = subvalue->GetAttribute(bound_name))      != NULL) &&
            ((coordinate = subvalue->GetAttribute(coordinate_name)) != NULL))
        {
          // check for valid bound value
          if ((*bound == bound_min) || (*bound == bound_max))
          {
            // NOTE: coordinate is unchecked
            // get positionInteractionRange for coordinate, creating if necessary
            PositionInteractionRange *range = GetPositionInteractionRangeWritable(*coordinate, true);

            if (range)
            {
              if      (*bound == bound_min) range->min.FromString(subvalue->value);
              else if (*bound == bound_max) range->max.FromString(subvalue->value);
            }
            else BBCERROR("No positionInteractionRange created for bound '%s' and coordinate '%s'", bound->c_str(), coordinate->c_str());
          }
          else BBCERROR("Unrecognized bound for positionInteractionRange of '%s'", bound->c_str());
        }
        else BBCERROR("No bound or coordinate specified for positionInteractionRange");
      }
      
      // erase value from main list
      subvalues->EraseValue(subvalue);
    }
  }
  
  if (!extravalues.Empty()) BBCDEBUG2(("Warning: elements not decoded in %s element", GetElementName().c_str()));

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Write values to XMLValue (creating 'audioObjectInteraction' element)
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioObjectInteraction::GetValues(XMLValue& value, bool full) const
{
  // copy existing unknown values and attributes
  value = extravalues;
  value.name = GetElementName();

  value.SetAttribute(onOffInteract, full);
  value.SetAttribute(gainInteract, full);
  value.SetAttribute(positionInteract, full);

  if (full || gainInteractionRange.min.IsSet()) {
    XMLValue subvalue;

    // create value representing this gainInteractionRange min
    subvalue.SetValue(GainInteractionRangeName, gainInteractionRange.min);
    subvalue.SetAttribute(bound_name, bound_min);

    value.AddSubValue(subvalue);
  }

  if (full || gainInteractionRange.max.IsSet()) {
    XMLValue subvalue;

    // create value representing this gainInteractionRange max
    subvalue.SetValue(GainInteractionRangeName, gainInteractionRange.max);
    subvalue.SetAttribute(bound_name, bound_max);

    value.AddSubValue(subvalue);
  }

  {
    PositionInteractionRanges::const_iterator it;

    // iterate through each positionInteractionRange object
    for (it = positionInteractionRanges.begin(); it != positionInteractionRanges.end(); ++it)
    {
      const PositionInteractionRange& range = it->second;

      if (full || range.min.IsSet()) {
        XMLValue subvalue;

        // create value representing this positionInteractionRange min
        subvalue.SetValue(PositionInteractionRangeName, range.min);
        subvalue.SetAttribute(bound_name, bound_min);
        subvalue.SetAttribute(coordinate_name, it->first);
        
        value.AddSubValue(subvalue);
      }

      if (full || range.max.IsSet()) {
        XMLValue subvalue;

        // create value representing this positionInteractionRange max
        subvalue.SetValue(PositionInteractionRangeName, range.max);
        subvalue.SetAttribute(bound_name, bound_max);
        subvalue.SetAttribute(coordinate_name, it->first);
        
        value.AddSubValue(subvalue);
      }
    }
  }
  
  // return whether resultant value is non-empty
  return !value.Empty();
}

/*--------------------------------------------------------------------------------*/
/** Return readonly positionInteractionRange for specified coordinate, if it exists
 */
/*--------------------------------------------------------------------------------*/
const ADMAudioObjectInteraction::PositionInteractionRange *ADMAudioObjectInteraction::GetPositionInteractionRange(const std::string& coordinate) const
{
  PositionInteractionRanges::const_iterator it;
  const PositionInteractionRange *range = NULL;

  // attempt to find correct positionInteractionRange
  if ((it = positionInteractionRanges.find(coordinate)) != positionInteractionRanges.end()) range = &it->second;

  return range;
}

/*--------------------------------------------------------------------------------*/
/** Return writable positionInteractionRange for specified coordinate, if it exists or creation is allowed
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObjectInteraction::PositionInteractionRange *ADMAudioObjectInteraction::GetPositionInteractionRangeWritable(const std::string& coordinate, bool create)
{
  PositionInteractionRanges::iterator it;
  PositionInteractionRange *range = NULL;

  // attempt to find correct positionInteractionRange
  if (((it = positionInteractionRanges.find(coordinate)) == positionInteractionRanges.end()) && create)
  {
    // if entry doesn't exist but creation is allowed, create new entry
    PositionInteractionRange range;

    range.coordinate = coordinate;
    positionInteractionRanges[coordinate] = range;

    // and find its location
    it = positionInteractionRanges.find(coordinate);
  }

  // if found, get address of range
  if (it != positionInteractionRanges.end()) range = &it->second;

  return range;
}
  

BBC_AUDIOTOOLBOX_END
