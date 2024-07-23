MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(MAKEFILE_DIR)base.mak
include $(MAKEFILE_DIR)libphone.mak

BUILD_DIR = $(BASE_BUILD_DIR)/libphone
SOURCE_DIR = $(MAKEFILE_DIR)../
TOOLCHAIN_DIR = $(MAKEFILE_DIR)../cmake

CODESIGN_IDENTITY = AWJ83G3EPY

.DEFAULT_GOAL := all

asan: $(INSTALL_DIR)/macos-asan/lib/libphone.dylib
	bash $(MAKEFILE_DIR)create-xcframework.sh $(INSTALL_DIR) $(STAGE_DIR) $(CODESIGN_IDENTITY) asan macos-asan

tsan: $(INSTALL_DIR)/macos-tsan/lib/libphone.dylib
	bash $(MAKEFILE_DIR)create-xcframework.sh $(INSTALL_DIR) $(STAGE_DIR) $(CODESIGN_IDENTITY) tsan macos-tsan

release: $(INSTALL_DIR)/macos/lib/libphone.dylib \
	 $(INSTALL_DIR)/ios/lib/libphone.dylib \
	 $(INSTALL_DIR)/simulator/lib/libphone.dylib \
	 $(INSTALL_DIR)/catalyst/lib/libphone.dylib
	bash $(MAKEFILE_DIR)create-xcframework.sh $(INSTALL_DIR) $(STAGE_DIR) $(CODESIGN_IDENTITY) "" macos ios simulator catalyst

all: release asan tsan

.PHONY: asan tsan release all