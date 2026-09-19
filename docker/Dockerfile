FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/London

# ============================================================
# System packages
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
    # SDL2
    # --------------------------------------------------------
    libsdl2-dev \
    \
    # --------------------------------------------------------
    # OpenGL / Mesa
    # --------------------------------------------------------
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    mesa-utils \
    \
    # --------------------------------------------------------
    # X11 development libraries
    # --------------------------------------------------------
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxfixes-dev \
    libxxf86vm-dev \
    \
    # --------------------------------------------------------
    # X11 virtual display
    # --------------------------------------------------------
    xvfb \
    \
    # --------------------------------------------------------
    # XFCE desktop
    # --------------------------------------------------------
    xfce4 \
    xfce4-terminal \
    dbus-x11 \
    \
    # --------------------------------------------------------
    # VNC
    # --------------------------------------------------------
    x11vnc \
    \
    # --------------------------------------------------------
    # noVNC
    # --------------------------------------------------------
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
    # OpenCV runtime/build dependencies
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
    # libusb / udev
    # --------------------------------------------------------
    libusb-1.0-0-dev \
    libudev-dev \
    \
    # --------------------------------------------------------
    # Misc
    # --------------------------------------------------------
    nano \
    vim \
    && rm -rf /var/lib/apt/lists/*


# ============================================================
# Create application user
# ============================================================

ARG USERNAME=app
ARG UID=1000
ARG GID=1000

RUN if getent group ${GID} >/dev/null; then \
        EXISTING_GROUP=$(getent group ${GID} | cut -d: -f1); \
        echo "GID ${GID} already exists as ${EXISTING_GROUP}"; \
        groupmod -n ${USERNAME} ${EXISTING_GROUP} 2>/dev/null || true; \
    else \
        groupadd --gid ${GID} ${USERNAME}; \
    fi \
    && if getent passwd ${UID} >/dev/null; then \
        EXISTING_USER=$(getent passwd ${UID} | cut -d: -f1); \
        echo "UID ${UID} already exists as ${EXISTING_USER}"; \
        usermod -l ${USERNAME} ${EXISTING_USER} 2>/dev/null || true; \
    else \
        useradd \
            --uid ${UID} \
            --gid ${GID} \
            --create-home \
            --shell /bin/bash \
            ${USERNAME}; \
    fi \
    && mkdir -p /home/${USERNAME} \
    && chown -R ${UID}:${GID} /home/${USERNAME} \
    && echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" \
        > /etc/sudoers.d/${USERNAME} \
    && chmod 0440 /etc/sudoers.d/${USERNAME}



# ============================================================
# Application source
# ============================================================

WORKDIR /workspace

COPY . /workspace

RUN chown -R ${USERNAME}:${USERNAME} /workspace


# ============================================================
# Build application
# ============================================================


WORKDIR /workspace/external/libasi

RUN rm -rf build \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make  install -j$(nproc)


USER ${USERNAME}
WORKDIR /workspace

RUN rm -rf build \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DHELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED=ON \
        -DHELLOIMGUI_USE_SDL2=ON \
        -DHELLOIMGUI_HAS_OPENGL3=ON \
        -DIMMVISION_FETCH_OPENCV=ON \
        -DIMMVISION_FETCH_OPENCV_FULL=OFF \
    && cmake --build . -j$(nproc)


# ============================================================
# Runtime configuration
# ============================================================

USER root

COPY docker/start.sh /start.sh

RUN chmod +x /start.sh


# ============================================================
# Environment
# ============================================================

ENV DISPLAY=:1
ENV LIBGL_ALWAYS_SOFTWARE=1
ENV XDG_RUNTIME_DIR=/tmp/runtime-app


# ============================================================
# VNC / noVNC
# ============================================================

EXPOSE 6080


# ============================================================
# Start
# ============================================================

CMD ["/start.sh"]
