#pragma once

namespace core {
class IUiSink;

class AppController {
   public:
    AppController(IUiSink& uiSink);
    bool init();

   private:
    IUiSink& mUiSink;
};

}  // namespace core
