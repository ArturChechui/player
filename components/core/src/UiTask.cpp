#include "UiTask.hpp"

namespace core {

UiTask::UiTask(services::UiService &ui) : mUiQueue(nullptr), mUiService(ui) {}

bool UiTask::init() { return false; }

void UiTask::post(const UiEvent &e) {
  // TBD: implement posting event to the queue
}

void UiTask::runLoop() {
  // TBD: implement the UI task loop
  while (true) {
    mUiService.render(/* TBD: get model from queue */);
  }
}

} // namespace core