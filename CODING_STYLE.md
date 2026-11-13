# OpenBoard coding style

## Principles

**Simple and readable.** Code reads close to plain language: a reader understands what it does without reconstructing it. Prefer the direct solution; no framework, template or abstraction until it is needed.

**SOLID.**

- **Single responsibility.** A class has one reason to change. Reading document files, rendering a PDF and showing a status to the user are three classes, not one.
- **Open for extension, closed for modification.** New behaviour is added by extending a class or adding one, not by special cases in code that already works.
- **Liskov substitution.** A derived class can be used wherever its base class is, without surprise: it keeps the base contract and the visibility of the base functions.
- **Interface segregation.** A class exposes what its users need and nothing more; a caller does not depend on functions it does not use.
- **Dependency inversion.** Low-level modules (`src/pdf`, `src/frameworks`) do not depend on application classes. The caller gives them what they need.

**Architecture first.** These principles shape the design of a feature before it is coded: what its responsibilities are, which classes carry them, how they depend on each other and on the existing code. A design that respects them usually needs no special case later.

## Files

- License header: `Copyright (C) 2015-<current year> Département de l'Instruction Publique (DIP-SEM)`, followed by the GPL text.
- Headers use `#pragma once`.
- Each file includes what it uses.
- New files are added to both the qmake `.pri` file and the `CMakeLists.txt` of their directory.
- Platform-specific code lives in `UBPlatformUtils` and its platform files (`_win.cpp`, `_linux.cpp`, `_mac.mm`), not in feature code.

## Classes and ownership

- Each class has a short `/** @brief ... */` block saying what it is responsible for.
- A class is a `QObject` only when it needs signals, slots or parent ownership.
- `std::shared_ptr` for documents, document proxies and scenes, `std::weak_ptr` to refer to them without keeping them alive, `std::unique_ptr` for other owned members, Qt parent ownership for widgets and other `QObject` children.
- Virtual functions meant to be overridden are `protected`.
- Helpers used in one file are `static` functions in that file, or private members of the class.

## Naming

- Names are natural: they say what a thing is for the people who use the code and the product, in their words, not how it works inside. A name built from the implementation becomes wrong when the implementation changes.
- Classes `UBSomething`, members `mSomething`, static members `sSomething`; functions and variables in camelCase.
- Full words; abbreviations only when they are common knowledge (`Pdf`, `Url`, `Toc`).
- Getters without `get`: `pageCount()`, `isLoaded()`, `setPixmap()`. `get...` is for a lookup that may create or load something.
- Booleans are positive: `isAvailable()`, not `isUnavailable()`.
- A name says what the thing is or does. A function that saves, waits or shows something is not named like a question.
- Signals say what happened (`documentChanged`); a slot reacting to an event is named after it (`onDocumentChanged`).

## C++ and Qt

- `nullptr`, `override`, `enum class`, range-for.
- Members are initialised where they are declared: `bool mLoaded{false};`.
- `auto` only when the type is obvious.
- `connect` with the function-pointer syntax. Lambdas capture explicitly and stay short; longer logic goes into a named function.
- Strings shown to the user go through `tr()` with `%1` placeholders.
- Versions of Qt and of libraries are handled with version checks at the call, so that OpenBoard builds on all supported distributions.

## Layout

- Four spaces, no tabs, no trailing whitespace.
- Braces always on their own line, also around a single statement: the scope of each block is visible at a glance.

```cpp
if (!document)
{
    return false;
}
```

- `*` and `&` next to the type: `const QString& name`.

## Comments

- In English, short, saying why rather than what.
- They describe the code as it is; the history of a change belongs in its commit message.
- No commented-out code.

## Errors and logging

- No exceptions. A function reports failure with a `bool`, an empty value or `nullptr`, and returns early.
- `qWarning()` for anomalies, `qCritical()` for failures that lose data or stop a feature, `qInfo()` for what must appear in a user's log.

## Commits

- Conventional Commits:
  - `fix:` and `feat:` for what changes for the user, `perf:` for speed and memory;
  - `refactor:` for a change of the structure of the code without change of behaviour (moving a responsibility, extracting a class, renaming a concept);
  - `cleanup:` for tidying the code without changing its structure or behaviour: formatting, comments, removing dead code, bringing existing code to these conventions;
  - `chore:` for the maintenance of the project: build system, dependencies, repository files; `build:` for build fixes, `docs:` for documentation, `i18n:` for translations;
  - `workaround:` for a problem outside OpenBoard, with its reference.
- `refactor:` and `cleanup:` commits are never mixed with a fix or a feature.
- Subject in lower case, imperative and short, so that the history reads as a list.
- Body after a blank line: the full explanation, as long as needed. Why the change is needed, what it does and how, in sentences or `- ` bullet lines wrapped at about 70 characters. The history is read years later by someone who has only the commit. `Fixes #NNN` when it closes an issue.
- One intent per commit, and each commit builds.
