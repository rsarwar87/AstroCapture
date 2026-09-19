#!/bin/bash

set -e

###############################################################################
# Environment
###############################################################################

export DISPLAY=:1
export XDG_RUNTIME_DIR=/tmp/runtime-app
export XDG_SESSION_TYPE=x11
export GDK_BACKEND=x11
export QT_QPA_PLATFORM=xcb
export LIBGL_ALWAYS_SOFTWARE=1

# Explicitly remove Wayland variables.
unset WAYLAND_DISPLAY
unset WAYLAND_SOCKET


###############################################################################
# Cleanup
###############################################################################

echo
echo "=========================================="
echo "Starting AstroCapture"
echo "=========================================="

echo "Cleaning up stale processes..."

pkill -TERM -x Xorg 2>/dev/null || true
pkill -TERM -x x11vnc 2>/dev/null || true
pkill -TERM -x websockify 2>/dev/null || true

sleep 1

pkill -KILL -x Xorg 2>/dev/null || true
pkill -KILL -x x11vnc 2>/dev/null || true
pkill -KILL -x websockify 2>/dev/null || true

rm -f /tmp/.X1-lock
rm -rf /tmp/.X11-unix/X1


###############################################################################
# XDG runtime directory
###############################################################################

echo "Preparing XDG_RUNTIME_DIR..."

rm -rf "${XDG_RUNTIME_DIR}"
mkdir -p "${XDG_RUNTIME_DIR}"

chown app:app "${XDG_RUNTIME_DIR}"
chmod 700 "${XDG_RUNTIME_DIR}"


###############################################################################
# Xorg
###############################################################################

echo
echo "=========================================="
echo "Starting Xorg dummy display"
echo "=========================================="

Xorg :1 \
    -config /etc/X11/xorg.conf \
    -noreset \
    -nolisten tcp \
    > /tmp/xorg.log 2>&1 &

XORG_PID=$!

echo "Xorg PID: ${XORG_PID}"
echo "Waiting for X server..."

XORG_READY=0

for i in $(seq 1 30); do

    if DISPLAY=:1 xdpyinfo >/dev/null 2>&1; then
        XORG_READY=1
        break
    fi

    if ! kill -0 "${XORG_PID}" 2>/dev/null; then
        echo
        echo "ERROR: Xorg exited."
        echo
        cat /tmp/xorg.log
        exit 1
    fi

    sleep 0.5
done

if [ "${XORG_READY}" -ne 1 ]; then
    echo
    echo "ERROR: Xorg failed to become ready."
    echo
    cat /tmp/xorg.log
    exit 1
fi

echo "Xorg started successfully."


###############################################################################
# Verify X11 environment
###############################################################################

echo
echo "=========================================="
echo "X11 environment"
echo "=========================================="

echo "DISPLAY          = ${DISPLAY}"
echo "XDG_SESSION_TYPE = ${XDG_SESSION_TYPE}"
echo "XDG_RUNTIME_DIR  = ${XDG_RUNTIME_DIR}"
echo "GDK_BACKEND      = ${GDK_BACKEND}"
echo "WAYLAND_DISPLAY  = ${WAYLAND_DISPLAY:-<unset>}"


###############################################################################
# OpenGL
###############################################################################

echo
echo "=========================================="
echo "OpenGL information"
echo "=========================================="

DISPLAY=:1 glxinfo -B || true


###############################################################################
# XFCE
###############################################################################

echo
echo "=========================================="
echo "Starting XFCE"
echo "=========================================="

su - app -c '
    export DISPLAY=:1
    export XDG_RUNTIME_DIR=/tmp/runtime-app
    export XDG_SESSION_TYPE=x11
    export GDK_BACKEND=x11
    export QT_QPA_PLATFORM=xcb
    export LIBGL_ALWAYS_SOFTWARE=1

    unset WAYLAND_DISPLAY
    unset WAYLAND_SOCKET

    dbus-launch --exit-with-session startxfce4
' > /tmp/xfce.log 2>&1 &

XFCE_PID=$!

sleep 5


###############################################################################
# x11vnc
###############################################################################

echo
echo "=========================================="
echo "Starting x11vnc"
echo "=========================================="

echo "x11vnc version:"
x11vnc -version 2>&1 | head -5 || true

echo
echo "Starting x11vnc against Xorg :1..."

# IMPORTANT:
#
# Use env -i so x11vnc receives no Wayland/session environment
# from Docker, XFCE, DBus, or the host.
#
# Only the minimum environment required by x11vnc is supplied.

env -i \
    PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    HOME=/root \
    DISPLAY=:1 \
    x11vnc \
        -display :1 \
        -rfbport 5900 \
        -rfbportv6 0 \
        -localhost \
        -forever \
        -shared \
        -nopw \
        -noxdamage \
        -nowf \
        -nowcr \
        -noxrecord \
        -noxfixes \
        -noxkb \
        > /tmp/x11vnc.log 2>&1 &

X11VNC_PID=$!

echo "x11vnc PID: ${X11VNC_PID}"
echo "Waiting for VNC server..."

VNC_READY=0

for i in $(seq 1 30); do

    if (echo > /dev/tcp/127.0.0.1/5900) >/dev/null 2>&1; then
        VNC_READY=1
        break
    fi

    if ! kill -0 "${X11VNC_PID}" 2>/dev/null; then
        echo
        echo "ERROR: x11vnc exited."
        echo
        cat /tmp/x11vnc.log
        exit 1
    fi

    sleep 0.5
done

if [ "${VNC_READY}" -ne 1 ]; then
    echo
    echo "ERROR: x11vnc is not listening on port 5900."
    echo
    cat /tmp/x11vnc.log
    exit 1
fi

echo "x11vnc is listening on 127.0.0.1:5900"


###############################################################################
# noVNC / websockify
###############################################################################

echo
echo "=========================================="
echo "Starting noVNC"
echo "=========================================="

env -i \
    PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    HOME=/root \
    websockify \
        --web=/usr/share/novnc \
        --verbose \
        0.0.0.0:6080 \
        127.0.0.1:5900 \
        > /tmp/novnc.log 2>&1 &

NOVNC_PID=$!

echo "websockify PID: ${NOVNC_PID}"
echo "Waiting for noVNC..."

NOVNC_READY=0

for i in $(seq 1 30); do

    if (echo > /dev/tcp/127.0.0.1/6080) >/dev/null 2>&1; then
        NOVNC_READY=1
        break
    fi

    if ! kill -0 "${NOVNC_PID}" 2>/dev/null; then
        echo
        echo "ERROR: websockify exited."
        echo
        cat /tmp/novnc.log
        exit 1
    fi

    sleep 0.5
done

if [ "${NOVNC_READY}" -ne 1 ]; then
    echo
    echo "ERROR: noVNC/websockify is not listening on port 6080."
    echo
    cat /tmp/novnc.log
    exit 1
fi

echo "noVNC is listening on 0.0.0.0:6080"


###############################################################################
# AstroCapture
###############################################################################

echo
echo "=========================================="
echo "Starting AstroCapture"
echo "=========================================="

su - app -c '
    export DISPLAY=:1
    export XDG_RUNTIME_DIR=/tmp/runtime-app
    export XDG_SESSION_TYPE=x11
    export GDK_BACKEND=x11
    export QT_QPA_PLATFORM=xcb
    export LIBGL_ALWAYS_SOFTWARE=1

    unset WAYLAND_DISPLAY
    unset WAYLAND_SOCKET

    cd /workspace/build

    ./astrocapture
' &

APP_PID=$!


###############################################################################
# Status
###############################################################################

echo
echo "=========================================="
echo "AstroCapture is running"
echo "=========================================="
echo
echo "Browser VNC:"
echo
echo "    http://localhost:6080/vnc.html"
echo
echo "VNC server:"
echo
echo "    127.0.0.1:5900"
echo
echo "Display:"
echo
echo "    DISPLAY=:1"
echo
echo "Renderer:"
echo
echo "    Mesa llvmpipe"
echo
echo "Session:"
echo
echo "    X11"
echo
echo "=========================================="


###############################################################################
# Wait for AstroCapture
###############################################################################

wait "${APP_PID}"

EXIT_CODE=$?

echo
echo "AstroCapture exited with code ${EXIT_CODE}"

exit "${EXIT_CODE}"
