# Quality and testing

How OpenBoard is tested, by people or by AI agents. The documents describe what must be observed and how to report it; the tester chooses how to get there.

## During development

A change is checked in proportion to what it touches: it builds, and its scenario is tried in the real application when its behaviour is visible, with the conditions that matter for it (for example a document location that fails, a second screen, a PDF document). The author says what was tried, on which platform, and what was not.

The missions of [MISSIONS.md](MISSIONS.md) can help to choose what to try around a change. Running them is not required for a commit or a pull request.

## Before a release

Interactive acceptance campaigns are run on a release candidate, when the maintainers consider publishing it. They are long and, run by an AI agent, costly: they are a release step, not a step of every change.

A full campaign accounts for every capability of the [catalogue](MISSIONS.md#capability-catalogue), following the [framework](FRAMEWORK.md), and is reported with the [report template](templates/report.md).

Anyone may use these documents to test a candidate, a nightly build or their own setting, in full or in part. Defects and regressions found this way are welcome as issues, with the conditions in which they were observed.

## Documents

| Document | Content |
| --- | --- |
| [FRAMEWORK.md](FRAMEWORK.md) | The rules of an acceptance campaign: what counts as an observation, the statuses, the candidate identity, the conclusion. Independent of OpenBoard's features. |
| [CONTEXT.md](CONTEXT.md) | OpenBoard's profile: users and installations, scope, platforms and conditions, test environment. |
| [MISSIONS.md](MISSIONS.md) | The interactive journeys (user purposes) and the catalogue of capabilities with their expected outcomes. |
| [FIXTURES.md](FIXTURES.md) | The test data and conditions a campaign needs, and how to prepare them without touching real data. |
| [templates/report.md](templates/report.md) | The structure of a campaign report. |

## Maintenance

When a feature is added, changed or removed, update its capabilities and missions in the same pull request. Version changes to these documents explicitly (each one carries a version); a campaign records the versions it used.

Automated checks (unit tests, scripted runs without a screen) are separate from interactive campaigns: they do not replace an observation in the real interface, and an acceptance report lists them in their own section.
