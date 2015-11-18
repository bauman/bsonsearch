Name: libbsoncompare		
Version: 1.2.1
Release: 5%{?dist}.db
Summary: compares bson docs	

Group:	bauman	
License: MIT	
URL:	TBD	
Source0: bsoncompare.c
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: gcc, mongo-c-driver-devel == %{version}, mongo-c-driver == %{version}, libbson-devel == %{version}, libbson == %{version}, cyrus-sasl-devel, openssl-devel
Requires: mongo-c-driver == %{version}, libbson == %{version}
%description


%prep
#%setup -q

%build
#rm -rf %{buildroot}
mkdir -p %{buildroot}
gcc -I/usr/include/libbson-1.0 -I/usr/include/libmongoc-1.0  -lsasl2 -lssl -lcrypto -lrt -lmongoc-1.0 -lbson-1.0 -shared -o $RPM_BUILD_DIR/libbsoncompare.so -fPIC %{SOURCE0}

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

