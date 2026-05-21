# Style & Git

## Response Rules (verified each output)
1. Language: Russian ONLY — even for technical queries. English ONLY if user writes in English.
2. Emoji: ZERO — never in text, code, or placeholders. No exceptions.
3. Summary tail: NEVER append "what I did" at end of response.
4. Explanation: max 5 sentences. Need more -> bulleted list.
5. Scope: NEVER edit outside stated task и NEVER добавлять сверх запрошенного ([YAGNI](software-laws.md#yagni))
6. Comments: NEVER add comments to lines you did not change.
7. [KISS](software-laws.md#kiss-principle)
8. [Least Astonishment](software-laws.md#principle-of-least-astonishment)

## Russian Language Rules
- Russian for all prose, explanations, reasoning
- Technical terms in original form: API, SDK, LSP, MCP, PR, CI, regex, refactor, etc.
- Code identifiers, filenames, CLI commands: NEVER translate or transliterate
- Abbreviations: keep original (TDD, BDD, SOLID, DRY, YAGNI)
- Law references: English anchor names, Russian descriptions where needed

## Artifact Policy
- Artifacts (code, configs, PRDs, plans, specs, YAML, JSON, markdown) — write to FILES
- NEVER inline artifacts in conversation output
- Return only: file path + 1-line description
- Exception: short snippets (<5 lines) for inline clarification are acceptable
- File creation: use Write tool; file modification: use Edit tool
- NEVER use ctx_execute, ctx_execute_file, or Bash for file creation/modification

## Response Format
- Concise summary structure:
  - Actions taken (2-3 bullets)
  - File paths created/modified
  - Key findings
- Technical substance: exact and terse. Fragments when clear.
- Short synonyms: fix not "implement a solution for"
- Auto-expand only for: security warnings, irreversible actions, user confusion

## Git Commits
- First line: <=50 chars, conventional prefix (fix:/feat:/refactor:/docs:/test:/chore:), imperative mood
- Body: explain WHY not WHAT
- Prefer new commit over amend
- NEVER force-push to main/master

## [Boy Scout Rule](software-laws.md#boy-scout-rule)

## Notepad
Write when: choice from 2+ approaches, trade-off accepted, workaround applied.

## [Technical Debt](software-laws.md#technical-debt)
