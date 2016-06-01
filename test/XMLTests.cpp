

#include <catch/catch.h>

#include "ADMData.h"

USE_BBC_AUDIOTOOLBOX

TEST_CASE("create_and_delete")
{
  std::unique_ptr<ADMData> adm(ADMData::Create());

  BOOST_CHECK(adm);
}

bool readtextfile(std::string& str, const char *filename)
{
  EnhancedFile file;
  bool success = false;
  
  if (file.fopen(filename, "r"))
  {
    file.fseek(0, SEEK_END);
    ulong_t len = file.ftell();
    file.rewind();

    char *p;
    if ((p = new char[len + 1]) != NULL)
    {
      file.fread(p, len, 1);
      p[len] = 0;

      str = p;
      delete[] p;

      success = true;
    }

    file.fclose();
  }

  return success;
}

TEST_CASE("xml_generation")
{
  std::unique_ptr<ADMData> adm(ADMData::Create());

  BOOST_CHECK(adm);

  std::string inputdata;
  BOOST_CHECK(readtextfile(inputdata, "inputdata.xml"));

  adm->SetAxml(inputdata);
  
  std::string xml = adm->GetAxml();
  EnhancedFile file;
  if (file.fopen("result.xml", "w"))
  {
    file.fprintf("%s", xml.c_str());
    file.fclose();
  }
  
  BOOST_CHECK(xml == inputdata);
}
