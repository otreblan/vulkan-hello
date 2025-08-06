FROM greyltc/archlinux-aur:paru AS dependencies
RUN echo 'MAKEFLAGS="-j$(nproc)"' >> /etc/makepkg.conf
WORKDIR /var/ab/pkg/arch
USER ab
COPY ./pkg/arch /var/ab/pkg/arch
RUN --mount=type=cache,target=/var/ab/.cache/paru,sharing=private,uid=971,gid=971 \
	./build-deps.sh

FROM archlinux:base-devel AS build
WORKDIR /vulkan-hello/pkg/arch
COPY ./pkg/arch .
RUN --mount=type=bind,from=dependencies,source=/var/cache/makepkg/pkg/,target=/var/cache/makepkg/pkg/ \
	./install-deps.sh
WORKDIR /vulkan-hello
COPY ./pkg/arch ./pkg/arch
COPY . .
RUN cmake -B build
RUN cmake --build build --parallel $(nproc)

FROM build AS exec
WORKDIR /vulkan-hello/build
