#!/bin/bash
set -e

VERSION=${1:-0.1.0}

echo "Building typit..."
make clean
make

echo "Creating RPM package..."
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
mkdir -p ~/rpmbuild/BUILDROOT/typit-${VERSION}-1.x86_64/usr/bin
mkdir -p ~/rpmbuild/BUILDROOT/typit-${VERSION}-1.x86_64/usr/share/typit

cp build/typer ~/rpmbuild/BUILDROOT/typit-${VERSION}-1.x86_64/usr/bin/typit
cp -r resources ~/rpmbuild/BUILDROOT/typit-${VERSION}-1.x86_64/usr/share/typit/

cat > ~/rpmbuild/SPECS/typit.spec << EOF
Name: typit
Version: ${VERSION}
Release: 1
Summary: A typing test application
License: MIT

%description
A typing test built with raylib.

%files
/usr/bin/typit
/usr/share/typit/resources
EOF

rpmbuild -bb ~/rpmbuild/SPECS/typit.spec

echo "RPM created in ~/rpmbuild/RPMS/x86_64/"
