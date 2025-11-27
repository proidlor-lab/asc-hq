/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "gui_interaction_provider.h"
#include "dialog.h"
#include "dialogs/messagedialog.h"

namespace asc {
namespace core {
namespace interactions {

int GUIInteractionProvider::selectOption(const std::string& prompt, const std::vector<std::string>& options,
                                          int defaultOption) {
   if (options.empty()) {
      return defaultOption;
   }

   // For 2 options, use choice_dlg (binary choice)
   if (options.size() == 2) {
      int result = choice_dlg(prompt.c_str(), options[0].c_str(), options[1].c_str());
      // choice_dlg returns 1 for left button, 2 for right button
      return (result == 1) ? 0 : 1;
   }

   // For more than 2 options, we don't have a simple dialog
   // Show a message and return default for now
   // TODO: Implement proper multi-choice dialog when needed
   displaymessage2("%s (selecting default option %d)", prompt.c_str(), defaultOption);
   return defaultOption;
}

bool GUIInteractionProvider::confirm(const std::string& message, bool defaultValue) {
   int result = choice_dlg(message.c_str(), "Yes", "No");
   // choice_dlg returns 1 for left button (Yes), 2 for right button (No)
   return (result == 1);
}

std::string GUIInteractionProvider::requestInput(const std::string& prompt, const std::string& defaultValue) {
   // Use the text input dialog
   ASCString result = defaultValue.c_str();

   // Note: The actual implementation would need a text input dialog
   // For now, using a message dialog to show the prompt and returning default
   // A proper implementation would use an input dialog when available
   displaymessage2("%s", prompt.c_str());

   return defaultValue;
}

void GUIInteractionProvider::showMessage(const std::string& message) { displaymessage2("%s", message.c_str()); }

void GUIInteractionProvider::showError(const std::string& error) {
   // Show error with appropriate formatting
   displaymessage2("ERROR: %s", error.c_str());
}

} // namespace interactions
} // namespace core
} // namespace asc
