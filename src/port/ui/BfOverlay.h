#pragma once

#include <libultraship/libultraship.h>

namespace BF {

// Live input overlay (TMInterface toggle_inputs equivalent).
class OverlayWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

  protected:
    void InitElement() override {
    }
    void UpdateElement() override {
    }
    void DrawElement() override;
};

} // namespace BF
