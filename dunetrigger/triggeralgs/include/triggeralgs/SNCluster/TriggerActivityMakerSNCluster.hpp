/**
 * @file TriggerActivityMakerSNCluster.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SNCLUSTER_TRIGGERACTIVITYMAKERSNCLUSTER_HPP_
#define TRIGGERALGS_SNCLUSTER_TRIGGERACTIVITYMAKERSNCLUSTER_HPP_

// Include necessary trigger algorithm and type definitions
#include "dunetrigger/triggeralgs/include/triggeralgs/TriggerActivityFactory.hpp"
#include "dunetrigger/triggeralgs/include/triggeralgs/Types.hpp"

#include <vector>

namespace triggeralgs {

// Class for creating TriggerActivities from TriggerPrimitives in the context of supernova clustering
class TriggerActivityMakerSNCluster : public TriggerActivityMaker
{
public:
  // Main function call operator: processes a single TriggerPrimitive and potentially outputs TriggerActivities
  void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta);

  // Configure the module with JSON configuration parameters
  void configure(const nlohmann::json &config);

private:
  // Internal class to maintain and manipulate a sliding time window of TriggerPrimitives
  class Window {
  public:

    // Check if the window currently contains any TriggerPrimitives
    bool is_empty() const {
      return tp_list.empty();
    }

    // Add a TriggerPrimitive to the current window
    void add(TriggerPrimitive const &input_tp) {
      tp_list.emplace_back(input_tp);
    }

    // Clear all TriggerPrimitives from the window
    void clear() {
      tp_list.clear();
    }

    // Slide the window forward: remove old TPs to maintain window size and add a new TP
    void move(TriggerPrimitive const &input_tp, timestamp_t const &window_length) {
      uint32_t n_tps_to_erase = 0;
      for (auto tp : tp_list) {
        if (!(input_tp.time_start - tp.time_start < window_length)) {
          n_tps_to_erase++;
        } else break;
      }

      // Safety check to avoid invalid range
      if (n_tps_to_erase > tp_list.size()) {
        n_tps_to_erase = tp_list.size();
      }

      // Remove outdated TPs from the beginning of the list
      tp_list.erase(tp_list.begin(), tp_list.begin() + n_tps_to_erase);

      // Update window start time and add the new TP
      if (!tp_list.empty()) {
        time_start = tp_list.front().time_start;
        add(input_tp);
      } else {
        // If window is now empty, reset it with the new TP
        reset(input_tp);
      }
    }

    // Reset the window with a single new TriggerPrimitive
    void reset(TriggerPrimitive const &input_tp) {
      tp_list.clear();
      time_start = input_tp.time_start;
      tp_list.emplace_back(input_tp);
    }

    // Output stream helper for debugging
    friend std::ostream& operator<<(std::ostream& os, const Window& window) {
      if (window.is_empty()) os << "Window is empty!\n";
      else {
        os << "Window start: " << window.time_start << ", end: " << window.tp_list.back().time_start;
        os << ". Total of: " << window.tp_list.size() << " TPs.\n";
      }
      return os;
    }

    // Get the number of TriggerPrimitives in the current window
    int get_n_members() const {
      return tp_list.size();
    }

    // Compute the total ADC integral of all TPs in the window
    uint64_t get_total_adc() const {
      uint64_t total_adc = 0;
      for (auto tp : tp_list) {
        total_adc += tp.adc_integral;
      }
      return total_adc;
    }

    // Time when the window starts (i.e., first TP's start time)
    timestamp_t time_start;

    // Container of TriggerPrimitives currently in the window
    std::vector<TriggerPrimitive> tp_list;
  };

  // Construct a TriggerActivity from the current window content
  TriggerActivity construct_ta() const;

  // Check whether the TriggerPrimitives in the window meet adjacency requirements (e.g., channel adjacency)
  bool adjacency_check(const Window& window) const;

  // Instance of current window being used to accumulate TPs
  Window m_current_window;

  // Counter to track total number of TPs processed
  uint64_t m_primitive_count = 0;

  // ----------------------
  // Configurable parameters
  // ----------------------

  // Time length of the sliding window (in timestamp units: clock ticks)
  timestamp_t m_window_length = 100000;

  // Minimum number of TPs required in a window to pass n TPs check
  uint32_t m_n_TPs_threshold = 10000;

  // Minimum total ADC integral required in a window to trigger
  uint32_t m_adc_threshold = 1200000;

  // Minimum and maximum time-over-threshold (tot) bounds for TPs, filtering
  uint32_t m_tot_min = 0;
  uint32_t m_tot_max = 99999999;

  // Maximum span (in channel IDs) allowed among TPs to form a valid cluster
  uint32_t m_channel_span_max = 20;

  // Minimum number of adjacent TPs needed to form a valid SN cluster
  uint32_t m_n_adjacent_tps_min = 5;

  // Maximum number of allowable gaps between adjacent channel TPs in a cluster
  uint32_t m_n_allowed_channel_gaps = 1;
};

} // namespace triggeralgs

#endif  // TRIGGERALGS_SNCLUSTER_TRIGGERACTIVITYMAKERSNCLUSTER_HPP_

