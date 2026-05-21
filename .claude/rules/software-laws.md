# Software Engineering Laws — Pipeline Rules

> **Canonical Source**: [Laws of Software Engineering](https://lawsofsoftwareengineering.com/) by Dr. Milan Milanovic. All 56 laws below are indexed from this source with OMC-specific application guidance. OMC extensions (6 additional laws) are marked with (OMC).

SINGLE SOURCE OF TRUTH. No other file should repeat law definitions — only reference `[Law Name](software-laws.md#anchor)`.

## Summary Tables

### Architecture

| # | Law | Anchor |
|---|-----|--------|
| 1 | Conway's Law | [conways-law](#conways-law) |
| 2 | Hyrum's Law | [hyrums-law](#hyrums-law) |
| 3 | Gall's Law | [galls-law](#galls-law) |
| 4 | Law of Leaky Abstractions | [law-of-leaky-abstractions](#law-of-leaky-abstractions) |
| 5 | Tesler's Law | [teslers-law](#teslers-law) |
| 6 | CAP Theorem | [cap-theorem](#cap-theorem) |
| 7 | Second-System Effect | [second-system-effect](#second-system-effect) |
| 8 | Fallacies of Distributed Computing | [fallacies-of-distributed-computing](#fallacies-of-distributed-computing) |
| 9 | Law of Unintended Consequences | [law-of-unintended-consequences](#law-of-unintended-consequences) |
| 10 | Zawinski's Law | [zawinskis-law](#zawinskis-law) |

### Teams

| # | Law | Anchor |
|---|-----|--------|
| 11 | Brooks's Law | [brooks-law](#brooks-law) |
| 12 | Dunbar's Number | [dunbars-number](#dunbars-number) |
| 13 | The Ringelmann Effect | [ringelmann-effect](#ringelmann-effect) |
| 14 | Price's Law | [prices-law](#prices-law) |
| 15 | Putt's Law | [putts-law](#putts-law) |
| 16 | Peter Principle | [peter-principle](#peter-principle) |
| 17 | Bus Factor | [bus-factor](#bus-factor) |
| 18 | Dilbert Principle | [dilbert-principle](#dilbert-principle) |

### Planning

| # | Law | Anchor |
|---|-----|--------|
| 19 | Premature Optimization | [premature-optimization](#premature-optimization) |
| 20 | Parkinson's Law | [parkinsons-law](#parkinsons-law) |
| 21 | The Ninety-Ninety Rule | [ninety-ninety-rule](#ninety-ninety-rule) |
| 22 | Hofstadter's Law | [hofstadters-law](#hofstadters-law) |
| 23 | Goodhart's Law | [goodharts-law](#goodharts-law) |
| 24 | Gilb's Law | [gilbs-law](#gilbs-law) |

### Quality

| # | Law | Anchor |
|---|-----|--------|
| 25 | The Boy Scout Rule | [boy-scout-rule](#boy-scout-rule) |
| 26 | Murphy's Law | [murphys-law](#murphys-law) |
| 27 | Postel's Law | [postels-law](#postels-law) |
| 28 | Broken Windows Theory | [broken-windows-theory](#broken-windows-theory) |
| 29 | Technical Debt | [technical-debt](#technical-debt) |
| 30 | Linus's Law | [linuss-law](#linuss-law) |
| 31 | Kernighan's Law | [kernighans-law](#kernighans-law) |
| 32 | Testing Pyramid | [testing-pyramid](#testing-pyramid) |
| 33 | Pesticide Paradox | [pesticide-paradox](#pesticide-paradox) |
| 34 | Lehman's Laws | [lehmans-laws](#lehmans-laws) |
| 35 | Sturgeon's Law | [sturgeons-law](#sturgeons-law) |

### Scale

| # | Law | Anchor |
|---|-----|--------|
| 36 | Amdahl's Law | [amdahls-law](#amdahls-law) |
| 37 | Gustafson's Law | [gustafsons-law](#gustafsons-law) |
| 38 | Metcalfe's Law | [metcalfes-law](#metcalfes-law) |

### Design

| # | Law | Anchor |
|---|-----|--------|
| 39 | YAGNI | [yagni](#yagni) |
| 40 | DRY Principle | [dry-principle](#dry-principle) |
| 41 | KISS Principle | [kiss-principle](#kiss-principle) |
| 42 | SOLID Principles | [solid-principles](#solid-principles) |
| 43 | Law of Demeter | [law-of-demeter](#law-of-demeter) |
| 44 | Principle of Least Astonishment | [principle-of-least-astonishment](#principle-of-least-astonishment) |
| 45 | Rule of Three (OMC) | [rule-of-three](#rule-of-three) |
| 46 | Miller's Law (OMC) | [millers-law](#millers-law) |
| 47 | Unix Philosophy (OMC) | [unix-philosophy](#unix-philosophy) |
| 48 | Least Privilege (OMC) | [least-privilege](#least-privilege) |
| 49 | Worse Is Better (OMC) | [worse-is-better](#worse-is-better) |

### Decisions

| # | Law | Anchor |
|---|-----|--------|
| 50 | Dunning-Kruger Effect | [dunning-kruger-effect](#dunning-kruger-effect) |
| 51 | Hanlon's Razor | [hanlons-razor](#hanlons-razor) |
| 52 | Occam's Razor | [occams-razor](#occams-razor) |
| 53 | Sunk Cost Fallacy | [sunk-cost-fallacy](#sunk-cost-fallacy) |
| 54 | The Map Is Not the Territory | [map-is-not-the-territory](#map-is-not-the-territory) |
| 55 | Confirmation Bias | [confirmation-bias](#confirmation-bias) |
| 56 | The Hype Cycle & Amara's Law | [hype-cycle-amaras-law](#hype-cycle-amaras-law) |
| 57 | The Lindy Effect | [lindy-effect](#lindy-effect) |
| 58 | First Principles Thinking | [first-principles-thinking](#first-principles-thinking) |
| 59 | Inversion | [inversion](#inversion) |
| 60 | Pareto Principle | [pareto-principle](#pareto-principle) |
| 61 | Cunningham's Law | [cunninghams-law](#cunninghams-law) |
| 62 | Chesterton's Fence (OMC) | [chestertons-fence](#chestertons-fence) |

## Conflict Priority

1. YAGNI > Metcalfe (не добавлять без текущей потребности)
2. Dunbar > каталог (max 5 активных агентов одновременно)
3. Gilb: стилистические правила (output-style.md) верифицируются пользователем
4. Lindy > Hype Cycle (проверенное > хайп)
5. Inversion > Confirmation Bias (искать поломки > подтверждать работу)
6. KISS > SOLID (простое решение > правильная абстракция, если конфликтуют)
7. Occam > Dunning-Kruger (простое объяснение > уверенное сложное)
8. Worse Is Better > Premature Optimization (работающий простой > сломанный оптимизированный)
9. Chesterton's Fence > Broken Windows (не удалять без понимания причины > убирать мусор)
10. Pareto > Amdahl (20% усилий > полная параллелизация, если конфликтуют)

## Architecture

Laws governing system structure and how organizational constraints shape technical boundaries.

### Conway's Law {#conways-law}

**Organizations design systems that mirror their own communication structure.**

Агент-исполнитель не планирует, агент-архитектор не пишет код. Не пересекать границы ответственности. Inverse Conway Maneuver: формировать команды под желаемую архитектуру pipeline, не наоборот.

*Cross-refs*: [Brooks's Law](#brooks-law), [Dunbar's Number](#dunbars-number), [Law of Demeter](#law-of-demeter)

### Hyrum's Law {#hyrums-law}

**With a sufficient number of API users, all observable behaviors of your system will be depended on by somebody.**

При достаточном числе пользователей API все наблюдаемые поведения становятся зависимостями. Документируй побочные эффекты в notepad. Фактический контракт pipeline — наблюдаемое поведение, а не документация.

*Cross-refs*: [Postel's Law](#postels-law), [Law of Unintended Consequences](#law-of-unintended-consequences), [Gilb's Law](#gilbs-law)

### Gall's Law {#galls-law}

**A complex system that works is invariably found to have evolved from a simple system that worked.**

Новое расширение OMC начинается с минимальной версии (1 trigger + 1 action). Минимальная версия демонстрируема за 1 итерацию.

*Cross-refs*: [KISS Principle](#kiss-principle), [Second-System Effect](#second-system-effect), [Worse Is Better](#worse-is-better)

### Law of Leaky Abstractions {#law-of-leaky-abstractions}

**All non-trivial abstractions, to some degree, are leaky.**

Все нетривиальные абстракции протекают. При ошибке sandbox/FTS5 — декомпозировать до Bash причины, не скрывать.

*Cross-refs*: [Tesler's Law](#teslers-law), [Fallacies of Distributed Computing](#fallacies-of-distributed-computing), [Chesterton's Fence](#chestertons-fence)

### Tesler's Law {#teslers-law}

**Every application has an inherent amount of irreducible complexity that can only be shifted, not eliminated.**

Сложность не исчезает — перемещается. Не перекладывать на агента без эквивалентной capability.

*Cross-refs*: [Law of Leaky Abstractions](#law-of-leaky-abstractions), [KISS Principle](#kiss-principle), [SOLID Principles](#solid-principles)

### CAP Theorem {#cap-theorem}

**A distributed system can guarantee only two of: consistency, availability, and partition tolerance.**

Перед write в shared_memory — read текущего состояния. Read-fail — эскалировать, не писать. Не работать на stale data.

*Cross-refs*: [Fallacies of Distributed Computing](#fallacies-of-distributed-computing), [Murphy's Law](#murphys-law)

### Second-System Effect {#second-system-effect}

**Small, successful systems tend to be followed by overengineered, bloated replacements.**

Второй pass склонен к over-engineering: verifier проверяет, не переписывает.

*Cross-refs*: [Gall's Law](#galls-law), [YAGNI](#yagni), [KISS Principle](#kiss-principle)

### Fallacies of Distributed Computing {#fallacies-of-distributed-computing}

**A set of eight false assumptions that new distributed system designers often make.**

1. MCP-сервер не всегда доступен — fallback (tool-priority.md)
2. Latency агента переменна — таймауты 300s
3. Bandwidth окна ограничен — context-mode при >20 строк
4. Shared_memory не мгновенна — перечитывай перед записью
5. Агент не доверяет входным данным без проверки
6. Агент не знает всех других агентов
7. Hook injection не гарантирует порядок
8. Skill может быть не установлен — graceful degrade

*Cross-refs*: [CAP Theorem](#cap-theorem), [Murphy's Law](#murphys-law), [Law of Leaky Abstractions](#law-of-leaky-abstractions)

### Law of Unintended Consequences {#law-of-unintended-consequences}

**Whenever you change a complex system, expect surprise.**

Перед добавлением нового skill/агента — grep rules/ на пересечения. Любое изменение сложной системы вызывает непредвиденные последствия.

*Cross-refs*: [Hyrum's Law](#hyrums-law), [Chesterton's Fence](#chestertons-fence), [Murphy's Law](#murphys-law)

### Zawinski's Law {#zawinskis-law}

**Every program attempts to expand until it can read mail.**

Перед добавлением нового skill/агента — удалить или объединить один существующий. count(skills) не растёт.

*Cross-refs*: [YAGNI](#yagni), [KISS Principle](#kiss-principle), [Gall's Law](#galls-law)

## Teams

Laws about group dynamics, coordination limits, and organizational structure.

### Brooks's Law {#brooks-law}

**Adding manpower to a late software project makes it later.**

Не клонировать executor для ускорения — декомпозировать задачу. Добавление агентов в поздний pipeline замедляет, не ускоряет.

*Cross-refs*: [Conway's Law](#conways-law), [Ringelmann Effect](#ringelmann-effect), [Dunbar's Number](#dunbars-number)

### Dunbar's Number {#dunbars-number}

**There is a cognitive limit of about 150 stable relationships one person can maintain.**

Max 5 активных агентов с координацией. Сверх — когнитивная перегрузка оркестратора. Требуется декомпозиция на подкоманды.

*Cross-refs*: [Conway's Law](#conways-law), [Brooks's Law](#brooks-law), [Miller's Law](#millers-law)

### The Ringelmann Effect {#ringelmann-effect}

**Individual productivity decreases as group size increases.**

Per-agent эффективность падает с ростом группы. Параллельные задачи — декомпозировать, не добавлять агентов.

*Cross-refs*: [Brooks's Law](#brooks-law), [Price's Law](#prices-law), [Dunbar's Number](#dunbars-number)

### Price's Law {#prices-law}

**The square root of the total number of participants does 50% of the work.**

sqrt(N) агентов делает 50% работы. Оптимизировать routing ключевых (executor, architect, verifier) сначала.

*Cross-refs*: [Ringelmann Effect](#ringelmann-effect), [Pareto Principle](#pareto-principle), [Amdahl's Law](#amdahls-law)

### Putt's Law {#putts-law}

**Those who understand technology don't manage it, and those who manage it don't understand it.**

Технические решения — техническому агенту (executor/architect). Orchestrator — routing, агент — решение.

*Cross-refs*: [Conway's Law](#conways-law), [Dilbert Principle](#dilbert-principle), [Peter Principle](#peter-principle)

### Peter Principle {#peter-principle}

**In a hierarchy, every employee tends to rise to their level of incompetence.**

haiku не становится opus от количества запросов. Агент fails 2 раза — декомпозировать или эскалировать, не повторять.

*Cross-refs*: [Putt's Law](#putts-law), [Dilbert Principle](#dilbert-principle), [Dunning-Kruger Effect](#dunning-kruger-effect)

### Bus Factor {#bus-factor}

**The minimum number of team members whose loss would put the project in serious trouble.**

Ни один критичный путь не зависит от единственного агента. Если только debugger трассирует error — bus factor = 1, добавить verifier.

*Cross-refs*: [Linus's Law](#linuss-law), [Murphy's Law](#murphys-law), [Least Privilege](#least-privilege)

### Dilbert Principle {#dilbert-principle}

**Companies tend to promote incompetent employees to management to limit the damage they can do.**

Routing по capability (delegation.md), не по availability. Не назначать агента потому что "свободен".

*Cross-refs*: [Putt's Law](#putts-law), [Peter Principle](#peter-principle), [Pareto Principle](#pareto-principle)

## Planning

Laws about estimation, time management, and project predictability.

### Premature Optimization {#premature-optimization}

**Premature optimization is the root of all evil.**

Не оптимизировать pipeline prematurely — только при измеримом bottleneck.

*Cross-refs*: [KISS Principle](#kiss-principle), [YAGNI](#yagni), [Gall's Law](#galls-law)

### Parkinson's Law {#parkinsons-law}

**Work expands to fill the time available for its completion.**

Каждая задача имеет max-оценку (steps * 1.5). Превышение = эскалация.

*Cross-refs*: [Hofstadter's Law](#hofstadters-law), [Ninety-Ninety Rule](#ninety-ninety-rule)

### The Ninety-Ninety Rule {#ninety-ninety-rule}

**The first 90% of the code accounts for the first 90% of development time; the remaining 10% accounts for the other 90%.**

Progress > 90% — перерассчитать оставшееся время * 2.

*Cross-refs*: [Parkinson's Law](#parkinsons-law), [Hofstadter's Law](#hofstadters-law), [Technical Debt](#technical-debt)

### Hofstadter's Law {#hofstadters-law}

**It always takes longer than you expect, even when you take into account Hofstadter's Law.**

estimate * 1.5 = реальная стоимость. Planner закладывает буфер.

*Cross-refs*: [Parkinson's Law](#parkinsons-law), [Ninety-Ninety Rule](#ninety-ninety-rule), [Sunk Cost Fallacy](#sunk-cost-fallacy)

### Goodhart's Law {#goodharts-law}

**When a measure becomes a target, it ceases to be a good measure.**

Метрика ≠ цель: "критические пути покрыты" = цель, "всё зелёное" ≠ цель. Когда мера становится целью, она перестаёт быть хорошей мерой.

*Cross-refs*: [Gilb's Law](#gilbs-law), [Confirmation Bias](#confirmation-bias), [Pesticide Paradox](#pesticide-paradox)

### Gilb's Law {#gilbs-law}

**Anything you need to quantify can be measured in some way better than not measuring it.**

Любое явление можно измерить каким-то способом лучше, чем не измерять вовсе. Приблизительная метрика лучше отсутствия метрики. Непроверяемое правило = декоративное. Исключение: стилистические правила верифицируются пользователем.

*Cross-refs*: [Goodhart's Law](#goodharts-law), [Hyrum's Law](#hyrums-law)

## Quality

Laws about reliability, testing, and code integrity.

### The Boy Scout Rule {#boy-scout-rule}

**Leave code cleaner than you found it.**

При редактировании файла: устаревший комментарий — удали, дублирование — факторни. Не открывать файлы ради чистки. Культура качества начинается с мелких правок — небрежность размножается.

*Cross-refs*: [Broken Windows Theory](#broken-windows-theory), [Technical Debt](#technical-debt)

### Murphy's Law {#murphys-law}

**Anything that can go wrong will go wrong.**

Всё, что может пойти не так — пойдёт не так. MCP-сервер МОЖЕТ упасть — упадёт. Каждый критический путь имеет fallback. Нет fallback = нет пути.

*Cross-refs*: [Fallacies of Distributed Computing](#fallacies-of-distributed-computing), [Bus Factor](#bus-factor), [Law of Unintended Consequences](#law-of-unintended-consequences)

### Postel's Law {#postels-law}

**Be conservative in what you do, be liberal in what you accept from others.**

Отправитель: строгий формат output. Получатель: толерантен к вариациям входа. Толерантность ≠ молчаливое принятие мусора — log warning + fallback.

*Cross-refs*: [Hyrum's Law](#hyrums-law), [Principle of Least Astonishment](#principle-of-least-astonishment), [SOLID Principles](#solid-principles)

### Broken Windows Theory {#broken-windows-theory}

**Don't leave broken windows (bad designs, wrong decisions, or poor code) unrepaired.**

CI red — фиксить до новых фич. Неиспользуемое правило в rules/ — удалить или уточнить.

*Cross-refs*: [Boy Scout Rule](#boy-scout-rule), [Technical Debt](#technical-debt), [Chesterton's Fence](#chestertons-fence)

### Technical Debt {#technical-debt}

**Technical Debt is everything that slows us down when developing software.**

Отступление от правил → запись в .omc/tech-debt.md: что, почему, когда исправить.

*Cross-refs*: [Broken Windows Theory](#broken-windows-theory), [Lehman's Laws](#lehmans-laws), [Gall's Law](#galls-law)

### Linus's Law {#linuss-law}

**Given enough eyeballs, all bugs are shallow.**

При достаточном количестве глаз все баги поверхностны. Критические изменения: минимум 2 ревьювера (code-reviewer + security-reviewer).

*Cross-refs*: [Bus Factor](#bus-factor), [Testing Pyramid](#testing-pyramid), [Pesticide Paradox](#pesticide-paradox)

### Kernighan's Law {#kernighans-law}

**Debugging is twice as hard as writing the code in the first place.**

Debugger = минимум sonnet, предпочтительно opus. Не отправлять haiku на root-cause.

*Cross-refs*: [Dunning-Kruger Effect](#dunning-kruger-effect), [Testing Pyramid](#testing-pyramid)

### Testing Pyramid {#testing-pyramid}

**A project should have many fast unit tests, fewer integration tests, and only a small number of UI tests.**

verifier = unit, code-reviewer = integration, ultraqa = e2e. Не путать уровни.

*Cross-refs*: [Linus's Law](#linuss-law), [Pesticide Paradox](#pesticide-paradox), [Goodhart's Law](#goodharts-law)

### Pesticide Paradox {#pesticide-paradox}

**Repeatedly running the same tests becomes less effective over time.**

QA-cycle 3 без новых findings — добавить test-case или изменить approach.

*Cross-refs*: [Testing Pyramid](#testing-pyramid), [Goodhart's Law](#goodharts-law), [Lehman's Laws](#lehmans-laws)

### Lehman's Laws {#lehmans-laws}

**Software that reflects the real world must evolve, and that evolution has predictable limits.**

1. Статичный pipeline мёртв — адаптируй
2. Сложность растёт — ежеквартальная рефакторинг-итерация rules/
3. git log rules/ = 0 за квартал → review требуется

*Cross-refs*: [Technical Debt](#technical-debt), [Sturgeon's Law](#sturgeons-law), [Pesticide Paradox](#pesticide-paradox)

### Sturgeon's Law {#sturgeons-law}

**90% of everything is crap.**

90% AI-output мусор. reviewer отсекает, verifier подтверждает. Не принимать без фильтрации. +1 агент = +1 verification-точка. Масштабирование без верификации = деградация.

*Cross-refs*: [Lehman's Laws](#lehmans-laws), [Pareto Principle](#pareto-principle), [Kernighan's Law](#kernighans-law)

## Scale

Laws about parallelization, network effects, and growth boundaries.

### Amdahl's Law {#amdahls-law}

**The speedup from parallelization is limited by the fraction of work that cannot be parallelized.**

Оркестратор = bottleneck → параллелизм не поможет. Делегировать решения агентам.

*Cross-refs*: [Gustafson's Law](#gustafsons-law), [Brooks's Law](#brooks-law), [Price's Law](#prices-law)

### Gustafson's Law {#gustafsons-law}

**It is possible to achieve significant speedup in parallel processing by increasing the problem size.**

Малая задача = 1 агент (KISS). Большая = параллель (ultrawork).

*Cross-refs*: [Amdahl's Law](#amdahls-law), [Pareto Principle](#pareto-principle), [KISS Principle](#kiss-principle)

### Metcalfe's Law {#metcalfes-law}

**The value of a network is proportional to the square of the number of users.**

Ценность pipeline ~ n^2 связей. Новая интеграция = trigger + routing + fallback. Нет любого — нет связи, а шум.

*Cross-refs*: [YAGNI](#yagni), [Law of Unintended Consequences](#law-of-unintended-consequences), [Fallacies of Distributed Computing](#fallacies-of-distributed-computing)

## Design

Laws about code structure, simplicity, and interface design.

### YAGNI {#yagni}

**Don't add functionality until it is necessary.**

Не реализовывать "на всякий случай". Признаки нарушения: "может понадобиться", "в будущем", "для полноты".

*Cross-refs*: [KISS Principle](#kiss-principle), [Premature Optimization](#premature-optimization), [Zawinski's Law](#zawinskis-law)

### DRY Principle {#dry-principle}

**Every piece of knowledge must have a single, unambiguous, authoritative representation.**

Единый источник истины для каждого знания. Дублирование правил в rules/ → выбрать одно, остальные — ссылка. Этот файл — единственный источник определений законов.

*Cross-refs*: [SOLID Principles](#solid-principles), [Rule of Three](#rule-of-three), [KISS Principle](#kiss-principle)

### KISS Principle {#kiss-principle}

**Designs and systems should be as simple as possible.**

Простейшее решение, решающее задачу — правильное. Не вводи абстракцию, если без неё яснее. Не создавай agent-workflow для 2 команд — делай напрямую.

*Cross-refs*: [YAGNI](#yagni), [Worse Is Better](#worse-is-better), [Tesler's Law](#teslers-law)

### SOLID Principles {#solid-principles}

**Five main guidelines that enhance software design, making code more maintainable and scalable.**

- SRP (Single Responsibility): один агент — одна ответственность
- OCP (Open/Closed): новые agents/skills добавляются без изменения существующих routing
- LSP (Liskov Substitution): verifier заменяем verifier — интерфейс идентичен
- ISP (Interface Segregation): агент получает только нужную секцию rules/
- DIP (Dependency Inversion): оркестрация зависит от routing table, не от конкретных агентов

*Cross-refs*: [Conway's Law](#conways-law), [KISS Principle](#kiss-principle), [Law of Demeter](#law-of-demeter)

### Law of Demeter {#law-of-demeter}

**An object should only interact with its immediate friends, not strangers.**

Агент общается только с ближайшими соседями. Цепочка: agent→orchestrator→agent, НЕ agent→agent→agent.

*Cross-refs*: [Conway's Law](#conways-law), [SOLID Principles](#solid-principles), [Least Privilege](#least-privilege)

### Principle of Least Astonishment {#principle-of-least-astonishment}

**Software and interfaces should behave in a way that least surprises users and other developers.**

Побочные эффекты — только явно запрошенные. Поведение pipeline не должно удивлять пользователя.

*Cross-refs*: [Postel's Law](#postels-law), [Hyrum's Law](#hyrums-law), [KISS Principle](#kiss-principle)

### Rule of Three (OMC) {#rule-of-three}

**Three duplicates of a pattern warrant refactoring into a shared abstraction.**

3 дубля паттерна — рефакторинг в общий шаг.

*Cross-refs*: [DRY Principle](#dry-principle), [SOLID Principles](#solid-principles)

### Miller's Law (OMC) {#millers-law}

**A person can hold roughly 7 (plus or minus 2) items in working memory at once.**

Max 9 пунктов в секции. Больше — разбить на подсекции.

*Cross-refs*: [Dunbar's Number](#dunbars-number), [KISS Principle](#kiss-principle)

### Unix Philosophy (OMC) {#unix-philosophy}

**Do one thing and do it well.**

Агент с 2+ несвязными задачами → разделить на 2 агента.

*Cross-refs*: [SOLID Principles](#solid-principles), [KISS Principle](#kiss-principle), [Conway's Law](#conways-law)

### Least Privilege (OMC) {#least-privilege}

**A subject should be given only those privileges needed for its task.**

В Agent prompt — только контекст задачи, не весь CLAUDE.md.

*Cross-refs*: [Law of Demeter](#law-of-demeter), [Bus Factor](#bus-factor), [SOLID Principles](#solid-principles)

### Worse Is Better (OMC) {#worse-is-better}

**A working simple solution is preferable to a broken complex one.**

Работающий простой > сломанный сложный. Ограничения → tech-debt.md.

*Cross-refs*: [KISS Principle](#kiss-principle), [Gall's Law](#galls-law), [Premature Optimization](#premature-optimization)

## Decisions

Laws about reasoning, judgment, and decision-making under uncertainty.

### Occam's Razor {#occams-razor}

**The simplest explanation is often the most accurate one.**

2 агента лучше 4 при эквивалентном результате.

*Cross-refs*: [KISS Principle](#kiss-principle), [Dunning-Kruger Effect](#dunning-kruger-effect), [Hanlon's Razor](#hanlons-razor)

### Dunning-Kruger Effect {#dunning-kruger-effect}

**The less you know about something, the more confident you tend to be.**

haiku + неопределённость → эскалация на sonnet/opus, не угадывание.

*Cross-refs*: [Occam's Razor](#occams-razor), [Peter Principle](#peter-principle), [Kernighan's Law](#kernighans-law)

### Hanlon's Razor {#hanlons-razor}

**Never attribute to malice that which is adequately explained by stupidity or carelessness.**

Ошибка агента = недостаток контекста, не дефект дизайна. Искать недостающий контекст.

*Cross-refs*: [Occam's Razor](#occams-razor), [Inversion](#inversion), [Confirmation Bias](#confirmation-bias)

### Sunk Cost Fallacy {#sunk-cost-fallacy}

**Sticking with a choice because you've invested time or energy in it, even when walking away helps you.**

3 fails одним подходом — сменить подход. Не вкладывать в failed direction.

*Cross-refs*: [Hofstadter's Law](#hofstadters-law), [Confirmation Bias](#confirmation-bias), [Lindy Effect](#lindy-effect)

### The Map Is Not the Territory {#map-is-not-the-territory}

**Our representations of reality are not the same as reality itself.**

AI-вывод, противоречащий наблюдаемому поведению (test fail, error log) — наблюдение побеждает.

*Cross-refs*: [Confirmation Bias](#confirmation-bias), [First Principles Thinking](#first-principles-thinking), [Gilb's Law](#gilbs-law)

### Confirmation Bias {#confirmation-bias}

**A tendency to favor information that supports our existing beliefs or ideas.**

architect предлагает, critic опровергает. Не самопроверять.

*Cross-refs*: [Inversion](#inversion), [Goodhart's Law](#goodharts-law), [Hanlon's Razor](#hanlons-razor)

### The Hype Cycle & Amara's Law {#hype-cycle-amaras-law}

**We tend to overestimate the effect of a technology in the short run and underestimate the impact in the long run.**

Новый skill: первые итерации хуже ожидаемого, потом лучше. Не удалять после 1 fail. Не ставить в critical path до стабилизации.

*Cross-refs*: [Lindy Effect](#lindy-effect), [Premature Optimization](#premature-optimization), [Sunk Cost Fallacy](#sunk-cost-fallacy)

### The Lindy Effect {#lindy-effect}

**The longer something has been in use, the more likely it is to continue being used.**

Старое проверенное правило надёжнее нового хайпа. Новый pattern, конфликтующий с устоявшимся — требует 2+ подтверждений.

*Cross-refs*: [Hype Cycle & Amara's Law](#hype-cycle-amaras-law), [Chesterton's Fence](#chestertons-fence), [Sunk Cost Fallacy](#sunk-cost-fallacy)

### First Principles Thinking {#first-principles-thinking}

**Breaking a complex problem into its most basic blocks and then building up from there.**

При выборе архитектуры — разложить задачу на базовые факты, строить от них, не от аналогий.

*Cross-refs*: [Inversion](#inversion), [Map Is Not the Territory](#map-is-not-the-territory), [Gall's Law](#galls-law)

### Inversion {#inversion}

**Solving a problem by considering the opposite outcome and working backward from it.**

Перед планом — минимум 1 вопрос "что может пойти не так?".

*Cross-refs*: [First Principles Thinking](#first-principles-thinking), [Confirmation Bias](#confirmation-bias), [Murphy's Law](#murphys-law)

### Pareto Principle {#pareto-principle}

**80% of the problems result from 20% of the causes.**

20% правил решают 80% проблем: delegation > verification > style. Оптимизируй усилия по максимальному impact, не по coverage.

*Cross-refs*: [Price's Law](#prices-law), [Sturgeon's Law](#sturgeons-law), [Amdahl's Law](#amdahls-law)

### Cunningham's Law {#cunninghams-law}

**The best way to get the correct answer on the Internet is not to ask a question, it's to post the wrong answer.**

При блокировке — предложить draft-решение, получить корректировку от пользователя.

*Cross-refs*: [Inversion](#inversion), [First Principles Thinking](#first-principles-thinking)

### Chesterton's Fence (OMC) {#chestertons-fence}

**Do not remove a fence until you know why it was put up.**

Не удалять правило без git blame/commit message. Причина неизвестна — задокументировать в tech-debt.md.

*Cross-refs*: [Lindy Effect](#lindy-effect), [Broken Windows Theory](#broken-windows-theory), [Law of Unintended Consequences](#law-of-unintended-consequences)
