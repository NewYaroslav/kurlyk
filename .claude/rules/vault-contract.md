# Vault Contract — Obsidian Knowledge Base Rules

> Applies to: Claude Code sessions working with notes in this Obsidian vault.
> General agent behavior (L0 meta-rules, L2 output templates, provenance, git): see [AGENTS.md](../../AGENTS.md).

## Purpose

The agent helps maintain the vault in a state suitable for long-term use:

- transforms external sources into structured notes;
- updates existing notes without losing context;
- collects conclusions, playbooks, and working workflows;
- preserves information provenance and confidence levels.

### Main Risk

Accumulating many sources, transcripts, and links, but few processed knowledge artifacts.

Bad: many resources + many transcripts + few concepts/playbooks/workflows.
Good: every important source gradually becomes working notes (concept, playbook, tool).

The agent must not only store sources but also ensure derivative artifacts appear. If an Ingest Agent created a resource breakdown but no playbook, concept, or update followed -- that is a signal to act, not the norm.

## Structure and Placement

When creating a new note, the agent selects a directory by `type`:

- `concept` -> `concepts`
- `agent` -> `agents`
- `automation` -> `automation`
- `programming` -> `programming`
- `content` -> `content`
- `tool` -> `tools`
- `experiment` -> `experiments`
- `playbook` -> `playbooks`
- `resource` -> `resources`

Before creating a new note the agent must:

- check whether a note on the same topic already exists;
- prefer updating an existing note over creating a duplicate;
- create a new note only if the topic is genuinely new or the current note is overloaded and requires deliberate splitting.

## Folder Organization

- Do not group notes by tool (claude/, chatgpt/, gemini/). Group by type and workflow. Variants for different tools go inside the note, not in separate folders.
- Keep structure flat while a folder has fewer than 20-30 notes.
- Create subfolders only for stable recurring patterns. Do not create a subfolder for one or two notes.
- Maximum nesting depth: 1-2 levels from the root type folder.
- If notes in a folder are few or directions have not stabilized yet -- keep the structure flat and move thematic navigation into `Navigator.md`. This provides topic routes without premature fragmentation.

## Atomicity Principle

One note captures one working thought, one concept, one tool, one source, or one reproducible process.

Good:

- "Triple-layer memory for AI agents" -- one memory architecture concept
- "AI-coding workflow" -- one reproducible process

Bad:

- "Everything about RAG" -- a mix of RAG types, specific tools, and workflows in one note
- "AI agents" -- too broad, does not provide a quick answer

If a note is overloaded -- deliberately split into atomic parts, each answering one question.

## Note Naming

Format: `<action> <object> <context>.md`

Examples:

- "Configure MCP for Claude Desktop.md"
- "Process YouTube video into Obsidian.md"
- "Triple-layer memory for AI agents.md"

For resource breakdowns: `<Title> - transcript.md` or `<Title> - article analysis.md`

## Required Frontmatter

For all new and substantially updated notes use at least this YAML:

```yaml
---
type: concept|agent|automation|programming|content|tool|experiment|playbook|resource
status: inbox|draft|reviewed|evergreen
source_type: youtube|article|docs|pdf|podcast|social|course|manual
sources:
  - <source-1>
  - <source-2>
created: YYYY-MM-DD
updated: YYYY-MM-DD
aliases: []
tags: []
---
```

Filling rules:

- `created` -- date of first note creation.
- `updated` -- date of last substantial update.
- `sources` -- list of links, identifiers, or short source entries, if a source exists.
- For multiple sources use only block YAML list, one scalar value per line. Do not write `sources` as inline array (`["url", ...]`) and do not put JSON/objects like `{"transcript":"auto"}` there: Obsidian shows this as one long string. Technical source details go into `Sources` or `Metadata` section.
- If there are no sources, `sources: []` is acceptable.
- `source_type: manual` -- use for manual notes, synthesis notes, and internal drafts without an external primary source.
- `aliases` -- alternative names, abbreviations, and spelling variants for search and wikilinks.

## Tags and Semantic Links

Tags (`tags` in frontmatter) -- only for status, technical, and source markers:

- source type: `youtube`, `article`, `pdf`
- technical marker: `wip`, `needs-review`, `mcp`
- format: `transcript`, `profile`

Tags are NOT used for thematic categorization (no `rag`, no `ai-agent`, no `memory`). For that -- use `[[wikilinks]]` in the `## Related Topics` section.

Semantic links between notes are established via `## Related Topics` with wikilinks:

- `[[Triple-layer memory for AI agents - Karpati method]]`
- `[[RAG system from architecture to production]]`

Existing notes with thematic tags do not require urgent migration. Maintenance Agent normalizes them gradually during updates.

## Status Lifecycle

- `inbox` -- raw material, just created. Transition to `draft`: all required sections and provenance are filled.
- `draft` -- structured note with sources, but not checked for completeness and links. Transition to `reviewed`: `Related Topics` links added, sources verified, trivial open questions closed.
- `reviewed` -- note passed review, links established, facts verified. Transition to `evergreen`: relevant, linked, regularly used, no open questions.
- `evergreen` -- stable note, maintained in current state. Updated when new data appears, status is never downgraded.

Transitions: `inbox` -> `draft` -> `reviewed` -> `evergreen`. Status downgrade is not allowed -- if information becomes outdated, add a note in `Open Questions`.

## Updating Existing Notes

When updating an existing note the agent must:

- preserve useful previously accumulated context;
- not delete old sources if they are still relevant;
- add new sources and update `updated`;
- restructure only if it genuinely improves readability;
- leave controversial points in `Open Questions`, not resolve them by fabrication.

If a note already contains authorial conclusions, the agent:

- does not delete them without explicit instruction;
- if necessary, rephrases only while preserving meaning;
- adds new interpretations separately and transparently.

## Knowledge Navigator

The central vault navigator is at [Navigator.md](../Navigator.md).

The agent updates the navigator only when creating new full-fledged material:

- `playbook`;
- `concept`;
- `tool`;
- `agent`;
- `experiment`;
- important `resource`, if it has standalone value rather than being just provenance.

Do NOT update the navigator when the agent:

- only patches an existing note;
- creates a raw video transcript;
- creates a regular article analysis without a derivative note;
- updates a channel profile;
- works with assets.

Update rules:

1. Find 1-3 relevant sections in `Navigator.md`.
2. Add a wikilink to the new note with a short explanation if it improves the route.
3. If the section is not obvious -- add the link to `New materials to distribute`.
4. Update `updated` in the navigator frontmatter.
5. Do not regenerate the navigator entirely.

## Note Template

The base template is at [templates/Base Note.md](../templates/Base%20Note.md).

Preferred body structure:

1. `Briefly`
2. `Key Ideas`
3. `Practice / workflow`
4. `Sources`
5. `Open Questions`
6. `Related Topics`

The agent may add additional sections only if it makes the note clearer and does not break the base structure.

## Source Handling Rules

### YouTube

For video notes the agent should preserve when possible:

- link to the video;
- video title;
- channel or author;
- publication date, if available;
- timestamps, chapters, or reference moments;
- a note whether official transcript, auto-subtitles, or manual summary was used.

If the transcript is incomplete or noisy, this must be explicitly stated.

### Articles and Documentation

For `article` and `docs` the agent must preserve:

- canonical URL;
- material title;
- access date;
- brief context description if the source is updatable or strongly version-dependent.

### PDF, Podcasts, Social Media, Courses

For `pdf`, `podcast`, `social`, `course` the agent preserves the most stable source identification:

- link or ID;
- title;
- author or platform, if available;
- access context or restrictions if the material is incomplete;
- explicit mark if conclusions were made from a fragment rather than the full source.

### Source as Raw Material

A source must produce a reusable artifact. Every resource breakdown must contain:

- **What this is** -- brief source description.
- **Why it matters** -- why this source is needed for the vault.
- **Key ideas** -- extracted theses.
- **What can be applied** -- concrete reproducible conclusions, practices, or workflows.

If nothing practical can be extracted from a source -- explicitly state this in `Open Questions`.

## Agent Roles

### Ingest Agent

Task: transform an external source into a vault-ready note or note update.

Workflow:

1. Determine source type and topic.
2. Find an existing note on the topic.
3. If a note exists, update it incrementally.
4. If no note exists, create a new one in the correct directory.
5. Fill frontmatter and `Sources` section.
6. Clearly separate source summary from practical conclusions.
7. Mark gaps and uncertain points in `Open Questions`.
8. If a new full-fledged material was created, update `Navigator.md`.

For YouTube videos: use the `/video <URL>` command (see `.claude/commands/video.md`), which automates transcript extraction, key frame extraction, interactive planning, and artifact creation. Transcript template: `templates/Video Source.md`, storage: `resources/videos/`.

For web articles and sites: use the `/article <URL>` command (see `.claude/commands/article.md`), which automates content extraction, meaningful illustration downloading, interactive planning, and artifact creation. Analysis template: `templates/Article Source.md`, storage: `resources/`.

### Synthesis Agent

Task: assemble stable conclusions, playbooks, and structured summaries from multiple notes.

Workflow rules:

- use only existing vault materials and explicitly provided new sources;
- do not lose links to source notes and primary sources;
- when sources conflict, document the disagreement rather than silently choosing;
- transform collections into more useful structures without erasing provenance.

### Maintenance Agent

Task: maintain vault quality without changing note meaning.

Allowed:

- normalize frontmatter;
- update `updated` on substantive edits;
- fix structural problems;
- improve readability of headings and sections;
- remove duplicates only if all useful context is preserved.

Forbidden:

- change meaning without source support;
- delete sources for "cleanliness";
- rewrite notes so that authorial voice or thought history is lost.

## Quality Criteria

A good note in this vault:

- written primarily in Russian;
- has a clear `type`, `status`, and `source_type`;
- provides a quick answer about what the material is and why it matters;
- contains verifiable sources or an honest mark of their absence;
- separates facts, summary, and interpretation;
- can be continued by another agent without losing context.
