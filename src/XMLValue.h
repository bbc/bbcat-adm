#ifndef __XML_VALUE__
#define __XML_VALUE__

#include <vector>
#include <map>
#include <string>

#include <bbcat-base/misc.h>

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
  /** Add a set of sub values to the lsit
   */
  /*--------------------------------------------------------------------------------*/
  void AddSubValues(const XMLValues& _subvalues);

  /*--------------------------------------------------------------------------------*/
  /** Add a single sub value to the lsit
   */
  /*--------------------------------------------------------------------------------*/
  void AddSubValue(const XMLValue& _subvalue);

  /*--------------------------------------------------------------------------------*/
  /** Return sub value list or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValues *GetSubValues() const {return subvalues;}
  
  /*--------------------------------------------------------------------------------*/
  /** Set XMLValue object as a simple XML attribute (name/value pair)
   */
  /*--------------------------------------------------------------------------------*/
  void SetAttribute(const std::string& _name, const std::string& _value) {SetValueOrAttribute(_name, _value, true);}
  void SetAttribute(const std::string& _name, const char        *_value) {SetValueOrAttribute(_name, _value, true);}
  void SetAttribute(const std::string& _name, bool               _value) {SetValueOrAttribute(_name, _value, true);}
  void SetAttribute(const std::string& _name, sint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, true, format);}
  void SetAttribute(const std::string& _name, uint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, true, format);}
  void SetAttribute(const std::string& _name, uint64_t           _value) {SetValueOrAttribute(_name, _value, true);}
  void SetAttribute(const std::string& _name, double             _value, const char *format = "0.6") {SetValueOrAttribute(_name, _value, true, format);}

  /*--------------------------------------------------------------------------------*/
  /** Set XMLValue object as a XML value (name/value pair) with optional attributes
   */
  /*--------------------------------------------------------------------------------*/
  void SetValue(const std::string& _name, const std::string& _value) {SetValueOrAttribute(_name, _value, false);}
  void SetValue(const std::string& _name, const char        *_value) {SetValueOrAttribute(_name, _value, false);}
  void SetValue(const std::string& _name, bool               _value) {SetValueOrAttribute(_name, _value, false);}
  void SetValue(const std::string& _name, sint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, false, format);}
  void SetValue(const std::string& _name, uint_t             _value, const char *format = "")    {SetValueOrAttribute(_name, _value, false, format);}
  void SetValue(const std::string& _name, uint64_t           _value) {SetValueOrAttribute(_name, _value, false);}
  void SetValue(const std::string& _name, double             _value, const char *format = "0.6") {SetValueOrAttribute(_name, _value, false, format);}

  /*--------------------------------------------------------------------------------*/
  /** Set attribute of XMLValue value object (initialised above)
   */
  /*--------------------------------------------------------------------------------*/
  void SetValueAttribute(const std::string& _name, const std::string& _value);
  void SetValueAttribute(const std::string& _name, const char        *_value);
  void SetValueAttribute(const std::string& _name, bool               _value) {SetValueAttribute(_name, StringFrom(_value));}
  void SetValueAttribute(const std::string& _name, sint_t             _value, const char *format = "")    {SetValueAttribute(_name, StringFrom(_value, format));}
  void SetValueAttribute(const std::string& _name, uint_t             _value, const char *format = "")    {SetValueAttribute(_name, StringFrom(_value, format));}
  void SetValueAttribute(const std::string& _name, uint64_t           _value);
  void SetValueAttribute(const std::string& _name, double             _value, const char *format = "0.6") {SetValueAttribute(_name, StringFrom(_value, format));}

  /*--------------------------------------------------------------------------------*/
  /** Return ptr to attributes within specified value with specified name or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const std::string *GetAttribute(const std::string& _name) const;
  
  /*--------------------------------------------------------------------------------*/
  /** Arbitary _values list handling
   */
  /*--------------------------------------------------------------------------------*/
  // each 'value' can also have a list of attributes (a consequence of XML)
  typedef std::map<std::string,std::string> ATTRS;

  bool        attr;         // true if this value is a simple XML attribute
  std::string name;         // name of value
  std::string value;        // value
  ATTRS       attrs;        // additional attributes (if attr == false)
  
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
  void SetValueOrAttribute(const std::string& _name, uint64_t           _value, bool _attr);
  void SetValueOrAttribute(const std::string& _name, double             _value, bool _attr, const char *format = "0.6") {SetValueOrAttribute(_name, StringFrom(_value, format), _attr);}

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
  /** Return ptr to value with specified name or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const XMLValue *GetValue(const std::string& name) const;

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
  bool SetValue(std::string& res, const std::string& name);
  bool SetValue(double& res, const std::string& name);
  bool SetValue(uint_t& res, const std::string& name, bool hex = false);
  bool SetValue(ulong_t& res, const std::string& name, bool hex = false);
  bool SetValue(sint_t& res, const std::string& name, bool hex = false);
  bool SetValue(slong_t& res, const std::string& name, bool hex = false);
  bool SetValue(bool& res, const std::string& name);
  bool SetValueTime(uint64_t& res, const std::string& name);
};

BBC_AUDIOTOOLBOX_END

#endif
