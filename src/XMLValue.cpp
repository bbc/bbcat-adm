
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BBCDEBUG_LEVEL 1
#include "XMLValue.h"

BBC_AUDIOTOOLBOX_START

XMLValue::XMLValue() : subvalues(NULL)
{
}

XMLValue::XMLValue(const XMLValue& obj) : subvalues(NULL)
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
/** Add a single sub value to the list (if set or forced)
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::AddSubValue(const INamedParameter& _parameter, bool force)
{
  if (force || _parameter.IsSet())
  {
    XMLValue subvalue;
    subvalue.SetValue(_parameter, force);
    AddSubValue(subvalue);
  }
}

/*--------------------------------------------------------------------------------*/
/** Set attribute from NamedParameter, if set (or forced)
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::SetAttribute(const INamedParameter& _parameter, bool force)
{
  if (force || _parameter.IsSet()) SetValueOrAttribute(_parameter.GetName(), _parameter.ToString(), true);
}

/*--------------------------------------------------------------------------------*/
/** Set XMLValue object as a XML value (name/value pair) with optional attributes
 */
/*--------------------------------------------------------------------------------*/
void XMLValue::SetValueOrAttribute(const std::string& _name, const std::string& _value, bool _attr)
{
  if (_attr) attrs[_name] = _value;
  else
  {
    name  = _name;
    value = _value;
  }
}

void XMLValue::SetValueOrAttribute(const std::string& _name, const char *_value, bool _attr)
{
  if (_attr) attrs[_name] = _value;
  else
  {
    name  = _name;
    value = _value;
  }
}

void XMLValue::SetTimeValueOrAttribute(const std::string& _name, uint64_t _value, bool _attr)
{
  SetValueOrAttribute(_name, GenerateTime(_value), _attr);
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

/*--------------------------------------------------------------------------------*/
/** Erase attribute
 */
/*--------------------------------------------------------------------------------*/
bool XMLValue::EraseAttribute(const std::string& _name)
{
  ATTRS::iterator it;
  bool success = false;
  
  if ((it = attrs.find(_name)) != attrs.end())
  {
    attrs.erase(it);
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set a NamedParameter from an attribute
 *
 * @note if the attribute is found, it is ERASED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValue::GetAttributeAndErase(INamedParameter& val)
{
  ATTRS::iterator it;
  bool success = false;

  if ((it = attrs.find(val.GetName())) != attrs.end())
  {
    success = val.FromString(it->second);
    BBCDEBUG5(("%s<%s>: Converted '%s' to '%s'", val.GetName(), StringFrom(&val).c_str(), it->second.c_str(), val.ToString().c_str()));
    
    // erase attribute from list
    attrs.erase(it);
  }
  else BBCDEBUG4(("%s<%s>: Attribute not found!", val.GetName(), StringFrom(&val).c_str()));
  
  return success;
}

/*--------------------------------------------------------------------------------*/
/** Return whether this object is 'empty'
 *
 * 'empty' means:
 * 1. no value
 * 2. no attributes
 * 3. no subvalues (or 0-length list)
 */
/*--------------------------------------------------------------------------------*/
bool XMLValue::Empty() const
{
  return value.empty() && !HasAttributes() && (!subvalues || !subvalues->size());
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
/** Set value from NamedParameter
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::AddValue(const INamedParameter& parameter, bool force)
{
  bool added = false;
  
  if (force || parameter.IsSet())
  {
    XMLValue value;
    value.SetValue(parameter, force);
    AddValue(value);
  }

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Return ptr to value with specified name or NULL
 *
 * @note index represents the index'th entry OF THE SPECIFIED NAME and therefore should start at 0 and increment by 1
 */
/*--------------------------------------------------------------------------------*/
const XMLValue* XMLValues::GetValue(const std::string& name, uint_t index) const
{
  const XMLValue *value = NULL;
  uint_t i;

  // simple search and compare
  // (MUST use a list because there can be MULTIPLE values of the same name)
  for (i = 0; i < size(); i++)
  {
    if (operator[](i).name == name)
    {
      // find indexed copy
      if (index) index--;
      else
      {
        value = &operator[](i);
        break;
      }
    }
  }

  if (!value) BBCDEBUG4(("No value named '%s' index %u!", name.c_str(), index));

  return value;
}

/*--------------------------------------------------------------------------------*/
/** Return ptr to value with specified name or NULL
 *
 * @note index represents the index'th entry OF THE SPECIFIED NAME and therefore should start at 0 and increment by 1
 */
/*--------------------------------------------------------------------------------*/
XMLValue* XMLValues::GetValueWritable(const std::string& name, uint_t index)
{
  XMLValue *value = NULL;
  uint_t i;

  // simple search and compare
  // (MUST use a list because there can be MULTIPLE values of the same name)
  for (i = 0; i < size(); i++)
  {
    if (operator[](i).name == name)
    {
      // find indexed copy
      if (index) index--;
      else
      {
        value = &operator[](i);
        break;
      }
    }
  }

  if (!value) BBCDEBUG4(("No value named '%s' index %u!", name.c_str(), index));

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
bool XMLValues::GetValueAndErase(std::string& res, const std::string& name)
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
bool XMLValues::GetValueAndErase(double& res, const std::string& name)
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
bool XMLValues::GetValueAndErase(uint_t& res, const std::string& name, bool hex)
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
bool XMLValues::GetValueAndErase(ulong_t& res, const std::string& name, bool hex)
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
bool XMLValues::GetValueAndErase(sint_t& res, const std::string& name, bool hex)
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
bool XMLValues::GetValueAndErase(slong_t& res, const std::string& name, bool hex)
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
bool XMLValues::GetValueAndErase(bool& res, const std::string& name)
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
bool XMLValues::GetTimeValueAndErase(uint64_t& res, const std::string& name)
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

/*--------------------------------------------------------------------------------*/
/** Set NamedParameter from value
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool XMLValues::GetValueAndErase(INamedParameter& parameter)
{
  const XMLValue *value;
  bool success = false;

  if ((value = GetValue(parameter.GetName())) != NULL)
  {
    success = parameter.FromString(value->value);
    EraseValue(value);
  }

  return success;
}

BBC_AUDIOTOOLBOX_END
