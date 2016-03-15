#ifndef __ADM_XML_GENERATOR__
#define __ADM_XML_GENERATOR__

#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A simple XML generator for ADM data, requires no external libraries
 */
/*--------------------------------------------------------------------------------*/
namespace ADMXMLGenerator
{
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
  extern std::string GetAxml(const ADMData *adm, bool ebumode = true, const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0);

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
  extern uint64_t GetAxmlBuffer(const ADMData *adm, uint8_t *buf, uint64_t buflen, bool ebumode = true, const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0);
};

BBC_AUDIOTOOLBOX_END

#endif
