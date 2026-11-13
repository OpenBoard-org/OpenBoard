# Interactive acceptance framework

Version: `1.0.0`

## Purpose

This framework defines how a release candidate of a desktop application is accepted through its real user interface before it is released, by a human tester or by an AI agent controlling the desktop. It is not required for ordinary changes. It sets what must be observed and how it is reported. It does not prescribe tools, a click sequence, a duration or an order: the tester chooses the route, the content and further exploration.

The application profile ([CONTEXT.md](CONTEXT.md)) adopts a version of this framework and owns the scope; the catalogue ([MISSIONS.md](MISSIONS.md)) owns the capabilities and their expected outcomes.

Functional acceptance, automated checks and the decision to release are separate. Passing one does not satisfy the others, and acceptance does not grant permission to publish or release.

## Observing through the real interface

- A capability is accepted only when its outcome is observed in the running application, through its user interface: pen, touch, mouse or keyboard, as a user would.
- Reading code, running a script, calling a function or changing files directly does not show that a user can do something. Such checks may support an observation; report them separately, and never use them to work around a broken user task.
- Expected results come from the product rules and the prepared data, not from what the screen shows. When the intended behaviour is unclear, write the question down and ask the maintainers instead of inventing the answer.
- Missions describe purposes, not scripts. Vary the content, the entry points and the order across campaigns, and pay attention to what changed since the last one. Natural user behaviour is part of the test: mistakes, undo, interruptions, going back.
- Conditions are real: real timers, real files, real screens. A failure condition (a storage that does not answer, a screen removed) is created by a disclosed, reversible means, never by changing the application's state behind its back.

## Candidate identity

A campaign identifies exactly what was tested:

- source revision (full commit), build type and how it was built, versions of Qt and of the main libraries;
- operating system and version, session type (e.g. X11 or Wayland), screens (count, resolution, scaling), input devices, interface language;
- configuration used (configuration files, data and recovery directories), without secrets;
- versions of this framework, of the profile and of the catalogue;
- test data used (see [FIXTURES.md](FIXTURES.md)).

A commit hash alone is not enough: confirm that the application running is the build described.

## Statuses

Each capability of the catalogue gets one status per campaign:

| Status | Meaning |
| --- | --- |
| `PASS` | Its expected outcome was observed on the identified candidate, with a reference to the session and the evidence. |
| `FAIL` | The observed behaviour contradicts the expected outcome. The defect and its evidence are kept, even if a retry succeeds. |
| `BLOCKED` | A prerequisite, a missing device or platform, or a tool limitation prevented the observation. Say which, and what is needed. |
| `NOT_RUN` | Not exercised. |
| `NOT_APPLICABLE` | The capability is genuinely absent from the candidate or the platform, confirmed on the candidate and accepted by a maintainer. Missing data, devices or time are not non-applicability. |

A `PASS` needs an observation of that outcome, not a general statement that a session went well. Defects are listed separately from the statuses, so that a later `PASS` cannot hide an unresolved defect.

## Sessions and evidence

For each session, keep: the platform and conditions, the purpose, the actions actually taken, the outcomes observed and the evidence (screenshots of test data, saved files, logs). Keep enough detail to reproduce. Evidence contains only test data.

## Conclusion

A full acceptance recommendation needs every applicable capability in `PASS`, no unresolved blocking defect, and an explicit decision for every gap. Anything failed, blocked or not run prevents claiming full acceptance. A maintainer may accept a documented exception; the report keeps it visible, with the affected capabilities, the reason and the remaining risk. A tester never grants an exception on the maintainers' behalf.

State the exact scope of the conclusion: platforms, devices, configurations and combinations tested, and those not tested.

## Changes during a campaign

A change to the candidate during a campaign creates a new candidate. Recheck the capabilities it may affect; earlier results are reused only with a stated reason. Keep both the earlier records and the recheck.

## Changes to this framework

Version every change. The profile chooses when to adopt a new version. The catalogue changes with the product, through the normal review.
