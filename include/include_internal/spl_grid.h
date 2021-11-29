/**
 * @file spl_grid.h
 *
 * @brief Componenets for creating grids and grid labels
 *
 * @ingroup SimpleCustomPlotInternal
 *
 * @author Frans Rosencrantz
 * Contact: Frans.Rosencrantz@gmail.com
 *
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "scp_datamodels.h"
#include "scp_internal_datamodels.h";

namespace scp {
/**
 * A Parameter struct
 * Containing parameters for grid bounds and limits of the grids.
 */
struct GridConfigParams {
  /**
   * X and Y limitation of the grid.
   * First grid is drawn at lim.min and the last grid is drawn at lim.max
   */
  scp::ParamVal<scp::Lim_f> x_lim, y_lim;

  /**
   * The bounds of where the grids will be drawn.
   */
  scp::ParamVal<juce::Rectangle<int>> grid_area;

  /**
   * Set to true if grid should be visable.
   */
  scp::ParamVal<bool> grid_on;

  /**
   * Set to true if tiny grids should be used.
   */
  scp::ParamVal<bool> tiny_grid_on;
};

/**
 * Grid class implementation of grid component
 *
 * Componenet for creating grids and grid labels. The idea with this componenet
 * is to create the grids behind the actual graph(s) together with graph labels
 * outside the graph area. It can also be used to only create the grid labels
 * without the grids. Default are grids off.
 *
 */
template <Scaling x_scaling_T = Scaling::linear,
          Scaling y_scaling_T = Scaling::linear>
class Grid : public BaseGrid {
 public:
  /** @brief Set the bounds of where the grids will be drawn
   *
   *  The grid area must be within the bounds of this componenet. The
   *  grid labels will be draw with a half 'font_size' outside the grid area.
   *
   *  @param grid_area The area of where the grids will be drawn
   *  @return void.
   */
  void setGridBounds(const juce::Rectangle<int>& grid_area) override;

  /** @brief Set the Y-limits
   *
   *  Set the limits of Y-axis.
   *
   *  @param min minimum value
   *  @param max maximum value
   *  @return void.
   */
  void setYLim(const float min, const float max) override;

  /** @brief Set the X-limits
   *
   *  Set the limits of X-axis.
   *
   *  @param min minimum value
   *  @param max maximum value
   *  @return void.
   */
  void setXLim(const float min, const float max) override;

  /** @brief Display grids
   *
   *  Grids will be shown if grid_on is set to true. Grid labels will be shown
   *  in either case. Default is false.
   *
   *  @param grid_on set to true to show grids
   *  @param tiny_grid_on set to true to show tiny grids
   *  @return void.
   */
  void setGridON(const bool grid_on, const bool tiny_grids_on) override;

  /** @brief Override the x-ticks
   *
   *  Ticks are the markers denoting data points on axes.
   *
   *  @param x_labels x-labels to be shown.
   *  @return void.
   */
  void setXTicks(const std::vector<float>& x_ticks) override;

  /** @brief Override the x-labels
   *
   *  Override the auto generated x-labels.
   *
   *  @param x_labels x-labels to be shown.
   *  @return void.
   */
  void setXLabels(const std::vector<std::string>& x_labels) override;

  /** @brief Override the y-labels
   *
   *  Override the auto generated x-labels.
   *
   *  @param y_labels y-labels to be shown.
   *  @return void.
   */
  void setYLabels(const std::vector<std::string>& y_labels) override;

  /** @brief Override the y-ticks
   *
   *  Ticks are the markers denoting data points on axes.
   *
   *  @param y_labels y-labels to be shown.
   *  @return void.
   */
  void setYTicks(const std::vector<float>& y_ticks) override;

  /** @brief Update grids and grid labels
   *
   *  This function updates the grid if any new parameter is set. Should be
   *  called after an parameter is set to update the grid.
   *
   *  @return void.
   */
  void updateGrid();

  void resized() override;
  void paint(juce::Graphics& g) override;
  void lookAndFeelChanged() override;

 private:
  /** @brief Populate the x and y ticks
   *
   *  The idea is to use this function to populate the x_ticks & y_ticks.
   *
   *  @param x_ticks x-ticks to be populated.
   *  @param y_ticks y-ticks to be populated.
   *  @return void.
   */
  void createAutoGridTicks(std::vector<float>& x_ticks,
                           std::vector<float>& y_ticks);

  void createLabels();

  void updateGridInternal();

  void addGridLines(const std::vector<float>& ticks,
                    const GridLine::Direction direction);

  std::vector<GridLine> m_grid_lines;
  std::vector<float> m_custom_x_ticks, m_custom_y_ticks;
  std::vector<std::string> m_custom_x_labels, m_custom_y_labels;
  std::vector<juce::Path> m_grid_path;

  static constexpr auto x_scaling = x_scaling_T;
  static constexpr auto y_scaling = y_scaling_T;

 protected:
  LookAndFeelMethodsBase* m_lookandfeel;
  GridConfigParams m_config_params;

  std::vector<std::pair<std::string, juce::Rectangle<int>>> m_y_axis_labels,
      m_x_axis_labels;
};

/*============================================================================*/
/**
 * \class SemiLogXGrid
 * \brief A class component to create X, Y grids and grid labels
 *
 *  Both the x and y axis are scaled linearly.
 */
struct LinearGrid : Grid<Scaling::linear, Scaling::linear> {};

/**
 * \class SemiLogXGrid
 * \brief A class component to create X, Y grids and grid labels
 *
 *  The x axis is scaled logarithmically and y axis linearly.
 */
struct SemiLogXGrid : Grid<Scaling::logarithmic, Scaling::linear> {};

/**
 * \class SemiLogYGrid
 * \brief A class component to create X, Y grids and grid labels
 *
 *  The x axis is scaled linearly and y axis logarithmically.
 */
struct SemiLogYGrid : Grid<Scaling::linear, Scaling::logarithmic> {};
}  // namespace scp