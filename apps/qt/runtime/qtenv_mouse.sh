#!/bin/sh
# Board Qt env for HDMI(linuxfb) + USB mouse (evdevmouse).
# Prefer this over stock /etc/qtenv.sh when testing mouse cursor.

export QTDIR=/usr/local/Qt_5.12.5
if [ ! -d "$QTDIR" ]; then
	echo "ERROR: missing $QTDIR — install Qt_5.12.5.tar.gz first"
	return 1 2>/dev/null || exit 1
fi

# Demo bins were linked with RPATH /usr/local/Qt-5.12.5/lib (hyphen)
if [ ! -e /usr/local/Qt-5.12.5 ]; then
	ln -sfn Qt_5.12.5 /usr/local/Qt-5.12.5
	echo "created symlink /usr/local/Qt-5.12.5 -> Qt_5.12.5"
fi

export QT_ROOT=$QTDIR
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:/usr/lib/cedarx:/usr/lib:$LD_LIBRARY_PATH
export QT_QPA_PLATFORM_PLUGIN_PATH=$QT_ROOT/plugins
export QT_QPA_FONTDIR=$QT_ROOT/fonts
export QT_QPA_PLATFORM=linuxfb:tty=/dev/fb0
# USB mouse + optional touch
export QT_QPA_GENERIC_PLUGINS=evdevmouse:evdevtouch
# Show cursor on linuxfb (comment out to hide)
# export QT_QPA_FB_HIDECURSOR=0
unset QT_QPA_FB_HIDECURSOR
mkdir -p /dev/shm
ulimit -c unlimited
echo "qt env ready: $QTDIR (platform=$QT_QPA_PLATFORM plugins=$QT_QPA_GENERIC_PLUGINS)"
