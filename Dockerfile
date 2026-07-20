FROM ghcr.io/wiiu-env/devkitppc:20260225

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260418 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/librpxloader:20260329 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmocha:20260331 /artifacts $DEVKITPRO

RUN dkp-pacman -Syu --noconfirm --needed \
    wiiu-dev \
    wiiu-sdl2 \
    wiiu-sdl2_image \
    wiiu-sdl2_mixer \
    wiiu-sdl2_ttf

WORKDIR /project

