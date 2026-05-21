# Mode Triggers & External Workers

## Precedence
delegation.md routing > mode-triggers keyword routing.
When both match, delegation.md assignment wins.

## Keyword Routing
- "analyze"/"debug" -> debugger (root-cause tracing)
- "tdd" -> test-engineer
- "review code" -> code-reviewer
- "security review" -> security-reviewer

## Skill Triggers (keyword -> /oh-my-claudecode:* skill)

See [delegation.md](delegation.md) for full agent catalog and routing.

### Execution, Planning & Analysis

| Keyword/Pattern | Skill |
|-----------------|-------|
| "autopilot" | /oh-my-claudecode:autopilot |
| "ralph" | /oh-my-claudecode:ralph |
| "ulw" / "ultrawork" | /oh-my-claudecode:ultrawork |
| "ccg" | /oh-my-claudecode:ccg |
| "cancelomc" | /oh-my-claudecode:cancel |
| "ralplan" / "ral plan" | /oh-my-claudecode:ralplan |
| "deep interview" | /oh-my-claudecode:deep-interview |
| "omc-plan" | /oh-my-claudecode:plan |
| "team" | /oh-my-claudecode:team |
| "deepinit" | /oh-my-claudecode:deepinit |
| "ultrathink" | deep reasoning -> architect |
| "project-session-manager" | /oh-my-claudecode:project-session-manager |
| "deep-analyze" | analysis mode -> analyst |
| "deepsearch" | codebase search -> explore |
| "sciomc" | /oh-my-claudecode:sciomc |
| "trace" / "evidence trace" | /oh-my-claudecode:trace |
| "autoresearch" | /oh-my-claudecode:autoresearch |
| "external-context" | /oh-my-claudecode:external-context |

### Quality, Tools & Configuration

| Keyword/Pattern | Skill |
|-----------------|-------|
| "deslop" / "anti-slop" / cleanup+slop-smell | /oh-my-claudecode:ai-slop-cleaner |
| "ultraqa" / "qa cycle" | /oh-my-claudecode:ultraqa |
| "verify" | /oh-my-claudecode:verify |
| "visual-verdict" | /oh-my-claudecode:visual-verdict |
| "self-improve" | /oh-my-claudecode:self-improve |
| "tdd" | TDD mode -> test-engineer |
| "release" | /oh-my-claudecode:release |
| "omc-doctor" | /oh-my-claudecode:omc-doctor |
| "mcp-setup" | /oh-my-claudecode:mcp-setup |
| "omc-setup" | /oh-my-claudecode:omc-setup |
| "configure-notifications" | /oh-my-claudecode:configure-notifications |
| "hud" | /oh-my-claudecode:hud |
| "ask-codex" | `omc ask codex` |
| "ask-gemini" | `omc ask gemini` |
| "learner" | /oh-my-claudecode:learner |
| "omc-help" | /oh-my-claudecode:omc-help |
| "skill" | /oh-my-claudecode:skill |
| "ralph-init" | /oh-my-claudecode:ralph-init |
| "deep-dive" | /oh-my-claudecode:deep-dive |
| "writer-memory" | /oh-my-claudecode:writer-memory |
| "note" | /oh-my-claudecode:note |
| "learn-about-omc" | /oh-my-claudecode:learn-about-omc |

## External Workers
| Command | Provider | Use Case |
|---------|----------|----------|
| omc team N:codex "task" | Codex | Analysis, review, architecture, critique |
| omc team N:gemini "task" | Gemini | Design, documentation, multi-modal review |
| omc ask codex "question" | Codex | Quick second opinion, alternative approach |
| omc ask gemini "question" | Gemini | Design feedback, visual analysis |
| /ccg | Claude+Codex+Gemini | Tri-model orchestration |

Per-role /team routing: configure provider/model per canonical role in `.claude/omc.jsonc` under `team.roleRouting`. Accepted aliases (e.g., `reviewer`) normalized at runtime.

## Metcalfe's Law

Ценность pipeline ~ n^2 связей. Новая интеграция валидна только при наличии всех трёх: trigger + routing + fallback. Нет любого — интеграция не связь, а шум. Проверка: grep три компонента перед добавлением нового триггера.
