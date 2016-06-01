#ifndef __AUDIO_OBJECT_PARAMETERS__
#define __AUDIO_OBJECT_PARAMETERS__

#include <bbcat-base/3DPosition.h>
#include <bbcat-base/NamedParameter.h>
#include <bbcat-base/ParameterSet.h>
#include <bbcat-base/RefCount.h>

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A class containing the parameters for rendering audio objects
 *
 * Each input channel to the renderer requires one of these objects which may change
 * over time as parameters for that channel change
 *
 * @note ADM parameters are MAPPED into these parameters - there is not necessarily
 *       a one-to-one relationship
 * 
 * For example:
 * The ADM parameter jumpPosition is defined as:
 *   If set to 1 the position will be interpolated over a period set by
 *   the attribute interpolationLength. If set to 0 then interpolation
 *   will take the entire length of the block. An interpolationLength
 *   value of zero will mean the object jumps without interpolation.
 *
 * jumpPosition and interpolationLength are mapped to interpolate and
 * interpolationtime and interpreted strictly by the rest of the
 * system (i.e. if interpolate is true then the interpolation time is
 * interpolationtime).  To achieve this:
 *
 * interpolationtime = jumpPosition ? interpolationLength : duration
 * interpolate       = (interpolationtime > 0)
 *
 * And the other way:
 * jumpPosition        = !interpolate || (interpolate && (interpolationtime != duration))
 * interpolationLength = interpolate ? interpolationtime : 0
 */
/*--------------------------------------------------------------------------------*/
class ADMAudioObject;
class AudioObjectParameters : public JSONSerializable
{
public:
  AudioObjectParameters();
  AudioObjectParameters(const AudioObjectParameters& obj);
#if ENABLE_JSON
  AudioObjectParameters(const JSONValue& obj);
#endif
  virtual ~AudioObjectParameters();

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  virtual AudioObjectParameters& operator = (const AudioObjectParameters& obj);

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Return object as JSON object
   *
   * @param force true to force setting of JSON members from internal values even if they have NOT been set
   */
  /*--------------------------------------------------------------------------------*/
  using JSONSerializable::ToJSON;
  virtual void ToJSON(JSONValue& obj, bool force) const;
  virtual void ToJSON(JSONValue& obj) const {ToJSON(obj, false);}
    
  /*--------------------------------------------------------------------------------*/
  /** Set object from JSON
   *
   * @param reset true to reset any parameters NOT found in JSON to their defaults
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool FromJSON(const JSONValue& obj, bool reset);
  virtual bool FromJSON(const JSONValue& obj) {return FromJSON(obj, true);}
  virtual AudioObjectParameters& FromJSONEx(const JSONValue& obj, bool reset = true) {FromJSON(obj, reset); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  virtual AudioObjectParameters& operator = (const JSONValue& obj) {FromJSON(obj); return *this;}
#endif

  /*--------------------------------------------------------------------------------*/
  /** Comparison operator
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool operator == (const AudioObjectParameters& obj) const;
  virtual bool operator != (const AudioObjectParameters& obj) const {return !operator == (obj);}

  /*--------------------------------------------------------------------------------*/
  /** Merge another AudioObjectParameters into this one
   *
   * @note any values set in obj will over-write the ones in this object
   */
  /*--------------------------------------------------------------------------------*/
  virtual AudioObjectParameters& Merge(const AudioObjectParameters& obj);
  
  /*--------------------------------------------------------------------------------*/
  /** Transform this object's position and return new copy
   */
  /*--------------------------------------------------------------------------------*/
  friend AudioObjectParameters operator * (const AudioObjectParameters& obj, const PositionTransform& transform);

  /*--------------------------------------------------------------------------------*/
  /** Transform this object's position
   */
  /*--------------------------------------------------------------------------------*/
  AudioObjectParameters& operator *= (const PositionTransform& transform);

  /*--------------------------------------------------------------------------------*/
  /** Get/Set channel
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetChannel()            const {return values.channel;}
  bool   GetChannel(uint_t& val) const {return GetParameter<>(Parameter_channel, values.channel, val);}
  bool   IsChannelSet()          const {return IsParameterSet(Parameter_channel);}
  void   SetChannel(uint_t  val)       {SetParameter<>(Parameter_channel, values.channel, val);}
  void   ResetChannel()                {ResetParameter<>(Parameter_channel, values.channel);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set block duration (ns by default)
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GetDuration()              const {return values.duration;}
  double   GetDurationS()             const {return ConvertNSToS(values.duration);}
  bool     GetDuration(uint64_t& val) const {return GetParameter<>(Parameter_duration, values.duration, val);}
  bool     GetDurationS(double&  val) const {return GetParameter<>(Parameter_duration, values.duration, val, &ConvertNSToS);}
  bool     IsDurationSet()            const {return IsParameterSet(Parameter_duration);}
  void     SetDuration(uint64_t  val)       {SetParameter<>(Parameter_duration, values.duration, val);}
  void     SetDurationS(double   val)       {SetParameter<>(Parameter_duration, values.duration, val, &ConvertSToNS);}
  void     ResetDuration()                  {ResetParameter<>(Parameter_duration, values.duration);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set physical position of this object
   *
   * @note position information is required for every channel
   */
  /*--------------------------------------------------------------------------------*/
  const Position& GetPosition()                    const {return position;}
  bool            GetPosition(Position& val)       const {return GetParameter<>(Parameter_position, position, val);}
  bool            IsPositionSet()                  const {return IsParameterSet(Parameter_position);}
  void            SetPosition(const Position& val)       {SetParameter<>(Parameter_position, position, val);}
  void            ResetPosition()                        {ResetParameter<>(Parameter_position, position);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set minimum physical position of this object
   *
   * @note position information is required for every channel
   */
  /*--------------------------------------------------------------------------------*/
  const Position& GetMinPosition()                    const {return minposition ? *minposition : nullposition;}
  bool            GetMinPosition(Position& val)       const {return GetParameter<>(Parameter_minposition, position, val);}
  bool            IsMinPositionSet()                  const {return IsParameterSet(Parameter_minposition);}
  void            SetMinPosition(const Position& val)       {SetParameter<>(Parameter_minposition, &minposition, val);}
  void            ResetMinPosition()                        {ResetParameter<>(Parameter_minposition, &minposition);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set maximum physical position of this object
   *
   * @note position information is required for every channel
   */
  /*--------------------------------------------------------------------------------*/
  const Position& GetMaxPosition()                    const {return maxposition ? *maxposition : nullposition;}
  bool            GetMaxPosition(Position& val)       const {return GetParameter<>(Parameter_maxposition, position, val);}
  bool            IsMaxPositionSet()                  const {return IsParameterSet(Parameter_maxposition);}
  void            SetMaxPosition(const Position& val)       {SetParameter<>(Parameter_maxposition, &maxposition, val);}
  void            ResetMaxPosition()                        {ResetParameter<>(Parameter_maxposition, &maxposition);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set screen edge lock for co-ordinate
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetScreenEdgeLock(const std::string& coordinate)                         const {return GetOtherValue(GetScreenEdgeLockKey(coordinate));}
  bool        GetScreenEdgeLock(const std::string& coordinate, std::string& val)       const {return GetOtherValue(GetScreenEdgeLockKey(coordinate), val);}
  bool        IsScreenEdgeLockSet(const std::string& coordinate)                       const {return IsOtherValueSet(GetScreenEdgeLockKey(coordinate));}
  void        SetScreenEdgeLock(const std::string& coordinate, const std::string& val)       {SetOtherValue(GetScreenEdgeLockKey(coordinate), val);}
  void        ResetScreenEdgeLock(const std::string& coordinate)                             {ResetOtherValue(GetScreenEdgeLockKey(coordinate));}
  static bool IsScreenEdgeLockValue(const std::string& val)                                  {return (val.find("screenedgelock.") == 0);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set cartesian
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetCartesian()          const {return (values.cartesian != 0);}
  bool   GetCartesian(bool& val) const {return GetBoolParameter<>(Parameter_cartesian, values.cartesian, val);}
  bool   IsCartesianSet()        const {return IsParameterSet(Parameter_cartesian);}
  void   SetCartesian(bool  val)       {SetParameter<>(Parameter_cartesian, values.cartesian, val);}
  void   ResetCartesian()              {ResetParameter<>(Parameter_cartesian, values.cartesian);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set gain
   */
  /*--------------------------------------------------------------------------------*/
  double GetGain()            const {return values.gain;}
  bool   GetGain(double& val) const {return GetParameter<>(Parameter_gain, values.gain, val);}
  bool   IsGainSet()          const {return IsParameterSet(Parameter_gain);}
  void   SetGain(double  val)       {SetParameter<>(Parameter_gain, values.gain, val);}
  void   ResetGain()                {ResetParameter<>(Parameter_gain, values.gain, 1.0);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set width
   */
  /*--------------------------------------------------------------------------------*/
  float  GetWidth()           const {return values.width;}
  bool   GetWidth(float& val) const {return GetParameter<>(Parameter_width, values.width, val);}
  bool   IsWidthSet()         const {return IsParameterSet(Parameter_width);}
  void   SetWidth(float  val)       {SetParameter<>(Parameter_width, values.width, val, &Limit0f);}
  void   ResetWidth()               {ResetParameter<>(Parameter_width, values.width);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set depth
   */
  /*--------------------------------------------------------------------------------*/
  float  GetDepth()           const {return values.depth;}
  bool   GetDepth(float& val) const {return GetParameter<>(Parameter_depth, values.depth, val);}
  bool   IsDepthSet()         const {return IsParameterSet(Parameter_depth);}
  void   SetDepth(float  val)       {SetParameter<>(Parameter_depth, values.depth, val, &Limit0f);}
  void   ResetDepth()               {ResetParameter<>(Parameter_depth, values.depth);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set height
   */
  /*--------------------------------------------------------------------------------*/
  float  GetHeight()           const {return values.height;}
  bool   GetHeight(float& val) const {return GetParameter<>(Parameter_height, values.height, val);}
  bool   IsHeightSet()         const {return IsParameterSet(Parameter_height);}
  void   SetHeight(float  val)       {SetParameter<>(Parameter_height, values.height, val, &Limit0f);}
  void   ResetHeight()               {ResetParameter<>(Parameter_height, values.height);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set divergence balance
   */
  /*--------------------------------------------------------------------------------*/
  float  GetDivergenceBalance()           const {return values.divergencebalance;}
  bool   GetDivergenceBalance(float& val) const {return GetParameter<>(Parameter_divergencebalance, values.divergencebalance, val);}
  bool   IsDivergenceBalanceSet()         const {return IsParameterSet(Parameter_divergencebalance);}
  void   SetDivergenceBalance(float  val)       {SetParameter<>(Parameter_divergencebalance, values.divergencebalance, val, &Limit0to1f);}
  void   ResetDivergenceBalance()               {ResetParameter<>(Parameter_divergencebalance, values.divergencebalance);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set divergence azimuth
   */
  /*--------------------------------------------------------------------------------*/
  float  GetDivergenceAzimuth()           const {return values.divergenceazimuth;}
  bool   GetDivergenceAzimuth(float& val) const {return GetParameter<>(Parameter_divergenceazimuth, values.divergenceazimuth, val);}
  bool   IsDivergenceAzimuthSet()         const {return IsParameterSet(Parameter_divergenceazimuth);}
  void   SetDivergenceAzimuth(float  val)       {SetParameter<>(Parameter_divergenceazimuth, values.divergenceazimuth, val, &Limit0f);}
  void   ResetDivergenceAzimuth()               {ResetParameter<>(Parameter_divergenceazimuth, values.divergenceazimuth);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set diffuseness
   */
  /*--------------------------------------------------------------------------------*/
  float  GetDiffuseness()           const {return values.diffuseness;}
  bool   GetDiffuseness(float& val) const {return GetParameter<>(Parameter_diffuseness, values.diffuseness, val);}
  bool   IsDiffusenessSet()         const {return IsParameterSet(Parameter_diffuseness);}
  void   SetDiffuseness(float  val)       {SetParameter<>(Parameter_diffuseness, values.diffuseness, val, Limit0to1f);}
  void   ResetDiffuseness()               {ResetParameter<>(Parameter_diffuseness, values.diffuseness);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set delay
   */
  /*--------------------------------------------------------------------------------*/
  float  GetDelay()           const {return values.delay;}
  bool   GetDelay(float& val) const {return GetParameter<>(Parameter_delay, values.delay, val);}
  bool   IsDelaySet()         const {return IsParameterSet(Parameter_delay);}
  void   SetDelay(float  val)       {SetParameter<>(Parameter_delay, values.delay, val, &Limit0f);}
  void   ResetDelay()               {ResetParameter<>(Parameter_delay, values.delay);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set channel importance
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetChannelImportance()            const {return values.channelimportance;}
  bool   GetChannelImportance(uint_t& val) const {return GetParameter<>(Parameter_channelimportance, values.channelimportance, val);}
  bool   IsChannelImportanceSet()          const {return IsParameterSet(Parameter_channelimportance);}
  void   SetChannelImportance(uint_t  val)       {SetParameter<>(Parameter_channelimportance, values.channelimportance, val, &LimitImportance);}
  void   ResetChannelImportance()                {ResetParameter<>(Parameter_channelimportance, values.channelimportance, GetChannelImportanceDefault());}
  static uint_t GetChannelImportanceDefault()    {return 10;}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set object importance
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetObjectImportance()            const {return values.objectimportance;}
  bool   GetObjectImportance(uint_t& val) const {return GetParameter<>(Parameter_objectimportance, values.objectimportance, val);}
  bool   IsObjectImportanceSet()          const {return IsParameterSet(Parameter_objectimportance);}
  void   SetObjectImportance(uint_t  val)       {SetParameter<>(Parameter_objectimportance, values.objectimportance, val, &LimitImportance);}
  void   ResetObjectImportance()                {ResetParameter<>(Parameter_objectimportance, values.objectimportance, GetObjectImportanceDefault());}
  static uint_t GetObjectImportanceDefault()    {return 10;}
  
  /*--------------------------------------------------------------------------------*/
  /** Get/Set dialogue
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetDialogue()            const {return values.dialogue;}
  bool   GetDialogue(uint_t& val) const {return GetParameter<>(Parameter_dialogue, values.dialogue, val);}
  bool   IsDialogueSet()          const {return IsParameterSet(Parameter_dialogue);}
  void   SetDialogue(uint_t  val)       {SetParameter<>(Parameter_dialogue, values.dialogue, val, &LimitDialogue);}
  void   ResetDialogue()                {ResetParameter<>(Parameter_dialogue, values.dialogue, GetDialogueDefault());}
  static uint_t GetDialogueDefault()    {return 0;}
    
  /*--------------------------------------------------------------------------------*/
  /** Get/Set channellock
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetChannelLock()          const {return (values.channellock != 0);}
  bool   GetChannelLock(bool& val) const {return GetBoolParameter<>(Parameter_channellock, values.channellock, val);}
  bool   IsChannelLockSet()        const {return IsParameterSet(Parameter_channellock);}
  void   SetChannelLock(bool  val)       {SetParameter<>(Parameter_channellock, values.channellock, val);}
  void   ResetChannelLock()              {ResetParameter<>(Parameter_channellock, values.channellock);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set channellock maxdistance
   */
  /*--------------------------------------------------------------------------------*/
  float  GetChannelLockMaxDistance()           const {return values.channellockmaxdistance;}
  bool   GetChannelLockMaxDistance(float& val) const {return GetParameter<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, val);}
  bool   IsChannelLockMaxDistanceSet()         const {return IsParameterSet(Parameter_channellockmaxdistance);}
  void   SetChannelLockMaxDistance(float  val)       {SetParameter<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, val, LimitMaxDistance);}
  void   ResetChannelLockMaxDistance()               {ResetParameter<>(Parameter_channellockmaxdistance, values.channellockmaxdistance);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set interact
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetInteract()          const {return (values.interact != 0);}
  bool   GetInteract(bool& val) const {return GetBoolParameter<>(Parameter_interact, values.interact, val);}
  bool   IsInteractSet()        const {return IsParameterSet(Parameter_interact);}
  void   SetInteract(bool  val)       {SetParameter<>(Parameter_interact, values.interact, val);}
  void   ResetInteract()              {ResetParameter<>(Parameter_interact, values.interact, GetInteractDefault());}
  static bool GetInteractDefault()    {return false;}
  
  /*--------------------------------------------------------------------------------*/
  /** Get/Set interpolate
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetInterpolate()          const {return (values.interpolate != 0);}
  bool   GetInterpolate(bool& val) const {return GetBoolParameter<>(Parameter_interpolate, values.interpolate, val);}
  bool   IsInterpolateSet()        const {return IsParameterSet(Parameter_interpolate);}
  void   SetInterpolate(bool  val)       {SetParameter<>(Parameter_interpolate, values.interpolate, val);}
  void   ResetInterpolate()              {ResetParameter<>(Parameter_interpolate, values.interpolate, true);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set interpolation time (ns by default)
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GetInterpolationTime()              const {return values.interpolationtime;}
  double   GetInterpolationTimeS()             const {return ConvertNSToS(values.interpolationtime);}
  bool     GetInterpolationTime(uint64_t& val) const {return GetParameter<>(Parameter_interpolationtime, values.interpolationtime, val);}
  bool     GetInterpolationTimeS(double&  val) const {return GetParameter<>(Parameter_interpolationtime, values.interpolationtime, val, &ConvertNSToS);}
  bool     IsInterpolationTimeSet()            const {return IsParameterSet(Parameter_interpolationtime);}
  void     SetInterpolationTime(uint64_t  val)       {SetParameter<>(Parameter_interpolationtime, values.interpolationtime, val);}
  void     SetInterpolationTimeS(double   val)       {SetParameter<>(Parameter_interpolationtime, values.interpolationtime, val, &ConvertSToNS);}
  void     ResetInterpolationTime()                  {ResetParameter<>(Parameter_interpolationtime, values.interpolationtime);}

  /*--------------------------------------------------------------------------------*/
  /** Return actual interpolation time in ns
   *
   * @note a return value of 0 means no interpolation
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GetActualInterpolationTime() const {return GetInterpolate() ? GetInterpolationTime() : 0;}

  /*--------------------------------------------------------------------------------*/
  /** Get equivalent ADM parameters jumpPosition and interpolationLength
   *
   * @param jumpPosition value to be updated with jumpPosition value
   * @param interpolationLength optional pointer to value to be updated with interpolationLength (valid ONLY IF jumpPosition == true)
   *
   * @return true if jumpPosition is valid (has been set)
   */
  /*--------------------------------------------------------------------------------*/
  bool GetJumpPosition(bool& jumpPosition, double *interpolationLength = NULL) const;

  /*--------------------------------------------------------------------------------*/
  /** Set interpolation parameters via jumpPosition and interpolationLength ADM parameters
   */
  /*--------------------------------------------------------------------------------*/
  void SetJumpPosition(bool jumpPosition, double interpolationLength);
  
  /*--------------------------------------------------------------------------------*/
  /** Get/Set onscreen
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetOnScreen()          const {return (values.onscreen != 0);}
  bool   GetOnScreen(bool& val) const {return GetBoolParameter<>(Parameter_onscreen, values.onscreen, val);}
  bool   IsOnScreenSet()        const {return IsParameterSet(Parameter_onscreen);}
  void   SetOnScreen(bool val)        {SetParameter<>(Parameter_onscreen, values.onscreen, val);}
  void   ResetOnScreen()              {ResetParameter<>(Parameter_onscreen, values.onscreen);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set disableducking
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetDisableDucking()          const {return (values.disableducking != 0);}
  bool   GetDisableDucking(bool& val) const {return GetBoolParameter<>(Parameter_disableducking, values.disableducking, val);}
  bool   IsDisableDuckingSet()        const {return IsParameterSet(Parameter_disableducking);}
  void   SetDisableDucking(bool val)        {SetParameter<>(Parameter_disableducking, values.disableducking, val);}
  void   ResetDisableDucking()              {ResetParameter<>(Parameter_disableducking, values.disableducking, GetDisableDuckingDefault());}
  static bool GetDisableDuckingDefault()    {return false;}

  /*--------------------------------------------------------------------------------*/
  /** Scale position and extent by scene size
   *
   * @note polar positions may have their angles altered by this if width != height != depth
   * @note excluded zones will be affected by these functions
   */
  /*--------------------------------------------------------------------------------*/
  void DivideByScene(float width, float height, float depth);
  void MultiplyByScene(float width, float height, float depth);
    
  /*--------------------------------------------------------------------------------*/
  /** Get/Set supplementary data
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetOtherValue(const std::string& name)                   const {std::string val; othervalues.Get(name, val); return val;}
  bool        GetOtherValue(const std::string& name, std::string& val) const {return othervalues.Get(name, val);}
  bool        IsOtherValueSet(const std::string& name)                 const {return othervalues.Exists(name);}
  void        ResetOtherValue(const std::string& name)                       {othervalues.Delete(name); MarkParameterSet(Parameter_othervalues, !othervalues.IsEmpty());}

  template<typename T>
  bool        GetOtherValue(const std::string& name, T& val)       const {return othervalues.Get(name, val);}

  template<typename T>
  void        SetOtherValue(const std::string& name, const T& val) {othervalues.Set(name, val); MarkParameterSet(Parameter_othervalues);}

  /*--------------------------------------------------------------------------------*/
  /** Get/Set entire set of othervalues
   *
   * @note Set() is *additive*
   */
  /*--------------------------------------------------------------------------------*/
  const ParameterSet&    GetOtherValues()                           const {return othervalues;}
  void                   SetOtherValues(const ParameterSet& values)       {othervalues += values; MarkParameterSet(Parameter_othervalues, !othervalues.IsEmpty());}
  void                   ResetOtherValues()                               {ResetParameter<>(Parameter_othervalues, othervalues);}
  
  /*--------------------------------------------------------------------------------*/
  /** Access iterators for othervalues
   */
  /*--------------------------------------------------------------------------------*/
  ParameterSet::Iterator GetOtherValuesBegin() const {return othervalues.GetBegin();}
  ParameterSet::Iterator GetOtherValuesEnd()   const {return othervalues.GetEnd();}
  
  /*--------------------------------------------------------------------------------*/
  /** Return whether *any* parameters have been set
   */
  /*--------------------------------------------------------------------------------*/
  bool AnyParametersSet() const {return (setbitmap != 0);}

  /*--------------------------------------------------------------------------------*/
  /** Sub class describing an excluded zone supporting the zoneExclusion ADM parameter
   */
  /*--------------------------------------------------------------------------------*/
  class ExcludedZone
  {
  public:
	ExcludedZone() : next(NULL),
					 minx(0.0),
					 miny(0.0),
					 minz(0.0),
					 maxx(0.0),
					 maxy(0.0),
					 maxz(0.0) {}
	ExcludedZone(ExcludedZone& obj)
	{
	  name = obj.name;
	  minx = obj.minx;
	  maxx = obj.maxx;
	  miny = obj.miny;
	  maxy = obj.maxy;
	  minz = obj.minz;
	  maxz = obj.maxz;
	  // also copy child object (which will in turn copy its child and so on)
	  if (obj.next) next = new ExcludedZone(*obj.next);
	  else			next = NULL;	// end of chain
	}
	~ExcludedZone() {if (next) delete next;}	// automatic deletion of child object

	/*--------------------------------------------------------------------------------*/
	/** Comparison operator
	 */
	/*--------------------------------------------------------------------------------*/
	bool operator == (const ExcludedZone& zone) const
	{
	  return ((name == zone.name) &&
			  (minx == zone.minx) &&
			  (miny == zone.miny) &&
			  (minz == zone.minz) &&
			  (maxx == zone.maxx) &&
			  (maxy == zone.maxy) &&
			  (maxz == zone.maxz));
	}

	/*--------------------------------------------------------------------------------*/
	/** Compare two chains of excluded zones
	 */
	/*--------------------------------------------------------------------------------*/
	friend bool Compare(const ExcludedZone *zone1, const ExcludedZone *zone2) {
	  return ((!zone1 && !zone2) ||															// end of BOTH chains; or
			  (zone1 && zone2 && (*zone1 == *zone2) && Compare(zone1->next, zone2->next)));	// both are valid and match and next on each match
	}
	
	/*--------------------------------------------------------------------------------*/
	/** Find equivalent of supplied zone in list
	 */
	/*--------------------------------------------------------------------------------*/
	const ExcludedZone *Find(const ExcludedZone& zone) const {return (zone == *this) ? this : (next ? next->Find(zone) : NULL);}
	  
	/*--------------------------------------------------------------------------------*/
	/** Set name
	 */
	/*--------------------------------------------------------------------------------*/
	void SetName(const std::string& _name) {name = _name;}
	
	/*--------------------------------------------------------------------------------*/
	/** Set minimum corner
	 */
	/*--------------------------------------------------------------------------------*/
	void SetMinCorner(float x, float y, float z) {minx = x; miny = y; minz = z;}

	/*--------------------------------------------------------------------------------*/
	/** Set maximum corner
	 */
	/*--------------------------------------------------------------------------------*/
	void SetMaxCorner(float x, float y, float z) {maxx = x; maxy = y; maxz = z;}
	
	/*--------------------------------------------------------------------------------*/
	/** Add another excluded zone to the END of the list
	 */
	/*--------------------------------------------------------------------------------*/
	void Add(ExcludedZone *zone)
	{
	  // if this object is not the end of the chain, pass the zone to the next in the list
	  if (next) next->Add(zone);
	  // otherwise set the next item
	  else      next = zone;
	}

	/*--------------------------------------------------------------------------------*/
	/** Get name
	 */
	/*--------------------------------------------------------------------------------*/
	const std::string& GetName() const {return name;}

	/*--------------------------------------------------------------------------------*/
	/** Return position of specified corner
	 *
	 * @param n corner number (bit 0: min/max x, bit 1: min/max y, bit 2: min/max z)
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	Position GetCorner(uint_t n) const {
	  return Position((n & 1) ? maxx : minx,
					  (n & 2) ? maxy : miny,
					  (n & 4) ? maxz : minz);
	}

	/*--------------------------------------------------------------------------------*/
	/** Return position of minimum corner
	 */
	/*--------------------------------------------------------------------------------*/
	Position GetMinCorner() const {return GetCorner(0);}

	/*--------------------------------------------------------------------------------*/
	/** Return position of maximum corner
	 */
	/*--------------------------------------------------------------------------------*/
	Position GetMaxCorner() const {return GetCorner(7);}

	/*--------------------------------------------------------------------------------*/
	/** Return whether position is within this or ANY child zone
	 */
	/*--------------------------------------------------------------------------------*/
	bool Within(const Position& _pos) const {
	  Position pos = _pos.Cart();
	  return ((limited::inrange(pos.pos.x, (double)minx, (double)maxx) &&
			   limited::inrange(pos.pos.y, (double)miny, (double)maxy) &&
			   limited::inrange(pos.pos.z, (double)minx, (double)maxz)) || (next && next->Within(pos)));
	}

	/*--------------------------------------------------------------------------------*/
	/** Return next excluded zone in the list
	 */
	/*--------------------------------------------------------------------------------*/
	const ExcludedZone *GetNext() const {return next;}

    /*--------------------------------------------------------------------------------*/
    /** Scale excluded zones by scene
     */
    /*--------------------------------------------------------------------------------*/
    void DivideByScene(float width, float height, float depth)
    {
      minx /= width;  maxx /= width;
      miny /= depth;  maxy /= depth;
      minz /= height; maxz /= height;
      if (next) next->DivideByScene(width, height, depth);
    }
    void MultiplyByScene(float width, float height, float depth)
    {
      minx *= width;  maxx *= width;
      miny *= depth;  maxy *= depth;
      minz *= height; maxz *= height;
      if (next) next->MultiplyByScene(width, height, depth);
    }
	
  protected:
	ExcludedZone *next;
	std::string name;
	float minx, miny, minz;
	float maxx, maxy, maxz;
  };

  /*--------------------------------------------------------------------------------*/
  /** Add a single excluded zone to list
   *
   * @note x1/x2 can be in any order since the min/max values are taken
   * @note and similarly for y and z 
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddExcludedZone(const std::string& name, float x1, float y1, float z1, float x2, float y2, float z2);

  /*--------------------------------------------------------------------------------*/
  /** Delete all excluded zones
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ResetExcludedZones();

  /*--------------------------------------------------------------------------------*/
  /** Return whether supplied position is any of the excluded zones
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool PositionWithinExcludedZones(const Position& pos) const {return excludedZones ? excludedZones->Within(pos) : false;}

  /*--------------------------------------------------------------------------------*/
  /** Return first excluded zone
   */
  /*--------------------------------------------------------------------------------*/
  const ExcludedZone *GetFirstExcludedZone() const {return excludedZones;}
  
  /*--------------------------------------------------------------------------------*/
  /** Convert all parameters into text and store them in a ParameterSet object 
   *
   * @param set ParameterSet object to receive parameters
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetAll(ParameterSet& set, bool force = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of overrideable parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetOverrideableParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

  /*--------------------------------------------------------------------------------*/
  /** Position needs to be handled different so this function returns the textual name of the position control
   */
  /*--------------------------------------------------------------------------------*/
  static const PARAMETERDESC& GetPositionDesc() {return parameterdescs[Parameter_position];}
  
  /*--------------------------------------------------------------------------------*/
  /** Set parameter from string
   *
   * @note setting position is NOT possible from this call
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool SetValue(const std::string& name, const std::string& value);

  /*--------------------------------------------------------------------------------*/
  /** Get parameter as string
   *
   * @note getting position is NOT possible from this call
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool GetValue(const std::string& name, std::string& value) const;

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter by name
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool ResetValue(const std::string& name);
  
  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a string
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToString(bool pretty = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Return an object that has continuous parameters interpolated at the given point
   *
   * @param dst destination object to be set
   * @param mul value between 1 and 0 representing progress through the interpolation
   * @param a start values
   * @param b end values
   *
   * @note when mul = 1, a is returned, when mul is 0, b is returned
   * @note for non-interpolatable parameters, their values from a are used if mul >= .5 and from b is mul < .5
   */
  /*--------------------------------------------------------------------------------*/
  static void Interpolate(AudioObjectParameters& dst, double mul, const AudioObjectParameters& a, const AudioObjectParameters& b);
  
  /*--------------------------------------------------------------------------------*/
  /** Parameter modifier class
   */
  /*--------------------------------------------------------------------------------*/
  class Modifier : public RefCountedObject, public JSONSerializable
  {
  public:
    Modifier() : RefCountedObject() {}
    Modifier(const Modifier& obj);
    virtual ~Modifier() {}

    NAMEDPARAMETER(Quaternion,rotation);
    NAMEDPARAMETER(Position,position);
    NAMEDPARAMETERDEF(double,gain,1.0);
    NAMEDPARAMETERDEF(double,scale,1.0);

    typedef std::vector<RefCount<Modifier> > LIST;

    /*--------------------------------------------------------------------------------*/
    /** Assignment operator
     */
    /*--------------------------------------------------------------------------------*/
    virtual Modifier& operator = (const Modifier& obj);

    /*--------------------------------------------------------------------------------*/
    /** Comparison operator
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool operator == (const Modifier& obj) const;

    /*--------------------------------------------------------------------------------*/
    /** Specific modifications
     */
    /*--------------------------------------------------------------------------------*/
    virtual void Modify(AudioObjectParameters& parameters, const ADMAudioObject *object = NULL) const;

#if ENABLE_JSON
    /*--------------------------------------------------------------------------------*/
    /** Return object as JSON object
     */
    /*--------------------------------------------------------------------------------*/
    virtual void ToJSON(JSONValue& obj) const;

    /*--------------------------------------------------------------------------------*/
    /** Set object from JSON
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool FromJSON(const JSONValue& obj);

    /*--------------------------------------------------------------------------------*/
    /** Assignment operator
     */
    /*--------------------------------------------------------------------------------*/
    virtual Modifier& operator = (const JSONValue& obj) {FromJSON(obj); return *this;}
#endif
  };

  /*--------------------------------------------------------------------------------*/
  /** Modify this object's parameters using a single modifier
   */
  /*--------------------------------------------------------------------------------*/
  AudioObjectParameters& Modify(const Modifier& modifier, const ADMAudioObject *object);

  /*--------------------------------------------------------------------------------*/
  /** Modify this object's parameters using a list of modifiers
   */
  /*--------------------------------------------------------------------------------*/
  AudioObjectParameters& Modify(const Modifier::LIST& list, const ADMAudioObject *object);

protected:
  void GetList(std::vector<INamedParameter *>& list);
  void InitialiseToDefaults();

  /*--------------------------------------------------------------------------------*/
  /** List of parameters for 'setbitmap'
   *
   * @note this list is COMPLETELY INDEPENDANT of the VALUES structure and the order is ONLY relevant to parameterdescs[]
   */
  /*--------------------------------------------------------------------------------*/
  typedef enum {
    Parameter_channel = 0,
    Parameter_duration,
    
    Parameter_cartesian,
    Parameter_position,
    Parameter_minposition,
    Parameter_maxposition,

    Parameter_gain,

    Parameter_width,
    Parameter_height,
    Parameter_depth,

    Parameter_divergencebalance,
    Parameter_divergenceazimuth,

    Parameter_diffuseness,
    Parameter_delay,

    Parameter_objectimportance,
    Parameter_channelimportance,
    Parameter_dialogue,

    Parameter_channellock,
    Parameter_channellockmaxdistance,
    Parameter_interact,
    Parameter_interpolate,
    Parameter_interpolationtime,
    Parameter_onscreen,
    Parameter_disableducking,

    Parameter_othervalues,

    Parameter_count,
  } Parameter_t;

  /*--------------------------------------------------------------------------------*/
  /** Return key for screenedgelock parameters stored in 'othervalues'
   */
  /*--------------------------------------------------------------------------------*/
  static std::string GetScreenEdgeLockKey(const std::string& coordinate) {return "screenedgelock." + coordinate;}
  
  /*--------------------------------------------------------------------------------*/
  /** Limit functions for various parameters
   */
  /*--------------------------------------------------------------------------------*/
  static uint8_t  LimitImportance(const uint_t& val)  {return std::min(val, 10u);}
  static uint8_t  LimitDialogue(const uint_t& val)    {return std::min(val, 2u);}
  static uint8_t  LimitBool(const int& val)           {return (val != 0);}
  static uint_t   Limit0u(const int& val)             {return std::max(val, 0);}                // at least 0
  static uint64_t Limit0u64(const sint64_t& val)      {return std::max(val, (sint64_t)0);}      // at least 0
  static double   Limit0(const double& val)           {return std::max(val, 0.0);}              // at least 0
  static float    Limit0f(const float& val)           {return std::max(val, 0.f);}              // at least 0
  static double   Limit0to1(const double& val)        {return limited::limit(val, 0.0, 1.0);}       // between 0 and 1
  static float    Limit0to1f(const float& val)        {return limited::limit(val, 0.f, 1.f);}       // between 0 and 1
  static float    LimitMaxDistance(const float& val)  {return limited::limit(val, 0.f, 2.f);}       // between 0 and 2
  static uint64_t ConvertSToNS(const double& val)     {return (uint64_t)std::max(val * 1.0e9, 0.0);}
  static double   ConvertNSToS(const uint64_t& val)   {return (double)val * 1.0e-9;}
  
  /*--------------------------------------------------------------------------------*/
  /** Set parameter from value
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of value
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param final destination of value
   * @param val new value
   * @param limit optional function ptr to a limiting/converting function that will convert the supplied value to a usable parameter value
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  void SetParameter(Parameter_t p, T1& param, const T2& val, T1 (*limit)(const T2& val) = NULL) {param = limit ? (*limit)(val) : T1(val); MarkParameterSet(p);}

  /*--------------------------------------------------------------------------------*/
  /** Set parameter from value
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of value
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param ptr to pointer type final destination of value
   * @param val new value
   * @param limit optional function ptr to a limiting/converting function that will convert the supplied value to a usable parameter value
   *
   * @note *param may be new'd as part of this function 
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  void SetParameter(Parameter_t p, T1 **param, const T2& val, T1 (*limit)(const T2& val) = NULL) {
    if (!*param) *param = new T1;
    **param = limit ? (*limit)(val) : T1(val);
    MarkParameterSet(p);
  }
  
  /*--------------------------------------------------------------------------------*/
  /** Get parameter to value
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of value to be return
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param parameter source
   * @param val destination value
   *
   * @return true if value has been set, false if it is at its default
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  bool GetParameter(Parameter_t p, const T1& param, T2& val, T2 (*convert)(const T1& val) = NULL) const {val = convert ? (*convert)(param) : T2(param); return IsParameterSet(p);}

  /*--------------------------------------------------------------------------------*/
  /** Get parameter to value
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of value to be return
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param ptr to parameter source
   * @param val destination value
   *
   * @return true if value has been set, false if it is at its default
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  bool GetParameter(Parameter_t p, const T1 *param, T2& val, T2 (*convert)(const T1& val) = NULL) const {val = (param && convert) ? (*convert)(*param) : T2(param); return IsParameterSet(p);}

  /*--------------------------------------------------------------------------------*/
  /** Get parameter to bool value
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of value to be return
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param parameter source
   * @param val destination value
   *
   * @return true if value has been set, false if it is at its default
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  bool GetBoolParameter(Parameter_t p, const T1& param, bool& val) const {val = (param != 0); return IsParameterSet(p);}

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of reset value
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param final destination of value
   * @param val reset value
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  void ResetParameter(Parameter_t p, T1& param, const T2& val) {param = val; MarkParameterReset(p);}

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of reset value
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param ptr to pointer type final destination of value
   * @param val reset value
   *
   * @note *param may be new'd as part of this function 
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  void ResetParameter(Parameter_t p, T1 **param, const T2& val) {
    if (!*param) *param = new T1;
    **param = val;
    MarkParameterReset(p);
  }

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter to 'zero'
   *
   * Template parameters:
   * @param T1 type of parameter
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param final destination of value
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  void ResetParameter(Parameter_t p, T1& param) {param = T1(); MarkParameterReset(p);}

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter to 'zero'
   *
   * Template parameters:
   * @param T1 type of parameter
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param ptr to pointer type final destination of value
   *
   * @note *param may be deleted as part of this function 
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  void ResetParameter(Parameter_t p, T1 **param) {
    if (*param)
    {
      delete *param;
      *param = NULL;
    }
    MarkParameterReset(p);
  }

  /*--------------------------------------------------------------------------------*/
  /** Set parameter in ParameterSet from specified parameter
   *
   * Template parameters:
   * @param T1 type of parameter
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param final destination of value
   * @param parameters ParameterSet object to be populated
   * @param force true to force parameters to be set, even if they have not been set
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  bool GetParameterFromParameters(Parameter_t p, const T1& param, ParameterSet& parameters, bool force = false) const {
    if (force || IsParameterSet(p)) parameters.Set(parameterdescs[p].name, param);
    return (force || IsParameterSet(p));
  }

  /*--------------------------------------------------------------------------------*/
  /** Set parameter from string representation
   *
   * @param p Parameter_xxx index to check against
   * @param param reference to parameter to set
   * @param name parameter name
   * @param value value
   * @param limit ptr to limit function or NULL
   *
   * @return true if parameter was set from string
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  bool SetFromValue(Parameter_t p, T1& param, const std::string& name, const std::string& value, T1 (*limit)(const T1& val) = NULL) {
    bool success = false;
    if (name == parameterdescs[p].name)
    {
      T1 val;
      if (Evaluate(value, val))
      {
        param = limit ? (*limit)(val) : val;
        MarkParameterSet(p);
        success = true;
      }
    }
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Set parameter from string representation
   *
   * @param p Parameter_xxx index to check against
   * @param param ptr to pointer type parameter to set
   * @param name parameter name
   * @param value value
   * @param limit ptr to limit function or NULL
   *
   * @return true if parameter was set from string
   *
   * @note *param may be new'd as part of this function
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  bool SetFromValue(Parameter_t p, T1 **param, const std::string& name, const std::string& value, T1 (*limit)(const T1& val) = NULL) {
    bool success = false;
    if (name == parameterdescs[p].name)
    {
      T1 val;
      if (Evaluate(value, val))
      {
        if (!*param) *param = new T1;
        **param = limit ? (*limit)(val) : val;
        MarkParameterSet(p);
        success = true;
      }
    }
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Set parameter from string representation (with type conversion)
   *
   * @param p Parameter_xxx index to check against
   * @param param reference to parameter to set
   * @param name parameter name
   * @param value value
   * @param limit ptr to limit function or NULL
   *
   * @return true if parameter was set from string
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1,typename T2>
  bool SetFromValueConv(Parameter_t p, T1& param, const std::string& name, const std::string& value, T1 (*limit)(const T2& val) = NULL) {
    bool success = false;
    if (name == parameterdescs[p].name)
    {
      T2 val;
      if (Evaluate(value, val))
      {
        param = limit ? (*limit)(val) : val;
        MarkParameterSet(p);
        success = true;
      }
    }
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Get parameter string representation
   *
   * @param p Parameter_xxx index to check against
   * @param param reference to parameter to get
   * @param name parameter name
   * @param value value
   *
   * @return true if string was set
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  bool GetToValue(Parameter_t p, const T1& param, const std::string& name, std::string& value) const {
    bool success = false;
    if (name == parameterdescs[p].name)
    {
      value = StringFrom(param);
      success = true;
    }
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter to zero by name
   *
   * @param p Parameter_xxx index to check against
   * @param param reference to parameter to reset
   * @param name parameter name
   *
   * @return true if parameter was reset
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  bool ResetValue(Parameter_t p, T1& param, const std::string& name) {
    bool success = false;
    if (name == parameterdescs[p].name)
    {
      ResetParameter<>(p, param);
      success = true;
    }
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter to zero by name
   *
   * @param p Parameter_xxx index to check against
   * @param param ptr to parameter to reset
   * @param name parameter name
   *
   * @return true if parameter was reset
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  bool ResetValue(Parameter_t p, T1 *param, const std::string& name) {
    bool success = false;
    if (name == parameterdescs[p].name)
    {
      ResetParameter<>(p, param);
      success = true;
    }
    return success;
  }

  /*--------------------------------------------------------------------------------*/
  /** Reset parameter to specified value by name
   *
   * @param p Parameter_xxx index to check against
   * @param param reference to parameter to reset
   * @param name parameter name
   * @param val reset value
   *
   * @return true if parameter was reset
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1,typename T2>
  bool ResetValue(Parameter_t p, T1& param, const std::string& name, const T2& val = T2()) {
    bool success = false;
    if (name == parameterdescs[p].name)
    {
      ResetParameter<>(p, param, val);
      success = true;
    }
    return success;
  }
  
#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Set parameter from JSON
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 intermediate type that JSON framework can manage
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param final destination of value
   * @param val intermediate value as read from JSON
   * @param obj JSON object
   * @param reset true to reset parameter to its default if it is not found in the JSON object
   * @param defval reset value if reset = true
   * @param limit optional function ptr to a limiting/converting function that will convert the JSON value to a usable parameter value
   *
   * @return true if parameter NOT found or set correctly from JSON, false otherwise
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  bool SetFromJSON(Parameter_t p, T1& param, T2& val, const JSONValue& obj, bool reset = false, const T1& defval = T1(), T1 (*limit)(const T2& val) = NULL)
  {
    bool success = false;
    // try and find named item in object
    // read value from JSON into intermediate value
    if (obj.isObject() && obj.isMember(parameterdescs[p].name))
    {
      if (json::FromJSON(obj[parameterdescs[p].name], val))
      {
        // use intermediate value to set parameter
        SetParameter<>(p, param, val, limit);
        success = true;
        reset   = false;    // prevent resetting of the parameter
      }
    }
    else success = true;
    // if not found or failed to decode, reset parameter to default
    if (reset) ResetParameter<>(p, param, defval);
    return success;
  }    

  /*--------------------------------------------------------------------------------*/
  /** Set parameter from JSON
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 intermediate type that JSON framework can manage
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param ptr to pointer type final destination of value
   * @param val intermediate value as read from JSON
   * @param obj JSON object
   * @param reset true to reset parameter to its default if it is not found in the JSON object
   * @param defval reset value if reset = true
   * @param limit optional function ptr to a limiting/converting function that will convert the JSON value to a usable parameter value
   *
   * @return true if parameter NOT found or set correctly from JSON, false otherwise
   *
   * @note *param may be new'd or deleted as part of this function
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  bool SetFromJSON(Parameter_t p, T1 **param, T2& val, const JSONValue& obj, bool reset = false, const T1& defval = T1(), T1 (*limit)(const T2& val) = NULL)
  {
    bool success = false;
    // try and find named item in object
    // read value from JSON into intermediate value
    if (obj.isObject() && obj.isMember(parameterdescs[p].name))
    {
      if (json::FromJSON(obj[parameterdescs[p].name], val))
      {
        // use intermediate value to set parameter
        SetParameter<>(p, param, val, limit);
        success = true;
        reset   = false;    // prevent resetting of the parameter
      }
    }
    else success = true;
    // if not found or failed to decode, reset parameter to default
    if (reset) ResetParameter<>(p, param, defval);
    return success;
  }    

  /*--------------------------------------------------------------------------------*/
  /** Set JSON from parameter
   *
   * Template parameters:
   * @param T1 type of parameter
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param parameter value
   * @param obj JSON object
   * @param force true to force parameters to be set, even if they have not been set
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  void SetToJSON(Parameter_t p, const T1& param, JSONValue& obj, bool force = false) const
  {
    if (force || IsParameterSet(p))
    {
      json::ToJSON(param, obj[parameterdescs[p].name]);
    }
  }    

  /*--------------------------------------------------------------------------------*/
  /** Set JSON from parameter
   *
   * Template parameters:
   * @param T1 type of parameter
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param ptr to parameter value
   * @param obj JSON object
   * @param force true to force parameters to be set, even if they have not been set
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  void SetToJSONPtr(Parameter_t p, const T1 *param, JSONValue& obj, bool force = false) const
  {
    if (force || IsParameterSet(p))
    {
      json::ToJSON(param ? *param : T1(), obj[parameterdescs[p].name]);
    }
  }    

  /*--------------------------------------------------------------------------------*/
  /** Set JSON from parameter
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of JSON storage
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param parameter value
   * @param obj JSON object
   * @param force true to force parameters to be set, even if they have not been set
   * @param convert function to convert T1 to T2
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  void SetToJSON(Parameter_t p, const T1& param, JSONValue& obj, bool force, T2 (*convert)(const T1& val)) const
  {
    if (force || IsParameterSet(p))
    {
      json::ToJSON((*convert)(param), obj[parameterdescs[p].name]);
    }
  }    

  /*--------------------------------------------------------------------------------*/
  /** Set JSON from parameter
   *
   * Template parameters:
   * @param T1 type of parameter
   * @param T2 type of JSON storage
   *
   * Function parameters:
   * @param p Parameter_xxx parameter enumeration
   * @param param ptr to parameter value
   * @param obj JSON object
   * @param force true to force parameters to be set, even if they have not been set
   * @param convert function to convert T1 to T2
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1, typename T2>
  void SetToJSON(Parameter_t p, const T1 *param, JSONValue& obj, bool force, T2 (*convert)(const T1& val)) const
  {
    if (force || IsParameterSet(p))
    {
      json::ToJSON((*convert)(param ? *param : T1()), obj[parameterdescs[p].name]);
    }
  }    
#endif

  /*--------------------------------------------------------------------------------*/
  /** Copy parameter from obj if it is set in obj
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  void CopyIfSet(const AudioObjectParameters& obj, Parameter_t p, T1& dst, const T1& src) {
	if (obj.IsParameterSet(p))
	{
	  dst = src;
	  MarkParameterSet(p);
	}
  }

  /*--------------------------------------------------------------------------------*/
  /** Copy parameter from obj if it is set in obj (pointer types)
   *
   * @note *dst may be new'd as part of this function
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T1>
  void CopyIfSet(const AudioObjectParameters& obj, Parameter_t p, T1 **dst, const T1& src) {
	if (obj.IsParameterSet(p))
	{
      if (!*dst) *dst = new T1;
	  **dst = src;
	  MarkParameterSet(p);
	}
  }

  /*--------------------------------------------------------------------------------*/
  /** Interpolate to given point between two values if parameter is set
   *
   * @param p Parameter_xxx value
   * @param mul progression of interpolation (IMPORTANT: see notes below!)
   * @param dst destination for interpolated value
   * @param a starting value
   * @param b end value
   * @param range if set, assumes the interpolation is circular and so all values remain within [-range, range]
   *
   * @note when mul = 1, dst will be set at a
   * @note when mul = 0, dst will be set at b
   * @note therefore, when interpolating, mul should *start* at 1 and *end* at 0
   */
  /*--------------------------------------------------------------------------------*/
  template<typename T>
  void Interpolate(Parameter_t p, double mul, T& dst, const T& a, const T& b, const T& range = T())
  {
	if (IsParameterSet(p))
    {
      T diff = a - b;
      
      // if range is supplied, only do circular interpolation if both start and end within [-range, range]
      if ((range != T()) &&
          limited::inrange(a, -range, range) &&
          limited::inrange(b, -range, range))
      {
        // calculate full range
        T modulus = range + range;
        // if the diff is -ve, choose maximum of diff and 
        if (diff < T()) diff = -std::min(-diff, modulus + diff);
        // else choose minimum of diff and modulus - diff (shortest distance)
        else            diff =  std::min( diff, modulus - diff);
        // examples (all assuming modulus = 360):
        //    a    b  diff  other diff  shortest
        //  -20   20    40         320        40
        //   20  -20   -40        -320       -40
        // -170  170   340          20        20
        //  170 -170  -340         -20       -20
        dst = T(b + std::max(mul, 0.0) * diff);
        // ensure result remains within range
        while (dst <  -range) dst += modulus;
        while (dst >=  range) dst -= modulus;
      }
      else dst = T(b + std::max(mul, 0.0) * diff);
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Interpolate position to given point between two values
   * @param p Parameter_xxx value
   * @param mul progression of interpolation (IMPORTANT: see notes below!)
   * @param pos destination for interpolated position
   * @param a starting value
   * @param b end value
   *
   * @note when mul = 1, pos will be set at a
   * @note when mul = 0, pos will be set at b
   * @note therefore, when interpolating, mul should *start* at 1 and *end* at 0
   */
  /*--------------------------------------------------------------------------------*/
  void Interpolate(Parameter_t p, double mul, Position& pos, const Position *a, const Position *b);

  /*--------------------------------------------------------------------------------*/
  /** Mark parameter p as being set
   */
  /*--------------------------------------------------------------------------------*/
  void MarkParameterSet(Parameter_t p, bool set = true)
  {
    if (set) setbitmap |=  (1U << p);
    else     setbitmap &= ~(1U << p);
  }

  /*--------------------------------------------------------------------------------*/
  /** Mark parameter p as being *not* set (reset)
   */
  /*--------------------------------------------------------------------------------*/
  void MarkParameterReset(Parameter_t p) {MarkParameterSet(p, false);}

  /*--------------------------------------------------------------------------------*/
  /** Return whether parameter p has been set
   */
  /*--------------------------------------------------------------------------------*/
  bool IsParameterSet(Parameter_t p) const {return ((setbitmap & (1U << p)) != 0);}
  
  /*--------------------------------------------------------------------------------*/
  /** Structure of simple data type items
   *
   * @note must contain POD types ONLY!  Is zeroed by code hence no objects can be used.
   *
   * @note the ORDER of the entries is to aid structure packing as much as possible (big to small) because
   * the size of this structure is a massive influence on the size of block formats!
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    uint64_t duration;
    uint64_t interpolationtime;
    double   gain;
    float    width;
    float    height;
    float    depth;
    float    diffuseness;
    float    delay;
    float    divergenceazimuth;
    float    divergencebalance;
	float    channellockmaxdistance;
    uint_t   channel;
    uint8_t  cartesian;
    uint8_t  objectimportance;
    uint8_t  channelimportance;
    uint8_t  dialogue;
    uint8_t  channellock;
    uint8_t  interact;
    uint8_t  interpolate;
    uint8_t  onscreen;
    uint8_t  disableducking;
  } VALUES;
      
protected:
  Position     position, *minposition, *maxposition;    // min and max position allocated a run time to save memory
  VALUES       values;
  uint_t       setbitmap;                               // bitmap of values that (good up to 32 items)
  ParameterSet othervalues;                             // additional, arbitrary parameters
  ExcludedZone *excludedZones;
  
  static const PARAMETERDESC parameterdescs[Parameter_count];
  static const Position nullposition;
};


#if ENABLE_JSON
inline void ToJSON(const AudioObjectParameters& val, JSONValue& obj, bool force = false) {val.ToJSON(obj, force);}
inline bool FromJSON(const JSONValue& obj, AudioObjectParameters& val, bool reset = true) {return val.FromJSON(obj, reset);}
#endif

BBC_AUDIOTOOLBOX_END

#endif
