# Conditional build
#
# --with crack		- with crack
# --with gpgme          - with gpgme
# --with gpgme10        - with gpgme10 (for vinelinux)

%define ver 0.2.10
%define rel 0vl1

Summary:	Mnid - a GTK+ based ID/Password management tool
Summary(ja):	Mnid - GTK+ベースのID/パスワード情報を一括管理するツール
Name:		mnid
Version:	%{ver}
%if%{?_with_gpgme:1}%{!?_with_gpgme:0}%{?_with_gpgme10:1}%{!?_with_gpgme10:0}
Release:        %{rel}_gpgme
%else
Release:        %{rel}
%endif
Copyright:	GPL
Group:		Applications/Internet
Source0:	http://www.org3.net/mnid/%{name}-%{ver}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
Requires:       gtk2 >= 2.4.0, glib2 >= 2.4.0
Requires:       libxml2 >= 2.6.0
Requires:       openssl
%{?_with_crack:Requires:                cracklib, cracklib-dicts}
%{?_with_gpgme:Requires:                gpgme >= 0.4.5}
%{?_with_gpgme10:Requires:              gpgme10 >= 1.0.0}

BuildRequires:  gtk2-devel >= 2.4.0, glib2-devel >= 2.4.0
BuildRequires:  libxml2-devel >= 2.6.0
BuildRequires:  openssl-devel
%{?_with_gpgme:BuildRequires:           gpgme-devel >= 0.4.5}
%{?_with_gpgme10:BuildRequires:         gpgme10-devel >= 1.0.0}

%description
Mnid (Mneme ID) is an ID/Password management tool based on GTK+ GUI toolkit,
and runs on X Window System.

Mnid is a free software distributed under the GNU GPL.

%description -l ja
Mnidは、X 上で動作する GTK+ ベースの ID/パスワード情報を一括管理できるツールです。

%prep
%setup  -q

%build
%configure \
%{?_with_crack:		--enable-crack } \
%{?_with_gpgme:		--enable-gpgme } \
%{?_with_gpgme10:	--enable-gpgme }

%__make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
[ "$RPM_BUILD_ROOT" != "/" ] && %__rm -rf $RPM_BUILD_ROOT
%makeinstall 
%__install -d $RPM_BUILD_ROOT%{_datadir}/applications
%__install -p -m644 %{name}.desktop $RPM_BUILD_ROOT%{_datadir}/applications/%{name}.desktop


%clean
%__rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog README INSTALL NEWS
%{_bindir}/%{name}
%{_datadir}/locale/*/LC_MESSAGES/%{name}.mo
%{_datadir}/pixmaps/*.png
%config(missingok) %{_datadir}/applications/%{name}.desktop

%changelog
* Fri Mar 18 2005 NOGUCHI Shoji <noguchi@org3.net>
- first release
