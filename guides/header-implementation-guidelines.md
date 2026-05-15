# Header / Implementation Ownership Playbook

Use this document when you are changing `.hpp`, `.ipp`, or `.tpp` structure.

## Decide the Subsystem Model First

`kurlyk` uses an **aggregate-first** model for public modules:

- Normal consumers include one aggregate entry header first.
- Shared prerequisites may be centralized there.
- Leaf headers may rely on that prepared include context.

Do not mix standalone-leaf-first casually inside one subsystem.

## Place Code by Ownership

### `.hpp`

Put in `.hpp`:
- public declarations
- public aliases and config types
- the include contract for the module

### `.tpp`

Put in `.tpp`:
- template-visible implementation

Rules:
- if a consumer can instantiate it, the definition must be visible from the header
- include `.tpp` from `.hpp` unconditionally when needed
- do not hide template definitions behind implementation-only paths

### `.ipp`

Put in `.ipp`:
- non-template implementation kept near the header
- shared implementation for header-only and compiled-library modes
- inline-capable helper bodies

## Keep Ownership Explicit

Use always-included implementation only for:
- templates
- tiny helpers
- small inline methods
- truly leaf-level header-owned logic

Use ownership-controlled implementation for:
- large non-template methods
- code that should be emitted once in compiled-library mode
- implementation shared between header-only and compiled-library builds

## Place Dependencies Intentionally

Centralize shared includes only when the subsystem intentionally uses an
aggregate entry header. Otherwise keep dependencies with the leaf header that
owns the public contract.

Implementation-only helpers such as build-mode macros should stay at the
narrowest valid boundary:
- preferably in the `.ipp` that uses them
- otherwise in the direct owning `.hpp`

## Respect Project Include Boundaries

- Do not use `../` in `#include` directives.
- Within one module sub-tree, include local headers with forward paths such as
  `#include "data/HttpRequest.hpp"`.
- Cross-module dependencies should come from the nearest umbrella header, not
  from direct sibling-module includes.
- Files under `kurlyk/detail/` may include other detail headers, but must not
  include public `kurlyk/...` headers.
- Consumers must not include `kurlyk/detail/...` directly.

## Use Forward Declarations Sparingly

Add forward declarations only when they give a real structural benefit:
- breaking a cycle
- reducing unavoidable coupling
- stabilizing a genuine leaf interface

Do not add them just to make a header look lighter if that hides dependencies,
increases include-order sensitivity, or makes the subsystem harder to read.

## Test the Intended Contract

For aggregate-first subsystems:
- test the public path through the aggregate entry header
- prefer include coverage similar to existing integration tests

For standalone-leaf-first subsystems:
- test direct inclusion of leaf headers where that is part of the contract

## Quick Review Checklist

Before finalizing:
- subsystem model is explicit
- `.tpp` contains all template-visible definitions that consumers need
- `.ipp` ownership is explicit and consistent
- shared includes are centralized only when the aggregate model is intentional
- implementation-only helpers stay local
- forward declarations are justified
- tests validate the intended include contract
