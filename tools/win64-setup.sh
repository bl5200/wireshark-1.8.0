#!/bin/bash
#
# $Id: win64-setup.sh 43143 2012-06-06 20:43:49Z gerald $

# 64-bit wrapper for win-setup.sh.

export DOWNLOAD_TAG="2012-06-06"
export WIRESHARK_TARGET_PLATFORM="win64"

WIN_SETUP=`echo $0 | sed -e s/win64/win/`

exec $WIN_SETUP $@
