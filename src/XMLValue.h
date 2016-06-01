#ifndef __XML_VALUE__
#define __XML_VALUE__

#include <vector>
#include <map>
#include <string>

#include <bbcat-base/NamedParameter.h>

BBC_AUDIOTOOLBOX_START

class XMLValues;
class XMLValue
{
public:
  XMLValue();
  XMLValue(const XMLValue& obj);
  ~XMLValue();

  /*--------------------------------------------------------------------------------*/
  /** Assignment operator
   */
  /*--------------------------------------------------------------------------------*/
  XMLValue& operator = (const XMLValue& obj);

  /*--------------------------------------------------------------------------------*/
  /** Set subvalues list
   */
  /*--------------------------------------------------------------------------------*/
  void SetSubValues(const XMLValues *_subvalues);

  /*--------------------------------------------------------------------------------*/
  /** Add a set of sub values to the list
   */
  /*--------------------------------------------------------------------------------*/
  void AddSubValues(const XMLValues& _subvalues);

  /*--------------------------------------------------------------------------------*/
  /** Add a single sub value to the list
   */
  /*--------------------------------------------------------------------------------*/
  void AddSubValue(const XMLValue& _subvalue);

  /*--------------------------------------------------------------------------------*/
  /** Add a single sub value to the list (if set or forced)
   */
  /*--------------------------------------------------------------------------------*/
  void AddSubValue(const INamedParameter& _parameter, bool force = false);

  /*--------------------------------------------------------------------------------*/
  /** Return sub value list or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValues *GetSubValues() const {return subvalues;}

  /*--------------------------------------------------------------------------------*/
  /** Return sub value list or NULL
   */
  /*--------------------------------------------------------------------------------*/
  XMLValues *GetSubValuesWritable() const {return subvalues;}
  
  /*--------------------------------------------------------------------------------*/
  /** Set XMLValue object as a simple XML attribute (name/value pair)
   */
  /*--------------------------------------------------------------------------------*/
  void SetAttribute(const std::string& _name, const std::string& _value) {SetValueOrAttribute(_name, _value, true);}
  void SetAttribute(const std::string& _name, const char        *_value) {SetValueOrAttribute(_name, _value, true);}
  void SetAttribute(const std::string& _name, bool               _value) {SetValueOrAttribute(_name, _value, true);}
  void SetAttribute(const std::string& _name, sint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, true, format);}
  void SetAttribute(const std::string& _name, uint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, true, format);}
  void SetAttribute(const std::string& _name, uint64_t           _value, const char *format = "")    {SetValueOrAttribute(_name, _value, true, format);}
  void SetAttribute(const std::string& _name, double             _value, const char *format = "0.6") {SetValueOrAttribute(_name, _value, true, format);}
  void SetTimeAttribute(const std::string& _name, uint64_t       _value)                             {SetTimeValueOrAttribute(_name, _value, true);}

  /*--------------------------------------------------------------------------------*/
  /** Set attribute from NamedParameter, if set (or forced)
   */
  /*--------------------------------------------------------------------------------*/
  void SetAttribute(const INamedParameter& _parameter, bool force = false);

  /*--------------------------------------------------------------------------------*/
  /** Set XMLValue object as a XML value (name/value pair) with optional attributes
   */
  /*--------------------------------------------------------------------------------*/
  void SetValue(const std::string& _name, const std::string& _value) {SetValueOrAttribute(_name, _value, false);}
  void SetValue(const std::string& _name, const char        *_value) {SetValueOrAttribute(_name, _value, false);}
  void SetValue(const std::string& _name, bool               _value) {SetValueOrAttribute(_name, _value, false);}
  void SetValue(const std::string& _name, sint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, false, format);}
  void SetValue(const std::string& _name, uint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, false, format);}
  void SetValue(const std::string& _name, uint64_t           _value, const char *format = "")    {SetValueOrAttribute(_name, _value, false, format);}
  void SetValue(const std::string& _name, double             _value, const char *format = "0.6") {SetValueOrAttribute(_name, _value, false, format);}
  void SetTimeValue(const std::string& _name, uint64_t       _value)                             {SetTimeValueOrAttribute(_name, _value, false);}

  /*--------------------------------------------------------------------------------*/
  /** Set value from NamedParameter, if set (or forced)
   */
  /*--------------------------------------------------------------------------------*/
  void SetValue(const INamedParameter& _parameter, bool force = false) {if (force || _parameter.IsSet()) SetValueOrAttribute(_parameter.GetName(), _parameter.ToString(), false);}

  /*--------------------------------------------------------------------------------*/
  /** Return ptr to attributes within specified value with specified name or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const std::string *GetAttribute(const std::string& _name) const;

  /*--------------------------------------------------------------------------------*/
  /** Erase attribute
   */
  /*--------------------------------------------------------------------------------*/
  bool EraseAttribute(const std::string& _name);

  /*--------------------------------------------------------------------------------*/
  /** Set a NamedParameter from an attribute
   *
   * @note if the attribute is found, it is ERASED from the list
   */
  /*--------------------------------------------------------------------------------*/
  bool GetAttributeAndErase(INamedParameter& val);
  
  /*--------------------------------------------------------------------------------*/
  /** Arbitary _values list handling
   */
  /*--------------------------------------------------------------------------------*/
  // each 'value' can also have a list of attributes (a consequence of XML)
  typedef std::map<std::string,std::string> ATTRS;

  std::string name;         // name of value
  std::string value;        // value
  ATTRS       attrs;        // additional attributes (if attr == false)

  /*--------------------------------------------------------------------------------*/
  /** Return whether value has attributes or not
   */
  /*--------------------------------------------------------------------------------*/
  bool HasAttributes() const {return (attrs.end() != attrs.begin());}

  /*--------------------------------------------------------------------------------*/
  /** Return whether this object is 'empty'
   *
   * 'empty' means:
   * 1. no value
   * 2. no attributes
   * 3. no subvalues (or 0-length list)
   */
  /*--------------------------------------------------------------------------------*/
  bool Empty() const;
  
protected:
  /*--------------------------------------------------------------------------------*/
  /** Set XMLValue object as a XML value (name/value pair) with optional attributes
   */
  /*--------------------------------------------------------------------------------*/
  void SetValueOrAttribute(const std::string& _name, const std::string& _value, bool _attr);
  void SetValueOrAttribute(const std::string& _name, const char        *_value, bool _attr);
  void SetValueOrAttribute(const std::string& _name, bool               _value, bool _attr) {SetValueOrAttribute(_name, StringFrom(_value), _attr);}
  void SetValueOrAttribute(const std::string& _name, sint_t             _value, bool _attr, const char *format = "")    {SetValueOrAttribute(_name, StringFrom(_value, format), _attr);}
  void SetValueOrAttribute(const std::string& _name, uint_t             _value, bool _attr, const char *format = "")    {SetValueOrAttribute(_name, StringFrom(_value, format), _attr);}
  void SetValueOrAttribute(const std::string& _name, uint64_t           _value, bool _attr, const char *format = "")    {SetValueOrAttribute(_name, StringFrom(_value, format), _attr);}
  void SetValueOrAttribute(const std::string& _name, double             _value, bool _attr, const char *format = "0.6") {SetValueOrAttribute(_name, StringFrom(_value, format), _attr);}
  void SetTimeValueOrAttribute(const std::string& _name, uint64_t       _value, bool _attr);

protected:
  XMLValues *subvalues;   // list of sub-values  
};

class XMLValues : public std::vector<XMLValue>
{
public:
  XMLValues() : std::vector<XMLValue>() {}
  ~XMLValues() {}

  /*--------------------------------------------------------------------------------*/
  /** Add a value to the internal list
   */
  /*--------------------------------------------------------------------------------*/
  void AddValue(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Add value from NamedParameter (if set or forced)
   */
  /*--------------------------------------------------------------------------------*/
  bool AddValue(const INamedParameter& parameter, bool force = false);
  
  /*--------------------------------------------------------------------------------*/
  /** Return ptr to value with specified name or NULL
   *
   * @note index represents the index'th entry OF THE SPECIFIED NAME and therefore should start at 0 and increment by 1
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValue *GetValue(const std::string& name, uint_t index = 0) const;

  /*--------------------------------------------------------------------------------*/
  /** Return ptr to writable value with specified name or NULL
   *
   * @note index represents the index'th entry OF THE SPECIFIED NAME and therefore should start at 0 and increment by 1
   */
  /*--------------------------------------------------------------------------------*/
  XMLValue *GetValueWritable(const std::string& name, uint_t index = 0);

  /*--------------------------------------------------------------------------------*/
  /** Remove and delete value from internal list of values
   *
   * @param value address of value to be erased
   *
   * @note value passed MUST be the address of the desired item
   */
  /*--------------------------------------------------------------------------------*/
  void EraseValue(const XMLValue *value);

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
  bool GetValueAndErase(std::string& res, const std::string& name);
  bool GetValueAndErase(double& res, const std::string& name);
  bool GetValueAndErase(uint_t& res, const std::string& name, bool hex = false);
  bool GetValueAndErase(ulong_t& res, const std::string& name, bool hex = false);
  bool GetValueAndErase(sint_t& res, const std::string& name, bool hex = false);
  bool GetValueAndErase(slong_t& res, const std::string& name, bool hex = false);
  bool GetValueAndErase(bool& res, const std::string& name);
  bool GetTimeValueAndErase(uint64_t& res, const std::string& name);

  /*--------------------------------------------------------------------------------*/
  /** Set NamedParameter from value
   *
   * @note the value, if found, is REMOVED from the list
   */
  /*--------------------------------------------------------------------------------*/
  bool GetValueAndErase(INamedParameter& parameter);
};

BBC_AUDIOTOOLBOX_END

#endif
