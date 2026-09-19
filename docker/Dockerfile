FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/London

# ============================================================
# System dependencies
# ============================================================

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    ca-certificates \
    wget \
    curl \
    sudo \
    \
    # --------------------------------------------------------
    # GLFW / OpenGL
    # --------------------------------------------------------
    libglfw3-dev \
    libglfw3 \
    libgl1-mesa-dev \
    libgl1-mesa-dri \
    libglu1-mesa-dev \
    libglx-mesa0 \
    mesa-utils \
    \
    # --------------------------------------------------------
    # X11 / Xorg
    # --------------------------------------------------------
    xserver-xorg \
    xserver-xorg-core \
    xserver-xorg-video-dummy \
    x11-xserver-utils \
    x11-utils \
    xauth \
    \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxfixes-dev \
    libxxf86vm-dev \
    libxrender-dev \
    libxdamage-dev \
    libxcomposite-dev \
    \
    # --------------------------------------------------------
    # Desktop
    # --------------------------------------------------------
    xfce4 \
    xfce4-terminal \
    dbus \
    dbus-x11 \
    \
    # --------------------------------------------------------
    # VNC / browser VNC
    # --------------------------------------------------------
    x11vnc \
    novnc \
    websockify \
    \
    # --------------------------------------------------------
    # Fonts
    # --------------------------------------------------------
    fonts-dejavu \
    fonts-liberation \
    \
    # --------------------------------------------------------
    # OpenCV
    # --------------------------------------------------------
    libgtk-3-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libswscale-dev \
    libv4l-dev \
    \
    # --------------------------------------------------------
    # USB / udev
    # --------------------------------------------------------
    libusb-1.0-0-dev \
    libudev-dev \
    \
    # --------------------------------------------------------
    # Misc
    # --------------------------------------------------------
    dbus \
    && rm -rf /var/lib/apt/lists/*

# ============================================================
# Application user
#
# Ubuntu 24.04 already has GID 1000, so use 1001.
# ============================================================

ARG USERNAME=app
ARG UID=1001
ARG GID=1001

RUN groupadd \
        --gid ${GID} \
        ${USERNAME} \
    && useradd \
        --uid ${UID} \
        --gid ${GID} \
        --create-home \
        --shell /bin/bash \
        ${USERNAME} \
    && echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" \
        > /etc/sudoers.d/${USERNAME} \
    && chmod 0440 /etc/sudoers.d/${USERNAME}

# ============================================================
# Xorg dummy configuration
# ============================================================

COPY docker/xorg.conf /etc/X11/xorg.conf

# ============================================================
# Application
# ============================================================

WORKDIR /workspace

COPY . /workspace

RUN chown -R ${USERNAME}:${USERNAME} /workspace

# ============================================================
# Build libasi
# ============================================================

WORKDIR /workspace/external/libasi

RUN rm -rf build \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && make install

# ============================================================
# Build AstroCapture
# ============================================================

USER ${USERNAME}

WORKDIR /workspace

RUN rm -rf build \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DHELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED=ON \
        -DHELLOIMGUI_USE_SDL2=OFF \
        -DHELLOIMGUI_USE_GLFW3=ON \
        -DHELLOIMGUI_HAS_OPENGL3=ON \
        -DIMMVISION_FETCH_OPENCV=ON \
        -DIMMVISION_FETCH_OPENCV_FULL=OFF \
    && cmake --build . -j$(nproc)

# ============================================================
# Runtime
# ============================================================

USER root

COPY docker/start.sh /start.sh

RUN chmod +x /start.sh

# ============================================================
# Runtime environment
# ============================================================

ENV DISPLAY=:1
ENV XDG_RUNTIME_DIR=/tmp/runtime-app

# ------------------------------------------------------------
# GLFW
#
# GLFW will use X11 because the container is running Xorg.
# ------------------------------------------------------------

ENV LIBGL_ALWAYS_SOFTWARE=1

# Explicitly prevent Wayland from being selected.
ENV WAYLAND_DISPLAY=

EXPOSE 6080

CMD ["/start.sh"]
