### Fachliche Übersicht zum Spiel "Advanced Strategic Command" (ASC) und den KI-Anforderungen

#### 1. Übersicht zum Spiel ASC
**Hintergrund und Genre**: ASC ist ein Open-Source, rundenbasiertes Strategiespiel (Turn-Based Strategy, TBS) aus dem Jahr 1998 (ursprünglich von Martin Bickel), inspiriert von der Battle Isle-Reihe (Blue Byte, 1991-1994). Es simuliert militärische Konflikte in einer futuristischen Sci-Fi-Umgebung mit hexagonalem Gitter (Hex-Grid) für taktische Tiefe. Das Spiel ist deterministisch (keine Zufallsfaktoren), was Berechenbarkeit und Fairness betont. Verfügbar für Windows, Linux und macOS; Community-gestützt mit Kampagnen und Multiplayer-Modi (Hot-Seat, PBEM). Aktuelle Versionen: 2.6 (stabil für Single-Player) und 2.8 (experimentell für Multiplayer).

**Kernmechaniken**:
- **Kampfsystem**: Vollständig berechenbar; Einheiten (z. B. Infanterie, Panzer, Flugzeuge) mit Typen-spezifischen Attributen (Reichweite, Schaden, Terrain-Boni). Besonderheiten: Reaction Fire (automatische Schüsse im Feindeszug bei Reichweite), versteckte Minen (Land/Wasser/Unterwasser, nur detektierbar durch Spezialeinheiten), multiple Höhenstufen (8 Stufen, beeinflussen Sicht und Kampfboni). Erfahrungssystem für veterane Einheiten mit progressiven Kampfboni.
- **Bewegungssystem**: **Wichtiger Unterschied zu Battle Isle**: Units haben **Movement Points** (nicht eine Bewegung pro Turn). Bewegung erfolgt **schrittweise** entlang eines Pfades, wobei jedes Feld Movement Points kostet (terrain-abhängig). Units können:
  - Mehrere Felder pro Turn bewegen (bis Movement Points aufgebraucht)
  - Zwischenstopps einlegen (z.B. um Actions auszuführen oder Reaction Fire zu vermeiden)
  - Routen wählen (Pathfinding mit A*, nicht nur direkte Bewegung)
  - Bewegung kann sogar über mehrere Turns fortgesetzt werden
  Dies ermöglicht taktisch komplexe Bewegungsplanung (z.B. "bewege 3 Felder, repariere, bewege weitere 2 Felder").
- **Basisbau und Wirtschaft**: Auf ausgewählten Karten möglich; Bau von Gebäuden (Straßen, Schienen, Versorgungsleitungen, Öl-Plattformen, Erzminen, Fabriken, Depots, Abwehrtürme). Drei Ressourcen-Typen: Energy (volatil, aus Kraftwerken), Material (aus Minen, für Konstruktion), Fuel (aus Öl-Plattformen, für Unit-Bewegung). Forschung: Tech-Tree mit Prerequisites für neue Einheiten.
- **Logistik**: Transporteinheiten für Wasser/Berge/schnelle Verlegung; Supply-Units für Munitions- und Fuel-Nachschub. Terrain-bedingte Bewegungskosten variieren stark (Straßen beschleunigen, Gebirge verlangsamen).
- **Einheiten-Domains**: Units operieren in drei Haupt-Domains (Naval/Air/Ground) mit domain-spezifischen Eigenschaften. Naval-Units haben standardmäßig Reaction Fire aktiviert. Air-Units sind hochmobil. Ground-Units sind stark terrain-abhängig.
- **Spielmodi und Skalierbarkeit**: Single-Player-Kampagnen gegen KI, Multiplayer (Hot-Seat, PBEM); Karten bis 256×256+ Felder. Fog-of-War für Unsicherheit; Einheitensets variieren je Szenario.

**Technische Basis**: Implementiert in C++; Open-Source unter GPL; Repository: https://github.com/ValHaris/asc-hq. Fokus auf Strategie ohne Echtzeit-Elemente, was MCTS-Integration ideal macht.

#### 2. KI-Anforderungen (Hochlevel-Übersicht)

**Ziel**: Überarbeitung der bestehenden regelbasierten KI zu einer **hierarchischen MCTS-basierten Variante** für bessere Skalierbarkeit und strategische Intelligenz.

**Haupt-Anforderungen**:
- **Strategische Planung**: Langfristige Ziele (Ressourcen-Sicherung, Tech-Tree-Entscheidungen, Sektor-Kontrolle)
- **Taktische Intelligenz**: Combat-Entscheidungen unter Berücksichtigung von Reaction Fire, Terrain-Boni, Flanking, Movement-Point-Management
- **Skalierbarkeit**: Effizient für große Maps (256×256+) und viele Units (50-100+) durch hierarchische Abstraktion
- **Fog-of-War Handling**: Intelligente Planung unter Unsicherheit ohne Cheating
- **Modulare Erweiterbarkeit**: Neue taktische Verhaltensweisen können einfach hinzugefügt werden

**Technische Ansätze**:
- **Hierarchische Architektur**: Strategic Layer (Gruppen + Sektoren) und Tactical Layer (einzelne Units)
- **MCTS-basiert**: Monte Carlo Tree Search mit UCT-Algorithmus für Entscheidungsfindung
- **Utility-Agent Framework**: Modulare, kombinierbare Agents für taktische Bewertungen (RF-Vermeidung, Aggressiveness, Support-Rollen, etc.)
- **Leichtgewichtige State-Snapshots**: Effizientes Kopieren statt Full-Clone für MCTS-Simulationen
- **Persistentes Memory**: Rundenübergreifende Planung und Fog-of-War-Intelligence

**Umsetzungsstrategie**: Iterativer Aufbau mit **MVP-Fokus** auf Tactical Combat; schrittweise Erweiterung um Strategic Planning, Memory-Features und Advanced Tactics.

**Detaillierte Architektur**: Siehe `docs/hierarchical_state_design.md` und `docs/implementation_roadmap.md`

---

## 3. Kritische Bewertung der Spielmechanik-Dokumentation

### Was ist klar:
- ✅ **Grundlegendes Genre und Spielart**: Rundenbasierte Strategie auf Hex-Grid, deterministisch, inspiriert von Battle Isle
- ✅ **Kampfsystem-Grundlagen**: Einheiten haben typenspezifische Attribute (Reichweite, Schaden), Terrain-Boni existieren
- ✅ **Ressourcen-Management existiert**: Gebäude, Produktion, Forschung sind Teil des Spiels
- ✅ **Fog-of-War**: Sichteinschränkungen sind implementiert
- ✅ **Spezielle Mechaniken**: Reaction Fire, Minen, multiple Höhenstufen werden erwähnt
- ✅ **Skalierung**: Große Karten (256x256+) mit Sektor-Unterteilung

### Was unklar oder unvollständig ist:

#### 3.1 Kampfsystem (KRITISCH für KI)
1. **Kampfberechnung**: Wie wird Schaden berechnet? Formel? Nur Typenvorteile oder auch Zahlen-basiert?
2. **Reaction Fire Details**: 
   - Wann genau wird RF ausgelöst? (Bewegung in Reichweite, Bewegung durch Reichweite, Angriff?)
   - Wie viele RF-Schüsse pro Runde/Einheit?
   - Verbraucht RF Munition/Bewegung?
   - Kann RF vermieden werden (Geschwindigkeit, Terrain)?
3. **Höhenstufen**: Wie viele gibt es? Wie beeinflussen sie Sicht/Angriff genau? (+/- Werte?)
4. **Minen-Mechanik**: 
   - Wie funktioniert Detektion? (Reichweite, Wahrscheinlichkeit, automatisch?)
   - Können Minen entschärft werden?
   - Schaden durch Minen? (sofortiger Tod, prozentualer Schaden?)
   - Werden ausgelöste Minen für alle sichtbar?

#### 3.2 Wirtschaftssystem
5. **Ressourcen-Typen**: Welche gibt es genau? (Material, Fuel, Energy laut TV-Tropes - korrekt?)
6. **Ressourcen-Fluss**: 
   - Wie werden Ressourcen von Minen zu Fabriken transportiert?
   - Pipeline-Mechanik im Detail (Durchsatz, Kapazität, Reichweite)?
   - Lagerlimits?
7. **Produktionskosten**: Kosten für Einheiten/Gebäude? Produktionszeit?
8. **Forschungssystem**: Wie lange dauert Forschung? Kosten? Tech-Tree-Struktur?

#### 3.3 Zugstruktur (KRITISCH für KI)
9. **Zugreihenfolge**: Gleichzeitig (simultaneous turns) oder abwechselnd (IGOUGO)? ✅ **BEANTWORTET** (siehe 4.3)
10. **Aktionen pro Einheit**: Kann eine Einheit bewegen UND angreifen? "Move after attack" erwähnt - Standard oder Feature? ✅ **BEANTWORTET** (siehe 4.3)
11. **Movement Points System**: Wie funktioniert schrittweise Bewegung genau? ✅ **BEANTWORTET** (Code-Analyse, siehe Abschnitt 1)
12. **Runden-Limit**: Gibt es Zeitlimits oder Zugbegrenzungen?

#### 3.4 Siegbedingungen & Spielziele
13. **Victory Conditions**: Welche gibt es? (HQ-Eroberung, Elimination, Punkte, Zeit?) ✅ **BEANTWORTET** (siehe 4.4)
14. **Gebäude-Eroberung**: Wie funktioniert das? (Einheit betreten, bestimmte Einheiten nötig?)
15. **Map-Events**: Wie komplex sind diese? Beeinflussen sie KI-Entscheidungen?

#### 3.5 Einheiten & Transport
16. **Transport-Mechanik**: ✅ **TEILWEISE BEANTWORTET** - Laden/Entladen funktioniert, Details erkennbar durch **Simulation mit Original-API**. **Wichtig**: Frisch entladene Units haben aktionsabhängige Restriktionen → durch Simulation evaluieren (keine Regel-Duplikation).
17. **Einheiten-Typen**: Grobe Kategorisierung bekannt (Infanterie, Fahrzeuge, Flugzeuge, Schiffe) → Details in VehicleType definitions
18. **Bewegungskosten-Details**: ✅ **Verfügbar in Unit-Informationen** (VehicleType properties). Durch **Simulation mit Original-API** nutzbar → keine manuelle Extraktion nötig.

#### 3.6 Sichtbarkeits-System
19. **Sichtweiten**: ✅ **BEANTWORTET** - Sichtweite ist **Unit-Eigenschaft** (VehicleType property). Beeinflusst durch **Terrain** (z.B. Wald reduziert Sicht) und **Jamming-Units** (siehe Frage 20).
20. **Jamming (Stealth)**: ✅ **BEANTWORTET** - Jamming ist **negative Sichtweite**. Formel: `Effektive_Sicht = Unit_Sichtweite - Jamming_Wert`. Beispiel: 40 Sicht + 40 Jamming = 0 (Unit sieht nichts). U-Boote/Speedboats nutzen vermutlich Jamming.
21. **Radar/Satelliten**: ✅ Vermutlich Units mit hoher Sichtweite oder Spezial-Properties (aus Unit-Info entnehmbar).

#### 3.7 Technische Spielmechanik
22. **Munition & Treibstoff**: ✅ **BEANTWORTET** (siehe 4.5)
23. **Reparatur**: Details vermutlich in Service-Unit properties und Gebäude-Functions (aus Unit-Info / Building-Info entnehmbar)
24. **Einheiten-Erfahrung**: ✅ **BEANTWORTET** (siehe 4.5)
25. **Wettersystem**: ✅ **BEANTWORTET** - Wetter verändert **Land- und Wasser-Terrain** (z.B. Eis, Schnee, Packeis). **Alle Units haben Properties** was sie können (z.B. "kann Eis brechen", "kann durch Packeis fahren um Rinnen zu bilden"). **Aus Unit-Informationen entnehmen**, nicht hardcoden.

### Empfohlene Nächste Schritte:
1. ✅ **Code-Analyse**: Untersuchung von combat.cpp, movement.cpp, resources.cpp für genaue Mechaniken
2. **Regelwerk-Dokumentation**: Suche nach ASC-Wiki, Handbuch, oder Ruleset-Dateien (.asctxt)
3. **Referenzpartie**: Test-Spiel spielen und Mechaniken dokumentieren
4. **Entwickler-Konsultation**: Bei unklaren Design-Entscheidungen den Original-Entwickler fragen

---

## 4. Code-Analyse Ergebnisse: Geklärte Mechaniken

**Quellen**: `attack.cpp`, `attack.h`, `reactionfire.cpp`, `typen.h`, `gamemap.h`, `turncontrol.cpp`, `explosivemines.h`, `player.h`, `vehicle.h`

### 4.1 Kampfsystem (BEANTWORTET)

#### **Kampfberechnung (Frage 1)** ✅
**Quelle**: `attack.cpp`, Zeilen 179-276, Funktion `tfight::calc()`

**Schadensformel**:
```
AbsoluteStrength = BaseStrength 
                   × (1 + ExperienceBonus + AttackBonusFromTerrain) 
                   × DamageFactor 
                   × HemmingFactor (Flanking)

AbsoluteDefense = (Armor / 5.0) 
                  × (1 + ExperienceBonus + DefenseBonusFromTerrain)

NewDamage = OldDamage + ⌈(AbsoluteStrength / AbsoluteDefense) × 1000 / AttackPowerParameter⌉
```

**Details**:
- **Erfahrung**: Exponentielles Wachstum bis max 1.000.000 XP
  - Formel: `maxBonus × (1 - (0.1^(1/ninety))^experience)`
  - Standard: 90% des Max-Bonus bei ~90 XP (konfigurierbar via GameParameter)
  - Max Attack Bonus: konfigurierbar, typ. +50-100%
  - Max Defense Bonus: konfigurierbar, typ. +50-100%
- **Terrain-Bonus**: `/8` der Werte (z.B. Bonus 8 = +100%)
- **Damage-Faktor**: `1 - (2 × damage / 300)` (bei 150 Damage = 0% Effektivität)
- **Hemming (Flanking)**: Faktor 1.0 bis 2.4 abhängig von Einheiten auf benachbarten Feldern
  - Maximaler Bonus: +140% bei vollständiger Umzingelung
  - Nur für Bodeneinheiten (height < `chtieffliegend`)
- **Armor Divisor**: Konstant 5.0
- **Weapon Strength**: Basisschaden × Wettermodifikator × Distanzmodifikator × Zielgenauigkeit

**Reziproke Angriffe**: Defender kann zurückschießen wenn er Waffe mit Reichweite hat

#### **Reaction Fire (Frage 2)** ✅
**Quelle**: `reactionfire.cpp`, `vehicle.h` Zeilen 142+

**Mechanik**:
- **Auslöser**: Einheit bewegt sich **in sichtbare Reichweite** einer feindlichen Einheit
- **Bedingung**: `reactionfire.status >= ready` UND Einheit hat Munition UND Sichtbarkeit
- **Schüsse**: Pro Waffe einzeln definiert via `weaponShots[]` Array
  - Jede Waffe kann RF separat aktiviert/deaktiviert haben
  - RF verbraucht reguläre Munition (`weaponShots[weaponNum]--`)
- **Limitierung**: RF-fähige Einheiten werden in Suchradius gescannt (max weapon range pro Höhenstufe)
- **Vermeidung**: 
  - Durch unsichtbare Bewegung (Fog-of-War, Stealth)
  - Außerhalb Reichweite bleiben
  - Fliegende Einheiten (height >= `chtieffliegend`) lösen kein Hemming aus

**Implementierung**: `tsearchreactionfireingunits` Klasse scannt alle möglichen RF-Einheiten

#### **Höhenstufen (Frage 3)** ✅
**Quelle**: `typen.h` Zeile 67

**Anzahl**: `choehenstufennum = 8` Höhenstufen (0-7)

**Konstanten** (aus Code):
- `chtieffliegend`: Grenze für fliegende Einheiten
- `chfahrend`: Grenze für bodenbasierte Einheiten

**Effekte**:
- Beeinflusst Sichtlinien (Line-of-Sight Berechnungen)
- Terrain-Boni gelten NUR für Bodeneinheiten (`height <= chfahrend`)
- Waffen haben `targ`-Bitmaske für targetierbare Höhen
- Waffenstärke variiert mit Distanz UND Höhenunterschied (siehe `WeapDist::getWeaponStrength()`)

#### **Minen (Frage 4)** ✅
**Quelle**: `explosivemines.h`, `attack.h`

**Typen**: `enum MineTypes` - Land/Wasser/Unterwasser-Minen

**Mechanik**:
- **Struct**: `Mine` hat `type`, `strength`, `player`, `identifier`
- **Angriff**: `attacksunit(Vehicle*)` prüft ob Mine Einheit angreift
- **Detektion**: Unit-spezifisch (muss im VehicleType definiert sein)
- **Sichtbarkeit**: Minen sind normalerweise unsichtbar, außer für Spieler mit Detektoren
- **Schaden**: Direkter Schaden via `strength` Wert (analog zu Waffenschaden)
- **Entschärfung**: Code deutet auf Removal via `RemoveMine` Action hin

### 4.2 Wirtschaftssystem (BEANTWORTET)

#### **Ressourcen-Typen (Frage 5)** ✅
**Quelle**: `typen.h` Zeilen 76-143

**Definitiv 3 Ressourcen**:
```cpp
class Resources {
    int energy;    // Volatile, nicht speicherbar (Kraftwerke)
    int material;  // Abbau aus Minen, für Konstruktion
    int fuel;      // Öl-Plattformen, für Unit-Bewegung
}
```

**Modi**:
- `_resourcemode == 0`: "ASC Mode" - komplexes System mit Mineralien, Pipelines
- `_resourcemode == 1`: "BI Mode" - vereinfachtes Battle Isle System

#### **Ressourcen-Fluss (Frage 6)** ✅
**Quelle**: `resourcenet.h/cpp`

**Mechanik**: **Sobald eine Pipeline-Verbindung existiert, sind alle Ressourcen in allen verbundenen Gebäuden verfügbar**.

**Details**:
- Keine Durchsatz-Limitierung oder Kapazitätsgrenzen für Pipelines
- Pipeline-Netzwerk ist binär: verbunden = alle Ressourcen geteilt, nicht verbunden = isoliert
- Pipelines verbinden Gebäude (Minen → Fabriken → Lager)
- Tribute-System für Ressourcentransfer zwischen Spielern (siehe `GameMap::ResourceTribute`)

**Lager-Limits**:
- Jedes Gebäude hat **spezifische Lagerkapazitäten** (Material, Fuel, Energy) in BuildingType properties
- **Depots**: Viel Material + Fuel, wenig Energy
- **Energie-Erzeuger/Batterien**: Mehr Energy-Storage
- Nicht jedes Gebäude kann alle Ressourcen abgeben

**KI-Implikation**: 
- Strategic Layer muss Pipeline-Konnektivität tracken (welche Gebäude sind im Netzwerk?)
- Resource-Management: Lagerkapazitäten berücksichtigen, ggf. mehr Depots/Batterien bauen

#### **Produktionskosten (Frage 7)** ✅
**Quelle**: VehicleType/BuildingType definitions in `.asctxt` Ruleset-Files

**Unit-Produktion**:
- **Bauen ist instant** (keine Produktionszeit)
- **Units haben Movement Points erst in der nächsten Runde**
- **Ausnahme**: Frisch gebaute Units können sofort in Transporter geladen und bewegt werden (Transporter-Movement, nicht Unit-Movement)
- Kosten definiert in VehicleType (Material, Fuel, Energy)

**Gebäude-Bau**:
- **Bauen verbraucht ALLE Movement Points der Builder-Unit** im aktuellen Turn
- Builder-Unit muss Runde VOR dem Bau richtig positioniert sein
- Gebäude sind instant fertig

**KI-Implikation**: Tactical MCTS muss berücksichtigen, dass Builder nach Gebäude-Bau immobil ist. Strategic Layer plant Builder-Positioning einen Turn im Voraus.

#### **Forschung (Frage 8)** ✅
**Quelle**: `research.h`, `research.cpp`

**Mechanik**:
- `class Research` verwaltet Tech-Tree
- Status: `Researched, Available, UnavailableNow, NeverAvailable`
- Tech-Tree mit Prerequisites (`BlockingTechnologies` für mutuelle Exklusion)
- Root-Technologie schaltet Unitsets frei
- **Forschung erfordert Labor-Gebäude und Ressourcen-Investition**
- Details zu Forschungszeit/Kosten in `.asctxt` Ruleset-Files

**Forschungs-Beschleunigung**:
- **Option 1**: Exponentiell wachsender Ressourcen-Verbrauch (mehr Ressourcen → schneller)
- **Option 2**: Mehrere Forschungseinrichtungen bauen
- **Mechanik**: Forschungseinrichtungen produzieren "Punkte", die pro Tech-Level verbraucht werden

**KI-Implikation**: 
- Strategic Layer muss Tech-Tree-Pfade evaluieren und Research-Queue priorisieren
- **Neue API nötig**: Forschungs-Status der KI offenlegen (Research-Points, aktive Forschungen, etc.)
- **Post-MVP**: Kommt nach Combat-MVP, da komplex

### 4.3 Zugstruktur (BEANTWORTET)

#### **Zugreihenfolge (Frage 9)** ✅
**Quelle**: `turncontrol.cpp` Zeilen 112-200

**IGOUGO (I Go, You Go)** - Sequentiell, NICHT simultan!

**Ablauf**:
1. `beginTurn()` - Player startet Turn (Vehicle::beginTurn() für alle Units)
2. Player macht alle Moves
3. `endTurn()` - Player beendet Turn
4. `findNextPlayer()` sucht nächsten aktiven Spieler (0-7, loop)
5. Wenn `nextPlayer <= currentPlayer` → `endRound()` (neue Runde)
6. AI-Spieler werden automatisch via `runai()` abgewickelt

**Wichtig für MCTS**: Keine simultanen Züge, aber **Reaction Fire** simuliert Interaktion!

#### **Aktionen pro Einheit (Frage 10)** ✅
**Quelle**: `vehicle.h`, `attack.cpp`

**Standard**: Einheit kann bewegen UND angreifen (in dieser Reihenfolge)

**Spezialfähigkeit**: "Move after attack" ist ein **Feature** (nicht Standard)
- Definiert in VehicleType
- Erlaubt Bewegung NACH Angriff (Hit-and-Run)
- Nur für spezielle Einheiten (Flugzeuge, U-Boote, Speedboats)

**Restriktionen**:
- `attacked` Flag markiert Unit als "bereits angegriffen"
- `_movement` Ressource limitiert Bewegung
- Fuel-Verbrauch kann Movement weiter einschränken

#### **Runden-Limit (Frage 11)** ⚠️
**Quelle**: `GameMap::time` (GameTime struct)

**Bekannt**:
- Zeit gemessen in `turns` und `moves`
- Keine harten Limits im Code gefunden
- Map-Events können zeitbasierte Trigger haben

### 4.4 Siegbedingungen (BEANTWORTET)

#### **Victory Conditions (Frage 12)** ✅
**Quelle**: `turncontrol.cpp` Zeilen 378-443, `checkforvictory()`

**Standard-Siegbedingung: Elimination**
```cpp
if ( plnum <= 1 ) {
    displaymessage("Congratulations!\nYou won!");
}
```

**Mechanik**:
- Spieler gilt als besiegt wenn `!player.exist()` (keine Units/Buildings)
- `existanceAtBeginOfTurn` Flag trackt Elimination
- Gewinner = letzter überlebender Spieler

**Weitere Bedingungen**:
- Campaign-Modus hat spezielle Siegbedingungen (nicht detail analysiert)
- Map-Events können alternative Win-Conditions definieren
- `GameMap::continueplaying` erlaubt Weiterspielen nach Sieg

#### **Gebäude-Eroberung (Frage 13)** ✅
**Quelle**: `ContainerBase::capture()`, VehicleType properties

**Mechanik**:
- **Im Allgemeinen Infanterie**, aber jede Unit, die erobern kann, hat dies als **Property in der Unit-Beschreibung**
- **Eroberung ist instant** (keine Dauer)
- **Man kann erobern UND in derselben Runde bauen** (wenn Ressourcen existieren)
- Gebäude haben `owner` Feld

**KI-Implikation**: 
- Tactical MCTS muss Units mit Capture-Property identifizieren
- Strategic Layer kann Building-Capture → sofortige Production planen

#### **Map-Events (Frage 15)** ✅
**Quelle**: `gameevents.h`, `gameeventsystem.h`

**Mechanik**: Event-System mit Triggern, Conditions, Actions
- Kann Reinforcements spawnen
- Kann Allianzen ändern
- Kann Siegbedingungen modifizieren
- **Beispiel**: Vulkanausbrüche ändern Terrain dynamisch

**KI-Strategie für MVP**: 
- **Initial nicht betrachten** (zu komplex)
- **On-demand Reaktion**: Wenn Terrain sich ändert (z.B. Vulkan), reagiert KI auf geändertes Terrain (Terrain-Analyse-Update)
- Keine Vorhersage von Events nötig

**Post-MVP**: Event-System kann beobachtet werden für proaktive Planung

### 4.5 Technische Details

#### **Munition & Treibstoff (Frage 21)** ✅
**Quelle**: `vehicle.h` Zeilen 89-90, 128-129

**Munition**: `int ammo[16]` - Pro Waffe separates Ammo-Tracking

**Treibstoff**: `Resources tank` - Container mit `fuel` Resource

**Nachfüllung**:
- Service-Units (siehe `ai/service.cpp`)
- Gebäude mit Repair/Rearm-Funktion
- Pipeline-Netzwerk für statische Versorgung

#### **Erfahrung (Frage 23)** ✅
**Quelle**: `vehicle.h` Zeilen 93-105, `attack.cpp` Zeilen 213-269

**Veteranen-System**: Ja!
- `experience_offensive` und `experience_defensive` getrennt
- Maximum: `maxunitexperience = 1.000.000`
- Gain bei Kampf: +1 base, +1 wenn Nahkampf (dist<=10), +1 wenn Defender Strength>0
- Bonus wirkt multiplikativ auf Strength/Defense
- 24 Experience Icons für visuelle Darstellung

### 4.6 Offene Fragen & Annahmen

#### **Verbleibende offene Punkte** (niedrige Priorität):

**Wirtschaftssystem**:
- ✅ Pipeline-Mechanik geklärt (binäre Konnektivität)
- ✅ Produktions-Mechanik geklärt (instant, aber Movement-Restrictions)
- ✅ Lager-Limits geklärt (Gebäude-spezifisch, aus BuildingType properties)

**Bewegung & Sichtbarkeit**:
- ✅ Bewegungskosten verfügbar in Unit-Properties (Simulation mit Original-API nutzen)
- ✅ Sichtweiten-Mechanik geklärt (Unit-Property, Terrain-Einfluss, Jamming)
- ✅ Jamming-Mechanik geklärt (negative Sichtweite)

**Gameplay-Mechaniken**:
- ✅ Capture-Mechanik geklärt (Unit-Property, instant)
- ✅ Weather-System geklärt (Terrain-Änderungen, Unit-Properties nutzen)
- ✅ Map-Events Strategie geklärt (MVP: on-demand Reaktion, keine Vorhersage)

**Detail-Fragen** (geklärt):
- ✅ **Reparatur**: Service-Units kosten **keine Movement Points**, aber **Material + Fuel**. **Vollständige Reparatur nur in Depots/Fabriken** möglich.
- ✅ **Forschung**: Beschleunigung durch exponentiellen Ressourcen-Verbrauch oder mehrere Labs. **Neue API nötig** (Post-MVP).
- ✅ **Lager-Limits**: Gebäude-spezifisch (BuildingType properties). Depots: viel Material/Fuel, wenig Energy.
- ⚠️ **Radar/Satelliten**: Vermutlich Units mit hoher Sichtweite oder Spezial-Properties (aus Unit-Info entnehmbar).

#### **Getroffene Annahmen** (für MVP-Entwicklung):

**Terrain-Analyse**:
- ✅ **Annahme**: KI darf beim Map-Start komplette Terrain-Daten analysieren (ohne Fog-of-War), da Terrain statisch ist und ein menschlicher Spieler diese Info schnell durch Scouting hätte.
- ✅ **Annahme**: Units, Gebäude, Minen bleiben im Fog-of-War verborgen (kein Cheat).
- ✅ **Annahme**: Terrain kann sich ändern (Gegner baut Infrastruktur) → inkrementelle Updates der Terrain-Statistiken.

**Unit-Gruppierung**:
- ✅ **Annahme**: Naval-Units gruppieren sich primär nach räumlicher Nähe (RF-Synergien nutzen).
- ✅ **Annahme**: Air-Units sind flexibel reassignable (können verschiedene Boden-/Naval-Gruppen unterstützen).
- ✅ **Annahme**: Ground-Units gruppieren sich terrain-basiert (Panzer nur für offenes Gelände, Infanterie für Städte/Wälder, Züge nur für Bahnlinien).
- ✅ **Annahme**: Gruppen sind dynamisch re-gruppierbar (bei Verlusten, Terrain-Wechsel, Strategic-Shifts).

**Combat & Tactics**:
- ✅ **Annahme**: Reaction Fire ist die kritischste taktische Überlegung → hohe Gewichtung im Utility-Agent-System.
- ✅ **Annahme**: Flanking (Hemming) bietet signifikante Vorteile → FlankingAgent wird implementiert.
- ✅ **Annahme**: Support-Rollen (Repair, Supply) sind essentiell für längere Kampagnen → SupportRoleAgent wird implementiert.
- ✅ **Annahme**: Terrain-Boni sind stark genug, um Movement-Entscheidungen zu beeinflussen → TerrainAdvantageAgent wird implementiert.

**Strategic Planning**:
- ✅ **Annahme**: Gegner hat äquivalente Start-Komposition (symmetrische Maps) → Early-Game-Strategy basiert auf eigener Komposition.
- ✅ **Annahme**: Unit-Type-Decisions sind multi-faktoriell (Terrain 40%, Gegner-Strength 30%, Eigene Start-Units 20%, Objective 10%).
- ✅ **Annahme**: Gewichtungen ändern sich über Spiel-Phase (Early-Game: Terrain-fokussiert, Mid-Game: Gegner-reaktiv, Late-Game: Objective-fokussiert).

**Performance & Skalierung**:
- ✅ **Annahme**: Snapshot-Erstellung kann auf <5ms optimiert werden (selektives Kopieren, kein Full-Clone).
- ✅ **Annahme**: Utility-Agents müssen <0.1ms pro Action-Evaluation sein (sonst Performance-Problem bei MCTS).
- ✅ **Annahme**: Sektor-basierte Abstraktion (256×256 → 64 Sektoren) ist ausreichend für Strategic Layer.
- ✅ **Annahme**: Tactical MCTS für 10-15 Units mit 500 Iterationen ist in <1 Sekunde machbar.

**Simulation & Regel-Extraktion**:
- ✅ **Prinzip**: **Simulation mit Original-API** statt Regel-Duplikation
  - Bewegungskosten: Aus Unit-Properties + Simulation ermitteln
  - Transport-Restriktionen: Durch Simulation evaluieren (keine Hardcoding)
  - Capture-Fähigkeiten: Aus Unit-Properties lesen
  - Weather-Effekte: Aus Unit-Properties (z.B. "kann Eis brechen")
- ✅ **Vorteil**: Regeln bleiben synchron mit ASC-Core, keine Wartungs-Duplikation
- ✅ **Performance**: Simulation nur während MCTS-Rollouts (akzeptabler Overhead)

**Vereinfachungen für MVP**:
- ⚠️ **Vereinfachung**: Strategic Layer ist initial Dummy (gibt immer "Attack nearest enemy" Objective).
- ⚠️ **Vereinfachung**: Memory-System ist initial minimal (nur letzte Enemy-Positionen, keine Threat-Heatmap).
- ⚠️ **Vereinfachung**: Gruppen-Bildung ist initial trivial (alle Units = 1 Gruppe).
- ⚠️ **Vereinfachung**: Nur 5 Utility-Agents im MVP (LegalMove, RF, Aggressiveness, TargetPriority, Material).

Diese Vereinfachungen werden nach erfolgreichem MVP schrittweise durch vollständige Implementierungen ersetzt (siehe `docs/implementation_roadmap.md`).
