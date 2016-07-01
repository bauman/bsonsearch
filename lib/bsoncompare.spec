Name: libbsoncompare		
Version: 1.3.5
Release: 5%{?dist}.db
Summary: compares bson docs	

Group:	 bauman
License: MIT	
URL:	 https://github.com/bauman/bsonsearch
Source0: bsoncompare.c
Source1: mongoc-matcher.c
Source2: mongoc-matcher-op.c
Source3: mongoc-error.h
Source4: mongoc-matcher.h
Source5: mongoc-matcher-op-private.h
Source6: mongoc-matcher-private.h
Source7: bsoncompare.h
Source8: mongoc-matcher-op-geojson.h
Source9: mongoc-matcher-op-geojson.c
Source10: mongoc-bson-descendants.c
Source11: mongoc-bson-descendants.h
Source12: mongoc-matcher-op-yara.c
Source13: mongoc-matcher-op-yara.h
Source14: mongoc-projection.c
Source15: mongoc-projection.h
Source16: BSONSEARCH_LICENSING.txt
Source17: mongoc-matcher-op-unwind.c
Source18: mongoc-matcher-op-unwind.h


BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: gcc, libbson-devel == %{version}, libbson == %{version}, pcre-devel, uthash-devel, yara
Requires: libbson == %{version}, pcre, yara
Provides: libbsoncompare.so()(64bit)

%description
Provides a shared object which can be used to perform mongo-like queries against BSON data.
See website for examples.

%package devel
Summary: Development files for libbsoncompare
Requires: libbsoncompare == %{version}-%{release}, uthash-devel
Group: Development/Libraries


%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
cp -fp %{SOURCE3} ./
cp -fp %{SOURCE4} ./
cp -fp %{SOURCE5} ./
cp -fp %{SOURCE6} ./
cp -fp %{SOURCE7} ./
cp -fp %{SOURCE8} ./
cp -fp %{SOURCE9} ./
cp -fp %{SOURCE10} ./
cp -fp %{SOURCE11} ./
cp -fp %{SOURCE12} ./
cp -fp %{SOURCE13} ./
cp -fp %{SOURCE14} ./
cp -fp %{SOURCE15} ./
cp -fp %{SOURCE16} ./
cp -fp %{SOURCE17} ./
cp -fp %{SOURCE18} ./

#%setup -q

%build
#rm -rf %{buildroot}
mkdir -p %{buildroot}
gcc %optflags -I/usr/include/libbson-1.0 -lbson-1.0 -lpcre -lyara -shared -DWITH_PROJECTION -D WITH_UTILS -D WITH_YARA -o $RPM_BUILD_DIR/libbsoncompare.so -fPIC %{SOURCE0} %{SOURCE1} %{SOURCE2} %{SOURCE9} %{SOURCE10} %{SOURCE12} %{SOURCE14} %{SOURCE17}

%install
mkdir -p $RPM_BUILD_ROOT/%{_usr}/%{_lib}
install -m 644 -p $RPM_BUILD_DIR/libbsoncompare.so $RPM_BUILD_ROOT/%{_usr}/%{_lib}/libbsoncompare.so

mkdir -p $RPM_BUILD_ROOT/%{_includedir}
install -m 644 -p $RPM_BUILD_DIR/bsoncompare.h $RPM_BUILD_ROOT/%{_includedir}/bsoncompare.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-error.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-error.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-matcher.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-matcher.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-matcher-op-private.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-matcher-op-private.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-matcher-private.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-matcher-private.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-matcher-op-geojson.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-matcher-op-geojson.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-bson-descendants.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-bson-descendants.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-matcher-op-yara.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-matcher-op-yara.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-projection.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-projection.h
install -m 644 -p $RPM_BUILD_DIR/mongoc-matcher-op-unwind.h $RPM_BUILD_ROOT/%{_includedir}/mongoc-matcher-op-unwind.h



mkdir -p $RPM_BUILD_ROOT/%{_docdir}/%{name}
install -m 644 -p $RPM_BUILD_DIR/BSONSEARCH_LICENSING.txt $RPM_BUILD_ROOT/%{_docdir}/%{name}/LICENSING.txt

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%{_usr}/%{_lib}/libbsoncompare.so
%doc
%{_docdir}/%{name}/LICENSING.txt
%files devel
%{_includedir}/*.h


%changelog

