# gen by makeobj.sh
############################################################
# project : yaml
# model   : 
# p.s. : 
############################################################

###### 
#@ c_FILE
c_FILE		=	\
			scanner.c \
			parser.c \

###### goals
include ../makeinstall
LIB		=	libmyyaml.so
LIBINST		=	$(_LIBINST)
HDER		=	myyaml.h
HDERINST	=	$(_HDERINST)

###### cflags
CFLAGS		=	$(_CFLAGS)

###### link
LFLAGS		=	$(_LFLAGS)

###### addition
CLEAN_ADDITION	=

###### load mktpl
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST
#@ make_uninstall
#@ make_uninstall_LIBINST
#@ make_uninstall_HDERINST
#@ make_uninstall_DFTHDERINST

include $(MKTPL2_HOME)/makeobj_$(MKTPL2_OS).inc

###### make rule
libmyyaml.so		:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)

