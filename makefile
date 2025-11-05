# ASTROVE Chess Engine Makefile
# Compiler and flags
CXX = g++
CXXFLAGS_BASE = -std=c++17 -Wall -Wextra

# Optimized build (use this for releases and testing)
CXXFLAGS_OPT = $(CXXFLAGS_BASE) -O3 -march=native -flto -DNDEBUG -funroll-loops -ffast-math

# Profiling build
CXXFLAGS_PROF = $(CXXFLAGS_BASE) -O2 -g -pg

# Debug build
CXXFLAGS_DEBUG = $(CXXFLAGS_BASE) -g -O0

# Default: optimized
CXXFLAGS = $(CXXFLAGS_OPT)

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := .

# Target executable
TARGET := $(BIN_DIR)/ASTROVE

all: $(TARGET)

profile: CXXFLAGS = $(CXXFLAGS_PROF)
profile: clean $(TARGET)

debug: CXXFLAGS = $(CXXFLAGS_DEBUG)
debug: clean $(TARGET)

# Source files - automatically find all .cpp files
BOARD_SRC := $(wildcard $(SRC_DIR)/board/*.cpp)
CORE_SRC := $(wildcard $(SRC_DIR)/core/*.cpp)
ENDGAME_SRC := $(wildcard $(SRC_DIR)/endgame/*.cpp)
EVAL_SRC := $(wildcard $(SRC_DIR)/evaluation/*.cpp)
ORDER_SRC := $(wildcard $(SRC_DIR)/ordering/*.cpp)
SEARCH_SRC := $(wildcard $(SRC_DIR)/search/*.cpp)
SEARCH_EXT_SRC := $(wildcard $(SRC_DIR)/search/extensions/*.cpp)
SEARCH_PRUNE_SRC := $(wildcard $(SRC_DIR)/search/pruning/*.cpp)
TABLE_SRC := $(wildcard $(SRC_DIR)/table/*.cpp)
TABLE_TUNE_SRC := $(wildcard $(SRC_DIR)/table/tuning/*.cpp)
UCI_SRC := $(wildcard $(SRC_DIR)/uci/*.cpp)
UTILS_SRC := $(wildcard $(SRC_DIR)/utils/*.cpp)
MAIN_SRC := $(SRC_DIR)/main.cpp

# Combine all sources
SOURCES := $(BOARD_SRC) $(CORE_SRC) $(ENDGAME_SRC) $(EVAL_SRC) \
           $(ORDER_SRC) $(SEARCH_SRC) $(SEARCH_EXT_SRC) $(SEARCH_PRUNE_SRC) \
           $(TABLE_SRC) $(TABLE_TUNE_SRC) $(UCI_SRC) $(UTILS_SRC) $(MAIN_SRC)

# Object files (in build directory, preserving structure)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Create necessary directories
DIRS := $(BUILD_DIR) $(BUILD_DIR)/board $(BUILD_DIR)/core $(BUILD_DIR)/endgame \
        $(BUILD_DIR)/evaluation $(BUILD_DIR)/ordering $(BUILD_DIR)/search \
        $(BUILD_DIR)/search/extensions $(BUILD_DIR)/search/pruning \
        $(BUILD_DIR)/table $(BUILD_DIR)/table/tuning $(BUILD_DIR)/uci \
        $(BUILD_DIR)/utils

# Default target
.PHONY: all
all: $(TARGET)

# Create directories
$(DIRS):
	@mkdir -p $@

# Link executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DIRS)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@echo "Clean complete"

# Debug build
.PHONY: debug
debug: CXXFLAGS := -std=c++17 -O0 -g -Wall -Wextra -DDEBUG
debug: clean all

# Release build with maximum optimizations
.PHONY: release
release: CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -flto -march=native -DNDEBUG
release: clean all

# Profile-guided optimization (PGO) build
.PHONY: pgo
pgo:
	@echo "Building with PGO..."
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fprofile-generate" LDFLAGS="$(LDFLAGS) -fprofile-generate"
	@echo "Running profiling..."
	./$(TARGET) bench > /dev/null 2>&1 || true
	@echo "Rebuilding with profile data..."
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -fprofile-use" LDFLAGS="$(LDFLAGS) -fprofile-use"
	@rm -f *.gcda

# Install (optional)
.PHONY: install
install: release
	@echo "Installing ASTROVE..."
	@cp $(TARGET) /usr/local/bin/
	@echo "Installation complete"

# Run engine
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Show help
.PHONY: help
help:
	@echo "ASTROVE Chess Engine Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the engine (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  pgo       - Build with profile-guided optimization"
	@echo "  run       - Build and run the engine"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  help      - Show this help message"

# Rebuild everything
.PHONY: rebuild
rebuild: clean all

# Show build configuration
.PHONY: info
info:
	@echo "Build Configuration:"
	@echo "  CXX      = $(CXX)"
	@echo "  CXXFLAGS = $(CXXFLAGS)"
	@echo "  LDFLAGS  = $(LDFLAGS)"
	@echo "  TARGET   = $(TARGET)"
	@echo "  SOURCES  = $(words $(SOURCES)) files"
