#!/bin/bash

set -e

export DISPLAY=:1
export XDG_RUNTIME_DIR=/tmp/runtime-app

echo "=========================================="
echo "Starting Astrocapture"
echo "=========================================="

# ------------------------------------------------------------
# Runtime directory
# ------------------------------------------------------------

mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"


# ------------------------------------------------------------
# Start virtual X server
# ------------------------------------------------------------

echo "Starting Xvfb..."

Xvfb :1 \
    -screen 0 1920x1080x24 \
    -ac \
    +extension GLX \
    +extension RANDR \
    +render \
    -noreset \
    &

XVFB_PID=$!

sleep 2


# ------------------------------------------------------------
# Start XFCE
# ------------------------------------------------------------

echo "Starting XFCE..."

su - app -c "
    export DISPLAY=:1
    export XDG_RUNTIME_DIR=/tmp/runtime-app

    dbus-launch \
        --exit-with-session \
        startxfce4
" &

XFCE_PID=$!

sleep 5


# ------------------------------------------------------------
# Start x11vnc
# ------------------------------------------------------------

echo "Starting x11vnc..."

x11vnc \
    -display :1 \
    -rfbport 5900 \
    -forever \
    -shared \
    -nopw \
    -noxdamage \
    -xkb \
    -repeat \
    &

X11VNC_PID=$!

sleep 2


# ------------------------------------------------------------
# Start noVNC
# ------------------------------------------------------------

echo "Starting noVNC..."

websockify \
    --web=/usr/share/novnc \
    6080 \
    localhost:5900 &

NOVNC_PID=$!

sleep 2


# ------------------------------------------------------------
# Start application
# ------------------------------------------------------------

echo "Starting astrocapture..."

su - app -c "
    export DISPLAY=:1
    export XDG_RUNTIME_DIR=/tmp/runtime-app
    export LIBGL_ALWAYS_SOFTWARE=1

    cd /workspace/build

    ./astrocapture
" &

APP_PID=$!


# ------------------------------------------------------------
# Information
# ------------------------------------------------------------

echo
echo "=========================================="
echo "Astrocapture is running"
echo "=========================================="
echo
echo "Browser VNC:"
echo
echo "    http://localhost:6080/vnc.html"
echo
echo "Container display:"
echo
echo "    DISPLAY=:1"
echo
echo "=========================================="


# ------------------------------------------------------------
# Keep container alive
# ------------------------------------------------------------

wait $APP_PID
