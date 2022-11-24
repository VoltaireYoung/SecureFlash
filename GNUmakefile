## @file
# GNU/Linux makefile for 'GetKey' module build.
#
# Copyright (c) 2016,Wonder Profressional. All rights reserved.<BR>
# This program and the accompanying materials
#
ARCH ?= IA32
MAKEROOT ?= ..

APPNAME = GetKey

SDK_C = Sdk/C

OBJECTS = \
  GetKey.o \
  $(SDK_C)/Alloc.o \
  $(SDK_C)/LzFind.o \
  $(SDK_C)/LzmaDec.o \
  $(SDK_C)/LzmaEnc.o \
  $(SDK_C)/7zFile.o \
  $(SDK_C)/7zStream.o \
  $(SDK_C)/Bra86.o  

include $(MAKEROOT)/Makefiles/app.makefile

LIBS = -lCommon


