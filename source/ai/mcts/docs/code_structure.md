# ASC Code Structure & Interface Documentation

**Zweck**: Dokumentation der Code-Struktur für MCTS-KI-Entwicklung - Wo sind Interfaces, wie greift man auf Spielzustände zu, was ist vorhanden, was fehlt?

**Stand**: 2025-11-05 (basierend auf Code-Analyse)

---

## 1. Architektur-Überblick

### 1.1 Zentrale Datenstrukturen

```
GameMap (gamemap.h)
├── MapField* field                    // Array aller Hex-Felder [xsize * ysize]
├── Player player[9]                   // 8 Spieler + Neutral (Index 8)
├── int actplayer                      // Aktueller Spieler (0-7)
├── GameTime time                      // Rundenzeit (turns, moves)
├── Weather weather                    // Wind, Wetter
├── Events events                      // Map-Events
├── ResourceTribute tribute            // Ressourcen-Transfers
└── IDManager idManager               // Unique IDs für Units

Player (player.h)
├── VehicleList vehicleList           // list<Vehicle*>
├── BuildingList buildingList         // list<Building*>
├── Research research                 // Tech-Tree Status
├── BaseAI* ai                        // Zeiger auf KI-Instanz
├── PlayerStatus stat                 // human/computer/off/supervisor/suspended
└── DiplomaticStateVector diplomacy   // Beziehungen zu anderen Spielern

MapField (mapfield.h)
├── TerrainType* terrain              // Gelände-Typ
├── Vehicle* vehicle                  // Einheit auf Feld (NULL wenn leer)
├── Building* building                // Gebäude auf Feld
├── Objects* objects                  // Objekte (Wracks, Dekorationen)
├── vector<Mine*> mines               // Minen auf dem Feld
└── int index                         // Position im field[] Array

Vehicle (vehicle.h) : ContainerBase
├── VehicleType* typ                  // Einheiten-Typ (Statistiken)
├── int xpos, ypos                    // Position
├── int height                        // Höhenstufe (0-7)
├── int damage                        // Schaden (0-100, 100=zerstört)
├── int _movement                     // Verbleibende Bewegung
├── int ammo[16]                      // Munition pro Waffe
├── Resources tank                    // Fuel im Tank
├── bool attacked                     // Bereits angegriffen diese Runde?
├── Cargo cargo                       // vector<Vehicle*> - Transportierte Einheiten
├── AiParameter* aiData               // KI-spezifische Daten
└── ReactionFire reactionfire         // RF-Status

Building (buildings.h) : ContainerBase
├── BuildingType* typ
├── int xpos, ypos
├── Resources storage                 // Ressourcenlager
└── Production production             // Produktionswarteschlange
```

---

## 2. Game State Access - Wie komme ich an Informationen?

### 2.1 **GameMap - Der zentrale Zugriffspunkt** ✅

**Sehr gut designt**: `GameMap` ist THE zentrale Struktur für alles.

```cpp
// Zugriff auf Felder
MapField* getField(int x, int y);
MapField* getField(const MapCoordinate& pos);

// Zugriff auf Units
Vehicle* getUnit(int networkid);              // Via unique ID
Vehicle* getUnit(int x, int y, int nwid);     // Via Position + ID
ContainerBase* getContainer(int nwid);        // Generisch (Unit oder Building)

// Zugriff auf Spieler
Player& getPlayer(PlayerID p);
Player& getCurrentPlayer();
int getPlayerCount();  // Returns 8

// Zugriff auf Typen (aus Ruleset)
VehicleType* getvehicletype_byid(int id);
BuildingType* getbuildingtype_byid(int id);
TerrainType* getterraintype_byid(int id);
Technology* gettechnology_byid(int id);

// Spiel-Parameter (aus Map-Config)
int getgameparameter(GameParameter num);
void setgameparameter(GameParameter num, int value);

// Ressourcen-Modus
bool isResourceGlobal(int resource);         // ASC vs BI Mode
```

**Bewertung**: ✅ **Exzellenter Zugriff** - Alle wichtigen Informationen über eine zentrale Klasse erreichbar.

### 2.2 **Spieler-Informationen** ✅

```cpp
// Player hat direkte Listen seiner Assets
Player& p = gamemap->getPlayer(playerID);
Player::VehicleList& units = p.vehicleList;    // list<Vehicle*>
Player::BuildingList& buildings = p.buildingList;

// Iteration sehr einfach
for (Player::VehicleList::iterator it = units.begin(); it != units.end(); ++it) {
    Vehicle* unit = *it;
    // ...
}

// Diplomatie
bool p.diplomacy.isHostile(PlayerID other);
bool p.diplomacy.isAllied(PlayerID other);
bool p.diplomacy.sharesView(PlayerID other);  // Shared Vision

// Forschung
Research& research = p.getResearch();
ResearchAvailabilityStatus status = research.getStatus(tech);
```

**Bewertung**: ✅ **Sehr praktisch** - Keine manuelle Suche nötig, direkte Listen vorhanden.

### 2.3 **Einheiten-Informationen** ⚠️

```cpp
Vehicle* unit = gamemap->getUnit(networkid);

// Basis-Informationen - EINFACH
int hp = 100 - unit->damage;
int movement = unit->getMovement();  // Berücksichtigt Fuel!
MapCoordinate pos(unit->xpos, unit->ypos);
VehicleType* type = unit->typ;

// Waffen-Informationen - GUT
for (int i = 0; i < type->weapons.count; ++i) {
    SingleWeapon& weapon = type->weapons.weapon[i];
    int ammo = unit->ammo[i];
    int strength = unit->weapstrength[i];
    int range = weapon.maxdistance;
}

// Cargo - EINFACH
const Cargo& cargo = unit->getCargo();
int cargoCount = unit->getCargoCount();

// Sichtbarkeit - KOMPLEX ⚠️
// KEIN direktes Interface! Muss über viewcalculation.h gehen:
bool fieldvisiblenow(MapField* field, int player);  // Global function
```

**Bewertung**: ⚠️ **Gemischt**
- ✅ Basis-Daten: Sehr gut
- ✅ Waffen/Cargo: Gut strukturiert
- ❌ **Sichtbarkeit fehlt als Unit-Methode**

### 2.4 **Kampf-Simulationen** ✅

```cpp
// Angriff möglich prüfen
AttackWeap* atw = attackpossible(attacker, targetX, targetY);
// atw->count: Anzahl möglicher Waffen
// atw->num[i]: Waffen-Index
// atw->strength[i]: Effektive Stärke
delete atw;  // Manuelles Cleanup! ⚠️

// Kampf simulieren
tunitattacksunit battle(attacker, defender, respond=true, weapon=-1);
battle.calc();  // Berechnet Schaden
// battle.av: Attacker Values (damage, experience, etc)
// battle.dv: Defender Values

// Reaction Fire prüfen
tsearchreactionfireingunits rfSearch(gamemap);
rfSearch.init(unit, position);
// rfSearch.unitlist[] enthält RF-fähige Einheiten
```

**Bewertung**: ✅ **Gut designt**, aber:
- ⚠️ Manuelles Memory-Management (`delete atw`)
- ⚠️ Komplexe Klassen-Hierarchie

### 2.5 **Bewegungs-Simulation** ⚠️

```cpp
// AStar-Pathfinding vorhanden, aber komplex
#include "astar2.h"

AStar3D ast;
ast.init(gamemap, vehicle, maxRange);
ast.findPath(targetPos);

// Movement-Cost
// KEIN einfaches Interface gefunden!
// Vermutlich in VehicleType::getMoveCost(TerrainType, weather)
```

**Bewertung**: ⚠️ **Pathfinding vorhanden, aber kein triviales API**
- ❌ Keine `getMoveCost(from, to)` Convenience-Funktion
- ❌ AStar-Setup ist aufwändig

---

## 3. Interfaces & Abstractions

### 3.1 **Vorhandene Interfaces** ✅

#### **BaseAI** (baseaiinterface.h)
```cpp
class BaseAI {
public:
    virtual void run(MapDisplayInterface* mapDisplay) = 0;
    virtual bool isRunning() = 0;
    virtual VisibilityStates getVision() = 0;
    virtual void read(tnstream& stream) = 0;
    virtual void write(tnstream& stream) const = 0;
};
```
**Zweck**: Basis-Interface für alle KI-Implementierungen
**Bewertung**: ✅ Sauber, aber minimalistisch

#### **MapDisplayInterface** (mapdisplayinterface.h)
```cpp
class MapDisplayInterface {
public:
    virtual void displayMap() = 0;
    virtual void displayPosition(int x, int y) = 0;
    virtual void cursor_goto(const MapCoordinate& pos) = 0;
    virtual void showBattle(tfight& battle) = 0;
    // ... weitere Display-Methoden
};
```
**Zweck**: Trennung von Logik und Darstellung
**Bewertung**: ✅ Gut für Headless-Betrieb

#### **ContainerBase** (containerbase.h)
```cpp
class ContainerBase {
public:
    virtual bool isBuilding() const = 0;
    virtual Surface getImage() const = 0;
    
    // Cargo-Management
    const Cargo& getCargo() const;
    void addToCargo(Vehicle* veh, int position = -1);
    bool removeUnitFromCargo(Vehicle* veh);
    
    // Production
    const Production& getProduction() const;
    void addProductionLine(const VehicleType* type);
    
    // Ressourcen
    Resources getStorageCapacity() const;
    Resources getAvailableResource() const;
};
```
**Zweck**: Gemeinsame Basis für Units und Buildings
**Bewertung**: ✅ Sehr gut - vermeidet Code-Duplikation

### 3.2 **Fehlende Interfaces** ❌

#### **Kein Game State Observer/Query Interface**
```cpp
// WÜNSCHENSWERT, aber NICHT vorhanden:
class GameStateQuery {
    vector<Vehicle*> getUnitsInRange(MapCoordinate center, int range);
    vector<Building*> getBuildingsOwnedBy(PlayerID player);
    bool isFieldVisible(MapCoordinate pos, PlayerID observer);
    int getMoveCost(Vehicle* unit, MapCoordinate from, MapCoordinate to);
    vector<MapCoordinate> getAttackableFields(Vehicle* unit);
    Resources getPlayerTotalResources(PlayerID player);
};
```
**Status**: ❌ **Nicht vorhanden** - Muss manuell implementiert werden!

#### **Kein Action Interface**
```cpp
// WÜNSCHENSWERT:
class ActionInterface {
    bool canMove(Vehicle* unit, MapCoordinate to);
    bool canAttack(Vehicle* unit, MapCoordinate target);
    bool canLoad(Vehicle* unit, ContainerBase* carrier);
    void executeMove(Vehicle* unit, Path path);
    void executeAttack(Vehicle* attacker, MapCoordinate target, int weapon);
};
```
**Status**: ⚠️ **Teilweise vorhanden** via Actions-System (siehe unten), aber nicht als einheitliches Interface

#### **Kein Evaluation Interface**
```cpp
// WÜNSCHENSWERT:
class GameEvaluator {
    float evaluateBoardPosition(PlayerID forPlayer);
    float evaluateUnit(Vehicle* unit);
    float evaluateTerritoryControl(PlayerID player);
    float evaluateResourceAdvantage(PlayerID player);
};
```
**Status**: ⚠️ **Teilweise in AI-Code** (`ai/valuation.cpp`), aber nicht abstrahiert

---

## 4. Actions-System (Command Pattern) ✅

ASC verwendet ein **Command-Pattern** für alle Spielaktionen:

```
actions/ Directory:
├── action.h                    // Basis-Klasse
├── moveunitcommand.cpp/h       // Bewegung
├── attackcommand.cpp/h         // Angriff
├── loadunitcommand.cpp/h       // Einladen
├── unloadunitcommand.cpp/h     // Ausladen
├── constructunitcommand.cpp/h  // Produktion
├── repairunitcommand.cpp/h     // Reparatur
├── servicing.cpp/h             // Versorgung (Fuel/Ammo)
└── ... viele weitere
```

**Struktur**:
```cpp
class GameAction {
public:
    virtual ActionResult execute(Context& context) = 0;
    virtual void readData(tnstream& stream) = 0;
    virtual void writeData(tnstream& stream) const = 0;
    virtual ~GameAction();
};

// Context enthält:
struct Context {
    GameMap* map;
    MapDisplayInterface* display;
    bool actOnError;
    // ...
};
```

**Vorteile**: ✅
- Undo/Redo möglich
- Replay-fähig
- Network-synchronisierbar
- Modular erweiterbar

**Nachteile**: ⚠️
- **Kein einfaches Query-Interface** ("Kann ich diese Action ausführen?")
- **Keine Action-Validierung ohne Execution**
- Commands müssen manuell erstellt werden

---

## 5. KI-Relevante Code-Module

### 5.1 **Bestehende KI** (`ai/`)

```
ai/
├── ai.h / base.cpp         // Haupt-KI-Klasse
├── tactics.cpp             // Taktische Entscheidungen (Angriff, Rückzug)
├── strategy.cpp            // Strategische Planung
├── service.cpp             // Logistik (Fuel, Ammo, Repair)
├── valuation.cpp           // Bewertung von Units/Positionen
├── misc.cpp                // Hilfsfunktionen
└── legacy/                 // Alte KI-Implementierung
```

**Wichtige Klassen**:

```cpp
class AI : public BaseAI {
    // Service Orders
    class ServiceOrder {
        enum Service { srv_repair, srv_resource, srv_ammo };
        Vehicle* getTargetUnit();
        Vehicle* getServiceUnit();
        bool execute1st(Vehicle* supplier);
    };
    list<ServiceOrder> serviceOrders;
    
    // Threat-Berechnung
    int maxTrooperMove;
    int maxWeapDist[8];  // Pro Höhenstufe
    
    // Main Loop
    void run(MapDisplayInterface* display);
    void runTactics();
    void runStrategy();
    void issueServices();
};
```

**Nutzbare Komponenten**: ✅
- `valuation.cpp`: Unit-Bewertung, Threat-Analyse
- `service.cpp`: Logistik-Koordination (könnte für MCTS-Rollouts nützlich sein)
- `misc.cpp`: Pathfinding-Wrapper, Sichtbarkeits-Checks

### 5.2 **Risk Evaluator**

**WICHTIG**: Es gibt einen `risk_evaluator.cpp` (erwähnt in Dokumentation)!

**Status**: ⚠️ **Nicht im analysierten Code gefunden** - möglicherweise:
- In `ai/valuation.cpp` integriert
- In separatem Branch
- Noch zu implementieren

**Zu klären**: Wo ist die RF/Mine-Risk-Bewertung?

---

## 6. Zugriffs-Patterns für MCTS

### 6.1 **Einfache Patterns** ✅

```cpp
// Game State kopieren - NICHT TRIVIAL! ⚠️
// GameMap hat KEINEN Copy-Constructor!
// Lösung: Serialisierung oder Delta-Tracking

// Alle eigenen Units iterieren
GameMap* map = /* ... */;
Player& player = map->getPlayer(playerID);
for (auto it = player.vehicleList.begin(); it != player.vehicleList.end(); ++it) {
    Vehicle* unit = *it;
    // Process unit
}

// Feld-Nachbarn prüfen (Hex-Grid)
for (int dir = 0; dir < 6; ++dir) {
    int nx = x + getnextdx(dir, y);
    int ny = y + getnextdy(dir);
    MapField* neighbor = map->getField(nx, ny);
    if (neighbor) {
        // Process neighbor
    }
}
```

### 6.2 **Komplexe Patterns** ⚠️

```cpp
// Alle möglichen Moves für Unit ermitteln
// KEIN Convenience-Wrapper! Muss manuell via AStar:
AStar3D ast;
ast.init(map, unit, unit->getMovement());
ast.findallfields(false);  // Findet alle erreichbaren Felder
// DANN: ast.path enthält gefundene Pfade

// Alle möglichen Angriffe ermitteln
// MANUELL:
vector<AttackOption> getAttackOptions(Vehicle* unit) {
    vector<AttackOption> options;
    for (int weapon = 0; weapon < unit->typ->weapons.count; ++weapon) {
        int range = unit->typ->weapons.weapon[weapon].maxdistance;
        // Manuell alle Felder im Range scannen
        for (int dx = -range; dx <= range; ++dx) {
            for (int dy = -range; dy <= range; ++dy) {
                int tx = unit->xpos + dx;
                int ty = unit->ypos + dy;
                if (beeline(unit->xpos, unit->ypos, tx, ty) <= range) {
                    MapField* target = map->getField(tx, ty);
                    if (target && target->vehicle) {
                        AttackWeap* atw = attackpossible(unit, tx, ty);
                        if (atw->count > 0) {
                            // Attack ist möglich
                            options.push_back({tx, ty, weapon});
                        }
                        delete atw;
                    }
                }
            }
        }
    }
    return options;
}
```

**Bewertung**: ❌ **Viel Boilerplate-Code nötig**

---

## 7. Empfehlungen für MCTS-Integration

### 7.1 **Was ist gut nutzbar** ✅

1. **GameMap als Zugriffspunkt**: Zentrale, gut strukturierte API
2. **Player-Listen**: Direkte Iteration über Units/Buildings
3. **Action-System**: Modular, replay-fähig
4. **Kampf-Simulationen**: Präzise, deterministisch
5. **Bestehende KI-Komponenten**: Valuation, Service-Koordination

### 7.2 **Was muss neu gebaut werden** ❌

1. **Game State Cloning**: 
   - GameMap hat keinen Copy-Constructor
   - **Lösung**: Serialisierung oder Delta-Tracking System
   
2. **Move Generation Interface**:
   ```cpp
   class MoveGenerator {
       vector<Move> getAllLegalMoves(GameMap* state, PlayerID player);
       Move getRandomMove(GameMap* state, PlayerID player);
   };
   ```

3. **Fast State Evaluation**:
   ```cpp
   class FastEvaluator {
       float evaluate(GameMap* state, PlayerID player);
       // Heuristik: Material, Territory, Mobility
   };
   ```

4. **Visibility/Information Set Manager**:
   ```cpp
   class InformationSet {
       bool isVisible(MapCoordinate pos, PlayerID observer);
       vector<MapCoordinate> getVisibleMines(PlayerID observer);
       BeliefState getMineBeliefs();
   };
   ```

5. **Hierarchical Abstraction Layer**:
   - Grouping von Units zu "Kampfgruppen"
   - Sektor-basierte Planung
   - High-level Action-Space

### 7.3 **Kritische Lücken** ⚠️

1. **Performance**:
   - Keine Spatial Indexing (alle Searches sind linear über Listen)
   - Kein Caching von teuren Berechnungen (z.B. Sichtbarkeit)
   
2. **Parallelisierung**:
   - GameMap ist NICHT thread-safe
   - Kein Immutable State-Modell
   
3. **Determinismus**:
   - ✅ Kampf ist deterministisch
   - ⚠️ Aber: Iterationsreihenfolge über `list<>` könnte problematisch sein

---

## 8. Code-Qualität & Wartbarkeit

### 8.1 **Positiv** ✅

- **Gute Struktur**: Klare Trennung (Units, Buildings, Map, Player)
- **Dokumentation**: Doxygen-Kommentare vorhanden
- **Tests**: `unittests/` Directory mit Testfällen
- **Modular**: Actions-System, Type-System (Ruleset-basiert)

### 8.2 **Negativ** ⚠️

- **Legacy-Code**: Mix aus altem (1998) und neuem Code
- **Pointer-Heavy**: Viel raw pointers, wenig smart pointers
- **German/English Mix**: Variablennamen teilweise deutsch
- **Globale Funktionen**: Viele Utilities sind global, nicht in Klassen
- **Kein const-correctness**: Viele Methoden sollten `const` sein

### 8.3 **Risiken für MCTS** ⚠️

- **Memory Leaks**: `delete atw` - manuelles Memory-Management
- **State Mutation**: Viele In-Place-Modifikationen
- **Hidden Dependencies**: Globale State (z.B. Fog-of-War in separaten Arrays)

---

## 9. Nächste Schritte

### Für MCTS-Entwicklung empfohlen:

1. ✅ **Wrapper-Klasse schreiben**:
   ```cpp
   class GameStateWrapper {
       GameMap* map;
       PlayerID perspective;
       
       vector<Action*> getLegalActions();
       void applyAction(Action* action);
       GameStateWrapper* clone();  // Deep copy
       float evaluate();
   };
   ```

2. ✅ **Move Generator implementieren**:
   - Basis: Iteration über `player.vehicleList`
   - Nutze bestehende `attackpossible()`, AStar
   - Caching von teuren Berechnungen

3. ✅ **Fast Forward Simulator**:
   - Vereinfachte Simulation für Rollouts
   - Ignoriere Grafik (Headless-Mode)
   - Skip unwichtige Details (z.B. Experience-Gain)

4. ⚠️ **Fog-of-War Management**:
   - Analyze `viewcalculation.h/cpp`
   - Baue Information-Set-Tracking
   - Implementiere Determinization für Minen

5. ⚠️ **Performance-Profiling**:
   - Benchmark State-Cloning
   - Optimize Hot-Paths
   - Consider Delta-Encoding statt Full-Copy

---

## Fazit

**Zugriff auf Game State**: ✅ **Gut strukturiert**
- GameMap als zentrale API ist exzellent
- Player-Listen, Unit-Zugriff sind praktisch

**Interfaces**: ⚠️ **Teilweise vorhanden**
- BaseAI, MapDisplayInterface gut
- **ABER**: Kein high-level Query/Action-Interface für KI

**Fehlende Komponenten**: ❌ **Erheblich**
- State Cloning
- Move Generation
- Fast Evaluation
- Information Set Management

**Empfehlung**: 
1. Baue dünne Wrapper-Schicht um bestehende Strukturen
2. Nutze Action-System, aber füge Validation-Layer hinzu
3. Implementiere eigenes State-Management (Clone/Delta)
4. Starte mit vereinfachtem Szenario (kleine Map, wenige Units)
