EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..$/..$/..$/..

PRJNAME=libvisio
TARGET=visiolib
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

.INCLUDE :  settings.mk

.IF "$(COM)"=="MSC"
CFLAGS+=-GR
.ENDIF
.IF "$(COM)"=="GCC"
CFLAGSCXX+=-frtti
.ENDIF

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS) -I..
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd
.ENDIF

.IF "$(SYSTEM_LIBWPG)" == "YES"
INCPRE+=$(LIBWPG_CFLAGS) -I..
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpg
.ENDIF

SLOFILES= \
	$(SLO)$/libvisio_utils.obj \
	$(SLO)$/VisioDocument.obj \
	$(SLO)$/VSD11Parser.obj \
	$(SLO)$/VSD6Parser.obj \
	$(SLO)$/VSDInternalStream.obj \
	$(SLO)$/VSDSVGGenerator.obj \
	$(SLO)$/VSDXCharacterList.obj \
	$(SLO)$/VSDXCollector.obj \
	$(SLO)$/VSDXContentCollector.obj \
	$(SLO)$/VSDXGeometryList.obj \
	$(SLO)$/VSDXOutputElementList.obj \
	$(SLO)$/VSDXParser.obj \
	$(SLO)$/VSDXShapeList.obj \
	$(SLO)$/VSDXStylesCollector.obj

LIB1ARCHIV=$(LB)$/libvisiolib.a
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1OBJFILES= $(SLOFILES)

.INCLUDE :  target.mk
