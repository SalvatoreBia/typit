#!/bin/bash
set -e

VERSION=${1:-0.1.0}
ARCH="amd64"
PKG_NAME="typit_${VERSION}_${ARCH}"

echo "Building typit..."
make clean
make

echo "Creating .deb package..."
mkdir -p "${PKG_NAME}/DEBIAN"
mkdir -p "${PKG_NAME}/usr/bin"
mkdir -p "${PKG_NAME}/usr/share/typit"

cp build/typer "${PKG_NAME}/usr/bin/typit"
cp -r resources "${PKG_NAME}/usr/share/typit/"

cat > "${PKG_NAME}/DEBIAN/control" << EOF
Package: typit
Version: ${VERSION}
Section: games
Priority: optional
Architecture: ${ARCH}
Depends: libraylib-dev
Maintainer: SalvatoreBia <salvatore@example.com>
Description: A terminal-inspired typing test application
 A typing test built with raylib.
EOF

dpkg-deb --build "${PKG_NAME}"
rm -rf "${PKG_NAME}"

echo "Created ${PKG_NAME}.deb"
