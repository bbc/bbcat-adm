
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = src test

adm_DATA  = share/licences.txt

admdir = $(prefix)/share/$(PACKAGE)

EXTRA_DIST = README COPYING autogen.sh $(adm_DATA)

pkgconfig_DATA = bbcat-adm-$(BBCAT_ADM_MAJORMINOR).pc

bbcat-adm-$(BBCAT_ADM_MAJORMINOR).pc: bbcat-adm.pc
	cp bbcat-adm.pc bbcat-adm-$(BBCAT_ADM_MAJORMINOR).pc

bbcat-adm-$(BBCAT_ADM_MAJORMINOR)-uninstalled.pc: bbcat-adm-uninstalled.pc
	cp bbcat-adm-uninstalled.pc bbcat-adm-$(BBCAT_ADM_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = bbcat-adm-$(BBCAT_ADM_MAJORMINOR)-uninstalled.pc gitbranch.txt

CLEANFILES = bbcat-adm-$(BBCAT_ADM_MAJORMINOR).pc bbcat-adm-$(BBCAT_ADM_MAJORMINOR)-uninstalled.pc src/register.cpp gitbranch.txt .branch

ACLOCAL_FLAGS = -I m4

.PHONY: gitbranch.txt

gitbranch.txt:
	@-git rev-parse --abbrev-ref HEAD >.branch && (diff gitbranch.txt .branch >/dev/null || cp .branch gitbranch.txt)
	@if [ ! -e "gitbranch.txt" ] ; then touch "gitbranch.txt" ; fi
	@echo "Branch: `cat gitbranch.txt`"
