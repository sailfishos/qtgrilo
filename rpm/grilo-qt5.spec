Name:       grilo-qt5

Summary:    Framework for discovering and browsing media, Qt bindings
Version:    0.3.3
Release:    1
License:    LGPLv2+
URL:        https://github.com/sailfishos/qtgrilo
Source0:    %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(grilo-0.3)

%description
%{summary}.

%package devel
Summary:    Development files for %{name}
Requires:   %{name} = %{version}-%{release}

%description devel
Development files for %{name}.

%package qml-plugin
Summary:    QML plugin for %{name}
Requires:   %{name} = %{version}-%{release}

%description qml-plugin
QML plugin for %{name}.

%prep
%setup -q -n %{name}-%{version}

%build

%qmake5 VERSION=`echo %{version} | sed 's/+.*//'`

make %{?_smp_mflags}


%install
rm -rf %{buildroot}
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/lib*.so.*

%files devel
%defattr(-,root,root,-)
%{_datarootdir}/qt5/mkspecs/features/%{name}.prf
%{_includedir}/qt5/GriloQt/GriloBrowse
%{_includedir}/qt5/GriloQt/GriloDataSource
%{_includedir}/qt5/GriloQt/GriloMedia
%{_includedir}/qt5/GriloQt/GriloModel
%{_includedir}/qt5/GriloQt/GriloMultiSearch
%{_includedir}/qt5/GriloQt/GriloQt
%{_includedir}/qt5/GriloQt/GriloQuery
%{_includedir}/qt5/GriloQt/GriloRegistry
%{_includedir}/qt5/GriloQt/grilobrowse.h
%{_includedir}/qt5/GriloQt/grilodatasource.h
%{_includedir}/qt5/GriloQt/grilomedia.h
%{_includedir}/qt5/GriloQt/grilomodel.h
%{_includedir}/qt5/GriloQt/grilomultisearch.h
%{_includedir}/qt5/GriloQt/griloqt.h
%{_includedir}/qt5/GriloQt/griloquery.h
%{_includedir}/qt5/GriloQt/griloregistry.h
%{_includedir}/qt5/GriloQt/grilosearch.h
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/%{name}.pc

%files qml-plugin
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/org/nemomobile/grilo/libgrilo-qt5-qml-plugin.so
%{_libdir}/qt5/qml/org/nemomobile/grilo/plugins.qmltypes
%{_libdir}/qt5/qml/org/nemomobile/grilo/qmldir
