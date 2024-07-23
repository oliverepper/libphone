MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(MAKEFILE_DIR)base.mak

SOURCE_DIR=$(MAKEFILE_DIR)../third-party/pjproject
BUILD_DIR = $(BASE_BUILD_DIR)/pjproject

BUILD_PLATFORMS = macos-arm64 macos-x86_64 macos-asan macos-tsan ios simulator-arm64 simulator-x86_64 catalyst-arm64 catalyst-x86_64
UNIVERSAL_PLATFORMS = macos simulator catalyst
SINGLE_ARCH_PLATFORMS = macos-asan macos-tsan ios

include $(SOURCE_DIR)/version.mak

.DEFAULT_GOAL := install

define BUILD_TEMPLATE
$(BUILD_DIR)/$(1)/aconfigure:
	mkdir -p $(BUILD_DIR)/$(1)
	git -C $(SOURCE_DIR) archive HEAD | tar -x -C $(BUILD_DIR)/$(1)

setup-$(1): $(BUILD_DIR)/$(1)/aconfigure

$(BUILD_DIR)/$(1)/config.log: $(BUILD_DIR)/$(1)/aconfigure
	bash $(MAKEFILE_DIR)configure-pjproject-$(1).sh $(BUILD_DIR)/$(1) $(INSTALL_DIR)/$(subst -arm64,,$(subst -x86_64,,$(1)))

config-$(1): $(BUILD_DIR)/$(1)/config.log

$(BUILD_DIR)/$(1)/build.flag: $(BUILD_DIR)/$(1)/config.log
	$(MAKE) -C $(BUILD_DIR)/$(1) dep clean all && touch $(BUILD_DIR)/$(1)/build.flag

build-$(1): $(BUILD_DIR)/$(1)/build.flag

$(STAGE_DIR)/$(1)/stage.flag: $(BUILD_DIR)/$(1)/build.flag
	$(MAKE) -C $(BUILD_DIR)/$(1) DESTDIR=$(STAGE_DIR)/$(1) install && touch $(STAGE_DIR)/$(1)/stage.flag

stage-$(1): $(STAGE_DIR)/$(1)/stage.flag

clean-$(1):
	rm -rf $(STAGE_DIR)/$(1)
	rm -rf $(BUILD_DIR)/$(1)

.PHONY: setup-$(1) config-$(1) build-$(1) stage-$(1) clean-$(1)
endef

define UNIVERSAL_TEMPLATE
$(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc: $(STAGE_DIR)/$(1)-arm64/stage.flag $(STAGE_DIR)/$(1)-x86_64/stage.flag
	mkdir -p $(INSTALL_DIR)/$(1)/{include,lib}
	cp -a $(STAGE_DIR)/$(1)-arm64/include/* $(INSTALL_DIR)/$(1)/include
	libtool -static -o $(INSTALL_DIR)/$(1)/lib/libpjproject.a $(STAGE_DIR)/$(1)-arm64/lib/*.a $(STAGE_DIR)/$(1)-x86_64/lib/*.a
	if [ -f $(MAKEFILE_DIR)libpjproject-$(1).pc.in ]; then \
		m4 -DPREFIX=$(INSTALL_DIR)/$(1) -DPJ_VERSION=$(PJ_VERSION) $(MAKEFILE_DIR)libpjproject-$(1).pc.in > $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc; \
	elif [ -f $(MAKEFILE_DIR)libpjproject-$(subst -$(lastword $(subst -, ,$(1))),,$(1)).pc.in ]; then \
		m4 -DPREFIX=$(INSTALL_DIR)/$(1) -DPJ_VERSION=$(PJ_VERSION) $(MAKEFILE_DIR)libpjproject-$(subst -$(lastword $(subst -, ,$(1))),,$(1)).pc.in > $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc; \
	else \
		m4 -DPREFIX=$(INSTALL_DIR)/$(1) -DPJ_VERSION=$(PJ_VERSION) $(MAKEFILE_DIR)libpjproject.pc.in > $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc; \
	fi

install-$(1): $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc

$(1): install-$(1)

clean-$(1): clean-$(1)-arm64 clean-$(1)-x86_64

.PHONY: install-$(1) $(1)
endef

define SINGLE_ARCH_TEMPLATE
$(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc: $(STAGE_DIR)/$(1)/stage.flag
	mkdir -p $(INSTALL_DIR)/$(1)/{include,lib}
	cp -a $(STAGE_DIR)/$(1)/include/* $(INSTALL_DIR)/$(1)/include
	libtool -static -o $(INSTALL_DIR)/$(1)/lib/libpjproject.a $(STAGE_DIR)/$(1)/lib/*.a
	if [ -f $(MAKEFILE_DIR)libpjproject-$(1).pc.in ]; then \
		m4 -DPREFIX=$(INSTALL_DIR)/$(1) -DPJ_VERSION=$(PJ_VERSION) $(MAKEFILE_DIR)libpjproject-$(1).pc.in > $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc; \
	elif [ -f $(MAKEFILE_DIR)libpjproject-$(subst -$(lastword $(subst -, ,$(1))),,$(1)).pc.in ]; then \
		m4 -DPREFIX=$(INSTALL_DIR)/$(1) -DPJ_VERSION=$(PJ_VERSION) $(MAKEFILE_DIR)libpjproject-$(subst -$(lastword $(subst -, ,$(1))),,$(1)).pc.in > $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc; \
	else \
		m4 -DPREFIX=$(INSTALL_DIR)/$(1) -DPJ_VERSION=$(PJ_VERSION) $(MAKEFILE_DIR)libpjproject.pc.in > $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc; \
	fi

install-$(1): $(INSTALL_DIR)/$(1)/lib/pkgconfig/libpjproject.pc

$(1): install-$(1)

.PHONY: install-$(1) $(1)
endef

$(foreach platform,$(BUILD_PLATFORMS),$(eval $(call BUILD_TEMPLATE,$(platform))))
$(foreach platform,$(UNIVERSAL_PLATFORMS),$(eval $(call UNIVERSAL_TEMPLATE,$(platform))))
$(foreach platform,$(SINGLE_ARCH_PLATFORMS),$(eval $(call SINGLE_ARCH_TEMPLATE,$(platform))))

install: $(foreach platform,$(UNIVERSAL_PLATFORMS),install-$(platform)) $(foreach platform,$(SINGLE_ARCH_PLATFORMS),install-$(platform))

clean: $(foreach platform,$(BUILD_PLATFORMS),clean-$(platform))
	rm -r $(BUILD_DIR)
	rm -r $(STAGE_DIR)

list-targets:
	@$(MAKE) -pRrq -f $(firstword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/(^|\n)# Files(\n|$$)/,/(^|\n)# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | grep -E -v -e '^[^[:alnum:]]' -e '^$@$$'


.PHONY: install clean list-targets
