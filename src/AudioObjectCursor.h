#ifndef __AUDIO_OBJECT_CURSOR__
#define __AUDIO_OBJECT_CURSOR__

//PUBLIBS:#include "AudioObject.h"
#include "AudioObjectParameters.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Base class for the tracking of audio object parameters as they change over time
 *
 * Typically, an instance of a derived version of this class would be used for each track
 */
/*--------------------------------------------------------------------------------*/
class ADMAudioObject;
class AudioObjectCursor
{
public:
  AudioObjectCursor() {}
  virtual ~AudioObjectCursor() {}

  /*--------------------------------------------------------------------------------*/
  /** Return cursor start time in ns
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint64_t GetStartTime() const {return 0;}

  /*--------------------------------------------------------------------------------*/
  /** Return cursor end time in ns
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint64_t GetEndTime() const {return 0;}

  /*--------------------------------------------------------------------------------*/
  /** Seek cursor to specified time (ns)
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Seek(uint64_t t) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return channel for this cursor
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t GetChannel() const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Get current audio object
   */
  /*--------------------------------------------------------------------------------*/
  virtual ADMAudioObject *GetAudioObject() const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Return audio object parameters at current time
   *
   * @return true if object parameters are valid and returned in currentparameters
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool GetObjectParameters(AudioObjectParameters& currentparameters) const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Set audio object parameters for current time
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetObjectParameters(const AudioObjectParameters& newparameters) {
    UNUSED_PARAMETER(newparameters);
  }

  /*--------------------------------------------------------------------------------*/
  /** End parameters updates by marking the end of the last block
   */
  /*--------------------------------------------------------------------------------*/
  virtual void EndChanges() {}

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON object
   */
  /*--------------------------------------------------------------------------------*/
  virtual json_spirit::mObject ToJSON() const {json_spirit::mObject obj; obj["parameters"] = ToJSONArray(); return obj;}

  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON array
   */
  /*--------------------------------------------------------------------------------*/
  virtual json_spirit::mArray ToJSONArray() const {json_spirit::mArray array; return array;}

  /*--------------------------------------------------------------------------------*/
  /** Operator overload
   */
  /*--------------------------------------------------------------------------------*/
  operator json_spirit::mObject() const {return ToJSON();}
 
  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON string
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToJSONString() const {return json_spirit::write(ToJSON(), json_spirit::pretty_print);}
#endif
};

BBC_AUDIOTOOLBOX_END

#endif
