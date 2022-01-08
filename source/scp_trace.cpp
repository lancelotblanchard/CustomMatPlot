#include "scp_trace.h"

#include "spl_plot.h"

namespace scp {
template <class ValueType>
bool TracePoint<ValueType>::setGraphValue(
    const juce::Point<ValueType>& graph_value) {
  if (m_graph_values != graph_value) {
    m_graph_values = graph_value;
    return true;
  }
  return false;
}

template <class ValueType>
void TraceLabel<ValueType>::setGraphLabelFrom(
    const juce::Point<ValueType>& graph_value,
    const GraphAttributesView& graph_attribute) {
  if (m_lookandfeel) {
    auto lnf = static_cast<Plot::LookAndFeelMethods*>(m_lookandfeel);

    m_x_label.first =
        "X: " + valueToString(graph_value.getX(), graph_attribute, true).first;
    m_y_label.first =
        "Y: " + valueToString(graph_value.getY(), graph_attribute, false).first;

    const auto [x_label_bounds, y_label_bounds] =
        lnf->getTraceXYLabelBounds(m_x_label.first, m_y_label.first);

    m_x_label.second = x_label_bounds;
    m_y_label.second = y_label_bounds;
  }
}

template <class ValueType>
void TraceLabel<ValueType>::resized() {}

template <class ValueType>
void TraceLabel<ValueType>::paint(juce::Graphics& g) {
  if (m_lookandfeel) {
    auto lnf = static_cast<Plot::LookAndFeelMethods*>(m_lookandfeel);
    lnf->drawTraceLabel(g, m_x_label, m_y_label);
  }
}

template <class ValueType>
void TraceLabel<ValueType>::lookAndFeelChanged() {
  if (auto* lnf = dynamic_cast<Plot::LookAndFeelMethods*>(&getLookAndFeel())) {
    m_lookandfeel = lnf;
  } else {
    m_lookandfeel = nullptr;
  }
}

Trace::~Trace() {
  m_lookandfeel = nullptr;
  updateTracePointsLookAndFeel();
}

const GraphLine* Trace::getAssociatedGraphLine(
    const juce::Component* trace_point) const {
  const auto it = findTraceLabelPointIteratorFrom(trace_point);
  if (it == m_trace_labelpoints.end())
    return nullptr;
  else
    return it->associated_graph_line;
}

juce::Point<float> Trace::getGraphPosition(
    const juce::Component* trace_point_label) const {
  const auto tlp_it = findTraceLabelPointIteratorFrom(trace_point_label);

  if (tlp_it != m_trace_labelpoints.end())
    return tlp_it->trace_point->m_graph_values;

  return juce::Point<float>();
}

void Trace::addOrRemoveTracePoint(
    const juce::Point<float>& trace_point_coordinate,
    const GraphLine* graph_line) {
  if (std::find_if(m_trace_labelpoints.begin(), m_trace_labelpoints.end(),
                   [&trace_point_coordinate](const auto& tl) {
                     return (*tl.trace_point) == trace_point_coordinate;
                   }) == m_trace_labelpoints.end()) {
    addSingleTracePointAndLabel(trace_point_coordinate, graph_line);
  } else {
    removeSingleTracePointAndLabel(trace_point_coordinate);
  }
}

void Trace::updateTracePointsBoundsFrom(
    const GraphAttributesView& graph_attributes) {
  for (auto& tlp : m_trace_labelpoints) {
    updateSingleTraceLabelTextsAndBoundsInternal(&tlp, graph_attributes);
  }
}

void Trace::updateSingleTracePointBoundsFrom(
    juce::Component* trace_label_or_point,
    const GraphAttributesView& graph_attributes) {
  const auto tlp_it = findTraceLabelPointIteratorFrom(trace_label_or_point);

  if (tlp_it == m_trace_labelpoints.end()) {
    return;
  }

  auto it = m_trace_labelpoints.erase(
      tlp_it, tlp_it);  // remove const for ::const_iterator

  updateSingleTraceLabelTextsAndBoundsInternal(&(*it), graph_attributes);
}

void Trace::addAndMakeVisibleTo(juce::Component* parent_comp) {
  for (const auto& tlp : m_trace_labelpoints) {
    parent_comp->addAndMakeVisible(tlp.trace_label.get());
    parent_comp->addAndMakeVisible(tlp.trace_point.get());
  }
}

void Trace::setLookAndFeel(juce::LookAndFeel* lnf) {
  m_lookandfeel = lnf;
  updateTracePointsLookAndFeel();
}

bool Trace::setGraphPositionFor(juce::Component* trace_point,
                                const juce::Point<float>& new_position,
                                const GraphAttributesView& graph_attributes) {
  const auto tlp_it = findTraceLabelPointIteratorFrom(trace_point);
  auto it = m_trace_labelpoints.erase(tlp_it, tlp_it);  // remove const for ::const_iterator

  if (it != m_trace_labelpoints.end() &&
      it->trace_point->setGraphValue(new_position)) {
    updateSingleTraceLabelTextsAndBoundsInternal(&(*it), graph_attributes);

    return true;
  }

  return false;
}

bool Trace::setCornerPositionForLabelAssociatedWith(
    juce::Component* trace_point, const juce::Point<int>& mouse_position) {
  const auto tlp_it = findTraceLabelPointIteratorFrom(trace_point);

  auto it = m_trace_labelpoints.erase(
      tlp_it, tlp_it);  // remove const for ::const_iterator

  const auto dxdy = mouse_position - it->trace_point->getBounds().getPosition();

  const auto is_x_pos = dxdy.getX() > 0;
  const auto is_y_pos = dxdy.getY() > 0;

  auto trace_label_corner_pos = TraceLabelCornerPosition::top_left;

  if (is_x_pos && is_y_pos)
    trace_label_corner_pos = TraceLabelCornerPosition::top_left;
  else if (is_x_pos && !is_y_pos)
    trace_label_corner_pos = TraceLabelCornerPosition::bottom_left;
  else if (!is_x_pos && is_y_pos)
    trace_label_corner_pos = TraceLabelCornerPosition::top_right;
  else if (!is_x_pos && !is_y_pos)
    trace_label_corner_pos = TraceLabelCornerPosition::bottom_right;

  if (it->trace_label->trace_label_corner_pos != trace_label_corner_pos) {
    it->trace_label->trace_label_corner_pos = trace_label_corner_pos;
    return true;
  }

  return false;
}

bool Trace::isComponentTracePoint(const juce::Component* component) const {
  return dynamic_cast<const TracePoint_f*>(component) &&
         findTraceLabelPointIteratorFrom(component) !=
             m_trace_labelpoints.end();
}

bool Trace::isComponentTraceLabel(const juce::Component* component) const {
  return dynamic_cast<const TraceLabel_f*>(component) &&
         findTraceLabelPointIteratorFrom(component) !=
             m_trace_labelpoints.end();
}

void Trace::addSingleTracePointAndLabel(
    const juce::Point<float>& trace_point_coordinate,
    const GraphLine* graph_line) {
  auto trace_label = std::make_unique<TraceLabel_f>();
  auto trace_point = std::make_unique<TracePoint_f>();

  if (m_lookandfeel) trace_label->setLookAndFeel(m_lookandfeel);
  if (m_lookandfeel) trace_point->setLookAndFeel(m_lookandfeel);

  trace_point->setGraphValue(trace_point_coordinate);
  m_trace_labelpoints.push_back(
      {move(trace_label), move(trace_point), graph_line});
}

void Trace::removeSingleTracePointAndLabel(
    const juce::Point<float>& trace_point_coordinate) {
  m_trace_labelpoints.erase(
      std::remove_if(m_trace_labelpoints.begin(), m_trace_labelpoints.end(),
                     [&trace_point_coordinate](const auto& tlp) {
                       return *tlp.trace_point == trace_point_coordinate;
                     }));
}

void Trace::updateSingleTraceLabelTextsAndBoundsInternal(
    TraceLabelPoint_f* tlp, const GraphAttributesView& graph_attributes) {
  if (m_lookandfeel) {
    auto lnf = static_cast<Plot::LookAndFeelMethods*>(m_lookandfeel);

    const auto graph_values = tlp->trace_point->m_graph_values;
    tlp->trace_label->setGraphLabelFrom(graph_values, graph_attributes);

    const auto x_bound = tlp->trace_label->m_x_label.second;
    const auto y_bound = tlp->trace_label->m_y_label.second;

    const auto local_trace_bounds =
        lnf->getTraceLabelLocalBounds(x_bound, y_bound);
    auto trace_bounds = local_trace_bounds;

    const auto trace_position =
        lnf->getTracePointPositionFrom(graph_attributes, graph_values) +
        graph_attributes.graph_bounds.getPosition();

    switch (tlp->trace_label->trace_label_corner_pos) {
      case TraceLabelCornerPosition::top_left:
        trace_bounds.setPosition(trace_position);
        break;
      case TraceLabelCornerPosition::top_right:
        trace_bounds.setPosition(
            trace_position.getX() - trace_bounds.getWidth(),
            trace_position.getY());
        break;
      case TraceLabelCornerPosition::bottom_left:
        trace_bounds.setPosition(
            trace_position.getX(),
            trace_position.getY() - trace_bounds.getHeight());
        break;
      case TraceLabelCornerPosition::bottom_right:
        trace_bounds.setPosition(
            trace_position.getX() - trace_bounds.getWidth(),
            trace_position.getY() - trace_bounds.getHeight());
        break;
      default:
        break;
    }

    tlp->trace_label->setBounds(trace_bounds);

    auto trace_point_bounds = lnf->getTracePointLocalBounds();
    trace_point_bounds.setCentre(trace_position);
    tlp->trace_point->setBounds(trace_point_bounds);
  }
}

void Trace::updateTracePointsLookAndFeel() {
  for (auto& trace_label_point : m_trace_labelpoints) {
    trace_label_point.trace_label->setLookAndFeel(m_lookandfeel);
    trace_label_point.trace_point->setLookAndFeel(m_lookandfeel);
  }
}

std::vector<TraceLabelPoint_f>::const_iterator
Trace::findTraceLabelPointIteratorFrom(
    const juce::Component* trace_point_or_label) const {
  return std::find_if(m_trace_labelpoints.begin(), m_trace_labelpoints.end(),
                      [&trace_point_or_label](const auto& tpl) {
                        return tpl.trace_point.get() == trace_point_or_label ||
                               tpl.trace_label.get() == trace_point_or_label;
                      });
}

template <class ValueType>
void TracePoint<ValueType>::resized() {}

template <class ValueType>
void TracePoint<ValueType>::paint(juce::Graphics& g) {
  if (m_lookandfeel) {
    auto lnf = static_cast<Plot::LookAndFeelMethods*>(m_lookandfeel);
    lnf->drawTracePoint(g, getLocalBounds());
  }
}

template <class ValueType>
void TracePoint<ValueType>::lookAndFeelChanged() {
  if (auto* lnf = dynamic_cast<Plot::LookAndFeelMethods*>(&getLookAndFeel())) {
    m_lookandfeel = lnf;
  } else {
    m_lookandfeel = nullptr;
  }
}

}  // namespace scp
