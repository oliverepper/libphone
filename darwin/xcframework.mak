MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(MAKEFILE_DIR)base.mak
include $(MAKEFILE_DIR)libphone.mak

BUILD_DIR = $(BASE_BUILD_DIR)/libphone
SOURCE_DIR = $(MAKEFILE_DIR)../
TOOLCHAIN_DIR = $(MAKEFILE_DIR)../cmake

ENV_CODESIGN_IDENTITY := CODESIGN_IDENTITY

ifeq ($(strip $(origin $(ENV_CODESIGN_IDENTITY))), undefined)
$(error The environment variable $(ENV_CODESIGN_IDENTITY) ist not set)
else ifeq ($(strip $($(ENV_CODESIGN_IDENTITY))),)
$(error The environment variable $(ENV_CODESIGN_IDENTITY) is empty)
endif

.DEFAULT_GOAL := all

asan: $(INSTALL_DIR)/macos-asan/lib/libphone.dylib
	bash $(MAKEFILE_DIR)create-xcframework.sh $(INSTALL_DIR) $(STAGE_DIR) "$(CODESIGN_IDENTITY)" "$(POSTFIX)-asan" macos-asan

tsan: $(INSTALL_DIR)/macos-tsan/lib/libphone.dylib
	bash $(MAKEFILE_DIR)create-xcframework.sh $(INSTALL_DIR) $(STAGE_DIR) "$(CODESIGN_IDENTITY)" "$(POSTFIX)-tsan" macos-tsan

release: $(INSTALL_DIR)/macos/lib/libphone.dylib \
	 $(INSTALL_DIR)/ios/lib/libphone.dylib \
	 $(INSTALL_DIR)/simulator/lib/libphone.dylib \
	 $(INSTALL_DIR)/catalyst/lib/libphone.dylib
	bash $(MAKEFILE_DIR)create-xcframework.sh $(INSTALL_DIR) $(STAGE_DIR) "$(CODESIGN_IDENTITY)" "$(POSTFIX)" macos ios simulator catalyst

all: release asan tsan

.PHONY: asan tsan release all