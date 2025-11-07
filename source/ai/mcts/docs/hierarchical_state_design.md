# Hierarchische MCTS-Architektur für ASC

**Zweck**: Design-Dokumentation einer mehrstufigen MCTS-Architektur, die durch Gruppierung den Suchraum reduziert und durch persistenten Memory-State strategische Langfrist-Planung ermöglicht.

---

## 1. Motivation und Grundproblem

### Das Skalierungsproblem

Advanced Strategic Command stellt für klassisches MCTS eine enorme Herausforderung dar: Eine typische Map mit 256×256 Feldern und 100 aktiven Einheiten erzeugt einen Suchraum, der praktisch nicht exploriert werden kann. Wenn jede Einheit im Durchschnitt 50 mögliche Aktionen hat (Bewegung zu verschiedenen Feldern, Angriffe mit verschiedenen Waffen, Laden/Entladen), ergibt sich ein kombinatorischer Branching-Factor, der selbst mit modernen MCTS-Optimierungen nicht handhabbar ist.

Hinzu kommt, dass ein vollständiger Clone des GameMap-Objekts für jede MCTS-Simulation zwischen 10 und 50 MB Speicher benötigt. Bei tausenden Simulationen pro Zug würde dies den verfügbaren RAM sprengen.

### Die hierarchische Lösung

Die vorgeschlagene Architektur begegnet diesem Problem durch zwei komplementäre Ansätze:

1. **Hierarchische Dekomposition**: Die Entscheidungsfindung wird in strategische und taktische Ebenen aufgeteilt, wobei jede Ebene mit einem stark reduzierten Suchraum arbeitet.

2. **Leichtgewichtige State-Representation**: Statt vollständiger GameMap-Clones werden kompakte Snapshots verwendet, die nur die für die jeweilige Entscheidungsebene relevanten Informationen enthalten.

---

## 2. Hierarchische Gruppen-Architektur

### 2.1 Das Konzept der Unit-Gruppen

Statt einzelne Einheiten zu betrachten, werden diese zu funktionalen Gruppen zusammengefasst. Jede Gruppe hat einen klar definierten Zweck und operiert innerhalb eines räumlich begrenzten Bereichs.

**Gruppen-Typen** könnten umfassen:

- **Combat Groups**: Offensive Kampfverbände mit gemischter Zusammensetzung (Panzer, Infanterie, Luftunterstützung)
- **Defense Groups**: Stationäre oder mobile Verteidigungseinheiten zur Sicherung wichtiger Positionen
- **Reconnaissance Groups**: Aufklärungseinheiten zur Fog-of-War-Erkundung
- **Logistics Groups**: Versorgungseinheiten und Transporter
- **Construction Groups**: Builder-Einheiten für Infrastruktur
- **Special Operations**: Spezialisierte Einheiten für taktische Schläge

### 2.2 Gruppen-Eigenschaften und -Beschränkungen

Jede Gruppe wird charakterisiert durch:

**Zuordnung und Identifikation**:
- Eine eindeutige Gruppen-ID für Referenzierung
- Liste der zugeordneten Einheiten (gespeichert nur als IDs, nicht als Kopien)
- Gruppen-Typ zur Bestimmung des Verhaltens

**Räumliche Begrenzung**:
- Zugewiesener Map-Sektor als primäres Operationsgebiet
- Maximale Entfernung vom Sektor-Zentrum
- Diese Beschränkung reduziert drastisch die zu prüfenden Bewegungsoptionen

**Operatives Ziel**:
- Aktuelles Haupt-Objektiv (z.B. "Erobere Sektor Nord", "Verteidige Ressourcen-Zone")
- Geschätzte Erfolgswahrscheinlichkeit
- Erwartete Anzahl Runden bis zur Zielerreichung
- Verknüpfung zu anderen Gruppen (z.B. Escort-Missions)

**Cached Metriken** zur schnellen Evaluation:
- Gesamtstärke der Gruppe
- Durchschnittliche Gesundheit
- Geometrischer Schwerpunkt (Center of Mass)
- Versorgungsstatus
- Verstärkungsbedarf

### 2.3 Leichtgewichtige Snapshots

Ein zentrales Konzept ist die Snapshot-Funktion: Statt die komplette GameMap zu klonen, extrahiert eine Gruppe nur die minimal notwendigen Daten ihrer Einheiten. Ein Snapshot enthält pro Einheit lediglich:

- Einheiten-ID
- Position
- Gesundheitspunkte
- Verbleibende Bewegung
- Angriffsstatus

Für eine Gruppe von 10 Einheiten bedeutet dies etwa 200-300 Bytes statt mehrerer Kilobytes pro Einheit bei vollständiger Kopie. Der entscheidende Vorteil: Snapshots können schnell erstellt und kopiert werden, ohne das originale GameMap-Objekt zu berühren.

### 2.4 Sektor-basierte Map-Unterteilung

Die Spielfeld-Map wird in Sektoren unterteilt (typischerweise 32×32 oder 64×64 Felder pro Sektor). Dies dient mehreren Zwecken:

**Räumliche Abstraktion**:
- Strategische Entscheidungen operieren auf Sektor-Ebene, nicht auf Feld-Ebene
- Reduktion der Map-Komplexität von 65.000+ Feldern auf etwa 60-200 Sektoren
- Eine 256×256 Map wird so zu einem 8×8 oder 4×4 Sektor-Grid

**Eigentums- und Kontrollverfolgung**:
- Jeder Sektor hat einen kontrollierenden Spieler (oder ist umkämpft/neutral)
- Kontrollstärke als Wert zwischen 0 und 1
- Diese Information ist zentral für strategische Bewertungen

**Strategische Bewertung**:
- Ressourcen-Wert: Geschätzte Resource-Produktion pro Runde
- Militärischer Wert: Defensive Stärke, vorhandene Garnisonen
- Konnektivität: Verbindung zu anderen eigenen Sektoren
- Infrastruktur: Fabriken, HQ, Forschungslabore

**Sichtbarkeits-Caching**:
- Pro Spieler wird gespeichert, ob der Sektor vollständig, teilweise oder gar nicht sichtbar ist
- Vermeidet wiederholte Line-of-Sight-Berechnungen
- Deutliche Performance-Verbesserung bei Fog-of-War

**Bedrohungs-Tracking**:
- Aktuelles Bedrohungslevel basierend auf gesichteten Feinden
- Diese Information kommt aus dem persistenten Memory-System (siehe Abschnitt 3)
- Erlaubt dynamische Risikobewertung ohne vollständige Feind-Information

### 2.5 Intelligente Gruppen-Bildung

Eine der kritischsten Entscheidungen ist die **initiale und dynamische Gruppierung** von Einheiten. Diese ist hochgradig abhängig von **Unit-Typ**, **Terrain** und **strategischer Situation**.

#### 2.5.1 Domain-spezifische Gruppierungs-Regeln

**Wasser-Einheiten (Naval Domain)**:

Wasser-Einheiten haben spezielle Eigenschaften, die ihre Gruppierung stark beeinflussen:

- **Räumliche Nähe ist kritisch**: Alle nahen Wasser-Einheiten sollten grundsätzlich zusammen gruppiert werden
- **Reaction Fire Synergien**: Naval Units haben standardmäßig RF aktiviert → Clustering maximiert gegenseitige Deckung
- **Keine Blocker-Rolle nötig**: Fast alle Naval Units haben hohe Reichweiten, daher ist eine Frontline-Formation nicht nötig
- **Homogene Mobilität**: Alle bewegen sich in Wasser mit ähnlicher Geschwindigkeit

**Gruppierungs-Heuristik für Naval**:
1. Finde alle eigenen Wasser-Einheiten
2. Clustere nach räumlicher Nähe (z.B. alle innerhalb 10 Hex)
3. Jedes Cluster wird eine Gruppe
4. Ausnahme: U-Boote mit Stealth können eigene Gruppen bilden (Independent Operations)

**Präferenzen**:
- Große Naval-Gruppen (10-15 Units) sind effektiv durch RF-Synergien
- Units mit Anti-Air-Fähigkeiten sollten verteilt sein (decken mehr Luftraum)
- Carrier-Units (wenn vorhanden) sind Zentrum der Gruppe

---

**Luft-Einheiten (Air Domain)**:

Luft-Einheiten haben fundamental andere Anforderungen:

- **Hohe Mobilität**: Können schnell überall hingelangen → Gruppierung weniger wichtig
- **Flexible Einsetzbarkeit**: Können verschiedene Boden-/Naval-Gruppen unterstützen
- **Aufklärungs-Flugzeuge**: Typischerweise **immer independent** (Recon Group mit 1 Unit)

**Gruppierungs-Heuristik für Air**:
1. **Reconnaissance Units**: Eigene 1-Unit-Gruppen, Objective = Scouting
2. **Fighter Jets**: Können zu 2-4er Gruppen zusammengefasst werden (Air Superiority Patrol)
3. **Bomber/Ground Attack**: Separate Gruppe oder attached zu Boden-Gruppen als "Support"
4. **Defensive Air (Anti-Air)**: Kann zu kritischen Sektoren assigned werden

**Besonderheit**: Air-Units müssen nicht permanent einer Gruppe zugeordnet sein. Sie können **dynamisch reassigned** werden:
- Strategic Layer entscheidet: "Air Group A unterstützt Ground Group B in dieser Runde"
- Nächste Runde: "Air Group A unterstützt Naval Group C"

---

**Land-Einheiten (Ground Domain)**:

Land-Einheiten sind am komplexesten, da **Terrain massiv** die Gruppierung beeinflusst:

**Terrain-bedingte Einschränkungen**:
- **Züge**: Können NUR auf Schienen fahren → bilden eigene "Rail Transport Groups"
- **Schwere Panzer**: Nicht durch Wald, nicht über hohe Hügel → nur für offenes Terrain-Gruppen
- **Infanterie**: Kann überall hin, aber langsam → braucht Transport oder operiert lokal
- **Leichte Fahrzeuge**: Flexibel, können in verschiedenen Gruppen

**Absolute Hindernisse** (kritisch für Gruppierung):
- **Tank Blocker**: Panzer können bestimmte Gebiete nicht betreten → Panzer-Gruppen müssen um diese herum operieren
- **Stacheldraht**: Blockiert Infanterie → Infanterie-Gruppen brauchen Engineer zum Räumen
- **Befestigungen**: Bieten enormen Verteidigungs-Bonus → Defense-Groups sollten sich dort positionieren

**Gruppierungs-Heuristik für Ground**:

1. **Terrain-Analyse durchführen** (siehe 2.5.2)
2. **Identifiziere "Operationsräume"**: Zusammenhängende Terrain-Bereiche, die für bestimmte Unit-Typen zugänglich sind
3. **Matche Units zu Operationsräumen**:
   - Panzer → Offene Ebenen, Straßen
   - Infanterie → Städte, Wälder, Verteidigungslinien
   - Züge → Rail-Netzwerk
4. **Bilde Mixed-Role-Gruppen** innerhalb Operationsräumen:
   - Combat Group: Panzer + Infanterie (in APCs) + Artillery + Supply
   - Wenn Terrain heterogen ist: Mehrere spezialisierte Gruppen

**Beispiel - Gebirgspass-Szenario**:
- Terrain: Schmaler Pass zwischen Bergen, Ebene auf beiden Seiten
- Gruppierung:
  - Group A: Schwere Panzer auf Ebene Ost
  - Group B: Infanterie + leichte Fahrzeuge im Pass (einziger Durchgang)
  - Group C: Artillery auf erhöhter Position mit Sicht auf Pass
  - Group D: Schwere Panzer auf Ebene West (nach Durchbruch)

---

#### 2.5.2 Terrain-Analyse für Gruppen-Bildung

Die Terrain-Analyse erfolgt auf **zwei Ebenen**: Eine einmalige **globale strategische Analyse** beim Map-Start für langfristige Planung, und eine **sektor-basierte taktische Analyse** für operationale Gruppierung.

---

**Globale Map-Profil-Analyse (einmalig beim Start)**

Diese Analyse wird **einmal beim Map-Start durchgeführt**, idealerweise **ohne Fog-of-War-Einschränkung** (die komplette Karte wird analysiert), um eine strategische Baseline zu erhalten. Sie beantwortet die Frage: "Welche Unit-Typen sind auf dieser Map überhaupt relevant?"

**Statistische Terrain-Erfassung (Global)**:

Die gesamte Karte wird durchlaufen und folgende Metriken berechnet:

**Terrain-Komposition** (prozentuale Verteilung):

**Wasser-Terrain** (differenziert nach Tiefe):
- `shallow_water_coverage`: % Flaches Wasser → für bestimmte amphibische Units/Landungen geeignet
- `medium_water_coverage`: % Normales Wasser → Standard-Naval-Operations
- `deep_water_coverage`: % Tiefsee → nur für große Schiffe (U-Boote, Carrier)
- `total_water_coverage`: Summe aller Wasser-Typen → Naval-Relevanz gesamt

**Land-Terrain** (differenziert nach Typ):
- `plains_coverage`: % Offene Ebenen → optimal für schwere Panzer, schnelle Mobilität
- `forest_coverage`: % Wald → beeinflusst Panzer-Typen (leicht bevorzugt), Infanterie-Vorteil
- `mountain_coverage`: % Berge/Hügel → Air-Vorteil, Infanterie-Vorteil, blockiert schwere Panzer
- `desert_coverage`: % Wüste → ähnlich wie Ebenen, aber ggf. Fuel-Probleme
- `urban_coverage`: % Städte/Gebäude → Infanterie-dominiert, wichtig für Eroberung
- `swamp_coverage`: % Sumpf → langsame Mobilität, Defensive-Bonus

**Infrastruktur-Dichte**:
- `road_density`: km Straßen / Gesamt-Felder → Fahrzeug-Mobilität
- `rail_density`: km Bahnlinien / Gesamt-Felder → Rail-Transport-Relevanz
- `bridge_count`: Anzahl Brücken → Choke-Point-Dichte

**Höhen-Verteilung**:
- `height_variance`: Standardabweichung der Höhenstufen → Gebirgs-Map vs. flache Map
- `high_ground_coverage`: % Felder mit Höhe ≥5 → Artillery/Air-Vorteil

**Connectivity-Metriken**:
- `water_connectivity`: Größe zusammenhängender Wasser-Bereiche → Naval-Operational-Range
- `land_connectivity`: Größe zusammenhängender Land-Bereiche → Ground-Operational-Range
- `island_count`: Anzahl separater Landmassen → Transport-Kritikalität

**Strategische Implikationen (automatisch abgeleitet)**:

Basierend auf den Statistiken werden strategische Empfehlungen generiert:

**Naval-Relevanz-Score** (0-100):
```
= water_coverage * 100 * water_connectivity_factor
```
- Score >60: Naval Units essentiell (viele Schiffe bauen/research)
- Score 30-60: Naval Units relevant (moderate Investition)
- Score <30: Naval Units vernachlässigbar (keine Schiffe bauen)

**Air-Dominance-Score** (0-100):
```
= (mountain_coverage * 2 + height_variance) * 50
```
- Score >70: Luft-Überlegenheit kritisch (Berge erschweren Ground)
- Score 40-70: Air Units hilfreich
- Score <40: Ground dominiert

**Armor-Suitability-Score** (0-100):
```
= land_coverage * (1 - forest_coverage * 2) * 100
```
- Score >70: Schwere Panzer optimal (offenes Gelände)
- Score 40-70: Leichte/mittlere Panzer (gemischtes Terrain)
- Score <40: Infanterie-fokussiert (viel Wald/Gebirge)

**Infrastructure-Dependency-Score** (0-100):
```
= (road_density + rail_density * 3) * modifier
```
- Score >60: Infrastruktur-Ausbau kritisch (Züge, Straßen bauen)
- Score <30: Infrastruktur vorhanden, Erhaltung wichtig

**Beispiel-Anwendung**:

**Map A** (Insel-Archipel):
- `water_coverage = 65%`, `island_count = 8`, `water_connectivity = high`
- **Naval-Relevanz = 85** → Schiffe kritisch, Amphibious Operations nötig
- **Air-Dominance = 70** → Lufttransport wichtig für Island-Hopping
- Strategic Memory initialisiert: Goal "Establish Naval Superiority" (High Priority)

**Map B** (Kontinental, Gebirge):
- `mountain_coverage = 40%`, `forest_coverage = 30%`, `water_coverage = 5%`
- **Naval-Relevanz = 10** → Keine Schiffe nötig
- **Air-Dominance = 80** → Flugzeuge dominieren (Berge blockieren Ground)
- **Armor-Suitability = 30** → Infanterie + leichte Units
- Strategic Memory initialisiert: Goal "Develop Air Force" (High Priority)

**Map C** (Flache Ebenen, Straßen):
- `land_coverage = 90%`, `road_density = high`, `mountain_coverage = 5%`
- **Armor-Suitability = 85** → Schwere Panzer optimal
- **Infrastructure-Dependency = 40** → Straßen nutzen, aber nicht kritisch
- Strategic Memory initialisiert: Goal "Build Tank Divisions" (High Priority)

---

**Sektor-basierte Detail-Analyse (kontinuierlich)**

Nach der globalen Analyse wird die Map in **Sektoren unterteilt** und pro Sektor detaillierte Metriken berechnet:

**Pro-Sektor-Metriken**:

Gleiche Terrain-Statistiken wie global, aber auf Sektor-Ebene:
- Terrain-Komposition (% Wasser/Land/Wald/etc. im Sektor)
- Infrastruktur (Straßen/Bahn durch Sektor)
- Höhen-Profil (durchschnittliche/max Höhe)
- **Connectivity zu Nachbar-Sektoren**: Für jeden Unit-Typ, welche Nachbar-Sektoren sind erreichbar?

**Operational-Range-Berechnung**:

Kritisch für Gruppierung: "Wie weit kommen Naval-Units, bevor Land sie blockiert?"

Für jeden Sektor wird pro Domain (Naval/Air/Ground) berechnet:
- **Reachable-Sectors**: Welche anderen Sektoren sind von hier aus erreichbar (ohne Blockade)?
- **Distance-To-Obstacle**: Wie viele Sektoren bis zum nächsten absoluten Hindernis?

**Beispiel - Naval Operational Range**:

Sektor Nord-West (hauptsächlich Wasser):
- `water_connectivity = [Nord-Ost, West, Süd-West]` → 3 Nachbar-Sektoren erreichbar
- `distance_to_land_blockade = 4` → Naval Group kann 4 Sektoren weit operieren bevor Land blockiert

Sektor Inland (Land):
- `water_connectivity = []` → Naval Units können hier nicht operieren
- Naval Groups werden NICHT für diesen Sektor gebildet

**Grouping-Heuristik nutzt Sektor-Daten**:

1. Identifiziere alle eigenen Units im Sektor
2. Filtere nach Domain (Naval/Air/Ground) basierend auf Sektor-Connectivity
3. Für Ground: Weitere Filterung nach Terrain-Kompatibilität (Panzer nur wenn `forest_coverage < 50%`)
4. Clustere Units mit ähnlichen Operational-Ranges

---

**Kritisches Feedback zur Terrain-Analyse**

**Stärken**:

✅ **Datengetrieben**: Objektive Metriken statt Heuristiken
✅ **Einmalige Berechnung**: Globale Analyse ist teuer, aber nur 1x beim Start
✅ **Fog-of-War-Bypass**: Wenn Map bekannt ist (z.B. Multiplayer-Standardmap), kann vollständig analysiert werden
✅ **Skaliert auf Sektor-Ebene**: Detaillierte Info wo nötig, Abstraktion wo möglich

**Schwachstellen & Risiken**:

⚠️ **Berechnungskomplexität**: Connectivity-Analyse für große Maps (256x256 = 65k Felder) ist rechenintensiv.

**Detaillierte Analyse der Komplexität**:

Die **Connectivity-Analyse** ist der teuerste Teil der Terrain-Analyse. Sie beantwortet: "Welche Bereiche sind für Unit-Typ X erreichbar?" und erfordert eine **Graph-Traversierung** pro Unit-Typ.

**Algorithmus**: Flood-Fill oder BFS (Breadth-First Search) pro Terrain-Domain

**Worst-Case-Komplexität**:
- **Global**: O(N) pro Domain, wobei N = Anzahl Felder (z.B. 65.000 bei 256x256)
- **3 Domains** (Naval/Ground/Air): 3 * O(N) = O(3N)
- **Für Multiple Unit-Types** (schwere Panzer vs. leichte Panzer): K * O(N), wobei K = Anzahl relevanter Unit-Kategorien
- **Gesamt**: Für 5 Unit-Kategorien (Heavy Armor, Light Armor, Infantry, Naval, Air) = 5 * 65.000 = 325.000 Feld-Checks

**Realistische Laufzeit**:
- Moderne CPU: ~1 Million Operationen/ms
- 325k Feld-Checks: ~300-500ms für vollständige Analyse
- **Akzeptabel** für einmalige Berechnung beim Map-Start

**Optimierungen**:

1. **Sektor-basierte Abstraktion** (wie bereits vorgeschlagen):
   - Statt 65.000 Felder → ~60 Sektoren
   - Connectivity auf Sektor-Ebene: 60 Sektoren, 5 Unit-Typen = 300 Checks
   - Laufzeit: ~5-10ms
   - **Nachteil**: Verliert Detail innerhalb von Sektoren (z.B. kleine Engpässe)

2. **Hierarchische BFS**:
   - Erste Ebene: Grobe Connectivity auf Sektor-Ebene (schnell)
   - Zweite Ebene: Detaillierte Connectivity nur für relevante Sektoren (bei Bedarf)
   - Hybrid-Ansatz: Sektor-Level für Strategic, Feld-Level für Tactical

3. **Caching & Incremental Updates**:
   - Initiale Berechnung: 300-500ms (akzeptabel)
   - **Dynamische Updates**: Nur betroffene Bereiche neu berechnen
   - Wenn Brücke zerstört wird: Nur Connectivity um Brücken-Sektor neu berechnen (~10ms)

4. **Preprocessing beim Map-Design**:
   - Map-Designer können Connectivity vorberechnen und in Map-Datei speichern
   - KI lädt vorberechnete Connectivity-Graphen
   - **Problem**: Funktioniert nicht wenn Terrain dynamisch ändert (Builder-Units)

**Empfehlung**: Sektor-basierte Abstraktion für Strategic Layer (5-10ms), on-demand Feld-Level-Berechnung für Tactical Layer (nur lokaler Bereich, 20-50ms).

---

⚠️ **Dynamischer Terrain-Bau**: Units können Infrastruktur bauen (Straßen, Depots, Brücken) oder zerstören, was `road_density` und Connectivity verändert.

**Mitigation - Inkrementelle Synchronisation**:

Wenn Builder-Units aktiv sind, werden Terrain-Änderungen sowohl in Sektor- als auch Global-Statistiken **zurück-synchronisiert**:

**Beispiel - Straßenbau**:
- Builder-Unit baut Straße in Sektor A
- **Sektor-Update**: `road_density` von Sektor A wird inkrementiert
- **Global-Update**: `global_road_density` wird angepasst (minimal, da nur 1 Sektor betroffen)
- **Connectivity-Update**: Nur wenn neue Straße zwei zuvor getrennte Bereiche verbindet (selten)

**Beispiel - Brücken-Zerstörung** (kritischer):
- Feind zerstört Brücke in Sektor B
- **Sektor-Update**: Sektor B verliert Connectivity zu Nachbar-Sektor C
- **Global-Update**: `bridge_count` dekrementiert, `land_connectivity` wird neu berechnet (nur betroffene Region)
- **Strategic Impact**: Gruppen, die über Brücke operieren sollten, müssen re-routen

**Inkrementelle vs. Vollständige Neuberechnung**:
- **Kleine Änderungen** (1-2 Straßen gebaut): Inkrementell (nur betroffener Sektor)
- **Große Änderungen** (10+ Builder-Units aktiv über 5 Runden): Vollständige Neuberechnung (alle 10-20 Runden)
- **Heuristik**: Wenn >10% der Sektoren Infrastruktur-Änderungen haben → Full-Recalc

**Performance-Impact**:
- Inkrementeller Update: 1-5ms pro Änderung
- Full-Recalc: 300-500ms (wie initial), aber nur selten nötig

---

⚠️ **Multi-Faktor-Entscheidung**: Terrain-Scores allein sind unzureichend für Unit-Typ-Entscheidungen.

**Problem**: Wenn KI basierend nur auf Terrain entscheidet "Naval-Score = 20 → keine Schiffe bauen", ignoriert sie:
1. **Gegner-Komposition**: Gegner könnte trotzdem Naval-Units haben
2. **Start-Units**: Eigene Start-Komposition könnte bereits Naval-Units enthalten
3. **Dynamische Bedrohung**: Gegner baut plötzlich Naval-Überlegenheit auf

**Lösung - Hybrid-Bewertung**:

**Unit-Type-Decision-Score** wird aus mehreren Faktoren berechnet:

```
Final_Naval_Score = 
  Terrain_Naval_Score * 0.4 +           // 40% Terrain-Baseline
  Enemy_Naval_Strength * 0.3 +          // 30% Gegner-Bedrohung
  Own_Starting_Naval * 0.2 +            // 20% Eigene Start-Units
  Strategic_Objective_Bonus * 0.1       // 10% Missions-spezifisch
```

**Beispiele**:

**Szenario A** (Insel-Map gegen Naval-fokussierten Gegner):
- `Terrain_Naval_Score = 85` (viel Wasser)
- `Enemy_Naval_Strength = 80` (Gegner hat viele Schiffe)
- `Own_Starting_Naval = 40` (wenige Start-Schiffe)
- **Final_Score = 85*0.4 + 80*0.3 + 40*0.2 + 0*0.1 = 34 + 24 + 8 = 66**
- **Entscheidung**: Naval-Ausbau kritisch (Terrain + Gegner-Bedrohung)

**Szenario B** (Land-Map, aber Gegner hat überraschend Schiffe):
- `Terrain_Naval_Score = 20` (wenig Wasser)
- `Enemy_Naval_Strength = 70` (Gegner investiert in Naval trotz Terrain!)
- `Own_Starting_Naval = 10` (fast keine Schiffe)
- **Final_Score = 20*0.4 + 70*0.3 + 10*0.2 + 0*0.1 = 8 + 21 + 2 = 31**
- **Entscheidung**: Moderate Naval-Investition nötig (Gegner-Adaptierung)

**Szenario C** (Gemischte Map, Gegner hat equivalent Start-Komposition):
- `Terrain_Naval_Score = 50` (moderate Wasser-Bereiche)
- `Enemy_Naval_Strength = 50` (equivalent zu eigenem Start)
- `Own_Starting_Naval = 50` (moderate Start-Schiffe)
- **Final_Score = 50*0.4 + 50*0.3 + 50*0.2 + 0*0.1 = 20 + 15 + 10 = 45**
- **Entscheidung**: Naval beibehalten, aber nicht übermäßig ausbauen

**Dynamische Anpassung**:

Die Gewichtungen (40/30/20/10) können sich über Zeit ändern:
- **Früh-Game** (Runde 1-5): Terrain-Score dominiert (50% Weight), Gegner noch unbekannt
- **Mid-Game** (Runde 6-15): Enemy-Strength wichtiger (40% Weight), Terrain bekannt
- **Late-Game** (Runde 16+): Strategic-Objectives zählen mehr (20% Weight), Endgame-Push

**Annahme "Equivalente Gegner-Komposition"**:

In den meisten Szenarien haben beide Spieler ähnliche Start-Units (symmetrische Maps). Dies vereinfacht Early-Game-Decisions:
- Wenn eigene Start-Komposition 60% Ground, 30% Naval, 10% Air ist
- Annahme: Gegner hat ähnliche Verteilung
- KI kann initiale Strategie darauf basieren
- **Aber**: Memory-System trackt tatsächliche Gegner-Sichtungen und adjustiert

**Design-Entscheidung: Globale Analyse ignoriert Fog-of-War**

**Gewählter Ansatz**: Die KI darf beim Map-Start die komplette Karte (nur Terrain, keine Units) analysieren, **auch wenn Fog-of-War aktiv ist**.

**Begründung**:
- Terrain-Information ist **statisch** und würde durch Scouting ohnehin schnell bekannt
- Verhindert suboptimale Early-Game-Entscheidungen basierend auf limitierter Sichtbarkeit
- Menschliche Spieler haben oft Meta-Wissen über Standard-Maps
- **Wichtig**: Units, Gebäude, feindliche Infrastruktur bleiben im Fog-of-War verborgen

**Was die KI weiß**:
✅ Terrain-Typ jedes Feldes (Wasser-Tiefe, Land-Typ, Höhe)
✅ Statische Objekte (Wracks, Dekorationen)
✅ Natürliche Hindernisse (Tank Blocker, Cliffs)

**Was die KI NICHT weiß**:
❌ Feindliche Units und deren Positionen
❌ Feindliche Gebäude und Basen
❌ Vom Gegner gebaute Infrastruktur (Straßen, Depots)
❌ Minen und Traps

Dies ist ein **pragmatischer Kompromiss**: Die KI "schummelt" nur bei statischen Terrain-Daten, nicht bei strategisch relevantem Gegner-Wissen.

---

**Zusammenfassung: Zwei-Ebenen-Analyse**

| **Ebene** | **Wann** | **Granularität** | **Zweck** |
|-----------|----------|------------------|-----------|
| **Global** | Map-Start (1x) | Gesamte Map | Strategische Baseline: Welche Unit-Typen relevant? |
| **Sektor** | Kontinuierlich | Pro Sektor | Operationale Gruppierung: Welche Units passen wohin? |

Die Map wird in **Operationsräume** unterteilt basierend auf:

**Terrain-Connectivity**: Welche Bereiche sind für welche Unit-Typen erreichbar?
- Berechne für jeden Unit-Typ separate "Erreichbarkeits-Graphen"
- Beispiel: Panzer-Erreichbarkeits-Graph zeigt nur Bereiche ohne Wald/hohe Berge
- Infanterie-Graph zeigt fast alles, aber Wasser ist blockiert (ohne Transport)

**Choke Points**: Engpässe identifizieren
- Bereiche, wo Terrain sich verengt (Brücken, Pässe, Stadttore)
- Kritisch für Defense-Gruppen
- Auch kritisch für Attack-Planung (Bottlenecks vermeiden)

**Defensive Strongholds**: Gebiete mit Befestigungen, Höhenvorteil
- Markiere Sektoren mit hohem Defensive-Bonus
- Defense-Groups sollten hier positioniert werden
- Angriffs-Gruppen müssen diese umgehen oder speziell dafür ausgerüstet sein

**Mobility Corridors**: Schnelle Bewegungswege
- Straßen-Netzwerk für Fahrzeuge
- Bahnlinien für Züge
- Offene Ebenen für Panzer
- Transport-Gruppen nutzen diese Korridore

**Phase 2: Unit-Terrain-Matching**

Für jede Einheit wird berechnet:

**Mobility-Score** pro Sektor:
- Wie viele Bewegungspunkte kostet es, diesen Sektor zu durchqueren?
- Basiert auf Unit-Typ und Terrain
- Sektoren mit schlechtem Score sind "verbotenes Gebiet" für diese Unit

**Combat-Effectiveness** pro Sektor:
- Wie effektiv kann die Unit hier kämpfen?
- Berücksichtigt Terrain-Boni/Mali
- Panzer in Wäldern: sehr schlecht
- Infanterie in Städten: sehr gut

**Strategic-Value** (aus Sektor-Management):
- Ressourcen-Wert
- Konnektivität
- Bedrohung

**Phase 3: Clustering-Algorithmus**

Basierend auf Terrain-Analyse:

1. **Initiales Clustering** nach räumlicher Nähe (wie bei Naval)
2. **Terrain-Filter**: Teile Cluster auf, wenn Units nicht im selben Operationsraum
3. **Role-Balancing**: Stelle sicher, dass Gruppen ausgewogen sind (nicht nur Panzer, sondern auch Infanterie/Supply)
4. **Size-Constraints**: Gruppen sollten 5-15 Units haben (zu klein = ineffektiv, zu groß = unhandlich)

**Beispiel-Output**:
- Sektor A (Ebene): Combat Group mit 8 Panzern, 4 APCs (mit Infanterie), 2 Artillery, 1 Supply
- Sektor B (Wald): Defense Group mit 6 Infanterie, 2 leichte Panzer, 1 Anti-Air
- Sektor C (Bahnlinie): Rail Transport Group mit 3 Zügen (kann Nachschub bringen)

---

#### 2.5.3 Dynamische Re-Gruppierung

Gruppen sind nicht statisch. Sie müssen sich anpassen:

**Trigger für Re-Gruppierung**:

**Verluste**: Gruppe fällt unter kritische Größe (z.B. nur noch 3 Units)
- Option A: Auflösen, Units zu anderen Gruppen
- Option B: Merger mit benachbarter Gruppe

**Terrain-Wechsel**: Gruppe wechselt von Ebene in Gebirge
- Panzer-lastige Gruppe muss umstrukturiert werden
- Schwere Panzer werden abgezogen, Infanterie verstärkt

**Strategic-Shift**: Objectives ändern sich
- Offensive Combat Group wird zu Defense Group
- Benötigt andere Unit-Zusammensetzung (mehr statische Units, weniger Mobile)

**Reinforcement**: Neue Units werden produziert
- Müssen bestehenden Gruppen zugeordnet werden
- Oder: Neue Gruppe bilden wenn genug Units

**Merge-Operation**: Zwei Gruppen kommen zusammen
- Wenn beide unter Sollstärke
- Wenn sie gleiches Objective haben

**Split-Operation**: Gruppe wird geteilt
- Wenn Gruppe zu groß wird (>20 Units)
- Wenn Gruppe in zwei verschiedene Richtungen operieren soll
- Wenn Terrain-Barriere die Gruppe trennt (z.B. Fluss ohne Brücke)

---

#### 2.5.4 Terrain-Analyse auf Tactical Layer

Die Terrain-Analyse ist nicht nur für Gruppierung relevant, sondern auch **während Tactical MCTS**:

**Micro-Terrain für Bewegungsplanung**:

Für Boden-Einheiten muss Tactical MCTS berücksichtigen:

**Cover und Concealment**: Bestimmte Terrain-Typen bieten Deckung
- Wald: Reduziert Sichtbarkeit, erhöht Defense
- Gebäude/Städte: Hoher Defense-Bonus für Infanterie
- Offenes Feld: Keine Deckung, anfällig für Artillery/Air-Strike

**Line-of-Sight**: Höhenvorteil
- Units auf Hügeln sehen weiter
- Units in Tälern sind anfälliger für Überraschungen
- Relevant für Artillery-Positionierung

**Reaction Fire Exposure**: Welche Bewegungen lösen RF aus?
- Bewegung durch offenes Gelände vor Feind-Units = gefährlich
- Bewegung durch Wald/Berge = sicherer (weniger sichtbar)
- Tactical MCTS sollte RF-Risk in Evaluation einfließen lassen

**Fortification Utilization**: Befestigungen optimal nutzen
- Units sollten IN Befestigungen positioniert werden, nicht davor
- Befestigungen sind oft linear (Mauern) → Formation anpassen

**Obstacle Navigation**: Hindernisse umgehen
- Tank Blocker: Route um diese herum planen
- Stacheldraht: Engineer vorschicken zum Räumen, oder umgehen
- Minen-Felder: Bekannte Minen umgehen, oder Minen-Räumung einplanen

**Beispiel Tactical-Decision mit Terrain**:

Objective: "Erobere Gebäude in Sektor Nord"

Terrain-Situation:
- Offenes Feld zwischen aktueller Position und Ziel (5 Hex)
- Wald östlich (Umweg +3 Hex)
- Feindliche Artillery mit Sicht auf offenes Feld

Tactical MCTS evaluiert:
- **Option A**: Direkt durch offenes Feld → schnell, ABER hohe RF-Exposure und Artillery-Risk
- **Option B**: Umweg durch Wald → langsamer, ABER sicherer
- **Evaluation**: Abhängig von Priorität (Speed vs. Preservation)

Wenn Strategic Layer "Risk-Toleranz = Low" gesetzt hat → Option B
Wenn "Risk-Toleranz = High" und Zeit kritisch → Option A

---

#### 2.5.5 Zusammenfassung: Gruppen-Bildungs-Pipeline

**Initiale Gruppierung (Map-Start)**:

1. **Domain-Separation**: Naval, Air, Ground trennen
2. **Terrain-Analyse**: Operationsräume identifizieren (nur für Ground)
3. **Unit-Terrain-Matching**: Welche Units passen in welche Operationsräume?
4. **Clustering**: Räumlich nahe Units mit kompatiblem Terrain gruppieren
5. **Role-Balancing**: Stelle ausgewogene Gruppen sicher
6. **Validate**: Check ob Gruppen sinnvolle Größe (5-15 Units) haben

**Laufende Anpassung (During Game)**:

1. **Monitor Gruppen-Status**: Verluste, Terrain-Wechsel, Strategic-Shifts
2. **Trigger Re-Gruppierung** wenn nötig
3. **Merge/Split/Reassign** Units
4. **Update Memory**: Speichere Gruppen-Änderungen persistent

**Tactical Execution (Per Turn)**:

1. **Load Terrain-Context** für Gruppen-Sektor
2. **Tactical MCTS** nutzt Micro-Terrain für Bewegung/Combat
3. **Feedback an Strategic**: Terrain-bedingte Schwierigkeiten melden

---

Diese mehrstufige Terrain- und Unit-Analyse stellt sicher, dass Gruppen **sinnvoll**, **effektiv** und **terrain-angepasst** sind.

### 3.1 Strategische Ebene

Die strategische MCTS-Ebene operiert auf Gruppen und Sektoren, nicht auf einzelnen Einheiten und Feldern.

**Suchraum auf strategischer Ebene**:
- 5-10 eigene Gruppen als Akteure
- Je Gruppe 5-10 mögliche high-level Objectives
- Gesamter Branching-Factor: etwa 50-100 strategische Entscheidungen
- Im Vergleich zu mehreren tausend bei unit-level Betrachtung

**Typische strategische Entscheidungen**:
- Welche Gruppe soll welchen Sektor angreifen?
- Welche Sektoren müssen verteidigt werden?
- Wo sollen neue Fabriken gebaut werden?
- Welche Forschungsziele werden priorisiert?
- Welche Ressourcen werden wo alloziert?

**State-Representation**:
Der strategische State enthält keine einzelnen Map-Felder oder detaillierte Unit-Positionen, sondern:
- Snapshots aller eigenen Gruppen (kompakt, nur wesentliche Daten)
- Bekannte feindliche Gruppen (aus Memory, nicht aus aktuellem Spielstand!)
- Sektor-Zustände (Kontrolle, Ressourcen, Bedrohung)
- Globale Ressourcen-Situation
- Forschungs-Status
- **Persistenter Terrain-Snapshot** (einmal aufgedecktes Gelände bleibt bekannt)

**Persistenter Terrain-Snapshot**:

Ein kritischer Aspekt für strategische Planung ist das **Terrain-Memory**: Sobald ein Gebiet einmal aufgedeckt wurde, muss die KI sich dauerhaft an das Terrain erinnern, auch wenn es aktuell wieder im Fog-of-War liegt.

Der Terrain-Snapshot speichert für einmal gesichtete Bereiche:
- **Terrain-Typ** pro Feld (Wasser, Grasland, Berge, etc.)
- **Höhenstufe** (0-7)
- **Infrastruktur**: Straßen, Bahnlinien, Brücken, Pipelines
- **Statische Objekte**: Wracks, Dekorationen
- **Letzte Sichtungs-Zeit** (um veraltete Information zu kennzeichnen)

Diese Information ist essentiell für:

**Bewegungsplanung**: Die KI kann fragen "Kann meine Panzer-Gruppe den Sektor Nord erreichen?" auch wenn dieser aktuell unsichtbar ist. Basierend auf dem Terrain-Memory kann ein Pathfinding durchgeführt werden, das realistische Bewegungskosten berücksichtigt.

**Infrastruktur-Nutzung**: Wenn die KI einmal eine Bahnlinie oder einen Wasserweg gesehen hat, kann sie langfristig planen, diese zu nutzen (z.B. Transport-Gruppen via Schiff schicken).

**Build-Planung**: Bei der Auswahl von Fabrik-Standorten kann die KI auf bekanntes Terrain zurückgreifen, auch wenn es aktuell nicht sichtbar ist.

**Caveat**: Das Terrain-Memory kann veraltet sein, wenn der Gegner z.B. Straßen gebaut oder zerstört hat. Die KI muss dies durch einen "Confidence-Wert" berücksichtigen, der mit der Zeit sinkt.

Dieser State ist typischerweise 100-200 KB groß (inklusive komprimiertem Terrain-Snapshot), kann also tausendfach für Simulationen kopiert werden.

**Evaluation-Funktion**:
Die Bewertung eines strategischen Zustands basiert auf:
- Anzahl und Wert kontrollierter Sektoren
- Gesamtstärke eigener Gruppen
- Ressourcen-Produktion und -Reserven
- Fortschritt bei strategischen Zielen
- Bedrohungs-Situation

### 3.2 Taktische Ebene

Die taktische Ebene wird für jede Gruppe separat ausgeführt und detailliert die Umsetzung der strategischen Vorgaben.

**Hierarchische Gruppen-Struktur**:

Gruppen sind nicht flach, sondern können **hierarchisch strukturiert** sein. Eine große Combat-Gruppe für ein komplexes Szenario könnte verschiedene Untereinheiten-Rollen enthalten:

- **Artillery / Fernkampf-Units**: Bleiben hinten, maximale Reichweite
- **Panzer / Blocker**: Frontline, absorbieren Schaden, blockieren Feinde
- **Infanterie / Eroberer**: Spezialrolle für Gebäude-Eroberung, beweglich
- **Transporter**: Bringen Infanterie schnell an kritische Positionen
- **Supply-Units**: Fuel- und Munitions-Versorgung für die Gruppe
- **Builder-Units**: Infrastruktur-Bau (Straßen, Depots, Verteidigungen)

Diese **Rollen-basierte Hierarchie** erlaubt:

**Koordinierte Taktiken**: Die Gruppe agiert als geschlossene Einheit. Panzer rücken vor, Artillery unterstützt von hinten, Supply-Units folgen, Infanterie wird per Transporter zu eroberbaren Gebäuden gebracht.

**Flexible Skalierung**: Kleine Szenarien können mit flachen Gruppen (nur Kampf-Units) arbeiten. Große Szenarien nutzen voll ausgestattete Combined-Arms-Gruppen.

**Spezialisierte Sub-MCTS**: Das taktische MCTS kann Rollen-spezifische Heuristiken nutzen. Artillery-Einheiten bekommen andere Bewertungskriterien als Blocker.

**Beispiel Combined-Arms-Gruppe**:
- 3 Panzer (Blocker)
- 2 Artillery (Fernkampf)
- 4 Infanterie (Eroberer)
- 2 APCs (Transporter)
- 1 Supply-Truck (Versorger)
- 1 Engineer (Builder)

Die Gruppe hat insgesamt 13 Units, aber durch die Rollen-Struktur wird die Planung vereinfacht: Statt 13 unabhängige Units werden 6 Rollen koordiniert.

**Suchraum auf taktischer Ebene**:
- 5-15 Einheiten pro Gruppe (oder mehr bei großen Combined-Arms-Gruppen)
- 30-50 konkrete Aktionen (Bewegungen, Angriffe, Laden/Entladen)
- Rollen-basierte Reduktion: Units mit gleicher Rolle können ähnlich behandelt werden
- Branching-Factor: etwa 40 pro Gruppe
- Parallelisierbar über Gruppen!

**Typische taktische Entscheidungen**:
- Welche konkrete Route nimmt die Gruppe?
- Welche feindlichen Einheiten werden angegriffen?
- Wie wird Formation und Spacing optimiert?
- Wann wird nachversorgt/repariert?
- Wie wird Reaction Fire minimiert?

**State-Representation**:
Der taktische State ist noch kompakter:
- Nur die Einheiten der eigenen Gruppe (10-15 Units)
- Bekannte Feinde im relevanten Umkreis (aus Memory + aktueller Sicht)
- Terrain im Operationsgebiet (etwa 50×50 Felder = 2500 Felder)
- Typischerweise 10-20 KB pro State

**Koordination zwischen Gruppen**:
Gruppen können aufeinander Bezug nehmen durch:
- Verknüpfte Objectives (z.B. Escort-Missions)
- Gemeinsame Ziel-Sektoren (koordinierte Angriffe)
- Ressourcen-Transfers (Logistics-Gruppen versorgen Combat-Gruppen)

### 3.3 Informationsfluss zwischen Ebenen

Ein kritischer Aspekt der hierarchischen Architektur ist der **bidirektionale Informationsfluss** zwischen strategischer und taktischer Ebene. Beide Ebenen müssen kontinuierlich Informationen austauschen, um effektiv zu arbeiten.

#### 3.3.1 Top-Down Communication (Strategic → Tactical)

Die strategische Ebene gibt der taktischen Ebene vor:

**Objectives und Prioritäten**:
- "Erobere Sektor Nord innerhalb von 3 Runden"
- "Verteidige Building XY um jeden Preis"
- "Erkunde Sektor Ost, minimiere dabei Verluste"
- Priorität des Objectives (Critical/High/Medium/Low)

**Ressourcen-Allokation**:
- "Gruppe A bekommt Priorität bei Supply-Units"
- "Spare Munition für Haupt-Angriff"
- "Builder-Units werden zu Gruppe B umgeleitet"

**Koordinations-Anweisungen**:
- "Warte auf Gruppe B, koordinierter Angriff in Runde N"
- "Escort-Mission: Begleite Transport-Convoy"
- "Halte Position bis Verstärkung eintrifft"

**Risk-Toleranz**:
- "Aggressives Vorgehen akzeptabel, Verluste in Kauf nehmen"
- "Defensiv spielen, Preservation vor Objectives"
- Wird durch übergeordnete strategische Situation bestimmt

**Terrain- und Intelligence-Daten**:
- Bekanntes Terrain aus Memory
- Vermutete Feind-Positionen
- Bekannte Minen-Positionen
- Sektor-Threat-Levels

#### 3.3.2 Bottom-Up Communication (Tactical → Strategic)

Die taktische Ebene liefert der strategischen Ebene zurück:

**Erfolgswahrscheinlichkeit und Zeitschätzung**:
- "Objective kann mit 70% Wahrscheinlichkeit in 3 Runden erreicht werden"
- "Objective nicht erreichbar mit aktueller Stärke"
- "Vorzeitige Zielerreichung möglich (2 statt 3 Runden)"
- Diese Rückmeldungen erlauben Strategic MCTS realistische Bewertungen

**Feindstärke-Assessment**:
- "Feindliche Gruppe im Sektor hat geschätzte Stärke 1500"
- "Feind hat Artillery-Unterstützung (Fernkampf-Bedrohung)"
- "Gegner ist schwächer als erwartet / stärker als erwartet"
- Updated die strategische Threat-Bewertung

**Eigene Gruppen-Stärke**:
- Gruppen Schwerpunkt. 
  - Luft gegen Boden
  - Luft gegen Luft
  - Orbit gegen Boden 
  - Boden gegen Boden
  - Boden gegen Luft
  - ...
  - Alles jeweils als Fernkampf und "Nahkampf"
- Aktuelle Kampfstärke der Gruppe (0-100%)
- Gesundheitszustand (Average HP)
- Munitions- und Fuel-Status
- Anzahl verbleibender funktionsfähiger Units
- Erlaubt Strategic MCTS, Gruppen-Einsatz zu optimieren

**Request for Retreat (Rückzugs-Anfrage)**:
- "Gruppe ist stark angeschlagen (20% Strength), Rückzug empfohlen"
- "Objective nicht erreichbar ohne Verstärkung"
- Strategic MCTS kann dann entscheiden:
  - Rückzugsbefehl erteilen (Gruppe erholt sich)
  - Verstärkung schicken
  - Gruppe opfern für strategisches Ziel

**Request for Reinforcement (Verstärkungs-Anfrage)**:
- "Benötige 2 zusätzliche Panzer für Durchbruch"
- "Supply-Unit nötig, Gruppe läuft leer"
- "Artillery-Unterstützung würde Erfolgschance auf 90% erhöhen"
- Strategic MCTS kann Ressourcen umverteilen

**Request for Support (Unterstützungs-Anfrage)**:
- "Feindliche Artillery bedroht Gruppe, Air-Strike anfordern"
- "Minen-Feld blockiert Fortschritt, Engineer-Unit nötig"
- "Gegner hat Luftüberlegenheit, Anti-Air benötigt"

**Intelligence Updates**:
- "Neue feindliche Einheit entdeckt bei Position (X,Y)"
- "Mine gefunden bei Position (X,Y)"
- "Gegner baut Fabrik in Sektor Z"
- "Terrain-Informationen veraltet (Straße wurde zerstört)"
- Fließt in Memory-System ein

**Ressourcen-Anfragen**:
- "Gruppe braucht 500 Fuel für nächste Phase"
- "Ammo-Nachschub kritisch, nur noch 2 Salven"
- Strategic MCTS priorisiert Supply-Gruppen entsprechend

#### 3.3.3 Echtzeit-Feedback-Loop

Während der Tactical-Execution kann sich die Situation ändern:

**Unerwartete Entwicklungen**:
- Feind stärker als erwartet → Tactical meldet sofort
- Objective schneller erreichbar → Tactical fragt nach neuem Objective
- Überraschende Verluste (Minen, Ambush) → Tactical fordert Re-Evaluation

**Dynamische Re-Planning**:
Strategic MCTS kann **während** der Tactical-Execution neu planen, wenn:
- Kritische Feedback-Meldungen eintreffen
- Andere Gruppen scheitern/übertreffen
- Globale Situation sich ändert (z.B. Feind startet Großangriff)

#### 3.3.4 Iterative Refinement

Der Planungsprozess ist iterativ:

**Phase 1: Strategic Planning**
- Strategic MCTS exploriert verschiedene Strategien
- Für vielversprechende Strategien: Frage Tactical Layer "Ist das machbar?"

**Phase 2: Tactical Validation**
- Tactical MCTS simuliert die Umsetzung
- Gibt Feedback zurück: Erfolgswahrscheinlichkeit, Kosten (Verluste), Zeit

**Phase 3: Strategic Refinement**
- Strategic MCTS adjusted Bewertungen basierend auf Tactical-Feedback
- Schlechte Strategien (Tactical meldet <30% Erfolg) werden verworfen
- Gute Strategien werden weiter verfeinert

**Phase 4: Final Decision**
- Strategic wählt beste Strategie
- Tactical führt aus
- Feedback während Execution kann zu Re-Planning führen (zurück zu Phase 1)

Dieser Loop stellt sicher, dass strategische Pläne taktisch umsetzbar sind, und taktische Entscheidungen strategisch sinnvoll sind.

---

## 4. Persistenter Strategie-Memory

### 4.1 Motivation

Im Gegensatz zum reaktiven Verhalten klassischer KIs benötigt eine MCTS-basierte KI für ASC ein "Gedächtnis" über mehrere Runden hinweg. Dies ist essentiell für:

- **Langfristige Planung**: Research dauert 5-10 Runden, Fabriken müssen vorbereitet werden
- **Fog-of-War Intelligence**: Wo wurden zuletzt Feinde gesehen? Wo sind vermutlich Minen?
- **Strategische Konsistenz**: Verfolgte Strategie soll nicht jede Runde komplett wechseln
- **Lernfähigkeit**: Welche Entscheidungen waren erfolgreich/erfolglos?

### 4.2 Komponenten des Strategy-Memory

Der Memory-State ist eine persistente Datenstruktur, die zwischen Runden erhalten bleibt und in Savegames gespeichert wird.

#### 4.2.1 Strategische Ziele (Long-term Goals)

Langfristige Ziele repräsentieren übergeordnete Strategien, die über viele Runden verfolgt werden:

**Beispiel "Lufthoheit erlangen"**:
- **Beschreibung**: Dominiere den Luftraum über der Map
- **Priorität**: Hoch (Critical/High/Medium/Low)
- **Prerequisites**: Technologien (z.B. "Advanced Aeronautics", "Jet Engines")
- **Benötigte Ressourcen**: 5000 Energy, 8000 Material, 3000 Fuel (kumulativ)
- **Benötigte Infrastruktur**: 2 Airfields, 1 Research Lab
- **Unteraufgaben** mit Fortschritt:
  - "Research Advanced Aeronautics" → 80% complete
  - "Build 2 Airfields" → 50% complete (1 von 2 fertig)
  - "Produce 10 Fighter Jets" → 0% (waiting for Airfields)
  - "Establish Air Patrols" → 0% (waiting for Fighters)

**Fortschritts-Tracking**:
- Anzahl aktiver Runden seit Start des Ziels
- Prozentsatz zur Fertigstellung (0.0 - 1.0)
- Completion-Status
- Blockierende Dependencies (welche Prerequisites fehlen noch?)

#### 4.2.2 Research-Queue

Eine priorisierte Warteschlange von Forschungsprojekten:

Jeder Eintrag enthält:
- Technologie-ID
- Priorität (verknüpft mit strategischem Ziel)
- Bereits allozierte Ressourcen
- Geschätzte Runden bis Fertigstellung
- Begründung (z.B. "Benötigt für Lufthoheit-Strategie")

Die KI sammelt Runde für Runde Ressourcen und investiert sie entsprechend dieser Queue. Dadurch wird Research nicht ad-hoc betrieben, sondern folgt einem Plan.
Research resourecen können explizit erhöht werden um Ziel schneller zu erreichen. Die kosten sind exponentiell aber der Progress linear.

#### 4.2.3 Build-Queue

Geplante Gebäude-Konstruktionen über mehrere Runden:

Jeder Eintrag enthält:
- Gebäude-Typ (z.B. Airfield, Factory, Defense Tower)
- Geplante Position auf der Map
- Zugewiesene Builder-Gruppe
- Bereits gesammelte Ressourcen
- Geschätzte Runden bis Finanzierung möglich

**Ressourcen-Sammlung**:
Die KI "spart" bewusst Ressourcen an, indem sie z.B. weniger Units produziert, um schneller ein wichtiges Gebäude bauen zu können. Dies erfordert Multi-Turn-Planung.

#### 4.2.4 Fog-of-War Memory: Enemy Contacts

Eine Historie aller gesichteten feindlichen Einheiten:

Für jede gesichtete Einheit wird gespeichert:
- Einheiten-ID
- Spieler-ID
- **Letzte bekannte Position**
- **Zeitpunkt der letzten Sichtung** (GameTime)
- Geschätzte Stärke/HP
- Einheiten-Typ

**Position-Prediction**:
Da Einheiten sich bewegen, wird eine **geschätzte aktuelle Position** berechnet:
- Basierend auf maximal möglicher Bewegungsreichweite seit letzter Sichtung
- Confidence-Wert sinkt mit Zeit (1.0 bei frischer Sichtung, 0.0 nach vielen Runden)
- Heuristische Annahmen: Feind bewegt sich vermutlich in Richtung eigener Sektoren

Diese Information fließt in die MCTS-Simulation ein: Auch unsichtbare Feinde werden als "vermutete Bedrohung" modelliert!

#### 4.2.5 Fog-of-War Memory: Minen

Entdeckte oder vermutete Minen werden dauerhaft gespeichert:

Für jede Mine:
- Position
- Entdeckungszeitpunkt
- **Confirmed vs. Suspected**: Wurde die Mine definitiv gesehen, oder nur vermutet (z.B. weil eine Einheit dort Schaden nahm)?
- Minen-Typ (Land/Wasser/Unterwasser)

Diese Liste wird aktiv bei Bewegungs-Planung berücksichtigt: MCTS vermeidet bekannte Minen oder plant explizite Minenräumung ein.

#### 4.2.6 Bedrohungs-Heatmap (Threat Memory)

Pro Sektor wird ein **Bedrohungs-Level** gespeichert (0.0 - 1.0):

- Aktualisiert bei jeder Feind-Sichtung
- **Decay über Zeit**: Threat nimmt exponentiell ab, wenn länger keine Feinde gesehen wurden
- Formel: `threat(t) = threat(t0) * exp(-0.1 * delta_turns)`

**Nutzung**:
- Strategic MCTS bevorzugt Bewegungen in Sektoren mit niedrigem Threat
- Defense Groups werden zu high-threat Sektoren geschickt
- Beeinflusst Risk-Assessment bei Entscheidungen

#### 4.2.7 Gruppen-Management

Das Memory-System verwaltet alle aktiven Gruppen:

- **Gruppe-zu-Einheiten Zuordnung**: Welche Units gehören zu welcher Gruppe?
- **Gruppen-Lebenszyklus**: Erstellung, Auflösung, Restrukturierung
- **Gruppen-Historie**: Vergangene Objectives und deren Erfolg

**Dynamische Restrukturierung**:
Wenn eine Gruppe stark dezimiert wird, kann sie automatisch aufgelöst und die verbleibenden Einheiten anderen Gruppen zugeordnet werden.

#### 4.2.8 Decision History (für Learning)

Eine zeitlich begrenzte Historie vergangener Entscheidungen:

Für jede wichtige Entscheidung wird gespeichert:
- Zeitpunkt
- Entscheidungs-Typ (Attack, Defend, Build, Research)
- Beteiligte Gruppe
- **Erwartetes Outcome** (Vorhersage durch MCTS)
- **Tatsächliches Outcome** (gemessen nach 1-3 Runden)

**Post-Game-Analyse**:
Diese Daten können nach dem Spiel ausgewertet werden:
- Welche Strategien waren erfolgreich?
- War die MCTS-Evaluation korrekt?
- Können Heuristiken angepasst werden?

### 4.3 Memory-Persistenz

**Serialisierung**:
Der gesamte Memory-State muss serialisierbar sein, um:
- In ASC-Savegames integriert zu werden
- Zwischen Sitzungen erhalten zu bleiben
- Replay-Fähigkeit zu unterstützen

**Update-Hooks**:
Der Memory-State wird aktiv aktualisiert durch Event-Hooks:

- **onTurnEnd()**: Am Ende jeder Runde
  - Update Enemy-Predictions
  - Decay alter Informationen
  - Check Goal-Progress
  
- **onEnemySpotted()**: Wenn neue Einheit gesichtet wird
  - Update Enemy-Contact
  - Update Sector-Threat
  
- **onMineDiscovered()**: Wenn Mine gefunden/ausgelöst wird
  - Add zu Mine-Liste
  - Update Threat für Position

- **onUnitLost()**: Wenn eigene Einheit zerstört wird
  - Remove von Gruppe
  - Prüfe Gruppen-Auflösung

---

## 5. Integration und Ablauf

### 5.1 Turn-Beginn: Strategic Decision

Zu Beginn der KI-Runde läuft der Strategic MCTS:

1. **State-Erstellung**: Erzeuge GroupGameState aus aktuellem GameMap + Memory
2. **MCTS-Search**: 1000-5000 Iterationen auf strategischer Ebene
3. **Entscheidung**: Wähle beste strategische Aktionen für alle Gruppen
4. **Aktualisiere Memory**: Speichere neue Objectives, Research-Priorities

### 5.2 Tactical Execution

Für jede Gruppe mit neuem Objective:

1. **State-Erstellung**: Erzeuge kompakten Tactical State (nur relevante Units/Terrain)
2. **MCTS-Search**: 500-2000 Iterationen auf taktischer Ebene
3. **Ausführung**: Führe geplante Moves/Angriffe aus
4. **Feedback**: Melde Erfolg/Misserfolg zurück an Strategic Layer

### 5.3 Turn-Ende: Memory-Update

Am Ende der Runde:

1. **Update Fog-of-War Memory**: Decay alte Informationen, update Predictions
2. **Check Goal-Progress**: Sind Prerequisites erfüllt? Objectives erreicht?
3. **Resource-Allocation**: Investiere gesammelte Ressourcen in Research/Build-Queue
4. **Serialize Memory**: Speichere State in Savegame

---

## 6. Vorteile dieser Architektur

### 6.1 Dramatische Space-Reduktion

**Ohne Hierarchie**:
- 100 Einheiten × 50 Moves = 5000+ Aktionen
- Branching-Factor: unmöglich
- State-Clone: 50 MB

**Mit Hierarchie**:
- Strategic: 10 Gruppen × 10 Objectives = 100 Aktionen
- Tactical: 10 Units × 5 Moves = 50 Aktionen pro Gruppe
- Branching-Factor: handhabbar
- State-Clone: 100 KB (Strategic) + 20 KB (Tactical)

### 6.2 Parallelisierbarkeit

- Tactical MCTS für verschiedene Gruppen kann parallel laufen
- Gruppen operieren in verschiedenen Sektoren → wenig Interaktion
- Moderne Multi-Core CPUs können voll genutzt werden

### 6.3 Strategische Intelligenz

- Langfrist-Planung durch Goal-System
- Konsistentes Verhalten (nicht chaotisch von Runde zu Runde)
- Lernfähigkeit durch Decision-History
- Intelligente Fog-of-War-Nutzung

### 6.4 Performance

- Keine teuren Full-Clones
- Cached Metriken (Threat, Strength)
- Sektor-basierte Abstraktion reduziert Berechnungen
- Spatial Locality durch Gruppen-Begrenzung

---

## 7. Herausforderungen und offene Fragen

### 7.1 Koordinations-Problem

Wie koordinieren Gruppen sich ohne zentrale Kontrolle?

**Mögliche Lösung**: Strategic MCTS berücksichtigt Gruppen-Interaktionen explizit bei der Evaluation. Gruppen, die sich gegenseitig blockieren, erhalten schlechte Bewertung.

### 7.2 Fog-of-War Determinization

Wie geht MCTS mit unsichtbaren Feinden um?

**Mögliche Lösung**: Nutze Memory-basierte Predictions als "Best-Guess". MCTS simuliert mit geschätzten Enemy-Positionen. Risk-aware Evaluation bevorzugt sichere Züge.

### 7.3 Gruppen-Bildung

Wie werden Units initial zu Gruppen zusammengefasst?

**Mögliche Lösung**: Heuristik basierend auf:
- Unit-Typ (Combat-Units zusammen)
- Räumliche Nähe
- Shared Objectives (z.B. alle Units im Sektor X)
- Kann dynamisch re-clustered werden

### 7.4 Strategic vs. Tactical Balance

Wie viel MCTS-Budget wird Strategic vs. Tactical zugewiesen?

**Mögliche Lösung**: Adaptive Allokation:
- Early Game: Mehr Strategic (wenig Units, viel Planning)
- Mid Game: Balance
- Late Game: Mehr Tactical (viele Units, Feinschliff wichtig)

---

## 8. Implementierung

Die detaillierte Implementierungs-Roadmap wurde in ein separates Dokument ausgelagert:

👉 **Siehe**: [`implementation_roadmap.md`](implementation_roadmap.md)

Die Roadmap fokussiert sich auf einen **schnellen funktionalen Durchstich** mit Schwerpunkt auf der **taktischen Ebene (Combat)**. Alle anderen Komponenten (Strategic Layer, Memory-System, Gruppen-Bildung) werden architektonisch vorbereitet, aber zunächst nur als Dummy-Implementierungen bereitgestellt.

**MVP-Timeline**: 7-11 Wochen für funktionierenden Tactical Combat mit MCTS

---

## 9. Erfolgskriterien

Die Architektur ist erfolgreich, wenn:

1. **Performance**: KI-Zug dauert < 10 Sekunden auf 128×128 Map mit 50 Units
2. **Skalierbarkeit**: System funktioniert auch auf 256×256 Maps mit 100+ Units
3. **Intelligenz**: KI verfolgt erkennbare Langzeit-Strategien (z.B. Research → Build → Produce)
4. **Memory**: KI reagiert auf vergangene Feind-Sichtungen sinnvoll
5. **Vergleichbarkeit**: KI schlägt bestehende ASC-KI in 70%+ der Testspiele

---

## 10. Zusammenfassung

Diese hierarchische Architektur löst die beiden zentralen Herausforderungen für MCTS in ASC:

**Problem 1: Kombinatorische Explosion**
- Lösung: Gruppierung reduziert Branching von ~5000 auf ~100
- Lösung: Sektor-Abstraktion reduziert Map-Komplexität von 65.000 auf 60 Felder

**Problem 2: Langfrist-Planung**
- Lösung: Persistenter Memory-State speichert Goals, Research, Enemy-History
- Lösung: Multi-Turn Tracking ermöglicht konsistente Strategien

Die Architektur ist modular aufgebaut und kann schrittweise implementiert werden. Jede Phase liefert eigenständig funktionierende Komponenten, die getestet und verfeinert werden können.
