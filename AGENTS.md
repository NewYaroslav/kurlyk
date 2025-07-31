# AGENTS.md

## Overview

**kurlyk** is a C++17 library that wraps `curl` and `Simple-WebSocket-Server` to provide easy HTTP and WebSocket clients. It exposes a simple API for asynchronous requests, rate limiting and reconnection logic.

The repository contains examples under `examples/` and various third party dependencies in the `libs/` directory.

## Commit Messages

Follow the [Conventional Commits](https://www.conventionalcommits.org/) style:

- `feat:` new features
- `fix:` bug fixes
- `docs:` documentation changes
- `refactor:` code refactoring without behaviour changes
- `test:` when adding or modifying tests

Format: `type(scope): short description` where the scope is optional. Keep messages short and imperative.

## Testing and Build

There is currently no automated test suite. If you modify library code, compile at least one example from `examples/` to ensure that headers remain valid. Examples rely on external libraries from `libs/`; make sure your environment has the required dependencies installed if you wish to build them.


