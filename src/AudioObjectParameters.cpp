
#include <stdlib.h>
#include <string.h>

#define BBCDEBUG_LEVEL 1
#include "AudioObjectParameters.h"

BBC_AUDIOTOOLBOX_START

// order taken from Parameter_t enumeration
const PARAMETERDESC AudioObjectParameters::parameterdescs[Parameter_count] =
{
  {"channel",                "Channel number (0-based)"},

  {"duration",               "Block duration (ns)"},
  
  {"cartesian",              "Whether channel co-ordinate system is cartesian or spherical"},
  {"position",               "Channel position"},
  {"minposition",            "Channel minimum position"},
  {"maxposition",            "Channel maximum position"},

  {"gain",                   "Channel gain (linear)"},

  {"width",                  "Channel width"},
  {"height",                 "Channel height"},
  {"depth",                  "Channel depth"},

  {"divergencebalance",      "Channel divergence balance (0-1)"},
  {"divergenceazimuth",      "Channel divergence azimuth (degrees)"},

  {"diffuseness",            "Channel diffuseness (0-1)"},
  {"delay",                  "Channel delay (seconds)"},

  {"objectimportance",       "Object importance (0-10)"},
  {"channelimportance",      "Channel importance (0-10)"},
  {"dialogue",               "Whether channel is dialogue (0, 1 or 2)"},

  {"channellock",            "Channel is locked to channel (speaker)"},
  {"channellockmaxdistance", "Channel is locked to channel (speaker) max distance"},
  {"interact",               "Channel can be interacted with"},
  {"interpolate",            "Interpolate channel metadata changes"},
  {"interpolationtime",      "Time for interpolation of channel metadata changes (ns)"},
  {"onscreen",               "Channel is on screen"},
  {"disableducking",         "Automatic ducking on channel is disabled"},

  {"othervalues",            "Other, arbitrary, channel values"},
};

const Position AudioObjectParameters::nullposition;

AudioObjectParameters::AudioObjectParameters() : minposition(NULL),
                                                 maxposition(NULL),
                                                 setbitmap(0),
                                                 excludedZones(NULL)
{
  InitialiseToDefaults();
}

AudioObjectParameters::AudioObjectParameters(const AudioObjectParameters& obj) : minposition(NULL),
                                                                                 maxposition(NULL),
                                                                                 setbitmap(0),
                                                                                 excludedZones(NULL)
{
  InitialiseToDefaults();
  operator = (obj);
}

#if ENABLE_JSON
AudioObjectParameters::AudioObjectParameters(const json_spirit::mObject& obj) : minposition(NULL),
                                                                                maxposition(NULL),
                                                                                setbitmap(0),
                                                                                excludedZones(NULL)
{
  InitialiseToDefaults();
  operator = (obj);
}
#endif

AudioObjectParameters::~AudioObjectParameters()
{
  // delete min and max position
  ResetMinPosition();
  ResetMaxPosition();
  // delete entire chain of excluded zones
  ResetExcludedZones();
}

/*--------------------------------------------------------------------------------*/
/** Initialise all parameters to defaults
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::InitialiseToDefaults()
{
  // reset all values to zero
  memset(&values, 0, sizeof(values));

  // reset set bitmap
  setbitmap = 0;
  
  // explicitly reset those parameters whose reset values are not zero
  ResetPosition();
  ResetGain();
  ResetObjectImportance();
  ResetChannelImportance();
  ResetInterpolate();
  ResetOtherValues();
  
  // delete min and max position
  ResetMinPosition();
  ResetMaxPosition();

  // delete entire chain of excluded zones
  ResetExcludedZones();
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters& AudioObjectParameters::operator = (const AudioObjectParameters& obj)
{
  // do not copy oneself
  if (&obj != this)
  {
    position       = obj.position;
    values         = obj.values;
    othervalues    = obj.othervalues;
    setbitmap      = obj.setbitmap;

    if (obj.IsMinPositionSet()) SetMinPosition(obj.GetMinPosition());
    else                        ResetMinPosition();
    if (obj.IsMaxPositionSet()) SetMaxPosition(obj.GetMaxPosition());
    else                        ResetMaxPosition();

    // delete entire chain of excluded zones
    ResetExcludedZones();
    
    // copy chain of excluded zones from obj
    if (obj.excludedZones) excludedZones = new ExcludedZone(*obj.excludedZones);
  }
  
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Scale position and extent by scene size
 *
 * @note polar positions may have their angles altered by this if width != height != depth
 * @note excluded zones will be affected by these functions
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::DivideByScene(float width, float height, float depth)
{
  Position pos = GetPosition().Cart();
  pos.pos.x /= width;
  pos.pos.y /= depth;
  pos.pos.z /= height;
  SetPosition(GetPosition().polar ? pos.Polar() : pos);

  if (IsMinPositionSet())
  {
    Position pos = GetMinPosition().Cart();
    pos.pos.x /= width;
    pos.pos.y /= depth;
    pos.pos.z /= height;
    SetMinPosition(GetMinPosition().polar ? pos.Polar() : pos);
  }

  if (IsMaxPositionSet())
  {
    Position pos = GetMaxPosition().Cart();
    pos.pos.x /= width;
    pos.pos.y /= depth;
    pos.pos.z /= height;
    SetMaxPosition(GetMaxPosition().polar ? pos.Polar() : pos);
  }

  SetWidth(GetWidth() / width);
  SetHeight(GetHeight() / height);
  SetDepth(GetDepth() / depth);
  
  if (excludedZones) excludedZones->DivideByScene(width, height, depth);
}

void AudioObjectParameters::MultiplyByScene(float width, float height, float depth)
{
  Position pos = GetPosition().Cart();
  pos.pos.x *= width;
  pos.pos.y *= depth;
  pos.pos.z *= height;
  SetPosition(GetPosition().polar ? pos.Polar() : pos);

  if (IsMinPositionSet())
  {
    Position pos = GetMinPosition().Cart();
    pos.pos.x *= width;
    pos.pos.y *= depth;
    pos.pos.z *= height;
    SetMinPosition(GetMinPosition().polar ? pos.Polar() : pos);
  }

  if (IsMaxPositionSet())
  {
    Position pos = GetMaxPosition().Cart();
    pos.pos.x *= width;
    pos.pos.y *= depth;
    pos.pos.z *= height;
    SetMaxPosition(GetMaxPosition().polar ? pos.Polar() : pos);
  }

  SetWidth(GetWidth() * width);
  SetHeight(GetHeight() * height);
  SetDepth(GetDepth() * depth);

  if (excludedZones) excludedZones->MultiplyByScene(width, height, depth);
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters& AudioObjectParameters::FromJSON(const json_spirit::mObject& obj, bool reset)
{
  json_spirit::mObject::const_iterator it;
  Position     pval;
  ParameterSet sval;
  std::string  str;
  double       dval;
  sint64_t     i64val;
  float        fval;
  uint_t       uval;
  int          ival;
  bool         bval;
  
  SetFromJSON<>(Parameter_channel, values.channel, ival, obj, reset, 0U, &Limit0u);
  SetFromJSON<>(Parameter_duration, values.duration, i64val, obj, reset, (uint64_t)0);
  SetFromJSON<>(Parameter_cartesian, values.cartesian, bval, obj, reset);
  SetFromJSON<>(Parameter_position, position, pval, obj, reset);
  SetFromJSON<>(Parameter_minposition, &minposition, pval, obj, reset);
  SetFromJSON<>(Parameter_maxposition, &maxposition, pval, obj, reset);
  SetFromJSON<>(Parameter_gain, values.gain, dval, obj, reset, 1.0);
  SetFromJSON<>(Parameter_width, values.width, fval, obj, reset, 0.f, &Limit0f);
  SetFromJSON<>(Parameter_depth, values.depth, fval, obj, reset, 0.f, &Limit0f);
  SetFromJSON<>(Parameter_height, values.height, fval, obj, reset, 0.f, &Limit0f);
  SetFromJSON<>(Parameter_divergencebalance, values.divergencebalance, fval, obj, reset, 0.f, &Limit0to1f);
  SetFromJSON<>(Parameter_divergenceazimuth, values.divergenceazimuth, fval, obj, reset, 0.f, &Limit0f);
  SetFromJSON<>(Parameter_diffuseness, values.diffuseness, fval, obj, reset, 0.f, &Limit0to1f);
  SetFromJSON<>(Parameter_delay, values.delay, fval, obj, reset, 0.f, &Limit0f);
  SetFromJSON<>(Parameter_objectimportance, values.objectimportance, uval, obj, reset, (uint8_t)GetObjectImportanceDefault(), &LimitImportance);
  SetFromJSON<>(Parameter_channelimportance, values.channelimportance, uval, obj, reset, (uint8_t)GetChannelImportanceDefault(), &LimitImportance);
  SetFromJSON<>(Parameter_dialogue, values.dialogue, uval, obj, reset, (uint8_t)GetDialogueDefault(), &LimitDialogue);
  SetFromJSON<>(Parameter_channellock, values.channellock, bval, obj, reset);
  SetFromJSON<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, fval, obj, reset, 0.f, &LimitMaxDistance);
  SetFromJSON<>(Parameter_interact, values.interact, bval, obj, reset, (uint8_t)GetInteractDefault());
  SetFromJSON<>(Parameter_interpolate, values.interpolate, bval, obj, reset, (uint8_t)1);
  SetFromJSON<>(Parameter_interpolationtime, values.interpolationtime, i64val, obj, reset, (uint64_t)0);
  SetFromJSON<>(Parameter_onscreen, values.onscreen, bval, obj, reset);
  SetFromJSON<>(Parameter_disableducking, values.disableducking, bval, obj, reset, (uint8_t)GetDisableDuckingDefault());
  SetFromJSON<>(Parameter_othervalues, othervalues, sval, obj, reset);

  // support legacy 'importance' parameter name for channel importance
  if ((it = obj.find("importance")) != obj.end())
  {
    // read value from JSON into intermediate value
    bbcat::FromJSON(it->second, uval);
    // use intermediate value to set parameter
    SetParameter<>(Parameter_channelimportance, values.channelimportance, uval, &LimitImportance);
  }

  // delete existing list of excluded zones
  if (excludedZones)
  {
    delete excludedZones;
    excludedZones = NULL;
  }
  
  {
    json_spirit::mObject::const_iterator it, it2;
    
    // try and find zoneExclusion item in object
    if (((it = obj.find("excludedzones")) != obj.end()) && (it->second.type() == json_spirit::array_type))
    {
      json_spirit::mArray zones = it->second.get_array();
      uint_t i;

      for (i = 0; i < zones.size(); i++)
      {
        if (zones[i].type() == json_spirit::obj_type)
        {
          json_spirit::mObject obj = zones[i].get_obj();
          std::string name;
          float minx, miny, minz, maxx, maxy, maxz;

          // extract name and limits of excluded zone
          if (((it2 = obj.find("name")) != obj.end()) && bbcat::FromJSON(it2->second, name) &&
              ((it2 = obj.find("minx")) != obj.end()) && bbcat::FromJSON(it2->second, minx) &&
              ((it2 = obj.find("miny")) != obj.end()) && bbcat::FromJSON(it2->second, miny) &&
              ((it2 = obj.find("minz")) != obj.end()) && bbcat::FromJSON(it2->second, minz) &&
              ((it2 = obj.find("maxx")) != obj.end()) && bbcat::FromJSON(it2->second, maxx) &&
              ((it2 = obj.find("maxy")) != obj.end()) && bbcat::FromJSON(it2->second, maxy) &&
              ((it2 = obj.find("maxz")) != obj.end()) && bbcat::FromJSON(it2->second, maxz))
          {
            AddExcludedZone(name, minx, miny, minz, maxx, maxy, maxz);
          }
          else BBCERROR("Unable to extract excluded zones from JSON '%s'", json_spirit::write(it->second).c_str());
        }
      }
    }
  }
  
  return *this;
}
#endif

/*--------------------------------------------------------------------------------*/
/** Comparison operator
 */
/*--------------------------------------------------------------------------------*/
bool AudioObjectParameters::operator == (const AudioObjectParameters& obj) const
{
  bool same = ((position == obj.position) &&
               (memcmp(&values, &obj.values, sizeof(obj.values)) == 0) &&
               Compare(excludedZones, obj.excludedZones) &&
               (GetMinPosition() == obj.GetMinPosition()) &&
               (GetMaxPosition() == obj.GetMaxPosition()) &&
               (othervalues == obj.othervalues));
#if BBCDEBUG_LEVEL>=4
  if (!same)
  {
    BBCDEBUG("Compare: %u/%u/%u/%u/%08x/%08x", (uint_t)(position == obj.position), (uint_t)(memcmp(&values, &obj.values, sizeof(obj.values)) == 0), (uint_t)Compare(excludedZones, obj.excludedZones), (uint_t)(othervalues == obj.othervalues), setbitmap, obj.setbitmap);

    std::string line;
    const uint8_t *p1 = (const uint8_t *)&values, *p2 = (const uint8_t *)&obj.values;
    uint_t i;
    for (i = 0; i < sizeof(values); i++)
    {
      if (line.empty())
      {
        Printf(line, "%04x:", i);
      }
      Printf(line, " %02x/%02x", (uint_t)p1[i], (uint_t)p2[i]);
      if ((i & 7) == 7)
      {
        BBCDEBUG("%s", line.c_str());
        line = "";
      }
    }
    if (!line.empty()) BBCDEBUG("%s", line.c_str());
  }
#endif
  return same;
}

/*--------------------------------------------------------------------------------*/
/** Merge another AudioObjectParameters into this one
 *
 * @note any values set in obj will over-write the ones in this object
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters& AudioObjectParameters::Merge(const AudioObjectParameters& obj)
{
  CopyIfSet<>(obj, Parameter_cartesian, values.cartesian, obj.values.cartesian);
  CopyIfSet<>(obj, Parameter_position, position, obj.GetPosition());
  CopyIfSet<>(obj, Parameter_minposition, &minposition, obj.GetMinPosition());
  CopyIfSet<>(obj, Parameter_maxposition, &maxposition, obj.GetMaxPosition());
  CopyIfSet<>(obj, Parameter_gain, values.gain, obj.values.gain);
  CopyIfSet<>(obj, Parameter_width, values.width, obj.values.width);
  CopyIfSet<>(obj, Parameter_depth, values.depth, obj.values.depth);
  CopyIfSet<>(obj, Parameter_height, values.height, obj.values.height);
  CopyIfSet<>(obj, Parameter_divergencebalance, values.divergencebalance, obj.values.divergencebalance);
  CopyIfSet<>(obj, Parameter_divergenceazimuth, values.divergenceazimuth, obj.values.divergenceazimuth);
  CopyIfSet<>(obj, Parameter_diffuseness, values.diffuseness, obj.values.diffuseness);
  CopyIfSet<>(obj, Parameter_delay, values.delay, obj.values.delay);
  CopyIfSet<>(obj, Parameter_objectimportance, values.objectimportance, obj.values.objectimportance);
  CopyIfSet<>(obj, Parameter_channelimportance, values.channelimportance, obj.values.channelimportance);
  CopyIfSet<>(obj, Parameter_dialogue, values.dialogue, obj.values.dialogue);
  CopyIfSet<>(obj, Parameter_channellock, values.channellock, obj.values.channellock);
  CopyIfSet<>(obj, Parameter_channellockmaxdistance, values.channellockmaxdistance, obj.values.channellockmaxdistance);
  CopyIfSet<>(obj, Parameter_interact, values.interact, obj.values.interact);
  CopyIfSet<>(obj, Parameter_interpolate, values.interpolate, obj.values.interpolate);
  CopyIfSet<>(obj, Parameter_interpolationtime, values.interpolationtime, obj.values.interpolationtime);
  CopyIfSet<>(obj, Parameter_onscreen, values.onscreen, obj.values.onscreen);
  CopyIfSet<>(obj, Parameter_disableducking, values.disableducking, obj.values.disableducking);
  CopyIfSet<>(obj, Parameter_othervalues, othervalues, obj.othervalues);
  if (obj.excludedZones)
  {
    // delete current zone(s)
    ResetExcludedZones();
    // copy other object's zone(s)
    excludedZones = new ExcludedZone(*obj.excludedZones);
  }
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Add a single excluded zone to list
 *
 * @note x1/x2 can be in any order since the min/max values are taken
 * @note and similarly for y and z 
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::AddExcludedZone(const std::string& name, float x1, float y1, float z1, float x2, float y2, float z2)
{
  ExcludedZone *zone;

  if ((zone = new ExcludedZone) != NULL)
  {
    zone->SetName(name);
    zone->SetMinCorner(std::min(x1, x2), std::min(y1, y2), std::min(z1, z2));
    zone->SetMaxCorner(std::max(x1, x2), std::max(y1, y2), std::max(z1, z2));

    // if chain already exists, append this one to the end
    if (excludedZones) excludedZones->Add(zone);
    // otherwise start the chain with this one
    else               excludedZones = zone;
  }                       
}

/*--------------------------------------------------------------------------------*/
/** Delete all excluded zones
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::ResetExcludedZones()
{
  if (excludedZones)
  {
    delete excludedZones;
    excludedZones = NULL;
  }
}

/*--------------------------------------------------------------------------------*/
/** Transform this object's position and return new copy
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters operator * (const AudioObjectParameters& obj, const PositionTransform& transform)
{
  AudioObjectParameters res = obj;
  res.SetPosition(obj.GetPosition() * transform);
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Transform this object's position
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters& AudioObjectParameters::operator *= (const PositionTransform& transform)
{
  Position centre = GetPosition(), corner = centre;

  SetPosition(centre * transform);
 
  if (centre.polar)
  {
    corner.pos.az += GetWidth();
    corner.pos.el += GetHeight();
    corner.pos.d  += GetDepth();
  }
  else
  {
    corner.pos.x  += GetWidth();
    corner.pos.y  += GetDepth();
    corner.pos.z  += GetHeight();
  }

  corner = (corner * transform) - centre;

  if (centre.polar)
  {
    SetWidth(static_cast<float>(corner.pos.az));
    SetHeight(static_cast<float>(corner.pos.el));
    SetDepth(static_cast<float>(corner.pos.d));
  }
  else
  {
    SetWidth(static_cast<float>(corner.pos.x));
    SetDepth(static_cast<float>(corner.pos.y));
    SetHeight(static_cast<float>(corner.pos.z));
  }
  
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Convert all parameters into text and store them in a ParameterSet object 
 *
 * @param set ParameterSet object to receive parameters
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::GetAll(ParameterSet& set, bool force) const
{
  GetParameterFromParameters<>(Parameter_channel, values.channel, set, force);
  GetParameterFromParameters<>(Parameter_duration, values.duration, set, force);
  GetParameterFromParameters<>(Parameter_cartesian, values.cartesian, set, force);
  if (force || IsParameterSet(Parameter_position)) position.SetParameters(set, parameterdescs[Parameter_position].name);
  if (force || IsParameterSet(Parameter_minposition))
  {
    const Position *pos = minposition ? minposition : &nullposition;
    pos->SetParameters(set, parameterdescs[Parameter_minposition].name);
  }
  if (force || IsParameterSet(Parameter_maxposition))
  {
    const Position *pos = maxposition ? maxposition : &nullposition;
    pos->SetParameters(set, parameterdescs[Parameter_maxposition].name);
  }
  GetParameterFromParameters<>(Parameter_gain, values.gain, set, force);
  GetParameterFromParameters<>(Parameter_width, values.width, set, force);
  GetParameterFromParameters<>(Parameter_depth, values.depth, set, force);
  GetParameterFromParameters<>(Parameter_height, values.height, set, force);
  GetParameterFromParameters<>(Parameter_divergencebalance, values.divergencebalance, set, force);
  GetParameterFromParameters<>(Parameter_divergenceazimuth, values.divergenceazimuth, set, force);
  GetParameterFromParameters<>(Parameter_diffuseness, values.diffuseness, set, force);
  GetParameterFromParameters<>(Parameter_delay, values.delay, set, force);
  GetParameterFromParameters<>(Parameter_objectimportance, values.objectimportance, set, force);
  GetParameterFromParameters<>(Parameter_channelimportance, values.channelimportance, set, force);
  GetParameterFromParameters<>(Parameter_dialogue, values.dialogue, set, force);
  GetParameterFromParameters<>(Parameter_channellock, values.channellock, set, force);
  GetParameterFromParameters<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, set, force);
  GetParameterFromParameters<>(Parameter_interact, values.interact, set, force);
  GetParameterFromParameters<>(Parameter_interpolate, values.interpolate, set, force);
  GetParameterFromParameters<>(Parameter_interpolationtime, values.interpolationtime, set, force);
  GetParameterFromParameters<>(Parameter_onscreen, values.onscreen, set, force);
  GetParameterFromParameters<>(Parameter_disableducking, values.disableducking, set, force);
  GetParameterFromParameters<>(Parameter_othervalues, othervalues, set, force);

  const ExcludedZone *zone;
  if ((zone = excludedZones) != NULL)
  {
    ParameterSet zones;
    uint_t n = 0;

    while (zone)
    {
      ParameterSet zonerep;
      Position c1 = zone->GetMinCorner();
      Position c2 = zone->GetMaxCorner();

      zonerep.Set("name", zone->GetName());
      zonerep.Set("minx", c1.pos.x);
      zonerep.Set("miny", c1.pos.y);
      zonerep.Set("minz", c1.pos.z);
      zonerep.Set("maxx", c2.pos.x);
      zonerep.Set("maxy", c2.pos.y);
      zonerep.Set("maxz", c2.pos.z);
      
      zones.Set(StringFrom(n), zonerep);
      
      zone = zone->GetNext();
      n++;
    }

    set.Set("excludedzones", zones);
  }
}

/*--------------------------------------------------------------------------------*/
/** Get a list of parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  AddParametersToList(parameterdescs, NUMBEROF(parameterdescs), list);
}

/*--------------------------------------------------------------------------------*/
/** Get a list of overrideable parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::GetOverrideableParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  // channel and othervalues CANNOT be overriden currently
  AddParametersToList(parameterdescs + 1, NUMBEROF(parameterdescs) - 2, list);
}

/*--------------------------------------------------------------------------------*/
/** Get equivalent ADM parameters jumpPosition and interpolationLength
 *
 * @param jumpPosition value to be updated with jumpPosition value
 * @param interpolationLength optional pointer to value to be updated with interpolationLength (valid ONLY IF jumpPosition == true)
 *
 * @return true if jumpPosition is valid (has been set)
 */
/*--------------------------------------------------------------------------------*/
bool AudioObjectParameters::GetJumpPosition(bool& jumpPosition, double *interpolationLength) const
{
  bool valid = GetInterpolate(jumpPosition);
  if (valid)
  {
    jumpPosition &= (GetInterpolationTime() != GetDuration());
    if (interpolationLength) *interpolationLength = jumpPosition ? GetInterpolationTimeS() : 0.0;
  }
  else if (interpolationLength) *interpolationLength = 0.0;
  
  BBCDEBUG3(("GetJumpPosition(): %s/%s -> %s/%s",
             StringFrom(GetInterpolate()).c_str(),
             StringFrom(GetInterpolationTimeS()).c_str(),
             StringFrom(jumpPosition).c_str(),
             interpolationLength ? StringFrom(*interpolationLength).c_str() : "<notset>"));
          
  return valid;
}

/*--------------------------------------------------------------------------------*/
/** Set interpolation parameters via jumpPosition and interpolationLength ADM parameters
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::SetJumpPosition(bool jumpPosition, double interpolationLength)
{
  // calcuate interpolation time
  uint64_t itime = jumpPosition ? ConvertSToNS(interpolationLength) : GetDuration();

  SetInterpolate(itime > 0);
  SetInterpolationTime(itime);

  BBCDEBUG3(("SetJumpPosition(): %s/%s -> %s/%s",
             StringFrom(jumpPosition).c_str(),
             StringFrom(interpolationLength).c_str(),
             StringFrom(GetInterpolate()).c_str(),
             StringFrom(GetInterpolationTimeS()).c_str()));
}

/*--------------------------------------------------------------------------------*/
/** Set parameter from string
 */
/*--------------------------------------------------------------------------------*/
bool AudioObjectParameters::SetValue(const std::string& name, const std::string& value)
{
  // only one of the sets will work and a compiler that does early abort optimisation
  // will improve the speed of this function but not break it
  return (SetFromValue<>(Parameter_gain, values.gain, name, value) ||
          SetFromValue<>(Parameter_duration, values.duration, name, value) ||
          SetFromValueConv<uint8_t,int>(Parameter_cartesian, values.cartesian, name, value, &LimitBool) ||
          SetFromValue<>(Parameter_width, values.width, name, value, &Limit0f) ||
          SetFromValue<>(Parameter_depth, values.depth, name, value, &Limit0f) ||
          SetFromValue<>(Parameter_height, values.height, name, value, &Limit0f) ||
          SetFromValue<>(Parameter_divergencebalance, values.divergencebalance, name, value, &Limit0to1f) ||
          SetFromValue<>(Parameter_divergenceazimuth, values.divergenceazimuth, name, value, &Limit0f) ||
          SetFromValue<>(Parameter_diffuseness, values.diffuseness, name, value, &Limit0to1f) ||
          SetFromValue<>(Parameter_delay, values.delay, name, value, &Limit0f) ||
          SetFromValueConv<uint8_t,uint_t>(Parameter_objectimportance, values.objectimportance, name, value, &LimitImportance) ||
          SetFromValueConv<uint8_t,uint_t>(Parameter_channelimportance, values.channelimportance, name, value, &LimitImportance) ||
          SetFromValueConv<uint8_t,uint_t>(Parameter_dialogue, values.dialogue, name, value, &LimitDialogue) ||
          SetFromValueConv<uint8_t,int>(Parameter_channellock, values.channellock, name, value, &LimitBool) ||
          SetFromValue<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, name, value, &LimitMaxDistance) ||
          SetFromValueConv<uint8_t,int>(Parameter_interact, values.interact, name, value, &LimitBool) ||
          SetFromValueConv<uint8_t,int>(Parameter_interpolate, values.interpolate, name, value, &LimitBool) ||
          SetFromValueConv<uint64_t,sint64_t>(Parameter_interpolationtime, values.interpolationtime, name, value, &Limit0u64) ||
          SetFromValueConv<uint8_t,int>(Parameter_onscreen, values.onscreen, name, value, &LimitBool) ||
          SetFromValueConv<uint8_t,int>(Parameter_disableducking, values.disableducking, name, value, &LimitBool));
}

/*--------------------------------------------------------------------------------*/
/** Get parameter as string
 */
/*--------------------------------------------------------------------------------*/
bool AudioObjectParameters::GetValue(const std::string& name, std::string& value) const
{
  // only one of the gets will work and a compiler that does early abort optimisation
  // will improve the speed of this function but not break it
  return (GetToValue<>(Parameter_channel, values.channel, name, value) ||
          GetToValue<>(Parameter_duration, values.duration, name, value) ||
          GetToValue<>(Parameter_cartesian, values.cartesian, name, value) ||
          GetToValue<>(Parameter_gain, values.gain, name, value) ||
          GetToValue<>(Parameter_width, values.width, name, value) ||
          GetToValue<>(Parameter_depth, values.depth, name, value) ||
          GetToValue<>(Parameter_height, values.height, name, value) ||
          GetToValue<>(Parameter_divergencebalance, values.divergencebalance, name, value) ||
          GetToValue<>(Parameter_divergenceazimuth, values.divergenceazimuth, name, value) ||
          GetToValue<>(Parameter_diffuseness, values.diffuseness, name, value) ||
          GetToValue<>(Parameter_delay, values.delay, name, value) ||
          GetToValue<>(Parameter_objectimportance, values.objectimportance, name, value) ||
          GetToValue<>(Parameter_channelimportance, values.channelimportance, name, value) ||
          GetToValue<>(Parameter_dialogue, values.dialogue, name, value) ||
          GetToValue<>(Parameter_channellock, values.channellock, name, value) ||
          GetToValue<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, name, value) ||
          GetToValue<>(Parameter_interact, values.interact, name, value) ||
          GetToValue<>(Parameter_interpolate, values.interpolate, name, value) ||
          GetToValue<>(Parameter_interpolationtime, values.interpolationtime, name, value) ||
          GetToValue<>(Parameter_onscreen, values.onscreen, name, value) ||
          GetToValue<>(Parameter_disableducking, values.disableducking, name, value));
}

/*--------------------------------------------------------------------------------*/
/** Reset parameter by name
 */
/*--------------------------------------------------------------------------------*/
bool AudioObjectParameters::ResetValue(const std::string& name)
{
  // only one of the gets will work and a compiler that does early abort optimisation
  // will improve the speed of this function but not break it
  return (ResetValue<>(Parameter_channel, values.channel, name) ||
          ResetValue<>(Parameter_duration, values.duration, name) ||
          ResetValue<>(Parameter_cartesian, values.cartesian, name) ||
          ResetValue<>(Parameter_position, position, name) ||
          ResetValue<>(Parameter_minposition, &minposition, name) ||
          ResetValue<>(Parameter_maxposition, &maxposition, name) ||
          ResetValue<>(Parameter_gain, values.gain, name, 1.0) ||
          ResetValue<>(Parameter_width, values.width, name) ||
          ResetValue<>(Parameter_depth, values.depth, name) ||
          ResetValue<>(Parameter_height, values.height, name) ||
          ResetValue<>(Parameter_divergencebalance, values.divergencebalance, name) ||
          ResetValue<>(Parameter_divergenceazimuth, values.divergenceazimuth, name) ||
          ResetValue<>(Parameter_diffuseness, values.diffuseness, name) ||
          ResetValue<>(Parameter_delay, values.delay, name) ||
          ResetValue<>(Parameter_objectimportance, values.objectimportance, name, 10) ||
          ResetValue<>(Parameter_channelimportance, values.channelimportance, name, 10) ||
          ResetValue<>(Parameter_dialogue, values.dialogue, name) ||
          ResetValue<>(Parameter_channellock, values.channellock, name) ||
          ResetValue<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, name) ||
          ResetValue<>(Parameter_interact, values.interact, name) ||
          ResetValue<>(Parameter_interpolate, values.interpolate, name) ||
          ResetValue<>(Parameter_interpolationtime, values.interpolationtime, name) ||
          ResetValue<>(Parameter_onscreen, values.onscreen, name) ||
          ResetValue<>(Parameter_disableducking, values.disableducking, name));
}

/*--------------------------------------------------------------------------------*/
/** Convert parameters to a string
 */
/*--------------------------------------------------------------------------------*/
std::string AudioObjectParameters::ToString(bool pretty) const
{
  ParameterSet params;

  GetAll(params);

  return params.ToString(pretty);
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Convert parameters to a JSON object
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::ToJSON(json_spirit::mObject& obj, bool force) const
{
  SetToJSON<>(Parameter_channel, (int)values.channel, obj, force);
  SetToJSON<>(Parameter_duration, (sint64_t)values.duration, obj, force);
  SetToJSON<>(Parameter_cartesian, values.cartesian, obj, force);
  SetToJSON<>(Parameter_position, position, obj, force);
  SetToJSONPtr<>(Parameter_minposition, minposition, obj, force);
  SetToJSONPtr<>(Parameter_maxposition, maxposition, obj, force);
  SetToJSON<>(Parameter_gain, values.gain, obj, force);
  SetToJSON<>(Parameter_width, values.width, obj, force);
  SetToJSON<>(Parameter_depth, values.depth, obj, force);
  SetToJSON<>(Parameter_height, values.height, obj, force);
  SetToJSON<>(Parameter_diffuseness, values.diffuseness, obj, force);
  SetToJSON<>(Parameter_divergencebalance, values.divergencebalance, obj, force);
  SetToJSON<>(Parameter_divergenceazimuth, values.divergenceazimuth, obj, force);
  SetToJSON<>(Parameter_delay, values.delay, obj, force);
  SetToJSON<>(Parameter_objectimportance, (int)values.objectimportance, obj, force);
  SetToJSON<>(Parameter_channelimportance, (int)values.channelimportance, obj, force);
  SetToJSON<>(Parameter_dialogue, (int)values.dialogue, obj, force);
  SetToJSON<>(Parameter_channellock, values.channellock, obj, force);
  SetToJSON<>(Parameter_channellockmaxdistance, values.channellockmaxdistance, obj, force);
  SetToJSON<>(Parameter_interact, values.interact, obj, force);
  SetToJSON<>(Parameter_interpolate, values.interpolate, obj, force);
  SetToJSON<>(Parameter_interpolationtime, (sint64_t)values.interpolationtime, obj, force);
  SetToJSON<>(Parameter_onscreen, values.onscreen, obj, force);
  SetToJSON<>(Parameter_disableducking, values.disableducking, obj, force);
  SetToJSON<>(Parameter_othervalues, othervalues, obj, force);
  
  // output all excluded zones
  const ExcludedZone *zone = GetFirstExcludedZone();
  if (zone)
  {
    json_spirit::mArray zones;
    while (zone)
    {
      json_spirit::mObject subobj;
      Position c1 = zone->GetMinCorner();
      Position c2 = zone->GetMaxCorner();

      subobj["name"] = zone->GetName();
      subobj["minx"] = c1.pos.x;
      subobj["miny"] = c1.pos.y;
      subobj["minz"] = c1.pos.z;
      subobj["maxx"] = c2.pos.x;
      subobj["maxy"] = c2.pos.y;
      subobj["maxz"] = c2.pos.z;

      zones.push_back(subobj);
    
      zone = zone->GetNext();
    }

    obj["excludedzones"] = zones;
  }
  
  BBCDEBUG2(("JSON: %s", json_spirit::write(obj, json_spirit::pretty_print).c_str()));
}
#endif

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
void AudioObjectParameters::Interpolate(AudioObjectParameters& dst, double mul, const AudioObjectParameters& a, const AudioObjectParameters& b)
{
  dst = (mul >= .5f) ? a : b; // best initial value for non-interpolatable parameters

  // now modify parameters that are interpolatable
  // outside of the endstops, actually interpolate
  if ((mul > 0.0) && (mul < 1.0))
  {
    // merge bitmaps of parameters set so that if parameter is set in *either* a or b it will be interpolated
    dst.setbitmap = a.setbitmap | b.setbitmap;

    if (dst.IsParameterSet(Parameter_position))
    {
      dst.Interpolate(Parameter_position, mul, dst.position, &a.position, &b.position);
    }
    if (dst.IsParameterSet(Parameter_minposition))
    {
      if (!dst.minposition) dst.minposition = new Position;
      dst.Interpolate(Parameter_minposition, mul, *dst.minposition, a.minposition, b.minposition);
    }
    if (dst.IsParameterSet(Parameter_maxposition))
    {
      if (!dst.maxposition) dst.maxposition = new Position;
      dst.Interpolate(Parameter_maxposition, mul, *dst.maxposition, a.maxposition, b.maxposition);
    }
    
    dst.Interpolate<>(Parameter_gain, mul, dst.values.gain, a.values.gain, b.values.gain);
    dst.Interpolate<>(Parameter_width, mul, dst.values.width, a.values.width, b.values.width);
    dst.Interpolate<>(Parameter_height, mul, dst.values.height, a.values.height, b.values.height);
    dst.Interpolate<>(Parameter_depth, mul, dst.values.depth, a.values.depth, b.values.depth);
    dst.Interpolate<>(Parameter_divergencebalance, mul, dst.values.divergencebalance, a.values.divergencebalance, b.values.divergencebalance);
    dst.Interpolate<>(Parameter_divergenceazimuth, mul, dst.values.divergenceazimuth, a.values.divergenceazimuth, b.values.divergenceazimuth);
    dst.Interpolate<>(Parameter_diffuseness, mul, dst.values.diffuseness, a.values.diffuseness, b.values.diffuseness);
    dst.Interpolate<>(Parameter_channellockmaxdistance, mul, dst.values.channellockmaxdistance, a.values.channellockmaxdistance, b.values.channellockmaxdistance);
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
void AudioObjectParameters::Interpolate(Parameter_t p, double mul, Position& pos, const Position *a, const Position *b)
{
  if (!a) a = &nullposition;
  if (!b) b = &nullposition;
  
  pos.polar = b->polar; // positions are interpolated in the same co-ordinate system as the end values

  // get position for a (start values) in same system as b to interpolate
  Position        posa = pos.polar ? a->Polar() : a->Cart();
  const Position& posb = *b;    // keep b's position as it is
  uint_t i;
      
  // interpolate each element of position
  for (i = 0; i < NUMBEROF(pos.pos.elements); i++)
  {
    // for polar co-ordinates, ensure the azmimuth is interpolated using circular interpolation
    Interpolate<>(p, mul, pos.pos.elements[i], posa.pos.elements[i], posb.pos.elements[i], (pos.polar && !i) ? 360.0 : 0.0);
  }
}

/*----------------------------------------------------------------------------------------------------*/

AudioObjectParameters::Modifier::Modifier(const Modifier& obj) : RefCountedObject()
{
  operator = (obj);
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters::Modifier& AudioObjectParameters::Modifier::operator = (const Modifier& obj)
{
  if (&obj != this)
  {
    rotation = obj.rotation;
    position = obj.position;
    gain     = obj.gain;
    scale    = obj.scale;
  }
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Comparison operator
 */
/*--------------------------------------------------------------------------------*/
bool AudioObjectParameters::Modifier::operator == (const Modifier& obj) const
{
  return ((rotation == obj.rotation) &&
          (position == obj.position) &&
          (gain     == obj.gain)     &&
          (scale    == obj.scale));
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters::Modifier& AudioObjectParameters::Modifier::FromJSON(const json_spirit::mObject& obj)
{
  INamedParameter *parameters[] =
  {
    &rotation,
    &position,
    &gain,
    &scale,
  };
  bbcat::FromJSON(obj, parameters, NUMBEROF(parameters));
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Convert parameters to a JSON object
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::Modifier::ToJSON(json_spirit::mObject& obj) const
{
  const INamedParameter *parameters[] =
  {
    &rotation,
    &position,
    &gain,
    &scale,
  };
  bbcat::ToJSON(parameters, NUMBEROF(parameters), obj);
}
#endif

/*--------------------------------------------------------------------------------*/
/** Specific modifications
 */
/*--------------------------------------------------------------------------------*/
void AudioObjectParameters::Modifier::Modify(AudioObjectParameters& parameters, const ADMAudioObject *object) const
{
  UNUSED_PARAMETER(parameters);
  UNUSED_PARAMETER(object);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Modify this object's parameters using a single modifier
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters& AudioObjectParameters::Modify(const Modifier& modifier, const ADMAudioObject *object)
{
  if (modifier.rotation.IsSet())
  {
    SetPosition(GetPosition() * modifier.rotation.Get());
    if (IsMinPositionSet()) SetMinPosition(GetMinPosition() * modifier.rotation.Get());
    if (IsMaxPositionSet()) SetMaxPosition(GetMaxPosition() * modifier.rotation.Get());

    Position size(GetWidth(), GetDepth(), GetHeight());
    size *= modifier.rotation.Get();
    SetWidth(static_cast<float>(size.pos.x));
    SetDepth(static_cast<float>(size.pos.y));
    SetHeight(static_cast<float>(size.pos.z));
  }
  if (modifier.position.IsSet()) SetPosition(GetPosition() + modifier.position.Get());
  if (modifier.scale.IsSet())
  {
    SetPosition(GetPosition() * modifier.scale.Get());
    if (IsMinPositionSet()) SetMinPosition(GetMinPosition() * modifier.scale.Get());
    if (IsMaxPositionSet()) SetMaxPosition(GetMaxPosition() * modifier.scale.Get());

    Position size(GetWidth(), GetDepth(), GetHeight());
    size *= modifier.scale.Get();
    SetWidth(static_cast<float>(size.pos.x));
    SetDepth(static_cast<float>(size.pos.y));
    SetHeight(static_cast<float>(size.pos.z));
  }
  if (modifier.gain.IsSet()) SetGain(GetGain() * modifier.gain.Get());

  // apply specific modifications (from derived classes)
  modifier.Modify(*this, object);

  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Modify this object's parameters using a list of modifiers
 */
/*--------------------------------------------------------------------------------*/
AudioObjectParameters& AudioObjectParameters::Modify(const Modifier::LIST& list, const ADMAudioObject *object)
{
  uint_t i;

  for (i = 0; i < list.size(); i++) Modify(*list[i].Obj(), object);

  return *this;
}

BBC_AUDIOTOOLBOX_END
