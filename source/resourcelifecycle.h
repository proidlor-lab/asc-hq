/*
    Advanced Strategic Command
    Resource lifecycle helper to coordinate cleanup of global systems.
*/

#ifndef RESOURCE_LIFECYCLE_H
#define RESOURCE_LIFECYCLE_H

#include <functional>
#include <utility>
#include <vector>
#include <cstdio>

class ResourceLifecycle {
      std::vector<std::function<void()>> callbacks;
      bool shutdownTriggered;
      ResourceLifecycle() : shutdownTriggered(false) {}
   public:
      static ResourceLifecycle& Instance()
      {
         static ResourceLifecycle instance;
         return instance;
      }

      void registerCleanup( std::function<void()> fn )
      {
         if ( shutdownTriggered )
            return;
         callbacks.push_back( std::move( fn ));
      }

      void shutdown()
      {
         if ( shutdownTriggered )
            return;
         shutdownTriggered = true;

         for ( std::vector<std::function<void()>>::reverse_iterator it = callbacks.rbegin(); it != callbacks.rend(); ++it ) {
            try {
               (*it)();
            } catch ( ... ) {
            }
         }
         callbacks.clear();
      }
};

#endif
