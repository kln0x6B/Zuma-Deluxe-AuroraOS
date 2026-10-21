Name:       ru.kln0x6b.AurZuma
Summary:    Port of the Zuma Deluxe HD game to Aurora OS
Version:    1.0
Release:    1
License:    MIT
URL:        https://github.com/kln0x6B/Zuma-Deluxe-AuroraOS
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   qt5-qtmultimedia-plugin-audio-pulseaudio
BuildRequires:  pkgconfig(auroraapp)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(vorbisfile)

%description
Zuma Deluxe AuroraOS is a port of the Zuma Deluxe HD game
(https://github.com/GalaxyShad/Zuma-Deluxe-HD) by GalaxyShad and s4lat to Aurora OS.
The source code is distributed under the MIT License of the original project.
Game graphics, sounds and music belong to PopCap Games and Electronic Arts and are
distributed on the same terms as in the original project.

%prep
%autosetup

%build
%qmake5
%make_build

%install
%make_install

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%defattr(644,root,root,-)
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
/manifest.json
