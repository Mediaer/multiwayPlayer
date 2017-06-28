################################################################################
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS += -D__STDC_CONSTANT_MACROS
$(warning Kitkat)
$(warning $(LOCAL_PATH))
LOCAL_SRC_FILES:= test_main_m.cpp


ifneq (,$(wildcard vendor/amlogic/frameworks/av/LibPlayer))
LIBPLAYER_PATH:=$(TOP)/vendor/amlogic/frameworks/av/LibPlayer
else
LIBPLAYER_PATH := $(TOP)/packages/amlogic/LibPlayer
endif

#LIBPLAYER_PATH += $(LOCAL_PATH)/arm_ffmpeg_3_1_8/lib \

LOCAL_C_INCLUDES := \
	$(LIBPLAYER_PATH)/amcodec/include \
	$(LIBPLAYER_PATH)/amcodec/amsub_ctl \
	$(JNI_H_INCLUDE)/ \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../include \
	$(TOP)/frameworks/av/ \
	$(TOP)/frameworks/av/media/libstagefright/include \
	$(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/external/ffmpeg/ \
#	$(TOP)/external/ffmpeg-3.0.8/include \
	
#	$(TOP)/external/ffmpeg-3-1-8/ \
#$(LOCAL_PATH)/arm_ffmpeg_3_1_8/include \

LOCAL_SHARED_LIBRARIES := liblog libbinder libutils libcutils
LOCAL_SHARED_LIBRARIES += libCMCC_MediaProcessor libstagefright_foundation libui libgui libamffmpeg

#LOCAL_SHARED_LIBRARIES += libavcodec libavfilter libavformat libavutil libswresample libswscale

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= cmcc_player

include $(BUILD_EXECUTABLE)

















#include $(CLEAR_VARS)
#LOCAL_CFLAGS += -D__STDC_CONSTANT_MACROS
#$(warning Kitkat)
#$(warning $(LOCAL_PATH))
#LOCAL_SRC_FILES:= test_main_m.cpp

#
#ifneq (,$(wildcard vendor/amlogic/frameworks/av/LibPlayer))
#LIBPLAYER_PATH:=$(TOP)/vendor/amlogic/frameworks/av/LibPlayer
#else
#LIBPLAYER_PATH := $(TOP)/packages/amlogic/LibPlayer
#endif
#LOCAL_C_INCLUDES := \
#	$(LIBPLAYER_PATH)/amcodec/include \
#	$(LIBPLAYER_PATH)/amcodec/amsub_ctl \
#	$(JNI_H_INCLUDE)/ \
#	$(LOCAL_PATH)/../ \
#	$(TOP)/frameworks/av/ \
#	$(TOP)/external/ffmpeg \
#	$(TOP)/frameworks/av/media/libstagefright/include \
#	$(TOP)/frameworks/native/include/media/openmax \

#LOCAL_SHARED_LIBRARIES := liblog libbinder libutils libcutils
#LOCAL_CFLAGS += -Wno-multichar

#LOCAL_MODULE_TAGS := optional

#LOCAL_MODULE:= cmcc_player_m

#include $(BUILD_EXECUTABLE)
