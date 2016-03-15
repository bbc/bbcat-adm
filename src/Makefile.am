pkgincludedir = $(includedir)/bbcat-adm-@BBCAT_ADM_MAJORMINOR@/bbcat-adm

lib_LTLIBRARIES = libbbcat-adm-@BBCAT_ADM_MAJORMINOR@.la

libbbcat_adm_@BBCAT_ADM_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined									\
	-version-info $(BBCAT_ADM_LIBVERSION)			\
     $(BBCAT_ADM_LDFLAGS)

libbbcat_adm_@BBCAT_ADM_MAJORMINOR@_la_LIBADD =	\
	$(BBCAT_BASE_LIBS)							\
	$(BBCAT_DSP_LIBS)

libbbcat_adm_@BBCAT_ADM_MAJORMINOR@_la_CPPFLAGS =	\
	$(BBCAT_BASE_CFLAGS)							\
	$(BBCAT_DSP_CFLAGS)								\
	$(BBCAT_ADM_CFLAGS)								\
	$(BBCAT_GLOBAL_ADM_CFLAGS)

libbbcat_adm_@BBCAT_ADM_MAJORMINOR@_la_SOURCES =	\
	ADMData.cpp										\
	ADMObjects.cpp									\
	ADMXMLGenerator.cpp								\
	AudioObjectParameters.cpp						\
	XMLValue.cpp									\
	version.cpp

pkginclude_HEADERS =							\
	ADMData.h									\
	ADMObjects.h								\
	ADMXMLGenerator.h							\
	AudioObjectCursor.h							\
	AudioObjectParameters.h						\
	SelfRegisteringObjects.h					\
	XMLValue.h									\
	version.h

CLEANFILES = version.h version.cpp SelfRegisteringObjects.h

SelfRegisteringObjects.h: $(libbbcat_adm_@BBCAT_ADM_MAJORMINOR@_la_SOURCES) Makefile Makefile.am
	@./findselfregisteringobjects.sh @PACKAGE@ $@ $(libbbcat_adm_@BBCAT_ADM_MAJORMINOR@_la_SOURCES)

noinst_HEADERS =

version.h: Makefile ../config.h ../gitbranch.txt
	echo "{\"@PACKAGE@\", \"@VERSION@-`cat ../gitbranch.txt`\"}," >$@

version.cpp: Makefile ../config.h ../gitbranch.txt
	echo "/* Auto-generated: DO NOT EDIT! */" >$@
	echo "#include <bbcat-base/LoadedVersions.h>" >>$@
	echo "BBC_AUDIOTOOLBOX_START" >>$@
	echo "volatile uint_t __keep_@PACKAGE@_version = bbcat::LoadedVersions::Get().Register(\"@PACKAGE@\", \"@VERSION@-`cat ../gitbranch.txt`\");" >>$@
	echo "BBC_AUDIOTOOLBOX_END" >>$@
