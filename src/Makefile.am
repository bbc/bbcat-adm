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

libbbcat_adm_sources =							\
	ADMData.cpp									\
	ADMObjects.cpp								\
	ADMSubObjects.cpp							\
	ADMXMLGenerator.cpp							\
	AudioObjectParameters.cpp					\
	XMLValue.cpp

pkginclude_HEADERS =							\
	ADMData.h									\
	ADMObjects.h								\
	ADMSubObjects.h								\
	ADMXMLGenerator.h							\
	AudioObjectCursor.h							\
	AudioObjectParameters.h						\
	XMLValue.h

#CLEANFILES = register.cpp

noinst_HEADERS =

# register.cpp is included in repo but will be updated by the script below 
register.cpp: Makefile ../config.h ../gitbranch.txt $(libbbcat_adm_sources)
	sh $(prefix)/share/bbcat-common/findregisterfunctions.sh "bbcat-adm" "@VERSION@-`cat ../gitbranch.txt`" "$(libbbcat_adm_sources)" "bbcat_dsp" >$@

libbbcat_adm_@BBCAT_ADM_MAJORMINOR@_la_SOURCES = $(libbbcat_adm_sources) register.cpp
