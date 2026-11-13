# OpenBoard quality profile

Profile version: `1.0.0`
Adopted framework: [interactive acceptance framework 1.0.0](FRAMEWORK.md)
Adopted catalogue: [OpenBoard missions and capabilities 1.0.0](MISSIONS.md)

## Users and installations

OpenBoard is open source and used in very different settings. A campaign keeps all of them in mind, not only the one of the tester:

- a teacher or trainer on a personal computer, documents stored locally;
- schools and organisations with shared computers, roaming or temporary user profiles, documents on a network share or a synchronised folder, settings deployed by an administrator;
- interactive whiteboards and touch screens, projectors that are connected and disconnected, laptops docked and undocked;
- Linux distributions with their own Qt and library versions, Windows and macOS installations, with or without administrator rights;
- many interface languages and keyboard layouts.

What can fail in one of these settings is part of the product: a document location that disappears or does not answer, a screen that changes, a profile deleted at logout, a document opened with another version of OpenBoard.

## Scope

Standard use of OpenBoard, including its preparation by the person who installs it:

- Board mode: writing, drawing and erasing, text, tools, library content (images, shapes, applications, sounds, videos), pages, backgrounds, zoom, undo;
- Documents mode: folders and documents, page management, import and export, trash;
- Desktop and Web modes: annotation over the desktop or a web page, capture to the board;
- presentation on several screens: control, display and previous-page screens, screen changes during use;
- saving and reading documents, including when their location fails, does not answer or comes back, and recovery after OpenBoard was interrupted;
- installation settings: configuration files of the administrator and of the system, data directory, administrator library, preferences, interface languages.

Out of scope of the functional catalogue: building and packaging, installers and automatic updates, third-party web sites, network infrastructure, and performance at large scale. Automated tests are separate evidence.

## Platforms and conditions

A full campaign records the platforms actually used and does not claim others. Over successive campaigns, cover:

- Windows, Linux with X11 and with Wayland, macOS;
- one screen, two screens (extended and duplicated), a screen added or removed during use, display scaling other than 100 %;
- pen or touch input and mouse;
- at least one interface language other than English;
- documents stored locally and on a network share.

A capability which depends on a platform (e.g. screen switching) is checked on each platform it applies to, or reported `BLOCKED` for the missing ones.

## Test environment

- Use a test copy of OpenBoard whose configuration file sets the data directory (`App/DataDirectory`) and, when relevant, the recovery directory (`App/RecoveryDirectory`) to test locations. Never run tests on the documents, settings or library of a real user.
- Administrator and system configuration files used for a test are test copies, removed or restored afterwards.
- Failure conditions are created with disclosed, reversible means, chosen for the platform: removing read or write permission, holding a file open so that reading it does not answer, disconnecting a share or a drive, unmounting a volume. Restore the condition afterwards.
- The tester may be a person or an AI agent controlling the desktop. An agent acts only on the windows of the test copy, and stops when the desktop is in use by someone else.

## Source anchors

When features change, the catalogue is reviewed against the code that defines what a user can do: the modes and their palettes (`src/board`, `src/document`, `src/desktop`, `src/web`, `src/gui`), the import and export adaptors (`src/adaptors`), the persistence of documents (`src/core`), screen handling (`src/core/UBDisplayManager*`) and settings (`src/core/UBSettings*`). This list helps maintenance; it is not a proof of coverage.

## Reports

Reports are written in English, follow [templates/report.md](templates/report.md), and contain only test data.
