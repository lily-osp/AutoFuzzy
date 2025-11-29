# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2025-11-29

### Added
- Comprehensive documentation suite with 5 detailed guides
- Integration guide for project migration and implementation
- System architecture documentation with technical details
- Troubleshooting guide with diagnostic procedures
- Enhanced usage documentation with advanced examples
- Professional mathematical centroid calculation for trapezoidal functions
- Arduino Mega optimization for heavy calculation applications
- Improved memory management with platform-specific defaults

### Changed
- Optimized default memory limits for Arduino Mega (8KB RAM)
- Enhanced mathematical accuracy in defuzzification algorithms
- Updated examples to target Arduino Mega instead of Uno
- Improved documentation structure with table of contents
- Professional documentation formatting (emoji-free, technical style)

### Fixed
- Corrected trapezoidal membership function centroid calculations
- Resolved One Definition Rule violations in examples
- Fixed memory limit conflicts between header and examples
- Improved error handling in fuzzy inference engine

## [1.1.0] - 2025-11-29

### Added
- Heuristic parameter tuning functionality (`autoTune()` method)
- Enhanced error handling with descriptive error messages
- Configurable limits via preprocessor defines
- Support for complex rules with multiple antecedents using AND/OR logic
- Multiple input/output variable support (MIMO systems)
- Memory-conscious design for Arduino environments

### Changed
- Improved API with better error checking
- Enhanced documentation with comprehensive examples
- Restructured according to Arduino library guidelines

### Fixed
- Various bug fixes and stability improvements

## [1.0.0] - 2025-01-01

### Added
- Initial release of AutoFuzzy library
- Basic fuzzy logic inference engine
- Triangular and trapezoidal membership functions
- Simple rule-based system
- Single input/output support
