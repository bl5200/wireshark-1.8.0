#!/bin/bash
#
# $Id: win32-setup.sh 42933 2012-05-30 22:55:02Z gerald $

# 32-bit wrapper for win-setup.sh.

export DOWNLOAD_TAG="2012-05-30"
export WIRESHARK_TARGET_PLATFORM="win32"

WIN_SETUP=`echo $0 | sed -e s/win32/win/`

exec $WIN_SETUP $@
