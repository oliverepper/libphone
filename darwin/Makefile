MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(MAKEFILE_DIR)base.mak

PLATFORMS = macos macos-asan macos-tsan ios simulator catalyst

.DEFAULT_GOAL := install

define TEMPLATE
install-$(1):
	$(MAKE) -f $(MAKEFILE_DIR)opus.mak $(1)
	$(MAKE) -f $(MAKEFILE_DIR)pjproject.mak $(1)
	$(MAKE) -f $(MAKEFILE_DIR)libphone.mak $(1)

clean-$(1):
	$(MAKE) -f $(MAKEFILE_DIR)pjproject.mak clean-$(1)
	$(MAKE) -f $(MAKEFILE_DIR)opus.mak clean-$(1)
	$(MAKE) -f $(MAKEFILE_DIR)libphone.mak clean-$(1)

$(1): install-$(1)

.PHONY: install-$(1) clean-$(1) $(1)
endef

$(foreach platform,$(PLATFORMS),$(eval $(call TEMPLATE,$(platform))))

install: $(foreach platform,$(PLATFORMS),install-$(platform))

clean:
	rm -rf $(INSTALL_DIR)
	rm -rf $(STAGE_DIR)
	rm -rf $(BASE_BUILD_DIR)
