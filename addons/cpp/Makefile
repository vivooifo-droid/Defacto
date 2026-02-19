CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  ADDON_EXT = dylib
  SHARED_FLAGS = -dynamiclib
  LOADER_LDFLAGS =
else
  ADDON_EXT = so
  SHARED_FLAGS = -shared -fPIC
  LOADER_LDFLAGS = -ldl
endif

LOADER = runtime_loader
ADDON  = sample_addon.$(ADDON_EXT)

all: $(LOADER) $(ADDON)

$(LOADER): runtime_loader.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LOADER_LDFLAGS)

$(ADDON): sample_addon.cpp
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) -o $@ $<

clean:
	rm -f $(LOADER) sample_addon.so sample_addon.dylib sample_addon.dll
