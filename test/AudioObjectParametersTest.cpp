
#include <catch/catch.hpp>

#include "AudioObjectParameters.h"

USE_BBC_AUDIOTOOLBOX

TEST_CASE("create_and_delete")
{
  std::unique_ptr<AudioObjectParameters> aop(new AudioObjectParameters());

  CHECK(aop.get() != NULL);
}

TEST_CASE("comparison")
{
  AudioObjectParameters aop1, aop2;
  JSONValue obj1, obj2;

  obj1 = aop1.ToJSON(); 
  obj2 = aop2.ToJSON();

  printf("%u: Checking: %s == %s\n", __LINE__, aop1.ToJSONString().c_str(), aop2.ToJSONString().c_str());

  CHECK(aop1 == aop2);
  CHECK(aop1.ToString() == aop2.ToString());
  CHECK(obj1 == obj2);

  printf("%u: Checking: %s == %s\n", __LINE__, AudioObjectParameters().FromJSONEx(aop1.ToJSON()).ToJSONString().c_str(), aop1.ToJSONString().c_str());
  CHECK(AudioObjectParameters().FromJSONEx(aop1.ToJSON()) == aop1);

  printf("%u: Checking: %s == %s\n", __LINE__, AudioObjectParameters().FromJSONEx(aop2.ToJSON()).ToJSONString().c_str(), aop2.ToJSONString().c_str());
  CHECK(AudioObjectParameters().FromJSONEx(aop2.ToJSON()) == aop2);

  aop1.SetCartesian(true);
  aop1.SetPosition(Position(5, 6, 7));
  aop1.SetMinPosition(Position(1, 2, 3));
  aop1.SetMaxPosition(Position(3, 2, 1));
  aop1.SetScreenEdgeLock("azimuth", "left");
  aop1.SetScreenEdgeLock("elevation", "top");
  aop1.SetGain(2.0);
  aop1.SetWidth(5.0);
  aop1.SetHeight(10.0);
  aop1.SetDepth(15.0);
  aop1.SetDiffuseness(20.0);
  aop1.SetDelay(25.0);
  aop1.SetObjectImportance(5);
  aop1.SetChannelImportance(6);
  aop1.SetDialogue(1);
  aop1.SetDivergenceBalance(0.1);
  aop1.SetDivergenceAzimuth(25.0);
  aop1.SetChannelLock(true);
  aop1.SetChannelLockMaxDistance(1.7);
  aop1.SetInteract(true);
  aop1.SetInterpolate(true);
  aop1.SetInterpolationTimeS(5.2);
  aop1.SetOnScreen(true);
  aop1.SetDisableDucking(true);
  aop1.AddExcludedZone("zone1", -1.0, -2.0, -3.0, 3.0, 2.0, 1.0);
  aop1.AddExcludedZone("zone2", -2.0, -3.0, -1.0, 1.0, 2.0, 3.0);
  ParameterSet othervalues = aop1.GetOtherValues();
  aop1.SetOtherValues(othervalues.Set("other1", 1).Set("other2", "2"));
  obj1 = aop1.ToJSON();

  printf("%u: Checking: %s != %s\n", __LINE__, aop1.ToJSONString().c_str(), aop2.ToJSONString().c_str());
  
  CHECK(aop1 != aop2);
  CHECK(aop1.ToString() != aop2.ToString());
  CHECK(obj1 != obj2);
  CHECK(AudioObjectParameters().FromJSONEx(aop1.ToJSON()) == aop1);
  CHECK(AudioObjectParameters().FromJSONEx(aop2.ToJSON()) == aop2);

  aop2 = aop1;
  obj2 = aop2.ToJSON();

  printf("%u: Checking: %s == %s\n", __LINE__, aop1.ToJSONString().c_str(), aop2.ToJSONString().c_str());

  CHECK(aop1 == aop2);
  CHECK(aop1.ToString() == aop2.ToString());
  CHECK(obj1 == obj2);
  CHECK(AudioObjectParameters().FromJSONEx(aop1.ToJSON()) == aop1);
  CHECK(AudioObjectParameters().FromJSONEx(aop2.ToJSON()) == aop2);

  aop2 = AudioObjectParameters();
  obj2 = aop2.ToJSON();

  printf("%u: Checking: %s != %s\n", __LINE__, aop1.ToJSONString().c_str(), aop2.ToJSONString().c_str());

  CHECK(aop1 != aop2);
  CHECK(aop1.ToString() != aop2.ToString());
  CHECK(obj1 != obj2);
  CHECK(AudioObjectParameters().FromJSONEx(aop1.ToJSON()) == aop1);
  CHECK(AudioObjectParameters().FromJSONEx(aop2.ToJSON()) == aop2);

  aop2.FromJSONEx(obj1);
  obj2 = aop2.ToJSON();

  printf("%u: Checking: %s == %s\n", __LINE__, aop1.ToJSONString().c_str(), aop2.ToJSONString().c_str());

  CHECK(aop1 == aop2);
  CHECK(aop1.ToString() == aop2.ToString());
  CHECK(obj1 == obj2);
  CHECK(AudioObjectParameters().FromJSONEx(aop1.ToJSON()) == aop1);
  CHECK(AudioObjectParameters().FromJSONEx(aop2.ToJSON()) == aop2);

  aop1.ResetCartesian();
  aop1.ResetPosition();
  aop1.ResetMinPosition();
  aop1.ResetMaxPosition();
  aop1.ResetScreenEdgeLock("azimuth");
  aop1.ResetScreenEdgeLock("elevation");
  aop1.ResetGain();
  aop1.ResetWidth();
  aop1.ResetHeight();
  aop1.ResetDepth();
  aop1.ResetDiffuseness();
  aop1.ResetDelay();
  aop1.ResetObjectImportance();
  aop1.ResetChannelImportance();
  aop1.ResetDialogue();
  aop1.ResetDivergenceBalance();
  aop1.ResetDivergenceAzimuth();
  aop1.ResetChannelLock();
  aop1.ResetChannelLockMaxDistance();
  aop1.ResetInteract();
  aop1.ResetInterpolate();
  aop1.ResetInterpolationTime();
  aop1.ResetOnScreen();
  aop1.ResetDisableDucking();
  aop1.ResetExcludedZones();
  aop1.ResetOtherValues();
  obj1 = aop1.ToJSON();

  aop2 = AudioObjectParameters();
  obj2 = aop2.ToJSON();

  printf("%u: Checking: %s == %s\n", __LINE__, aop1.ToJSONString().c_str(), aop2.ToJSONString().c_str());

  CHECK(aop1 == aop2);
  CHECK(aop1.ToString() == aop2.ToString());
  CHECK(obj1 == obj2);
  CHECK(AudioObjectParameters().FromJSONEx(aop1.ToJSON()) == aop1);
  CHECK(AudioObjectParameters().FromJSONEx(aop2.ToJSON()) == aop2);
}
