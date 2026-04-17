#
# (C) Copyright 2005-2006 Hitachi Global Storage Technologies Corporation.
# All rights reserved.
# All information contained herein is proprietary and confidential to
# Hitachi Global Storage Technologies Corporation. Any use, reproduction,
# or disclosure without written permission of Hitachi Global Storage
# Technologies Corporation is prohibited.
# 
#

# 
# $Author: don $
# $Date: 2008/12/03 18:02:11 $
# $Revision: 1.1 $
# 

include ../../../../../../../Makeconf

TARGET = remodel

CC        = g++
CCINC     = -I. -I../include
CCINC    += -I`gcc --print-file-name=include`

CCWARN   = -Wall 

CCDEFS   = -D_GNU_SOURCE

CCFLAGS   = ${CCDEFS} ${CCINC}
CCFLAGS  += -O2 -Wall
CCFLAGS  += -fno-strict-aliasing

LDFLAGS   = -O2 -L../lib
LDLIBS    = -lga

SRCS = remodel.cc
OBJS = ${SRCS:.cc=.o}

.SUFFIXES: .cc .s .S

all: ${TARGET}

remodel: ${OBJS} 
	${CC} ${LDFLAGS} -o $@ ${OBJS} ${LDLIBS}

.cc.o:
	${CC} ${CCFLAGS} -o $@ -c $<

install:
	install --mode=755 -d ${AVSMINSTALLBIN}
	install --mode=755 ${TARGET} ${AVSMINSTALLBIN}

uninstall:
	-rm -f ${AVSMINSTALLBIN}/${TARGET}

depend:
	rm -f ${DEPFILE}.tmp
	@list='${SRCS}'; for f in $$list; do \
	    ${MKDEP} ${CCDEFS} ${CCINC} $$f; \
	    cat ${DEPFILE} >> ${DEPFILE}.tmp; \
	done
	mv ${DEPFILE}.tmp ${DEPFILE}
	-rm -f ${DEPFILE}.bak

clean:
	-rm -f ${TARGET} lex.yy.cc y.tab.cc *.o *.bak .*.bak

distclean: clean
	-rm -f ${DEPFILE}
	-rm -f ${DEPFILE}.bak
	-rm -f ${DEPFILE}.tmp

-include ${DEPFILE}

