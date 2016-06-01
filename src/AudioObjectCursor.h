#ifndef __AUDIO_OBJECT_CURSOR__
#define __AUDIO_OBJECT_CURSOR__

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
};

BBC_AUDIOTOOLBOX_END

#endif
