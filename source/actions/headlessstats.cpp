#include "../headlessstats.h"

#include <algorithm>
#include <set>
#include <vector>

#include <sigc++/sigc++.h>

#include "../containerbase.h"
#include "../gamemap.h"

namespace {

bool g_enabled = false;
GameMap* g_map = NULL;
std::vector<HeadlessPlayerStats> g_stats;
std::vector<int> g_participants;
std::string g_logFile;
std::vector<std::vector<long long>> g_damageMatrix;
std::vector<std::vector<long long>> g_killMatrix;
struct HeadlessStatsContext {
   int attacker;
   int defender;
   bool reactionFire;
};
std::vector<HeadlessStatsContext> g_contextStack;
std::set<const ContainerBase*> g_recentlyRecordedKills;
sigc::connection g_destroyConnection;

void onContainerDestroyed(ContainerBase* container) {
   if (!g_enabled || !container)
      return;

   if (g_map && container->getMap() != g_map)
      return;

   std::set<const ContainerBase*>::iterator recorded = g_recentlyRecordedKills.find(container);
   if (recorded != g_recentlyRecordedKills.end()) {
      g_recentlyRecordedKills.erase(recorded);
      return;
   }

   int owner = container->getOwner();
   if (owner >= 0 && owner < static_cast<int>(g_stats.size()))
      g_stats[owner].unitsLost++;
}

}  // namespace

void headlessStatsBegin(GameMap* map, int playerCount, const std::vector<int>& participants,
                        const std::string& logFilePath) {
   if (g_destroyConnection.connected())
      g_destroyConnection.disconnect();

   g_enabled = true;
   g_map = map;
   g_stats.assign(playerCount, HeadlessPlayerStats());
   g_participants = participants;
   g_logFile = logFilePath;

   g_damageMatrix.assign(playerCount, std::vector<long long>(playerCount, 0));
   g_killMatrix.assign(playerCount, std::vector<long long>(playerCount, 0));
   g_contextStack.clear();
   g_recentlyRecordedKills.clear();

   g_destroyConnection =
      ContainerBase::anyContainerDestroyed.connect(sigc::ptr_fun(&onContainerDestroyed));
}

void headlessStatsEnd() {
   if (g_destroyConnection.connected())
      g_destroyConnection.disconnect();

   g_enabled = false;
   g_map = NULL;
   g_stats.clear();
   g_participants.clear();
   g_logFile.clear();
   g_damageMatrix.clear();
   g_killMatrix.clear();
   g_contextStack.clear();
}

void headlessStatsRecordDamage(int attackerPlayerIndex, int defenderPlayerIndex, int amount) {
   if (!g_enabled || amount <= 0)
      return;

   int effectiveAttacker = attackerPlayerIndex;
   int effectiveDefender = defenderPlayerIndex;
   bool reactionFire = false;

   if (!g_contextStack.empty()) {
      const HeadlessStatsContext& ctx = g_contextStack.back();
      if (ctx.attacker >= 0)
         effectiveAttacker = ctx.attacker;
      if (ctx.defender >= 0)
         effectiveDefender = ctx.defender;
      reactionFire = ctx.reactionFire;
   }

   if (effectiveDefender >= 0 && effectiveDefender < static_cast<int>(g_stats.size())) {
      g_stats[effectiveDefender].damageTaken += amount;
      if (reactionFire)
         g_stats[effectiveDefender].reactionFireDamageTaken += amount;
   }

   if (effectiveAttacker >= 0 && effectiveAttacker < static_cast<int>(g_stats.size())) {
      g_stats[effectiveAttacker].damageDone += amount;
      if (reactionFire)
         g_stats[effectiveAttacker].reactionFireDamageDone += amount;

      if (effectiveDefender >= 0 && effectiveDefender < static_cast<int>(g_stats.size()))
         g_damageMatrix[effectiveAttacker][effectiveDefender] += amount;
   }
}

void headlessStatsRecordKill(int attackerPlayerIndex, int defenderPlayerIndex,
                             ContainerBase* destroyedContainer) {
   if (!g_enabled)
      return;

   int effectiveAttacker = attackerPlayerIndex;
   int effectiveDefender = defenderPlayerIndex;
   bool reactionFire = false;

   if (!g_contextStack.empty()) {
      const HeadlessStatsContext& ctx = g_contextStack.back();
      if (ctx.attacker >= 0)
         effectiveAttacker = ctx.attacker;
      if (ctx.defender >= 0)
         effectiveDefender = ctx.defender;
      reactionFire = ctx.reactionFire;
   }

   if (effectiveAttacker >= 0 && effectiveAttacker < static_cast<int>(g_stats.size())) {
      g_stats[effectiveAttacker].unitsDestroyed++;
      if (reactionFire)
         g_stats[effectiveAttacker].reactionFireKills++;
   }

   if (effectiveDefender >= 0 && effectiveDefender < static_cast<int>(g_stats.size())) {
      g_stats[effectiveDefender].unitsLost++;
      if (reactionFire)
         g_stats[effectiveDefender].reactionFireLosses++;
   }

   if (effectiveAttacker >= 0 && effectiveAttacker < static_cast<int>(g_stats.size()) &&
       effectiveDefender >= 0 && effectiveDefender < static_cast<int>(g_stats.size()))
      g_killMatrix[effectiveAttacker][effectiveDefender]++;

   if (destroyedContainer)
      g_recentlyRecordedKills.insert(destroyedContainer);
}

void headlessStatsPushContext(int attackerPlayerIndex, int defenderPlayerIndex, bool reactionFire) {
   if (!g_enabled)
      return;

   g_contextStack.push_back(
      HeadlessStatsContext{attackerPlayerIndex, defenderPlayerIndex, reactionFire});
}

void headlessStatsPopContext() {
   if (!g_enabled)
      return;

   if (!g_contextStack.empty())
      g_contextStack.pop_back();
}
const std::vector<HeadlessPlayerStats>& headlessStatsData() {
   return g_stats;
}

const std::vector<int>& headlessStatsParticipants() {
   return g_participants;
}

const std::string& headlessStatsLogFile() {
   return g_logFile;
}

const std::vector<std::vector<long long>>& headlessStatsDamageMatrix() {
   return g_damageMatrix;
}

const std::vector<std::vector<long long>>& headlessStatsKillMatrix() {
   return g_killMatrix;
}

bool headlessStatsEnabled() {
   return g_enabled;
}
