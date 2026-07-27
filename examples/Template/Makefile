# Project Name
TARGET = Template

# Use the LGPL DaisySP modules if needed
USE_DAISYSP_LGPL = 1

# Sources
CPP_SOURCES = template.cpp

OPT = -O1

# Library Locations (relative to this example folder)
LIBDAISY_DIR = ../../libs/libDaisy
DAISYSP_DIR  = ../../libs/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Include the Funbox SDK headers (funbox.h, expressionHandler.h)
C_INCLUDES += -I../../include
