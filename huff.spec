Name:           huff
Summary:        simple implementation of the Huffman algorithm
Version:        0.1
Release:        1
License:        BSD
Group:          System Environment/Base
Packager:       Yahor Zabolotski  <yahorzabolotsky@gmail.com>
URL:            http://github.com/yahorzabolotsky/archivator

Source:         %{name}-%{version}.tar

BuildRequires: gcc, make

%description
The Huffman algorithm implementation

%prep
%setup

%build
make

%install
mkdir -p %{buildroot}%{_bindir}
cp -f build/huff %{buildroot}%{_bindir}/huff

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/*

%changelog
* Tue Apr 5 2016 Yahor Zabolotski <yahorzabolotski@gmail.com> 0.1-1
- Initial spec
