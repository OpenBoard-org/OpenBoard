# Test data and conditions

Version: `1.0.0`

A campaign uses only test data, prepared for it, in test directories. It never uses, copies or derives data from a real user's documents, settings or library.

## Test copy of OpenBoard

- Run the candidate from a copy (or an installation) whose configuration file points to test directories: `App/DataDirectory` for documents and the user library, `App/RecoveryDirectory` for local copies kept while the document location is unavailable.
- Administrator and system configuration files used by OB-07 are test files, put in place for the test and removed or restored afterwards.
- Record the paths and the configuration values in the report, without secrets.

## Data families

| Family | Useful content |
| --- | --- |
| `documents` | A few documents in a few folders, with several pages each; one empty document; names with accents and spaces; one document with many pages. |
| `content` | Pages with strokes, text, images (PNG, JPEG, SVG), shapes, an application from the library, a sound and a video, items grouped, rotated and resized. |
| `pdf` | A PDF of one page and a PDF of several pages, created for the test; a document made by importing each. |
| `exchange` | An OpenBoard document file exported for the test, and documents written by another version of OpenBoard when available. |
| `library` | Own images and favourites in the user library; content in a test administrator library. |
| `screens` | The screen configurations available to the tester, described in the report. |
| `media` | A short sound and a short video of common formats; a microphone or a virtual audio input for OB-08. |

Prepare the data through the user interface or with documented scripts that create new files. Record how it was made. Content generated for the test (e.g. a PDF with a coloured rectangle and a sentence) is preferable to downloaded material, whose licence and content are unknown.

## Failure conditions

Missions on the document location (OB-06) need conditions the tester can create and undo. Choose the means for the platform and disclose them in the report:

| Condition | Examples of means |
| --- | --- |
| Location missing | disconnect a network drive or share, remove a drive created for the test, unmount a test volume |
| Location present but not readable or writable | remove read or write permission on a test folder or file |
| A file does not answer | hold the file open in a way that makes other readers wait (e.g. an opportunistic lock on Windows), or block a test share at network level |
| Location comes back | undo the condition while OpenBoard runs, or only before the next start |
| Changed elsewhere | modify, rename or delete a test document from another process or computer while OpenBoard keeps local work |
| Abrupt stop | end the OpenBoard process from the operating system |

A condition that fails at once (no permission) and one that does not answer (held file, unreachable server) behave differently; a full campaign covers both. Restore every condition at the end, and check that the test data is in the expected state before the next mission.

## Expected results

Write down what each mission should produce before running it, from the product rules and the prepared data: the pages and items a document must contain, the files that must exist. The screen is not its own reference. When the expected result is unclear, ask the maintainers.
