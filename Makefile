ROOT_DIR:=.

SUDO?=sudo

# Allows me to crosscompile with my main machine
nopynq:=0
x86_64:=x86_64
ifeq ($(shell uname -m), $(x86_64))
	ifeq ($(nopynq), 0)
		CC=arm-linux-gnueabihf-gcc
	endif
endif

RELEASE=$(shell grep '\#define LIBPYNQ_RELEASE' library/version.c | sed s'!^[^"]*\("[^"][^"]*"\)$$!\1!g')
MAJOR=$(shell grep '\#define LIBPYNQ_VERSION_MAJOR' library/version.c | sed s'![^0-9]!!g')
MINOR=$(shell grep '\#define LIBPYNQ_VERSION_MINOR' library/version.c | sed s'![^0-9]!!g')
PATCH=$(shell grep '\#define LIBPYNQ_VERSION_PATCH' library/version.c | sed s'![^0-9]!!g')
DATE=$(shell date "+%Y-%m-%d %H:%M")
RELEASE_BASENAME=libpynq-${RELEASE}-v${MAJOR}.${MINOR}.${PATCH}
LABEL="release ${RELEASE} version ${MAJOR}.${MINOR}.${PATCH} of ${DATE}"

# when compiling empty library:
# CFLAGS:=-I. -Iplatform/ -Ilibrary/ -Iexternal/ -lm -O0 -g3 -ggdb
MYFLAGS:=

LIBRARIES_SOURCES:=$(wildcard library/*.c)
LIBRARIES_HEADERS:=$(wildcard library/*.h)
LIBRARIES_OBJECTS:=$(LIBRARIES_SOURCES:.c=.o) $(EXTERNAL_SOURCES:.c=.o)


BUILD_DIR:=${ROOT_DIR}/build
LIB_DIR:=${BUILD_DIR}/lib/
LIB_PYNQ:=${LIB_DIR}/libpynq.a
LIB_SCPI:=external/scpi-parser/libscpi/dist
LIB_SCPI_LIB:=${LIB_SCPI}/libscpi.a
OBJ_DIR:=${BUILD_DIR}/obj

OBJECTS_LIBRARIES:=$(foreach obj,$(LIBRARIES_OBJECTS),${BUILD_DIR}/$(obj))

DIRS:=$(foreach obj,$(LIBRARIES_OBJECTS),${BUILD_DIR}/$(dir $(obj))/)

SRC_DIR:=${ROOT_DIR}/src
SOURCES:=$(wildcard ${SRC_DIR}/*.c)
EXPERIMENTS_DIR:=${ROOT_DIR}/src/experiments
LIBS_DIR:=${SRC_DIR}/libs
LIBS:=$(wildcard ${LIBS_DIR}/*.c)
LIBS_OBJ:=$(patsubst $(LIBS_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIBS))
EXPERIMENTS:=$(wildcard ${EXPERIMENTS_DIR}/*.c)
EXPERIMENTS_BIN:=$(patsubst $(EXPERIMENTS_DIR)/%.c, $(BUILD_DIR)/%,$(EXPERIMENTS))

CFLAGS:=-I. -Iplatform/ -Ilibrary/ -Iexternal/ -lm -O0 -g3 -ggdb -Wextra -Wall

all: ${LIB_PYNQ} ${LIB_SCPI} ${EXPERIMENTS_BIN} ${BUILD_DIR}/rover 

${OBJ_DIR}/%.o: ${LIBS_DIR}/%.c
	@mkdir -p $(@D)
	${CC} -c -o $@ $< ${CFLAGS} ${MYFLAGS}
ifeq ($(nopynq), 0)
	$(VERBOSE)${SUDO} setcap cap_sys_rawio+ep ./${@}
endif

${BUILD_DIR}/rover: ${SOURCES} ${LIBS_OBJ} ${LIB_PYNQ} ${LIB_SCPI_LIB}
	$(VERBOSE)${CC} -o $@ $^ ${CFLAGS} ${LDFLAGS} ${MYFLAGS}
ifeq ($(nopynq), 0)
	$(VERBOSE)${SUDO} setcap cap_sys_rawio+ep ./${@}
endif

rover: ${BUILD_DIR}/rover

${BUILD_DIR}/%: ${EXPERIMENTS_DIR}/%.c ${LIBS_OBJ} ${LIB_PYNQ} ${LIB_SCPI_LIB}
	$(VERBOSE)${CC} -o $@ $< $(filter-out $(wildcard $(<D)/*.c ), $^) ${CFLAGS} ${LDFLAGS} ${MYFLAGS}
ifeq ($(nopynq), 0)
	$(VERBOSE)${SUDO} setcap cap_sys_rawio+ep ./${@}
endif

experiments: ${EXPERIMENTS_BIN}
exp: experiments

${LIB_SCPI}:
	$(MAKE) -C external/scpi-parser/libscpi/

ifneq (clean,$(MAKECMDGOALS))
-include ${D_FILES}
endif

${BUILD_DIR}/%.d: %.c | ${DIRS}
	${CC} -c -MT"${BUILD_DIR}/$*.o" -MM  -o $@ $^ ${CFLAGS}

${BUILD_DIR}/%.o: %.c | ${DIRS}
	${CC} -c -o $@ $< ${CFLAGS}

${LIB_PYNQ}: ${OBJECTS_LIBRARIES} | ${LIB_DIR}
	$(AR) rcs $@ $?


%/:
	mkdir -p ${@}

indent: indent-library indent-src

indent-library: ${LIBRARIES_SOURCES} ${LIBRARIES_HEADERS}
	clang-format -i $^

indent-src: ${EXPERIMENTS} ${SOURCES} ${SOURCES_H}
	clang-format -i $^

version:
	@echo libpynq $(LABEL)
	@echo libpynq-$(RELEASE)-v$(MAJOR).$(MINOR).$(PATCH)

doc: ${BUILD_DIR}/html

${BUILD_DIR}/html: documentation/ryb.doxy ${LIBRARIES_SOURCES} ${LIBRARIES_HEADERS}
	sed -e 's/^PROJECT_NUMBER         =.*/PROJECT_NUMBER         = \"('$(LABEL)'\")/' documentation/ryb.doxy > documentation/ryb.doxy.new
	doxygen documentation/ryb.doxy.new
	rm documentation/ryb.doxy.new
	${MAKE} -C ${BUILD_DIR}/latex/

# can only clean both library and applications
clean: 
	rm -rf ${OBJ_DIR} ${BUILD_DIR}/rover ${EXPERIMENTS_BIN}

realclean:
	rm -rf ${BUILD_DIR}

sync:
	rsync -a --delete . student@10.43.0.8:/home/student/venus # ROBOT

s:
	rsync -a --delete . student@10.43.0.9:/home/student/venus # MY PYNQ``

.PHONY: indent indent-library indent-applications doc clean release install doc version
