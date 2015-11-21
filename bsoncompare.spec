Name: libbsoncompare		
Version: 1.2.1
Release: 6%{?dist}.db
Summary: compares bson docs	

Group:	bauman	
License: MIT	
URL:	TBD	
Source0: bsoncompare.c
Source1: mongoc-matcher.c
Source2: mongoc-matcher-op.c
Source3: mongoc-error.h
Source4: mongoc-matcher.h
Source5: mongoc-matcher-op-private.h
Source6: mongoc-matcher-private.h
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: gcc, libbson-devel == %{version}, libbson == %{version}, cyrus-sasl-devel, openssl-devel
Requires: libbson == %{version}
%description


%prep
#%setup -q

%build
#rm -rf %{buildroot}
mkdir -p %{buildroot}
gcc -I/usr/include/libbson-1.0   -lsasl2 -lssl -lcrypto -lrt -lbson-1.0 -shared -o $RPM_BUILD_DIR/libbsoncompare.so -fPIC %{SOURCE0} %{SOURCE1} %{SOURCE2}

%install
mkdir -p $RPM_BUILD_ROOT/%{_usr}/%{_lib}
install -m 644 -p $RPM_BUILD_DIR/libbsoncompare.so $RPM_BUILD_ROOT/%{_usr}/%{_lib}/libbsoncompare.so

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%{_usr}/%{_lib}/libbsoncompare.so
%doc



%changelog

