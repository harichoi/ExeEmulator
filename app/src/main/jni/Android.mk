LOCAL_PATH := $(call my-dir) 
include $(CLEAR_VARS)

LOCAL_MODULE    := dasm
LOCAL_SRC_FILES := kr_selfcontrol_exeemulator_dasm.c \ dasm.c \ disasm.c \ asmserv.c
include $(BUILD_SHARED_LIBRARY)
