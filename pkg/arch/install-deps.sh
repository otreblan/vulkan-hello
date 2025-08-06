#!/usr/bin/env bash

set -ex

source PKGBUILD || true

pacman -Sy
pacman -U --noconfirm /var/cache/makepkg/pkg/*

printf "%s\n" "${depends[@]}" "${makedepends[@]}" |\
grep -vxFf <(pacman -Qi -p /var/cache/makepkg/pkg/* | awk '/Name/ {print $3}') |\
xargs -- pacman -Syu --noconfirm --needed
