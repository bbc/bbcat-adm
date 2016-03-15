
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BBCDEBUG_LEVEL 1
#include "ADMXMLGenerator.h"

BBC_AUDIOTOOLBOX_START

namespace ADMXMLGenerator
{
  /*--------------------------------------------------------------------------------*/
  /** Context structure for generating XML (this object ONLY!)
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    const ADMData *adm;
    struct {
      std::string *str;                 ///< ptr string into which XML is generated (or NULL)
      char        *buf;                 ///< raw char buffer (PRE-ALLOCATED) to generate XML in
      uint64_t    buflen;               ///< max length of above length (EXCLUDING terminator)
    } destination;                      ///< destination control
    std::string indent;                 ///< indent string for each level
    std::string eol;                    ///< end-of-line string
    uint64_t    length;                 ///< length of XML data at completion
    uint_t      ind_level;              ///< current indentation level
    bool        ebumode;                ///< true to generate EBU XML format, false to generate ITU XML format
    bool        opened;                 ///< true if object is started but not ready for data (needs '>')
    bool        complete;               ///< dump ALL objects, not just programme, content or objects
    bool        eollast;                ///< string currently ends with an eol
    std::vector<std::string> stack;     ///< object stack
  } TEXTXML;

  /*--------------------------------------------------------------------------------*/
  /** Encode XML string
   */
  /*--------------------------------------------------------------------------------*/
  std::string EscapeXML(const std::string& _str)
  {
    std::string str = _str;
    str = SearchAndReplace(str, "&",  "&amp;");
    str = SearchAndReplace(str, "<",  "&lt;");
    str = SearchAndReplace(str, ">",  "&gt;");
    str = SearchAndReplace(str, "\"", "&quot;");
    str = SearchAndReplace(str, "'",  "&apos;");
    return str;
  }
  
  /*--------------------------------------------------------------------------------*/
  /** Append string to XML context
   *
   * @param xml user supplied argument representing context data
   * @param str string to be appended/counted
   */
  /*--------------------------------------------------------------------------------*/
  void AppendXML(TEXTXML& xml, const std::string& str)
  {
    if (str.length())
    {
      // copy string to destination, if one specified
      if      (xml.destination.str) *xml.destination.str += str;
      else if (xml.destination.buf &&
               ((xml.length + str.length()) <= xml.destination.buflen)) strcpy(xml.destination.buf + xml.length, str.c_str());
  
      // update length
      xml.length += str.length();

      // update flag to indicate whether buffer ends with an eol 
      xml.eollast = (xml.eol.length() && (str.length() >= xml.eol.length()) && (str.substr(str.length() - xml.eol.length()) == xml.eol));
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Start XML
   *
   * @param xml user supplied argument representing context data
   * @param version XML version
   * @param encoding XML encoding
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void StartXML(TEXTXML& xml, const std::string& version = "1.0", const std::string& encoding = "UTF-8")
  {
    std::string str;
    Printf(str,
           "%s<?xml version=\"%s\" encoding=\"%s\"?>%s",
           CreateIndent(xml.indent, xml.ind_level).c_str(),
           version.c_str(),
           encoding.c_str(),
           xml.eol.c_str());

    AppendXML(xml, str);
  }

  /*--------------------------------------------------------------------------------*/
  /** Start an XML object
   *
   * @param xml user supplied argument representing context data
   * @param name object name
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void OpenXMLObject(TEXTXML& xml, const std::string& name)
  {
    // ensure any previous object is properly marked ready for data
    if (xml.stack.size() && xml.opened)
    {
      AppendXML(xml, ">");
      xml.opened = false;
    }

    // add a newline if last bit of string isn't an eol
    if (!xml.eollast) AppendXML(xml, xml.eol);

    std::string str;
    Printf(str,
           "%s<%s",
           CreateIndent(xml.indent, xml.ind_level + (uint_t)xml.stack.size()).c_str(),
           name.c_str());

    AppendXML(xml, str);
  
    // stack this object name (for closing)
    xml.stack.push_back(name);
    xml.opened = true;
  }
  
  /*--------------------------------------------------------------------------------*/
  /** Add an attribute to the current XML object
   *
   * @param xml user supplied argument representing context data
   * @param name attribute name
   * @param name attribute value
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void AddXMLAttribute(TEXTXML& xml, const std::string& name, const std::string& value)
  {
    std::string str;
    Printf(str, " %s=\"%s\"", name.c_str(), EscapeXML(value).c_str());

    AppendXML(xml, str);
  }

  /*--------------------------------------------------------------------------------*/
  /** Set XML data
   *
   * @param xml user supplied argument representing context data
   * @param data data
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void SetXMLData(TEXTXML& xml, const std::string& data)
  {
    // ensure any object is marked ready for data
    if (xml.stack.size() && xml.opened)
    {
      AppendXML(xml, ">");
      xml.opened = false;
    }

    AppendXML(xml, EscapeXML(data));
  }

  /*--------------------------------------------------------------------------------*/
  /** Close XML object
   *
   * @param xml user supplied argument representing context data
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void CloseXMLObject(TEXTXML& xml)
  {
    if (xml.stack.size() && xml.opened)
    {
      // object is empty
      AppendXML(xml, " />" + xml.eol);
      xml.opened = false;
    }
    else
    {
      if (xml.eollast) AppendXML(xml, CreateIndent(xml.indent, xml.ind_level + (uint_t)xml.stack.size() - 1));

      std::string str;
      Printf(str, "</%s>%s", xml.stack.back().c_str(), xml.eol.c_str());
      AppendXML(xml, str);
    }

    xml.stack.pop_back();
  }

  /*--------------------------------------------------------------------------------*/
  /** Add attributes from list of XML values to current object header
   *
   * @param xml user supplied argument representing context data
   * @param values list of XML values
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void AddXMLAttributes(TEXTXML& xml, const XMLValues& values)
  {
    uint_t i;
  
    // output attributes
    for (i = 0; i < values.size(); i++)
    {
      const XMLValue& value = values[i];

      if (value.attr)
      {
        AddXMLAttribute(xml, value.name, value.value);
      }
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Add values (non-attributres) from list of XML values to object
   *
   * @param xml user supplied argument representing context data
   * @param values list of XML values
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void AddXMLValues(TEXTXML& xml, const XMLValues& values)
  {
    uint_t i;
  
    // output values
    for (i = 0; i < values.size(); i++)
    {
      const XMLValue& value = values[i];

      if (!value.attr)
      {
        XMLValue::ATTRS::const_iterator it;
        const XMLValues *subvalues;
        
        OpenXMLObject(xml, value.name);

        for (it = value.attrs.begin(); it != value.attrs.end(); ++it)
        {
          AddXMLAttribute(xml, it->first, it->second);
        }

        // if value has sub-values, output those
        if ((subvalues = value.GetSubValues()) != NULL) AddXMLValues(xml, *subvalues);
        // otherwise output single value
        else SetXMLData(xml, value.value);

        CloseXMLObject(xml);
      }
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Append extra (non-ADM) XML for level in the stack
   */
  /*--------------------------------------------------------------------------------*/
  void AppendExtraXML(TEXTXML& xml)
  {
    if (xml.stack.size() && xml.opened)
    {
      const XMLValues *values;
      std::string parentname = xml.stack[xml.stack.size() - 1];

      // for root node, use empty name
      if ((parentname == "ebuCoreMain") ||
          (parentname == "ituADM")) parentname = "";

      if ((values = xml.adm->GetNonADMXML(parentname)) != NULL)
      {
        // extra data exists, add it
        AddXMLValues(xml, *values);
      }
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Generic XML creation
   *
   * @param obj ADM object to generate XML for
   * @param xml user supplied argument representing context data
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void GenerateXML(const ADMObject *obj, TEXTXML& xml)
  {
    if (xml.complete || !obj->IsStandardDefinition())
    {
      std::vector<ADMObject::REFERENCEDOBJECT> objects;
      XMLValues values;
      uint_t    i;
      bool      emptyobject = true;

      obj->GetValuesAndReferences(values, objects);

      // if object has contained objects, it cannot be empty
      emptyobject &= (obj->GetContainedObjectCount() == 0);
    
      // test to see if this object is 'empty'
      for (i = 0; emptyobject && (i < values.size()); i++)
      {
        emptyobject &= !(!values[i].attr);                            // if any values (non-attribute) found, object cannot be empty
      }
      for (i = 0; emptyobject && (i < objects.size()); i++)
      {
        emptyobject &= !objects[i].genref;                            // if any references to be generated, object cannot be empty
      }

      // start XML object
      OpenXMLObject(xml, obj->GetType());
      AddXMLAttributes(xml, values);
                     
      if (!emptyobject)
      {
        AddXMLValues(xml, values);
                   
        // output references
        for (i = 0; i < objects.size(); i++)
        {
          const ADMObject::REFERENCEDOBJECT& object = objects[i];

          if (object.genref)
          {
            // output reference to object.obj
            OpenXMLObject(xml, object.obj->GetReference());
            SetXMLData(xml, object.obj->GetID());
            CloseXMLObject(xml);
          }
        }

        // output contained data
        ADMObject::CONTAINEDOBJECT object;
        for (i = 0; obj->GetContainedObject(i, object); i++)
        {
          OpenXMLObject(xml, object.type);
          AddXMLAttributes(xml, object.attrs);
          AddXMLValues(xml, object.values);
          CloseXMLObject(xml);
        }

        // end XML object
        CloseXMLObject(xml);
      }
      else
      {
        // end empty XML object
        CloseXMLObject(xml);
      }
    }
  }
  
  /*--------------------------------------------------------------------------------*/
  /** Generic XML creation
   *
   * @param xml user supplied argument representing context data
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  void GenerateXML(TEXTXML& xml)
  {
    std::vector<const ADMObject *> list;
    std::string types[] = {
      ADMAudioProgramme::Type,
      ADMAudioContent::Type,
      ADMAudioObject::Type,
      ADMAudioPackFormat::Type,
      ADMAudioChannelFormat::Type,
      ADMAudioStreamFormat::Type,
      ADMAudioTrackFormat::Type,
      ADMAudioTrack::Type,
    };
    uint_t i, j;

    for (i = 0; i < 3; i++) // only collect objects of the first three types (programme, content and object)
    {
      // get all objects of the specified type and add them to the lsit
      xml.adm->GetObjects(types[i], list);
    }

    // add referenced objects to list
    xml.adm->GetReferencedObjects(list);

    StartXML(xml);
    
    if (xml.ebumode)
    {
      // EBU version of XML
      OpenXMLObject(xml, "ebuCoreMain");
      AddXMLAttribute(xml, "xmlns:dc", "http://purl.org/dc/elements/1.1/");
      AddXMLAttribute(xml, "xmlns", "urn:ebu:metadata-schema:ebuCore_2014");
      AddXMLAttribute(xml, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
      AddXMLAttribute(xml, "schema", "EBU_CORE_20140201.xsd");
      AddXMLAttribute(xml, "xml:lang", "en");
    }
    else
    {
      // ITU version of XML
      OpenXMLObject(xml, "ituADM");
      AddXMLAttribute(xml, "xmlns", "urn:metadata-schema:adm");
    }

    AppendExtraXML(xml);

    OpenXMLObject(xml, "coreMetadata");
    AppendExtraXML(xml);

    OpenXMLObject(xml, "format");
    AppendExtraXML(xml);

    OpenXMLObject(xml, "audioFormatExtended");
    AppendExtraXML(xml);

    for (i = 0; i < NUMBEROF(types); i++)
    {
      // find objects of correct type and output them
      for (j = 0; j < list.size(); j++)
      {
        const ADMObject *obj = list[j];

        // can this object be outputted?
        if (obj->GetType() == types[i])
        {
          GenerateXML(obj, xml);
        }
      }
    }

    CloseXMLObject(xml);
    CloseXMLObject(xml);
    CloseXMLObject(xml);
    CloseXMLObject(xml);
  }

  /*--------------------------------------------------------------------------------*/
  /** Create XML representation of ADM
   *
   * @param adm ADMData structure holding description of ADM
   * @param str std::string to be modified with XML
   * @param ebumode true (default) to generate EBU XML format, false to generate ITU XML format
   * @param indent indentation for each level of objects
   * @param eol end-of-line string
   * @param level initial indentation level
   *
   * @return total length of XML
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GenerateXML(const ADMData *adm, std::string& str, bool ebumode, const std::string& indent, const std::string& eol, uint_t ind_level, bool complete = false)
  {
    TEXTXML context;

    context.adm       = adm;
    
    // clear destination data
    memset(&context.destination, 0, sizeof(context.destination));

    // set string destination to use
    context.destination.str = &str;

    context.ebumode   = ebumode;
    context.indent    = indent;
    context.eol       = eol;
    context.ind_level = ind_level;
    context.length    = 0;
    context.opened    = false;
    context.complete  = complete;
    context.eollast   = false;
  
    GenerateXML(context);

    return context.length;
  }

  /*--------------------------------------------------------------------------------*/
  /** Create XML representation of ADM
   *
   * @param adm ADMData structure holding description of ADM
   * @param buf buffer to store XML in (or NULL to just get the length)
   * @param buflen maximum length of buf (excluding terminator)
   * @param ebumode true (default) to generate EBU XML format, false to generate ITU XML format
   * @param indent indentation for each level of objects
   * @param eol end-of-line string
   * @param level initial indentation level
   *
   * @return total length of XML
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GenerateXMLBuffer(const ADMData *adm, uint8_t *buf, uint64_t buflen, bool ebumode, const std::string& indent, const std::string& eol, uint_t ind_level, bool complete = false)
  {
    TEXTXML context;

    context.adm       = adm;

    // clear destination data
    memset(&context.destination, 0, sizeof(context.destination));

    // set string destination to use
    context.destination.buf    = (char *)buf;
    context.destination.buflen = buflen;

    context.ebumode   = ebumode;
    context.indent    = indent;
    context.eol       = eol;
    context.ind_level = ind_level;
    context.length    = 0;
    context.complete  = complete;
    context.eollast   = false;
  
    GenerateXML(context);

    return context.length;
  }

  /*--------------------------------------------------------------------------------*/
  /** Create axml chunk data
   *
   * @param adm ADMData structure holding description of ADM
   * @param ebumode true (default) to generate EBU XML format, false to generate ITU XML format
   * @param indent indent string to use within XML
   * @param eol end of line string to use within XML
   * @param ind_level initial indentation level
   *
   * @return string containing XML data for axml chunk
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetAxml(const ADMData *adm, bool ebumode, const std::string& indent, const std::string& eol, uint_t ind_level)
  {
    std::string str;

    GenerateXML(adm, str, ebumode, indent, eol, ind_level);

    BBCDEBUG3(("Generated XML:\n%s", str.c_str()));

    return str;
  }

  /*--------------------------------------------------------------------------------*/
  /** Create XML representation of ADM in a data buffer
   *
   * @param adm ADMData structure holding description of ADM
   * @param buf buffer to store XML in (or NULL to just get the length)
   * @param buflen maximum length of buf (excluding terminator)
   * @param ebumode true (default) to generate EBU XML format, false to generate ITU XML format
   * @param indent indentation for each level of objects
   * @param eol end-of-line string
   * @param level initial indentation level
   *
   * @return total length of XML
   */
  /*--------------------------------------------------------------------------------*/
  uint64_t GetAxmlBuffer(const ADMData *adm, uint8_t *buf, uint64_t buflen, bool ebumode, const std::string& indent, const std::string& eol, uint_t ind_level)
  {
    return GenerateXMLBuffer(adm, buf, buflen, ebumode, indent, eol, ind_level);
  }
};

BBC_AUDIOTOOLBOX_END
