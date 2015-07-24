BUILDTYPE?=Release
XCB=$(shell xcrun -f xcodebuild)
NODEGYP=node-gyp
XCODEPROJ=build/binding.xcodeproj
TARGET=build/addon.node
SOURCES:=$(shell find -E src -type f -regex ".*\.(cc|c|swift|h)")

.PHONY: all clean

all: $(TARGET)

clean:
	rm -fr build

$(TARGET): $(XCODEPROJ) $(SOURCES)
	$(XCB) -project $(XCODEPROJ) -configuration $(BUILDTYPE)

$(XCODEPROJ): binding.gyp
	$(NODEGYP) configure -- -f xcode
