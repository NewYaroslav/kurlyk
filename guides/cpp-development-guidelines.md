# C++ Development Guidelines

## Variable Naming

- Class fields must use the `m_` prefix (e.g., `m_event_hub`, `m_task_manager`).
- Prefixes `p_` and `str_` are optional and should not be introduced
  mechanically. Use them only when a function or method has more than five
  variables or arguments of different types.
- Boolean variables must start with `is`, `has`, `use`, `enable`, or member-field
  forms like `m_is_`, `m_has_`, etc. (e.g., `is_connected`, `m_is_active`).
- Prefer the surrounding file's established naming style over applying the
  boolean-prefix rule mechanically. Public request/config and response data
  structs usually use property or mode names rather than predicate-style names
  when neighboring fields do (e.g., `head_only`, `verbose`, `debug_header`,
  `streaming`, `ready`, `stream_chunk`).
- Do not use the prefixes `b_`, `n_`, or `f_`.

## Doxygen Comments

- All comments and Doxygen annotations must be in English.
- Prepend functions and classes with `/// \\brief`.
- Do not start descriptions with `The`.
- Describe current behavior and contract in declarative present-tense style.
- Avoid migration/comparison wording such as `now`, `no longer`, `remains`,
  `stays`, `previously`, and `currently` unless the text is explicitly
  documenting history or migration.

## File Names

- Use `CamelCase` if the file contains only one class (e.g., `TradeManager.hpp`).
- Use `snake_case` if the file contains multiple classes, utilities, or helper
  structures (e.g., `trade_utils.hpp`, `market_event_listener.hpp`).

## Entity Names

- Class, struct, and enum names use `CamelCase`.
- Method names use `snake_case`.

## Method Naming

- Methods must be named in `snake_case`.
- Getter names may omit the `get_` prefix when they simply return a reference or
  value, expose an internal object, or behave like a property (e.g., `size()`,
  `empty()`).
- Use `get_` when the method performs computations or when omitting it would
  be misleading.

## Code Style

- Use 4 spaces for indentation; do not use tabs.
- Indent declarations and definitions inside `namespace` blocks by one level.
- Keep opening braces on the same line for classes, methods, and namespaces.
- Do not use `using namespace`; always qualify names such as `std::`.
- Keep project headers before system headers in include lists.
- Header files must start with `#pragma once`; if an include guard is also used,
  prefer a `_KURLYK_*_HPP_INCLUDED` style guard that matches the file.
- Keep source and documentation files in UTF-8.
- Write non-ASCII C++ string literals as `u8"..."`.
- The default standard is C++11. Guard C++17-only code with
  `#if __cplusplus >= 201703L` as in existing headers.

## Constants and Macros

- Constants and macro names use `UPPER_SNAKE_CASE`.
- Public enum values use `CamelCase`.
- Mark non-inheritable classes with `final`.
- Use `override` for overridden virtual methods.
