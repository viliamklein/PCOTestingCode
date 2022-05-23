#==============================================#
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INC_DIR := include

#==============================================#
PCODIR      = /home/viliam/PCO/pco_camera/pco_common/pco_include
PCOLIBDIR   = /home/viliam/PCO/pco_camera/pco_common/pco_lib
CPCODIR     = /home/viliam/PCO/pco_camera/pco_common/pco_classes
CPCODIRCLHS = /home/viliam/PCO/pco_camera/pco_clhs/pco_classes
PCOCLHSDIR  = /home/viliam/PCO/pco_camera/pco_clhs/pco_clhs_common
ASIOINC := /home/viliam/sources/asio-1.18.1/include/
TOMLINC := /home/viliam/sources/tomlplusplus/include

CFLAGS += -std=c++2a -Wall -DLINUX -DASIO_STANDALONE -I$(TOMLINC) -I$(ASIOINC) -I$(PCODIR) -I$(CPCODIR) -I$(CPCODIRCLHS) -I$(PCOCLHSDIR)
LFLAGS += -L$(PCOLIBDIR)
CXXCMD = gcc

DISPLIB    = $(PCOLIBDIR)/libpcodisp.a
LOGLIB     = $(PCOLIBDIR)/libpcolog.a
FILELIB    = $(PCOLIBDIR)/libpcofile.a
REORDERLIB = $(PCOLIBDIR)/libreorderfunc.a
CAMLIB     = $(PCOLIBDIR)/libpcocam_clhs.a

HEADERS = 	$(CPCODIR)/Cpco_com.h \
			$(CPCODIRCLHS)/VersionNo.h \
			$(CPCODIRCLHS)/Cpco_grab_clhs.h \
			$(CPCODIRCLHS)/Cpco_com_clhs.h \
			$(PCODIR)/PCO_errt.h \
			$(INC_DIR)/networkingControl.h \
			$(INC_DIR)/pcoCamTS.h \
			$(INC_DIR)/ImageMessages.pb.h

CPPFLAGS := -Iinclude

PCOLIB  = -lpcodisp -lpcofile -lpcocam_clhs -lpcoclhs
LIBADD  = -lInfluxDB -lcurl -lprotobuf -pthread  -lrt -ldl -lX11 -lXext
#==============================================#

# EXE := $(BIN_DIR)/PCO_test
SRC := $(wildcard $(SRC_DIR)/*.cpp )
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
EXECUTABLE  := $(BIN_DIR)/pcoTesting
EXECUTABLE_DEBUG  := $(BIN_DIR)/pcoTesting_debug

# .PHONY: all debug clean
# .PHONY: all
all: CFLAGS += -O2
all: $(EXECUTABLE)

debug: CPPFLAGS += -DDEBUG -g
debug: CFLAGS += -DDEBUG -g
debug: clean $(EXECUTABLE)

# $(info $(SRC))
# $(info $(OBJ))
# $(info $(CPPFLAGS))

#==============================================#
# normal build
#==============================================#

# build: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@ $(LFLAGS) $(PCOLIB) $(LIBADD)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(CAMLIB) $(HEADERS)  | $(OBJ_DIR) 
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ $(LFLAGS) $(PCOLIB) $(LIBADD)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@


# @$(RM) -rv $(OBJ_DIR)

#==============================================#
# debug build
#==============================================#

# .PHONY: debug

# $(info "CFLAGS": $(CFLAGS))

# CFLAGS += -ggdb
# $(EXECUTABLE_DEBUG): $(OBJ) | $(BIN_DIR)
# 	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@ $(LFLAGS) $(PCOLIB) $(LIBADD)

# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(CAMLIB) $(HEADERS)  | $(OBJ_DIR) 
# 	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ $(LFLAGS) $(PCOLIB) $(LIBADD)


# debug: $(EXECUTABLE_DEBUG)

# $(BIN_DIR) $(OBJ_DIR):
# 	mkdir -p $@

# $(info "HERE")

# .PHONY: clean
clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)

