/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "overview_map_generator_service.h"
#include "gamemap.h"
#include "overviewmapimage.h"
#include "spfst.h"
#include "events.h"
#include <cassert>

namespace asc {
namespace ui {

// Static member initialization
sigc::signal<void> OverviewMapGeneratorService::generationComplete;

OverviewMapGeneratorService::OverviewMapGeneratorService(GameMap& gamemap)
   : map(gamemap),
     initialized(false),
     secondMapReady(false),
     completed(false),
     connected(false),
     x(0),
     y(0) {}

void OverviewMapGeneratorService::connect() {
   if (!connected) {
      idleEvent.connect(sigc::mem_fun(*this, &OverviewMapGeneratorService::idleHandler));
      connected = true;
   }
}

bool OverviewMapGeneratorService::idleHandler() {
   int t = ticker;
   while (!completed && (t + 5 > ticker))
      drawNextField(true);
   return true;
}

bool OverviewMapGeneratorService::updateField(const MapCoordinate& pos) {
   SPoint imgpos = OverviewMapImage::map2surface(pos);

   MapField* fld = map.getField(pos);
   VisibilityStates visi = fieldVisibility(fld, map.getPlayerView());
   if (visi == visible_not) {
      OverviewMapImage::fill(overviewMapImage, imgpos, 0xff545454);
   } else {
      if (fld->building && fieldvisiblenow(fld, map.getPlayerView()))
         OverviewMapImage::fill(overviewMapImage, imgpos,
                                map.player[fld->building->getOwner()].getColor());
      else {
         int w = fld->getWeather();
         fld->typ->getQuickView()->blit(overviewMapImage, imgpos);
         for (MapField::ObjectContainer::iterator i = fld->objects.begin(); i != fld->objects.end();
              ++i)
            if (visi > visible_ago || i->typ->visibleago)
               i->getOverviewMapImage(w)->blit(overviewMapImage, imgpos);

         if (fld->vehicle && fieldvisiblenow(fld, map.getPlayerView()))
            OverviewMapImage::fillCenter(overviewMapImage, imgpos,
                                         map.player[fld->vehicle->getOwner()].getColor());

         if (visi == visible_ago)
            OverviewMapImage::lighten(overviewMapImage, imgpos, 0.7);
      }
   }
   return true;
}

void OverviewMapGeneratorService::drawNextField(bool signalOnCompletion) {
   if (!init())
      return;

   if (x == map.xsize) {
      x = 0;
      ++y;
   }
   if (y < map.ysize) {
      if (!updateField(MapCoordinate(x, y)))
         return;

      ++x;
   }
   if (y == map.ysize) {
      completed = true;
      if (signalOnCompletion)
         generationComplete();

      completedMapImage = overviewMapImage.Duplicate();
      secondMapReady = true;
   }
}

Surface OverviewMapGeneratorService::createNewSurface() {
   Surface s;
   if (map.xsize > 0 && map.ysize > 0) {
      s = Surface::createSurface((map.xsize + 1) * 6, 4 + map.ysize * 2, 32, 0);
   }
   return s;
}

bool OverviewMapGeneratorService::init() {
   if (map.ysize <= 0 || map.xsize <= 0)
      return false;

   if (!initialized) {
      overviewMapImage = createNewSurface();
      initialized = true;
   }
   return initialized;
}

void OverviewMapGeneratorService::resetSize() {
   initialized = false;
}

const Surface& OverviewMapGeneratorService::getOverviewMap(bool complete) {
   bool initialized = init();
   assert(initialized);
   if (complete)
      while (!completed)
         drawNextField(false);

   if (secondMapReady)
      return completedMapImage;
   else
      return overviewMapImage;
}

void OverviewMapGeneratorService::startUpdate() {
   completed = false;
   x = 0;
   y = 0;
}

void OverviewMapGeneratorService::clear(bool allImages) {
   if (!initialized)
      return;

   overviewMapImage.Fill(Surface::transparent);
   if (allImages) {
      if (completedMapImage.valid())
         completedMapImage.Fill(Surface::transparent);
      secondMapReady = false;
   }

   startUpdate();
}

void OverviewMapGeneratorService::clearmap(GameMap* actmap) {
   // This static method is provided for backward compatibility
   // In the new architecture, the UI owns the generator service
   // and can call clear() directly on it
   if (actmap && actmap->getOverviewMapGenerator()) {
      actmap->getOverviewMapGenerator()->clear();
   }
}

void OverviewMapGeneratorService::onGameEvent(const core::events::GameEvent& event) {
   using namespace core::events;

   // Listen to game events and update the overview map accordingly
   switch (event.type) {
      case GameEventType::CoordinateShift:
         // Map coordinates have shifted, mark for update
         startUpdate();
         break;

      case GameEventType::MapCreated:
      case GameEventType::MapDestroyed:
         // Significant map changes, clear and restart
         clear(true);
         break;

      default:
         // Other events don't affect the overview map
         break;
   }
}

} // namespace ui
} // namespace asc
