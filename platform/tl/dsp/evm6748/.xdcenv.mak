#
_XDCBUILDCOUNT = 1
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/CCS/tirtos_c6000_2_00_01_23/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/bios_6_40_01_15/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/ipc_3_10_01_11/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/ndk_2_23_01_01/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/uia_2_00_00_28/packages
override XDCROOT = C:/CCS/xdctools_3_30_03_47_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/CCS/tirtos_c6000_2_00_01_23/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/bios_6_40_01_15/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/ipc_3_10_01_11/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/ndk_2_23_01_01/packages;C:/CCS/tirtos_c6000_2_00_01_23/products/uia_2_00_00_28/packages;C:/CCS/xdctools_3_30_03_47_core/packages;../../..
HOSTOS = Windows
endif
