#
# color code
#
NONE            := \033[00m
BLACK           := \033[22;30m
RED             := \033[01;31m
GREEN           := \033[22;32m
BROWN           := \033[22;33m
BLUE            := \033[22;34m
MAGENTA         := \033[22;35m
CYAN            := \033[22;36m
GRAY            := \033[22;37m
DARK_GRAY       := \033[01;30m
LIGHT_RED       := \033[01;31m
LIGHT_GREEN     := \033[01;32m
YELLOW          := \033[01;33m
LIGHT_BLUE      := \033[01;34m
LIGHT_MAGENTA   := \033[01;35m
LIGHT_CYAN      := \033[01;36m
WHITE           := \033[01;37m

#
# files
#

TOP_DIR := $(shell pwd)
PROJECT_NAME_IN := release/project_name.in
INFO_IN := release/info.in
PROJECT_NAME := release/project_name
DEPEND_IN := release/objs/depend.in
OBJS := release/objs
OTTO_MAKEFILES := Makefile Makefile.in
SOC_H := release/soc.h

#
# compiler flags
#

CC_OPT_FLAGS    := -Os -g

#
# message level
#
ifneq ($(strip $(OTTO_DEBUG)),)
    D :=
    DECHO := echo
else
    D := \#
    DECHO := \#
endif

#
# commands
#
MARK_REBUILD := touch --date="1970-01-02"


# Create and check supported preconfig project list
ifeq ($(wildcard ./project), ./project)
    CFG_LIST:=$(shell ls ./project)
    PRE_CFG_LIST:=$(addprefix preconfig_,$(CFG_LIST))
    LPRE_CFG_LIST:=$(addprefix lpreconfig_,$(CFG_LIST))
endif
ifeq ($(CFG_LIST),)
    $(error EE: unable find projects in ./project)
endif

all:
	@rm -f $(DEPEND_IN); $(MAKE) -f Makefile.in $(DEPEND_IN)
	@$(MAKE) -f Makefile.in $@
	@echo "Making procedure was completed normally."

project_list: ; @echo "$(GREEN)$(CFG_LIST)$(NONE)"

$(PRE_CFG_LIST): distclean
	@echo "preconfig.... '$(@:preconfig_%=%)'"
	@[ -d $(OBJS) ] || mkdir -p $(OBJS)
	@if [ ! -f project/$(@:preconfig_%=%)/info.in ]; then echo "missing project/$(@:preconfig_%=%)/info.in"; exit 1; fi
	@echo "project_name := $(@:preconfig_%=%)" > $(PROJECT_NAME)
	@$(MAKE) -f Makefile.name $(INFO_IN)
	@$(MAKE) -f Makefile.in $(SOC_H)
	@echo "building utilities...."
	@$(MAKE) -C util preconfig
	@echo "All preconfig processes have been finished!"
	@exit 0

$(LPRE_CFG_LIST): 
	@echo "load preconfig.... '$(@:lpreconfig_%=%)'"
	@[ -d $(OBJS) ] || mkdir -p $(OBJS)
	@if [ ! -f project/$(@:lpreconfig_%=%)/info.in ]; then echo "missing project/$(@:lpreconfig_%=%)/info.in"; exit 1; fi
	@echo "project_name := $(@:lpreconfig_%=%)" > $(PROJECT_NAME)
	@$(MAKE) -f Makefile.name $(INFO_IN)
	@echo "building utilities...."
	@cp -u project/$(@:lpreconfig_%=%)/conf.h ./release
	@$(MAKE) -f Makefile.in release/info.h
	@cp -u project/$(@:lpreconfig_%=%)/soc.h ./release
	@echo "All preconfig processes have been finished!"
	@exit 0

rpreconfig:
	@if [ ! -f $(PROJECT_NAME) ]; then echo "$(RED)please use preconfig_<project_name> firstly.$(NONE)"; exit 1; fi
	@project_name=`cat $(PROJECT_NAME) | cut -d " " -f 3`; \
	cp -u ./release/conf.h project/$${project_name} ; \
	cp -u ./release/soc.h project/$${project_name} ; \
	echo "restored preconfig to project/$${project_name}"

preconfig_% lpreconfig_%:
	@echo "Unknown project name, $(RED)'$*'$(NONE), please refer ./project/"
	@exit 1

reconfig:
	@[ -f $(PROJECT_NAME) ] && $(MAKE) preconfig_`cut -d " " -f 3 $(PROJECT_NAME)` || echo "$(RED)please use preconfig_<project_name> firstly.$(NONE)"

clean:
	-rm -rf $(OBJS) ./release/*.out ./release/*.img ./release/*.code
	@-mkdir -p $(OBJS)
	@-$(MARK_REBUILD) $(INFO_IN) $(SOC_H)

distclean:
	-rm -rf ./release
	@$(MAKE) -C util $@
	@rm -f ./util/bin/*
	@rm -f ./total_verification.cfg

#build_test: ; @echo "$(RED)Not implemented yet: $@$(NONE)"

total_verification.cfg:
	@cp toolkit_for_verification.cfg $@

build_test: total_verification.cfg
	@while read test_platform test_ub_platform test_sep test_toolkit_path;\
  do \
    test -e ./project/$${test_platform} -a -e ./uboot/vendors/$${test_ub_platform} || continue ;\
    rm -rf release ;\
    export CROSS_COMPILE=$${test_toolkit_path} ;\
    export PLR_PCFG_CMD="$(MAKE) preconfig_$${test_platform}" ;\
    export UBT_MAKE_CMD="cd uboot && $(MAKE) preconfig_$${test_ub_platform} && make all" ;\
    export PLR_MAKE_CMD="cd .. && $(MAKE) all" ;\
    echo -n "export CROSS_COMPILE=\"$${CROSS_COMPILE}\"; $${PLR_PCFG_CMD}" ;\
    eval $${PLR_PCFG_CMD} > $${test_platform}.$${test_ub_platform}.log 2>&1 ;\
    echo -n " && $${UBT_MAKE_CMD}" ;\
    eval $${UBT_MAKE_CMD} >> $${test_platform}.$${test_ub_platform}.log 2>&1 ;\
    echo -n " && $${PLR_MAKE_CMD}; unset CROSS_COMPILE" ;\
    eval $${PLR_MAKE_CMD} >> $${test_platform}.$${test_ub_platform}.log 2>&1 ;\
    eval $(MAKE) verify_test ;\
  done < $^

.EXPORT_ALL_VARIABLES:;

.PHONY: all clean distclean build_test release $(PRE_CFG_LIST) $(LPRE_CFG_LIST) rpreconfig

Makefile rlz.makefile: ;

ifeq ($(MAKECMDGOALS),verify_test)
include ./release/info2.in
TARGET_OUT ?= ./release/encode_uboot.img
endif
verify_test:
	@echo -n " [Check ${TARGET_OUT}]"; test -e ${TARGET_OUT} && echo " $(GREEN)[Pass]$(NONE)" || echo " $(RED)[Fail]$(NONE)"

ifeq ($(wildcard $(PROJECT_NAME)), $(PROJECT_NAME))
include ./release/info2.in
rfunc := REG_INIT_FUNC
_ilvl_grep_dir := project/$(project_name)
_ilvl_grep_dir += src/template/$(template_name)

_ilvl_grep := grep -R "$(rfunc)" $(_ilvl_grep_dir) --exclude-dir=platform --exclude-dir=lib
# sed search is sep. with '|' to look for 1: file name, 2: func. name, and 3: stage
_ilvl_sed := sed -e 's|\(.\+\):$(rfunc)(\(.\+\),[[:space:]]\([0-9]\+\));|\3\t\2()\t\1|g'

init_level:
	@echo "Preconfig: $(project_name); searching $(rfunc)() directories:"
	@echo "  $(_ilvl_grep_dir)"
	@echo "  Note: some listed functions may be excluded during build" ; echo ''
	@$(_ilvl_grep) | $(_ilvl_sed) | sort -h | column -t
else
init_level:
	@echo "EE: calling init_level before preconfig"
endif
