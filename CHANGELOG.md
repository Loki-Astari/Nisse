# Changelog

All notable changes to this project will be documented in this file.

This component is part of [ThorsAnvil](https://github.com/Loki-Astari/ThorsAnvil). See the [parent changelog](https://github.com/Loki-Astari/ThorsAnvil/blob/master/CHANGELOG.md) for full release history.

The format is based on [Keep a Changelog](https://keepachangelog.com/).

## [11.0.0] - 2026-06-24

### Changed
- Ensure URL paths have leading slash
- Support viewing buffer content or forcing download
- Changed behavior of `preloadStreamIntoBuffer()`
- Improved stream handling
- Fixed `getContentSize()`

### Fixed
- Removed bad check on stream state

## [9.2.0] - 2026-04-10

### Security
- Limited header size to prevent header-based attacks
- Used `std::atomic` for values accessed by multiple threads

## [0.05.001] - 2026-03-22

### Changed
- Updated build tools
- Updated logging to be consistent
