# 
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.27
# 

Name:       grilo-qt

# >> macros
# << macros

Summary:    Framework for discovering and browsing media, Qt bindings
Version:    0.1.11
Release:    1
Group:      Development/Libraries
License:    BSD
URL:        https://github.com/nemomobile/qtgrilo
Source0:    %{name}-%{version}.tar.bz2
Source1:    %{name}.yaml
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(QtCore) >= 4.7.0
BuildRequires:  pkgconfig(QtDeclarative)
BuildRequires:  pkgconfig(grilo-0.2) >= 0.2.12

%description
%{summary}.

%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Development files for %{name}.

%package qml-plugin
Summary:    QML plugin for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Provides:   nemo-qml-plugin-grilo > 0.1.6
Obsoletes:  nemo-qml-plugin-grilo <= 0.1.6

%description qml-plugin
QML plugin for %{name}.

%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre

%qmake 

make %{?_smp_mflags}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake_install

# >> install post
# << install post

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/lib*.so.*
# >> files
# << files

%files devel
%defattr(-,root,root,-)
%{_datarootdir}/qt4/mkspecs/features/%{name}.prf
%{_includedir}/qt4/GriloQt/GriloBrowse
%{_includedir}/qt4/GriloQt/GriloDataSource
%{_includedir}/qt4/GriloQt/GriloMedia
%{_includedir}/qt4/GriloQt/GriloModel
%{_includedir}/qt4/GriloQt/GriloMultiSearch
%{_includedir}/qt4/GriloQt/GriloQt
%{_includedir}/qt4/GriloQt/GriloQuery
%{_includedir}/qt4/GriloQt/GriloRegistry
%{_includedir}/qt4/GriloQt/GriloSearch
%{_includedir}/qt4/GriloQt/GriloSingleDataSource
%{_includedir}/qt4/GriloQt/grilobrowse.h
%{_includedir}/qt4/GriloQt/grilodatasource.h
%{_includedir}/qt4/GriloQt/grilomedia.h
%{_includedir}/qt4/GriloQt/grilomodel.h
%{_includedir}/qt4/GriloQt/grilomultisearch.h
%{_includedir}/qt4/GriloQt/griloqt.h
%{_includedir}/qt4/GriloQt/griloquery.h
%{_includedir}/qt4/GriloQt/griloregistry.h
%{_includedir}/qt4/GriloQt/grilosearch.h
%{_includedir}/qt4/GriloQt/grilosingledatasource.h
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/%{name}.pc
# >> files devel
# << files devel

%files qml-plugin
%defattr(-,root,root,-)
%{_libdir}/qt4/imports/org/nemomobile/grilo/libgrilo-qt-qml-plugin.so
%{_libdir}/qt4/imports/org/nemomobile/grilo/qmldir
# >> files qml-plugin
# << files qml-plugin
