Name:           typit
Version:        0.1.0
Release:        1%{?dist}
Summary:        A typing test application

License:        MIT
URL:            https://github.com/SalvatoreBia/typit
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc make raylib-devel
Requires:       raylib

%description
A typing test application built with raylib.
Features infinite word scrolling, error tracking, and WPM calculation.

%prep
%autosetup

%build
make %{?_smp_mflags}

%install
install -Dm755 build/typer %{buildroot}%{_bindir}/typit
install -d %{buildroot}%{_datadir}/typit/resources
cp -r resources/* %{buildroot}%{_datadir}/typit/resources/

%files
%{_bindir}/typit
%{_datadir}/typit/resources/*

%changelog
* %(date "+%a %b %d %Y") SalvatoreBia <salvatore@example.com> - 0.1.0-1
- Initial package
