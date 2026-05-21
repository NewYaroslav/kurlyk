# Coding Style

## Naming Conventions

### Variables

- Always use the `m_` prefix for class fields (e.g., `m_event_hub`, `m_task_manager`).
- Optional `p_` and `str_` prefixes may be used when a function or method has more than five variables or arguments of different types. Otherwise, omit these prefixes.
- Boolean variables usually start with `is`, `has`, `use`, `enable`, or for class fields, `m_is_`, `m_has_`, etc. (e.g., `is_connected`, `m_is_active`).
- Prefer the surrounding file's established naming style over applying the boolean-prefix rule mechanically. Public request/config and response data structs usually use property or mode names rather than predicate-style names when neighboring fields do (e.g., `head_only`, `verbose`, `debug_header`, `streaming`, `ready`, `stream_chunk`).
- Do not use the prefixes `b_`, `n_`, or `f_`.

### Files

- Use `CamelCase` if the file contains only one class (e.g., `TradeManager.hpp`).
- Use `snake_case` if the file contains multiple classes, utilities, or helper structures (e.g., `trade_utils.hpp`, `market_event_listener.hpp`).

### Entities

- Class, struct, and enum names use `CamelCase`.
- Method names use `snake_case`.

### Methods

- Methods are named using `snake_case`.
- Getter methods may omit the `get_` prefix when they simply return a reference or value, expose an internal object, or behave like a property (e.g., `size()`, `empty()`).
- Use `get_` when the method performs computations or when omitting it would be misleading.

## Doxygen Comments

- All code comments and Doxygen annotations must be in English.
- Prepend functions and classes with `/// \brief`.
- Do not start descriptions with `The`.
