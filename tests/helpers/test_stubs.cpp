/***************************************************************************
 * Test Stubs - Headless implementations for GUI dependencies
 *
 * This file provides stub implementations of GUI-dependent functions
 * to allow game logic tests to run in headless mode without SDL/ParaGUI.
 ***************************************************************************/

#include "events.h"
#include "soundList.h"
#include "sdl/sound.h"
#include "attack.h"
#include "gamemap.h"
#include "reactionfire.h"
#include "actions/context.h"
#include "vehicle.h"
#include "astar2.h"
#include "dialog.h"
#include "loaders.h"
#include "music.h"
#include "mapdisplayinterface.h"
#include "actions/cancelresearchcommand.h"
#include "containerbase-functions.h"
#include "sg.h"
#include <deque>

// ===================================================================
// Event System Stubs
// ===================================================================

// Global ticker variable (normally updated by SDL event loop)
volatile int ticker = 0;

// Release timeslice stub (normally calls SDL_Delay)
int releasetimeslice(void) {
   // In tests, we don't need to yield to OS
   return 0;
}

// ===================================================================
// Sound System Stubs
// ===================================================================

// SoundList singleton stub
SoundList& SoundList::getInstance() {
   static SoundList instance;
   return instance;
}

SoundList::~SoundList() {
   // Stub destructor
}

Sound* SoundList::getSound(SoundList::Sample sample, int player, const ASCString& filename, int id) {
   // Return null sound - tests don't need audio
   (void)sample; (void)player; (void)filename; (void)id;  // Suppress unused warnings
   return nullptr;
}

Sound* SoundList::playSound(SoundList::Sample sample, int player, bool positional, const ASCString& filename) {
   // No-op in tests - return null sound
   (void)sample; (void)player; (void)positional; (void)filename;  // Suppress unused warnings
   return nullptr;
}

// Sound class stub
void Sound::play() {
   // No-op in tests
}

// ===================================================================
// Animation Stubs
// ===================================================================

// Attack animation stub (normally shows GUI battle panel)
void showAttackAnimation(tfight& battle, GameMap* actmap, int ad, int dd) {
   // No-op in tests - battles execute without animation
}

// ===================================================================
// Reaction Fire Stubs
// ===================================================================

// tsearchreactionfireingunits implementations (complex class for reaction fire mechanics)
tsearchreactionfireingunits::tsearchreactionfireingunits(GameMap* map) : treactionfire() {
   // Stub constructor
   (void)map;  // Suppress unused warning
}

tsearchreactionfireingunits::~tsearchreactionfireingunits() {
   // Stub destructor
}

void tsearchreactionfireingunits::init(Vehicle* veh, const MapCoordinate3D& dest) {
   // Stub - no reaction fire in tests
}

void tsearchreactionfireingunits::init(Vehicle* veh, const std::deque<AStar3D::PathPoint>& path) {
   // Stub - no reaction fire in tests
}

int tsearchreactionfireingunits::checkfield(const MapCoordinate3D& pos, Vehicle*& attacker, const Context& context) {
   // Stub - no reaction fire in tests
   return 0;
}

int tsearchreactionfireingunits::finalCheck(int result, const Context& context) {
   // Stub - no reaction fire in tests
   return 0;
}

// ===================================================================
// Dialog System Stubs
// ===================================================================

// Dialog choice stub - returns "left button" (1) by default
int choice_dlg(const ASCString& title, const ASCString& leftButton, const ASCString& rightButton) {
   // In tests, always choose the left button (typically "Yes" or "OK")
   (void)title; (void)leftButton; (void)rightButton;  // Suppress unused warnings
   return 1;  // 1 = left button, 2 = right button
}

// ===================================================================
// Loader Infrastructure Stubs
// ===================================================================

// tspfldloaders base class stub implementations
tspfldloaders::tspfldloaders(void) : stream(nullptr), spfld(nullptr) {
   // Stub constructor
}

tspfldloaders::~tspfldloaders() {
   // Stub destructor
}

// Signal stub (normally used to notify UI of map load completion)
sigc::signal<void, GameMap*> tspfldloaders::mapLoaded;

// tgameloaders stub implementation
void tgameloaders::initmap(void) {
   // Stub - in tests, no map initialization needed
}

// tsavegameloaders stub for loading map preview images
GameFileInformation tsavegameloaders::loadMapimageFromFile(const ASCString& filename) {
   // Stub - return empty game file info (no preview in tests)
   (void)filename;
   return GameFileInformation();
}

// ===================================================================
// Music Playlist Stubs
// ===================================================================

// MusicPlayList stub - returns empty track name
const ASCString& MusicPlayList::getNextTrack() {
   // Stub - return empty string (no music in tests)
   static ASCString emptyTrack = "";
   return emptyTrack;
}

// MusicPlayList diagnostic stub
ASCString MusicPlayList::getDiagnosticText() {
   // Stub - return simple diagnostic
   return "Headless mode - no music";
}

// ===================================================================
// Map Display Stubs (Layer 2)
// ===================================================================

// Headless MapDisplay stub class
class HeadlessMapDisplay : public MapDisplayInterface {
public:
   int displayMovingUnit(const MapCoordinate3D& start, const MapCoordinate3D& dest,
                        Vehicle* vehicle, int fieldnum, int totalmove,
                        SoundStartCallback startSound, int duration) override {
      (void)start; (void)dest; (void)vehicle; (void)fieldnum;
      (void)totalmove; (void)startSound; (void)duration;
      return 0;
   }
   void displayMap(void) override {}
   void displayMap(Vehicle* additionalVehicle) override { (void)additionalVehicle; }
   void displayPosition(int x, int y) override { (void)x; (void)y; }
   void resetMovement(void) override {}
   void startAction(void) override {}
   void stopAction(void) override {}
   void cursor_goto(const MapCoordinate& pos) override { (void)pos; }
   void displayActionCursor(int x1, int y1, int x2, int y2) override {
      (void)x1; (void)y1; (void)x2; (void)y2;
   }
   void removeActionCursor(void) override {}
   void updateDashboard() override {}
   void repaintDisplay() override {}
   void setTempView(bool view) override { (void)view; }
   void showBattle(tfight& battle) override { (void)battle; }
   void playPositionalSound(const MapCoordinate& pos, Sound* snd) override {
      (void)pos; (void)snd;
   }
   int getUnitMovementDuration() const override { return 0; }
};

// Global headless map display instance
MapDisplayInterface& getDefaultMapDisplay() {
   static HeadlessMapDisplay instance;
   return instance;
}

// ===================================================================
// Game Loading Stubs
// ===================================================================

// Game loading stub
bool loadGameFromFile(const ASCString& filename) {
   // Stub - in tests, don't actually load games
   (void)filename;
   return false;  // Return false to indicate no game loaded
}

// ===================================================================
// Command Infrastructure Stubs
// ===================================================================

// CancelResearchCommand stub
CancelResearchCommand::CancelResearchCommand(GameMap* map) : Command(map) {
   // Stub constructor
}

void CancelResearchCommand::setPlayer(const Player& player) {
   // Stub - store player reference
   (void)player;
}

// ===================================================================
// PowerPlant Stubs
// ===================================================================

// RegenerativePowerPlant stub
RegenerativePowerPlant::RegenerativePowerPlant(ContainerBase* _bld) {
   // Stub constructor
   bld = _bld;
   toProduce = Resources();  // Initialize to zero resources
}

// WindPowerplant stubs
Resources WindPowerplant::getPlus() {
   // Stub - return zero power generation
   return Resources();
}

// SolarPowerplant stubs
Resources SolarPowerplant::getPlus() {
   // Stub - return zero power generation
   return Resources();
}

// Additional RegenerativePowerPlant method stubs
bool RegenerativePowerPlant::finished() {
   // Stub - return false (not finished)
   return false;
}

bool RegenerativePowerPlant::run() {
   // Stub - return false (no work done)
   return false;
}

Resources RegenerativePowerPlant::getUsage() {
   // Stub - return zero resources
   return Resources();
}
