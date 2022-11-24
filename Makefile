## @file
# Windows makefile for 'GetKey' module build.
#
# Copyright (c) 2016, Wonder Professional. All rights reserved.<BR>
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
!INCLUDE ..\Makefiles\ms.common

APPNAME = GetKey

LIBS = $(LIB_PATH)\Common.lib  $(LIB_PATH)\jsonD.lib $(LIB_PATH)\Crypto.lib
SDK_C = Sdk/C


OBJECTS = \
  GetKey.obj \
  $(SDK_C)/Alloc.obj \
  $(SDK_C)/LzFind.obj \
  $(SDK_C)/LzmaDec.obj \
  $(SDK_C)/LzmaEnc.obj \
  $(SDK_C)/7zFile.obj \
  $(SDK_C)/7zStream.obj \
  $(SDK_C)/Bra86.obj

!INCLUDE ..\Makefiles\ms.app
