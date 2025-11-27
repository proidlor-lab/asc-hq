# Test Helpers

This directory contains helper utilities for headless testing.

## test_stubs.cpp

Provides stub implementations for GUI-dependent functions to enable headless testing without SDL/ParaGUI dependencies.

### Purpose

The ASC codebase has deep coupling between business logic and GUI presentation. To test game logic without requiring a full GUI stack, this file provides minimal stub implementations that:

1. Allow tests to compile and link without ParaGUI/SDL libraries
2. Provide no-op implementations for GUI operations (animations, sounds, etc.)
3. Enable headless execution for CI/CD environments

### Stubs Provided

**Event System**:
- `ticker` - Global timer variable (normally updated by SDL event loop)
- `releasetimeslice()` - Yield to OS (no-op in tests)

**Sound System**:
- `SoundList::getInstance()` - Singleton accessor
- `SoundList::getSound()` - Returns nullptr (no audio in tests)
- `SoundList::playSound()` - Returns nullptr (no audio in tests)
- `Sound::play()` - No-op

**Animation System**:
- `showAttackAnimation()` - No-op (battles execute without visual display)

**Reaction Fire**:
- `tsearchreactionfireingunits` class - Complete stub implementation (no reaction fire in tests)

**Dialog System**:
- `choice_dlg()` - Returns left button choice (no UI dialogs in tests)

**Loader Infrastructure**:
- `tspfldloaders` - Base loader class stubs
- `tgameloaders::initmap()` - Map initialization stub
- `tsavegameloaders::loadMapimageFromFile()` - Returns empty preview
- `tspfldloaders::mapLoaded` - Signal stub

**Music System**:
- `MusicPlayList::getNextTrack()` - Returns empty track
- `MusicPlayList::getDiagnosticText()` - Returns "Headless mode" message

**Map Display (Layer 2)**:
- `getDefaultMapDisplay()` - Returns headless MapDisplayInterface
- `HeadlessMapDisplay` class - Complete no-op implementation of all display methods

**Game Infrastructure (Layer 2)**:
- `loadGameFromFile()` - Returns false (no game loading in tests)
- `CancelResearchCommand` - Stub command class

**PowerPlant Classes (Layer 2)**:
- `RegenerativePowerPlant` - Base powerplant stub
- `WindPowerplant::getPlus()` - Returns zero resources
- `SolarPowerplant::getPlus()` - Returns zero resources

### Usage

These stubs are automatically included in `libcommon.la` during test builds. No special configuration is needed in individual test files.

### Limitations

The stub approach is a **tactical solution** to enable testing while the codebase undergoes architectural refactoring. The long-term solution involves:

1. Extracting business logic from GUI-coupled classes (see `docs/GAMEMAP_REFACTORING_PLAN.md`)
2. Implementing proper dependency injection
3. Creating clear separation between presentation and domain layers

### Status (Updated 2025-11-27)

**Working**: Event system, sound system, animation system, reaction fire, loaders, music, map display, power plants
**Incomplete**: ParaGUI widgets (dashboard, panels), dialog system (ASC_PG_Dialog), additional GUI components
**Blocker**: Multiple definition conflicts with real libraries (ticker in libsdl.a, CancelResearchCommand in libdirectunitactions.a)

**Conclusion**: The stub approach successfully demonstrates the architectural coupling problem but cannot fully resolve it. Approximately ~9,000 lines of GUI code remain embedded in "common" library files (dlg_box.cpp, gamedlg.cpp, controls.cpp, etc.) that require comprehensive ParaGUI stubbing or removal.

**Recommendation**: Use these stubs as a foundation for future headless code, but proceed with proper architectural refactoring (dependency injection, interface extraction) for Phase 2 test migration.

See `docs/modernization/STUB_PATCH_FINDINGS.md` for complete analysis and `docs/modernization/PHASE_2_TEST_BLOCKERS.md` for detailed status.
