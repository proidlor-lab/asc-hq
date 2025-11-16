#ifndef HEADLESSSTATS_H
#define HEADLESSSTATS_H

#include <string>
#include <vector>

class GameMap;
class ContainerBase;

struct HeadlessPlayerStats {
   long long damageDone;
   long long damageTaken;
   int unitsLost;
   int unitsDestroyed;
   long long reactionFireDamageDone;
   long long reactionFireDamageTaken;
   int reactionFireKills;
   int reactionFireLosses;
   HeadlessPlayerStats()
      : damageDone(0),
        damageTaken(0),
        unitsLost(0),
        unitsDestroyed(0),
        reactionFireDamageDone(0),
        reactionFireDamageTaken(0),
        reactionFireKills(0),
        reactionFireLosses(0) {}
};

void headlessStatsBegin(GameMap* map, int playerCount, const std::vector<int>& participants,
                        const std::string& logFilePath);
void headlessStatsEnd();

void headlessStatsRecordDamage(int attackerPlayerIndex, int defenderPlayerIndex, int amount);
void headlessStatsRecordKill(int attackerPlayerIndex, int defenderPlayerIndex,
                             ContainerBase* destroyedContainer);

void headlessStatsPushContext(int attackerPlayerIndex, int defenderPlayerIndex, bool reactionFire);
void headlessStatsPopContext();

class HeadlessStatsContextGuard {
   bool active;

  public:
   HeadlessStatsContextGuard(int attackerPlayerIndex, int defenderPlayerIndex, bool reactionFire)
      : active(true) {
      headlessStatsPushContext(attackerPlayerIndex, defenderPlayerIndex, reactionFire);
   }

   ~HeadlessStatsContextGuard() {
      if (active)
         headlessStatsPopContext();
   }

   void release() {
      if (active) {
         headlessStatsPopContext();
         active = false;
      }
   }
};

const std::vector<HeadlessPlayerStats>& headlessStatsData();
const std::vector<int>& headlessStatsParticipants();
const std::string& headlessStatsLogFile();
const std::vector<std::vector<long long>>& headlessStatsDamageMatrix();
const std::vector<std::vector<long long>>& headlessStatsKillMatrix();
bool headlessStatsEnabled();

#endif
