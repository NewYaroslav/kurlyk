# Changelog

All notable changes to this project will be documented in this file.

## 2025-12-11
### Added
- Added CMakeLists and CMake helper functions for dependency integration and fallback resolution
### Fixed
- Resolved One Definition Rule issues

## 2025-08-08
### Changed
- Clarified console output messages
### Fixed
- Fixed hang during cancel_requests
### Added
- Added error tracing system via KURLYK_HANDLE_ERROR

## 2025-07-31
### Changed
- Reorganized include structure and filenames 
### Added
- Added custom CSS theme and updated Doxygen settings

## 2025-05-21
### Added
- Added support for custom valid HTTP status codes in requests

## 2025-05-03
### Added
- Added CURL timing metrics (DNS, connect, SSL, etc.) and support for HEAD requests via head_only flag

## 2025-05-01
### Added
- Added extended error handling system with ClientError, WebSocketError, and HttpError categories
### Fixed
- Set status 451 for blocked sites with no HTTP response
### Changed
- Global project reorganization and header structure overhaul

## 2025-02-17
### Fixed
- Fixed support Cyrillic characters in file paths

## 2025-02-15
### Added
- Added methods for proxy configuration and `Accept-Language` handling
- Added feature: return HTTP status 499 on timeout
### Fixed
- Fixed rate limiter to use the provided time argument in `check_limit`

## 2024-11-17
### Added
- Add support for canceling HTTP requests by ID
- Added setters to `HttpClient` and `HttpRequest`
- Added generation of `sec-ch-ua` from a standard User-Agent
- Added email validation utility
### Fixed
- Fix bug with User-Agent setup

## 2024-11-14
### Added
- Added ability to remove request limits
- Examples added

## 2024-11-12
### Fixed
- Fixed compatibility issues with Boost
- Fixed compilation errors for C++17