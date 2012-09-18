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
INCPRE+=$(WPD_CFLAGS) -I..
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd
.ENDIF

.IF "$(SYSTEM_LIBWPG)" == "YES"
INCPRE+=$(WPG_CFLAGS) -I..
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpg
.ENDIF

.IF "$(SYSTEM_ZLIB)" != "YES"
INCPRE+=-I$(SOLARVER)$/$(INPATH)$/inc$/external/zlib
.ENDIF

SLOFILES= \
	$(SLO)$/libvisio_utils.obj \
	$(SLO)$/VisioDocument.obj \
	$(SLO)$/VSD11Parser.obj \
	$(SLO)$/VSD6Parser.obj \
	$(SLO)$/VSDCharacterList.obj \
	$(SLO)$/VSDContentCollector.obj \
	$(SLO)$/VSDFieldList.obj \
	$(SLO)$/VSDGeometryList.obj \
	$(SLO)$/VSDInternalStream.obj \
	$(SLO)$/VSDOutputElementList.obj \
	$(SLO)$/VSDPages.obj \
	$(SLO)$/VSDParagraphList.obj \
	$(SLO)$/VSDParser.obj \
	$(SLO)$/VSDShapeList.obj \
	$(SLO)$/VSDStencils.obj \
	$(SLO)$/VSDStringVector.obj \
	$(SLO)$/VSDStylesCollector.obj \
	$(SLO)$/VSDStyles.obj \
	$(SLO)$/VSDSVGGenerator.obj \
	$(SLO)$/VSDZipStream.obj

LIB1ARCHIV=$(LB)$/libvisiolib.a
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1OBJFILES= $(SLOFILES)

.INCLUDE :  target.mk
