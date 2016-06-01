LDADD = $(BBCAT_BASE_LIBS)								\
		$(BBCAT_DSP_LIBS)								\
		$(BBCAT_GLOBAL_ADM_LIBS)						\
        ../src/libbbcat-adm-@BBCAT_ADM_MAJORMINOR@.la

AM_CPPFLAGS = $(BBCAT_BASE_CFLAGS)						\
			  $(BBCAT_DSP_CFLAGS)						\
			  $(BBCAT_ADM_CFLAGS)						\
			  $(BBCAT_GLOBAL_ADM_CFLAGS)				\
			  -I$(prefix)/share/bbcat-common/include	\
			  -I../src

AM_CXXFLAGS = $(AM_CPPFLAGS)					\
			  -std=c++11

check_PROGRAMS =
TESTS =

tests_SOURCES = testbase.cpp AudioObjectParametersTest.cpp
check_PROGRAMS += tests
TESTS += tests
