MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(MAKEFILE_DIR)base.mak

BUILD_DIR = $(BASE_BUILD_DIR)/libphone
SOURCE_DIR = $(MAKEFILE_DIR)../
TOOLCHAIN_DIR = $(MAKEFILE_DIR)../cmake

PLATFORMS = macos macos-asan macos-tsan ios simulator catalyst # xrsimulator

GENERATOR = "Ninja Multi-Config" # This may get overwritten by the configure script
CONFIG = RelWithDebInfo

.DEFAULT_GOAL := install

define TEMPLATE
$(BUILD_DIR)/$(1):
	bash $(MAKEFILE_DIR)configure-libphone-$(1).sh $(GENERATOR) $(BUILD_DIR)/$(1) $(SOURCE_DIR) $(INSTALL_DIR)/$(1)

config-$(1): $(BUILD_DIR)/$(1)

$(BUILD_DIR)/$(1)/build.flag: $(BUILD_DIR)/$(1)
	if [ "$$(shell grep CMAKE_GENERATOR:INTERNAL $(BUILD_DIR)/$(1)/CMakeCache.txt | cut -d '=' -f 2)" = "Xcode" ] && which xcpretty > /dev/null 2>&1; then \
		cmake --build $(BUILD_DIR)/$(1) --config=$(CONFIG) | xcpretty && touch $(BUILD_DIR)/$(1)/build.flag; \
	else \
		cmake --build $(BUILD_DIR)/$(1) --config=$(CONFIG) && touch $(BUILD_DIR)/$(1)/build.flag; \
	fi

build-$(1): $(BUILD_DIR)/$(1)/build.flag

$(INSTALL_DIR)/$(1)/lib/libphone.dylib: $(BUILD_DIR)/$(1)/build.flag
	if [ "$$(shell grep CMAKE_GENERATOR:INTERNAL $(BUILD_DIR)/$(1)/CMakeCache.txt | cut -d '=' -f 2)" = "Xcode" ] && which xcpretty > /dev/null 2>&1; then \
		cmake --build $(BUILD_DIR)/$(1) --config=$(CONFIG) --target=install | xcpretty; \
	else \
		cmake --build $(BUILD_DIR)/$(1) --config=$(CONFIG) --target=install; \
	fi

install-$(1): $(INSTALL_DIR)/$(1)/lib/libphone.dylib

clean-$(1):
	rm -rf $(BUILD_DIR)/$(1)

$(1): install-$(1)

.PHONY: config-$(1) build-$(1) install-$(1) clean-$(1) $(1)
endef

$(foreach platform,$(PLATFORMS),$(eval $(call TEMPLATE,$(platform))))

install: $(foreach platform,$(PLATFORMS),install-$(platform))

clean: $(foreach platform,$(PLATFORMS),clean-$(platform))
	rm -r $(BUILD_DIR)

list-targets:
	@$(MAKE) -pRrq -f $(firstword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/(^|\n)# Files(\n|$$)/,/(^|\n)# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | grep -E -v -e '^[^[:alnum:]]' -e '^$@$$'

.PHONY: install clean list-targets
