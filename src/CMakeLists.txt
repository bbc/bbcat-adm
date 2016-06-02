#sources
set(_sources
	ADMData.cpp
	ADMObjects.cpp
	ADMSubObjects.cpp
	ADMXMLGenerator.cpp
	AudioObjectParameters.cpp
	XMLValue.cpp
)

# public headers
set(_headers
	ADMData.h
	ADMObjects.h
	ADMSubObjects.h
	ADMXMLGenerator.h
	AudioObjectCursor.h
	AudioObjectParameters.h
	XMLValue.h
	register.h
)

# auto-generate register.cpp
bbcat_generate_register_cpp(DEPENDENCIES "bbcat_dsp")

#include all the parts that are consistent across all projects
include(${BBCAT_COMMON_DIR}/CMakeLibrary.txt)

TARGET_LINK_LIBRARIES(bbcat-adm bbcat-dsp bbcat-base)
