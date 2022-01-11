#include "scp_lookandfeel.h"
#include "test_macros.h"


#define PI2 6.28318530718

TEST(test_xy_ticks, RealTime) {
  ADD_PLOT;

  std::vector<float> y_test_data(10);
  std::iota(y_test_data.begin(), y_test_data.end(), 1.f);

  std::vector<float> x_ticks = {0, 1, 7};
  std::vector<float> y_ticks = {3, 2, 9};

  PLOT_Y({y_test_data});
  X_TICKS(x_ticks);
  Y_TICKS(y_ticks);
}