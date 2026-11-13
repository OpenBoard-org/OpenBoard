# Working on OpenBoard

This is the entry point for anyone changing OpenBoard, people and AI agents alike. Read it before your first change, then the documents it links to when your task concerns them:

- [CODING_STYLE.md](CODING_STYLE.md): how the code is written.
- [docs/quality/README.md](docs/quality/README.md): how changes are checked, and how release candidates are tested.

## Who uses OpenBoard

OpenBoard is an open source interactive whiteboard, mainly used by teachers and trainers in front of a class, with a pen, a touch screen or a mouse. It runs on personal computers as well as on shared computers of schools and organisations, with one or more screens or projectors, and documents stored locally, on a network share or in a synchronised folder. A change must work for all of them, not only for the setting of its author. Every change is judged from the point of view of the person in front of the class.

## What matters most, in this order

1. **Never lose data or information.** This covers what the user wrote, drew and imported, and everything a page refers to: images, PDFs, applications, sounds, videos, their size and position, the pages of a document and the folders of the user. Only write what was read completely or what the user changed. Content which could not be read is never saved in a degraded form.
2. **Never block the user.** A lesson cannot wait. No freeze while a file is read or written, no modal dialog unless there is no other way, and the user can always continue working.
3. **Keep the experience simple.** No new step, choice or message unless the user needs it. Information is discreet, short, in the user's language and in words a teacher uses.
4. **Keep documents compatible.** Documents are shared between computers, users and OpenBoard versions. A version never damages a document written by another one, older or newer.

## How to change the code

- **Design before coding a feature.** A new feature starts with its architecture: which responsibilities it brings, which classes carry them, what they are called, how they depend on each other and on the existing code. The principles of [CODING_STYLE.md](CODING_STYLE.md#principles) guide that design; they are not a checklist applied afterwards. Describe the design in the pull request.
- **Fix the cause, not the symptom.** A guard, a forced refresh or a special case that hides a problem is not a fix. When the cause cannot be fixed in the scope of the change, say so and document the limitation.
- **Simple and readable.** Follow [CODING_STYLE.md](CODING_STYLE.md). A reader must not be able to tell who wrote which part.
- **When a fix calls for a redesign.** Sometimes the best fix of a problem would mean reworking the architecture around it. Then separate the two:
  1. Fix the problem now with the smallest change that removes its cause in the current design. If no such change exists without hiding the problem, stop and discuss it with the maintainers first.
  2. Describe the redesign in an issue: what is wrong in the current design, what the better one is, what it would touch and what it would make possible.
  3. Do the redesign as work of its own, in its own pull request, once the maintainers agree, usually for a feature release rather than a patch release.
- **Keep the scope.** Follow-up work is written down, not slipped in. A patch release does not change the document format.
- **A change shows what it changes.** The lines written by a fix or a feature follow [CODING_STYLE.md](CODING_STYLE.md), but the code around them is not reformatted in the same commit: reformatting or bringing existing code to the conventions is a `cleanup:` commit of its own, restructuring it a `refactor:` commit, so that the real change is not lost among unrelated lines.
- **All platforms.** OpenBoard runs on Windows, Linux (X11 and Wayland) and macOS, with the Qt and library versions of their distributions. Platform-specific code stays in the platform files; a change made on one platform says what was not checked on the others.

## Data and safety

- Never use the real documents or settings of the person running OpenBoard for tests. Use a test copy whose configuration points to test directories (see [docs/quality/CONTEXT.md](docs/quality/CONTEXT.md)).
- Nothing is pushed, merged, published or posted without the maintainers' review.
- Commit messages, comments, pull requests and reports contain only information that can be published.

## Commits

- One commit per intent; each commit builds on its own. The message format is in [CODING_STYLE.md](CODING_STYLE.md#commits).
- Translations: run `lupdate` when user-visible strings change, in a separate `i18n:` commit.

## Before saying a change is done

- It builds, and its visible behaviour was tried in the real application, in proportion to what it touches ([docs/quality/README.md](docs/quality/README.md#during-development)). The full interactive campaigns are a release step, not a requirement for a change.
- What was tried and what was not is stated plainly, with the platforms and conditions used.
