# Missions and capability catalogue

Version: `1.0.0`

The missions describe what a user wants to achieve; the catalogue lists the capabilities a campaign must account for, with their expected outcomes. Read the [framework](FRAMEWORK.md), the [profile](CONTEXT.md) and the [test data](FIXTURES.md) first.

A mission is not a click script. The tester chooses the documents, the content, the entry points and the order, reacts to what the application does, and may combine missions as long as each outcome is observed and recorded on its own. The variations given are examples.

## Missions

### OB-01 — Prepare a lesson

**Purpose:** build a document for tomorrow's lesson from existing material, and organise it among other documents.

**Prerequisites:** a test data directory with a few documents and folders; PDF files (one page and several pages), images of common formats, an OpenBoard document file.

**Coverage:** `DOC-*`, `IMPORT-*`.

**Variation:** start from an empty document or from an imported PDF; rename, move between folders, duplicate a document before changing it, reorder pages by dragging, delete a page and undo; sort the list differently.

### OB-02 — Teach with the board

**Purpose:** present, write and explain during a lesson, using the board's tools and content.

**Prerequisites:** documents with several pages, a PDF background, images, an application from the library, a sound and a video.

**Coverage:** `BOARD-*`, `TOOL-*`, `LIB-*`.

**Variation:** use a pen or touch as well as a mouse; switch between pen, highlighter and eraser often; write text and change its size; place, move, resize, rotate and group items; use rulers and compasses to draw; zoom in to show a detail; undo several steps; change the background between plain, ruled and grid, light and dark.

### OB-03 — Present on several screens

**Purpose:** show the board to the class on a projector while keeping control on another screen.

**Prerequisites:** two screens or more, or a computer where screens can be added and removed; display scaling other than 100 % if available.

**Coverage:** `SCREEN-*`.

**Variation:** start with one screen and connect a projector; switch between duplicated and extended modes; change the screen order in the preferences; remove a screen while a document is shown; restart with a different configuration.

### OB-04 — Use the desktop and the web

**Purpose:** annotate another application or a web page, and bring what is useful into the lesson.

**Prerequisites:** a desktop with another application open; access to a test web page.

**Coverage:** `DESKTOP-*`, `WEB-*`.

**Variation:** annotate over an application, capture part of the screen to the board, open a page in Web mode, capture it, return to the board and continue the lesson.

### OB-05 — Keep and share

**Purpose:** give the lesson to students or colleagues and find it again later.

**Prerequisites:** documents prepared in OB-01 and OB-02.

**Coverage:** `EXPORT-*`, `DOC-REOPEN`.

**Variation:** export to PDF and to an OpenBoard document, import that document on another computer or test directory, open the exported PDF in another reader, quit and restart before reopening.

### OB-06 — Keep working when the document location fails

**Purpose:** a lesson goes on when the location of the documents (a network share, a removable drive, a synchronised folder) disappears, does not answer or comes back, and nothing the user did is lost.

**Prerequisites:** a test data directory on a location whose availability the tester can change by disclosed, reversible means (see [FIXTURES.md](FIXTURES.md)); documents with images, a PDF and an application.

**Coverage:** `STORE-*`.

**Variation:** make the location unavailable before starting, while drawing, while opening a page, or while quitting; make a single file not answer; bring the location back during the session or only at the next start; change a document from another computer meanwhile; stop OpenBoard abruptly and restart.

### OB-07 — Set up OpenBoard for others

**Purpose:** the person who installs OpenBoard for a school or for themselves sets defaults, imposed values and shared content, and the user finds the expected setup.

**Prerequisites:** test copies of the administrator and system configuration files; an administrator library with some content; a second interface language.

**Coverage:** `SETUP-*`.

**Variation:** set a value as a default and as an imposed value; change the user's own preference; set a data directory that exists, that must be created, or that is not reachable; switch the interface language.

### OB-08 — Record a lesson

**Purpose:** record the board and the voice to share an explanation.

**Prerequisites:** a microphone or a virtual audio input.

**Coverage:** `REC-*`.

**Variation:** record while changing pages and modes; pause and resume; play the result in a standard player.

## Capability catalogue

Each row is accounted for in a full campaign, with one status (see [FRAMEWORK.md](FRAMEWORK.md#statuses)).

| ID | Mission | Capability and expected outcome |
| --- | --- | --- |
| DOC-CREATE | OB-01 | Create a document and a folder; they appear in the list with the expected name and are kept after a restart. |
| DOC-ORGANISE | OB-01 | Rename, move, duplicate and sort documents and folders; the list and the content of each document stay consistent. |
| DOC-PAGES | OB-01 | Add, duplicate, move and delete pages; thumbnails and page content follow, and undo restores what was removed. |
| DOC-TRASH | OB-01 | Delete a document or folder to the trash, restore it, then empty the trash; only what was chosen disappears. |
| DOC-REOPEN | OB-05 | After quitting and restarting, every document, page, item and its position and size are as left. |
| IMPORT-PDF | OB-01 | Import a PDF of one and of several pages; each page becomes a page with the PDF in the background, readable at every zoom level. |
| IMPORT-IMAGE | OB-01 | Import images of the supported formats into a document or a page; they keep their aspect and orientation. |
| IMPORT-DOCUMENT | OB-01 | Import an OpenBoard document file; it appears complete in the list, without replacing a document unexpectedly. |
| BOARD-WRITE | OB-02 | Write and draw with pen, highlighter and eraser, with the colours and widths offered; strokes follow the input without visible delay. |
| BOARD-TEXT | OB-02 | Add, edit and format text; it is kept after changing pages and after a restart. |
| BOARD-ITEMS | OB-02 | Select, move, resize, rotate, group, ungroup, duplicate and delete items; undo and redo restore each step. |
| BOARD-PAGES | OB-02 | Navigate between pages, add a page during the lesson; the content of each page is kept. |
| BOARD-BACKGROUND | OB-02 | Change the background (plain, ruled, grid, light, dark); writing stays readable and the choice is kept with the page. |
| BOARD-ZOOM | OB-02 | Zoom and move the view; content, including PDF backgrounds, stays sharp and in place. |
| TOOL-MEASURE | OB-02 | Use the ruler, triangle, protractor, compass and axes to draw and measure; the drawn result is kept. |
| TOOL-MASK | OB-02 | Hide and reveal parts of the page with the mask and curtain tools during a lesson. |
| LIB-CONTENT | OB-02 | Insert images, shapes, applications, sounds and videos from the library; they work on the page and are kept in the document. |
| LIB-USER | OB-02 | Add own content and favourites to the library and find them again after a restart. |
| SCREEN-EXTEND | OB-03 | With two screens, the board is shown full screen on the display screen and controlled from the other; writing appears on both. |
| SCREEN-CHANGE | OB-03 | Switching between duplicated and extended modes, adding or removing a screen: the views end on the right screens, visible and up to date, without restarting. |
| SCREEN-ORDER | OB-03 | Change the order of screens in the preferences; it is applied and kept for that screen configuration. |
| DESKTOP-ANNOTATE | OB-04 | Annotate over the desktop and capture an area to the board; the capture becomes content of the current document. |
| WEB-CAPTURE | OB-04 | Open a web page in Web mode, annotate it and capture it to the board. |
| EXPORT-PDF | OB-05 | Export a document to PDF; every page, with its background and content, is present and readable in another reader. |
| EXPORT-DOCUMENT | OB-05 | Export a document as an OpenBoard document file; importing it elsewhere gives the same document. |
| STORE-START | OB-06 | When the configured document location is not available or cannot be read at start, OpenBoard says so without blocking, waits for it or lets the user start locally, and never writes a substitute in its place. |
| STORE-LESSON | OB-06 | When the location fails during a lesson, the user can keep working; a discreet indication says the location is unavailable, and everything done meanwhile is written once it is back, which is also indicated. |
| STORE-HANG | OB-06 | When a file does not answer, the interface does not freeze beyond a short delay; the page opens without the missing content, which appears once it can be read. |
| STORE-INTEGRITY | OB-06 | Content that could not be read is never lost or damaged when the page is saved: images, PDFs and applications keep their references, size and position. |
| STORE-RECOVERY | OB-06 | After OpenBoard was stopped abruptly, or quit while the location was unavailable, the work is written at the next start; a document changed elsewhere meanwhile is kept, and the local work is saved as a separate copy. |
| STORE-VERSIONS | OB-06 | A document written by another version of OpenBoard is never damaged; what cannot be merged is kept as a separate copy. |
| SETUP-CONFIG | OB-07 | Values of the administrator file act as defaults, values of the system policy file are imposed, and the user's preferences apply in between; updates do not remove them. |
| SETUP-DATA | OB-07 | A configured data directory is used when reachable, created when missing on a reachable location, and never replaced silently by another one. |
| SETUP-LIBRARY | OB-07 | Content of the administrator library appears in the library for every user and cannot be deleted by them. |
| SETUP-LANGUAGE | OB-07 | The interface, dialogs and messages appear in the chosen language. |
| REC-RECORD | OB-08 | Record the board with sound across pages and modes; the result plays in a standard player. |
