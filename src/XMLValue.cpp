
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "XMLValue.h"

BBC_AUDIOTOOLBOX_START

XMLValue::XMLValue() : attr(false),
                       subvalues(NULL)
{
}

XMLValue::XMLValue(const XMLValue& obj) : attr(false),
                                          subvalues(NULL)
{
  operator = (obj);
}

XMLValue::~XMLValue()
{
  if (subvalues) delete subvalues;
}

/*--------------------------------------------------------------------------------*/
/** Assignment operator
 */
/*--------------------------------------------------------------------------------*/
XMLValue& XMLValue::operator = (const XMLValue& obj)
{
  attr  = obj.attr;
  name  = obj.name;
  value = obj.value;
  attrs = obj.attrs;

  // create/copy set of subvalues
  SetSubValues(obj.subvalues);
  
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Set subvalues list
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::SetSubValues(const XMLValues *_subvalues)
{
  // try not to delete and re-allocate, use std::vector<> assignment instead
  if (_subvalues)
  {
    // if subvalues list already exists, use assignment to avoid object delete and new
    if (subvalues) *subvalues = *_subvalues;
    // else no alternative but to create new object
    else           subvalues  = new XMLValues(*_subvalues);
  }
  // delete existing list of sub values
  else if (subvalues)
  {
    delete subvalues;
    subvalues = NULL;
  }
}

/*--------------------------------------------------------------------------------*/
/** Add a set of sub values to the lsit
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::AddSubValues(const XMLValues& _subvalues)
{
  if (_subvalues.size())
  {
    if (!subvalues) subvalues = new XMLValues;
    if (subvalues)
    {
      uint_t i;
      for (i = 0; i < _subvalues.size(); i++)
      {
        subvalues->AddValue(_subvalues[i]);
      }
    }
  }
  // delete existing list of sub values
  else if (subvalues)
  {
    delete subvalues;
    subvalues = NULL;
  }
}

/*--------------------------------------------------------------------------------*/
/** Add a single sub value to the lsit
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::AddSubValue(const XMLValue& _subvalue)
{
  if (!subvalues) subvalues = new XMLValues;
  if (subvalues)  subvalues->AddValue(_subvalue);
}

/*--------------------------------------------------------------------------------*/
/** Set XMLValue object as a XML value (name/value pair) with optional attributes
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::SetValueOrAttribute(const std::string& _name, const std::string& _value, bool _attr)
{
  attr  = _attr;
  name  = _name;
  value = _value;
  attrs.clear();
}

void XMLValue::SetValueOrAttribute(const std::string& _name, const char *_value, bool _attr)
{
  attr  = _attr;
  name  = _name;
  value = _value;
  attrs.clear();
}

void XMLValue::SetValueOrAttribute(const std::string& _name, uint64_t _value, bool _attr)
{
  SetValueOrAttribute(_name, GenerateTime(_value), _attr);
}

/*--------------------------------------------------------------------------------*/
/** Set attribute of XMLValue value object (initialised above)
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::SetValueAttribute(const std::string& _name, const std::string& _value)
{
  attrs[_name] = _value;
}

void XMLValue::SetValueAttribute(const std::string& _name, const char *_value)
{
  attrs[_name] = _value;
}

void XMLValue::SetValueAttribute(const std::string& _name, uint64_t _value)
{
  attrs[_name] = GenerateTime(_value);
}

/*--------------------------------------------------------------------------------*/
/** Return ptr to attributes within specified value with specified name or NULL
 */
/*--------------------------------------------------------------------------------*/
const std::string *XMLValue::GetAttribute(const std::string& _name) const
{
  ATTRS::const_iterator it;
  const std::string *attr = NULL;

  if ((it = attrs.find(_name)) != attrs.end())
  {
    attr = &it->second;
  }

  return attr;
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Add a value to the internal list
 */
/*--------------------------------------------------------------------------------*/
void XMLValues::AddValue(const XMLValue& value)
{
  push_back(value);
}

/*--------------------------------------------------------------------------------*/
/** Return ptr to value with specified name or NULL
 */
/*--------------------------------------------------------------------------------*/
const XMLValue* XMLValues::GetValue(const std::string& name) const
{
  const XMLValue *value = NULL;
  uint_t i;

  // simple search and compare
  // (MUST use a list because there can be MULTIPLE values of the same name)
  for (i = 0; i < size(); i++)
  {
    if (operator[](i).name == name)
    {
      value = &operator[](i);
      break;
    }
  }

  if (!value) BBCDEBUG4(("No value named '%s'!", name.c_str()));

  return value;
}

/*--------------------------------------------------------------------------------*/
/** Remove and delete value from internal list of values
 *
 * @param value address of value to be erased
 *
 * @note value passed MUST be the address of the desired item
 */
/*--------------------------------------------------------------------------------*/
void XMLValues::EraseValue(const XMLValue *value)
{
  iterator it;
    
  for (it = begin(); it != end(); ++it)
  {
    const XMLValue& value1 = *it;

    // note address comparison not value comparison!
    if (value == &value1)
    {
      erase(it);
      break;
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValue(std::string& res, const std::string& name)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    res = value->value;
    EraseValue(value);
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValue(double& res, const std::string& name)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = (sscanf(value->value.c_str(), "%lf", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 * @param hex true if value is expected to be hexidecimal
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValue(uint_t& res, const std::string& name, bool hex)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%x", &res) > 0);
    else     success = (sscanf(value->value.c_str(), "%u", &res) > 0);

    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValue(ulong_t& res, const std::string& name, bool hex)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%lx", &res) > 0);
    else     success = (sscanf(value->value.c_str(), "%lu", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 * @param hex true if value is expected to be hexidecimal
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValue(sint_t& res, const std::string& name, bool hex)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%x", (uint_t *)&res) > 0);
    else     success = (sscanf(value->value.c_str(), "%d", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 * @param hex true if value is expected to be hexidecimal
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValue(slong_t& res, const std::string& name, bool hex)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%lx", (ulong_t *)&res) > 0);
    else     success = (sscanf(value->value.c_str(), "%ld", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValue(bool& res, const std::string& name)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    res = (value->value == "true");
    EraseValue(value);
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::SetValueTime(uint64_t& res, const std::string& name)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = CalcTime(res, value->value);
    EraseValue(value);
  }

  return success;
}


BBC_AUDIOTOOLBOX_END
