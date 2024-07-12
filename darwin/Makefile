MAKEFILE_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(MAKEFILE_DIR)base.mak

PLATFORMS = macos macos-asan ios simulator

define TEMPLATE
$(1):
	$(MAKE) -f $(MAKEFILE_DIR)opus.mak $(1)
	$(MAKE) -f $(MAKEFILE_DIR)pjproject.mak $(1)

clean-$(1):
	$(MAKE) -f $(MAKEFILE_DIR)pjproject.mak clean-$(1)
	$(MAKE) -f $(MAKEFILE_DIR)opus.mak clean-$(1)
endef

$(foreach platform,$(PLATFORMS),$(eval $(call TEMPLATE,$(platform))))

clean:
	rm -rf $(INSTALL_DIR)
	rm -rf $(STAGE_DIR)
	rm -rf $(BASE_BUILD_DIR)
