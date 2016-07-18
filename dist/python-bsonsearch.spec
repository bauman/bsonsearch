%define name python-bsonsearch
%define version 1.3.5
%define unmangled_version 1.3.5
%define unmangled_version 1.3.5
%define release 9

Summary: Ctypes wrapper to libbsoncompare
Name: %{name}
Version: %{version}
Release: %{release}%{?dist}
Source0: %{name}-%{unmangled_version}.tar.gz
License: MIT
Group: Development/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}
BuildArch: x86_64
Requires: libbsoncompare-lite == %{version}, python-bson
BuildRequires: libbson-devel == %{version}
Vendor: Dan Bauman

%description
Library wrapping libbsoncompare.  Also includes feature to convert a given spec to work on documents with lists embedded in the namespace.
This wrapper requires AT LEAST libbsoncompare-lite.  If libbsoncompare (full version) is available, it will prefer and use that.

%prep
%setup -n %{name}-%{unmangled_version} -n %{name}-%{unmangled_version}

%build
python setup.py build

%install
python setup.py install -O1 --root=$RPM_BUILD_ROOT --record=INSTALLED_FILES

%clean
rm -rf $RPM_BUILD_ROOT

%files -f INSTALLED_FILES
%defattr(-,root,root)
