/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HEADLESS_INTERACTION_PROVIDER_H
#define HEADLESS_INTERACTION_PROVIDER_H

#include "core/ui_interfaces/user_interaction_provider.h"
#include <iostream>

namespace asc {
namespace core {
namespace interactions {

/**
 * @brief Headless implementation of IUserInteractionProvider
 *
 * This implementation is designed for server/headless mode where no
 * GUI is available. It provides sensible defaults for all interactions:
 * - Always uses default values for selections
 * - Confirms all requests with default answers
 * - Logs errors to stderr
 * - Optionally logs messages to stdout
 *
 * This enables the game to run in automated/server mode without user input.
 *
 * Phase 3 of GameMap refactoring: Remove guiHooked() tracking
 */
class HeadlessInteractionProvider : public ui_interfaces::IUserInteractionProvider {
  public:
   /**
    * @brief Constructor
    * @param verbose If true, log messages to stdout. If false, suppress messages.
    */
   explicit HeadlessInteractionProvider(bool verbose = false) : verbose_(verbose) {}

   ~HeadlessInteractionProvider() override = default;

   int selectOption(const std::string& prompt, const std::vector<std::string>& options, int defaultOption) override {
      if (verbose_) {
         std::cout << "[Headless] Selection: " << prompt << " -> Using default option " << defaultOption << std::endl;
      }
      return defaultOption;
   }

   bool confirm(const std::string& message, bool defaultValue) override {
      if (verbose_) {
         std::cout << "[Headless] Confirm: " << message << " -> " << (defaultValue ? "yes" : "no") << std::endl;
      }
      return defaultValue;
   }

   std::string requestInput(const std::string& prompt, const std::string& defaultValue) override {
      if (verbose_) {
         std::cout << "[Headless] Input: " << prompt << " -> \"" << defaultValue << "\"" << std::endl;
      }
      return defaultValue;
   }

   void showMessage(const std::string& message) override {
      if (verbose_) {
         std::cout << "[Headless] Message: " << message << std::endl;
      }
      // In non-verbose mode, suppress messages
   }

   void showError(const std::string& error) override {
      // Always log errors, even in non-verbose mode
      std::cerr << "[Headless] ERROR: " << error << std::endl;
   }

   bool isGuiAvailable() const override { return false; }

  private:
   bool verbose_;
};

} // namespace interactions
} // namespace core
} // namespace asc

#endif // HEADLESS_INTERACTION_PROVIDER_H
