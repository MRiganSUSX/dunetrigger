/**
 * @file TriggerActivityMakerSNCluster.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "dunetrigger/triggeralgs/include/triggeralgs/SNCluster/TriggerActivityMakerSNCluster.hpp"

#include "TRACE/trace.h"
#define TRACE_NAME "TriggerActivityMakerSNClusterPlugin"

#include <vector>
#include <unordered_set>

using namespace triggeralgs;
using Logging::TLVL_DEBUG_ALL;
using Logging::TLVL_DEBUG_HIGH;
using Logging::TLVL_DEBUG_LOW;
using Logging::TLVL_IMPORTANT;

void
TriggerActivityMakerSNCluster::operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta)
{
  
  // The first time operator is called, reset
  // window object.
  if(m_current_window.is_empty()){
    m_current_window.reset(input_tp);
    m_primitive_count++;
    return;
  } 

  // ----> ToT Filter Stage <----
  if (input_tp.time_over_threshold <= m_tot_min || input_tp.time_over_threshold >= m_tot_max) {
    std::cout << "[TAM:SNCluster] TP rejected due to ToT: " << input_tp.time_over_threshold << std::endl;
    return;
  }

  // ----> Window logic starts here <----
  // If the difference between the current TP's start time and the start of the window
  // is less than the specified window size, add the TP to the window.
  if((input_tp.time_start - m_current_window.time_start) < m_window_length){
    std::cout << "[TAM:SNCluster] Window not yet complete, adding the input_tp to the window." << std::endl;
    m_current_window.add(input_tp);
  }
  // If the addition of the current TP to the window would make it longer
  // than the specified window length, don't add it but check whether the number of TPs in
  // the existing window is above the specified threshold. If it is, continue other TH checks...
  else {

    std::cout << "[TAM:SNCluster] Window full, running checks sequentially." << std::endl;

    // Check 1: TP count
    if (m_current_window.get_n_members() >= m_n_TPs_threshold) {
        std::cout << "[TAM:SNCluster] TP count check passed." << std::endl;

        // Check 2: ADC threshold
        if (m_current_window.get_total_adc() >= m_adc_threshold) {
            std::cout << "[TAM:SNCluster] ADC threshold check passed." << std::endl;

            // Check 3: Adjacency (expensive)
            
	    if (adjacency_check(m_current_window)) {
                std::cout << "[TAM:SNCluster] Adjacency check passed. Creating TA." << std::endl;
                output_ta.push_back(construct_ta());
                m_current_window.reset(input_tp);
                return;
            }
            else {
                std::cout << "[TAM:SNCluster] Adjacency check failed." << std::endl;
            }
	    
        }
        else {
            std::cout << "[TAM:SNCluster] ADC threshold check failed." << std::endl;
        }
    }
    else {
        std::cout << "[TAM:SNCluster] TP count check failed." << std::endl;
    }

    // If any check failed, move the window forward
    std::cout << "[TAM:SNCluster] Moving window forward." << std::endl;
    m_current_window.move(input_tp, m_window_length);
  }

  std::cout << "[TAM:SNCluster] " << m_current_window << std::endl;

  m_primitive_count++;

  return;
}

void
TriggerActivityMakerSNCluster::configure(const nlohmann::json &config)
{
  //FIXME use some schema here
  if (config.is_object()){
    if (config.contains("window_length")) m_window_length = config["window_length"];
    if (config.contains("adc_threshold")) m_adc_threshold = config["adc_threshold"];
    if (config.contains("n_of_TPs")) m_n_TPs_threshold = config["n_of_TPs"];
    if (config.contains("tot_min")) m_tot_min = config["tot_min"];
    if (config.contains("tot_max")) m_tot_max = config["tot_max"];
    if (config.contains("channel_span_max")) m_channel_span_max = config["channel_span_max"];
    if (config.contains("n_adjacent_tps_min")) m_n_adjacent_tps_min = config["n_adjacent_tps_min"];
    if (config.contains("n_allowed_channel_gaps")) m_n_allowed_channel_gaps = config["n_allowed_channel_gaps"];
    std::cout << "[TAM:SNCluster] Config:" << std::endl;
    std::cout << "[TAM:SNCluster] window_length:" << m_window_length << std::endl;
    std::cout << "[TAM:SNCluster] adc_threshold:" << m_adc_threshold << std::endl;
    std::cout << "[TAM:SNCluster] n_of_TPs:" << m_n_TPs_threshold << std::endl;
    std::cout << "[TAM:SNCluster] tot_min:" << m_tot_min << std::endl;
    std::cout << "[TAM:SNCluster] tot_max:" << m_tot_max << std::endl;
    std::cout << "[TAM:SNCluster] m_channel_span_max:" << m_channel_span_max << std::endl;
    std::cout << "[TAM:SNCluster] m_n_adjacent_tps_min:" << m_n_adjacent_tps_min << std::endl;
    std::cout << "[TAM:SNCluster] m_n_allowed_channel_gaps:" << m_n_allowed_channel_gaps << std::endl << std::endl;
  }
  else{
    std::cout << "[TAM:SNCluster] The DEFAULT values of window_length and adc_threshold are being used." << std::endl;
  }
}

TriggerActivity
TriggerActivityMakerSNCluster::construct_ta() const
{
  std::cout << "[TAM:SNCluster] I am constructing a trigger activity!" << std::endl;

  TriggerPrimitive latest_tp_in_window = m_current_window.tp_list.back();
  // The time_peak, time_activity, channel_* and adc_peak fields of this TA are irrelevent
  // for the purpose of this trigger alg.
  TriggerActivity ta;
  ta.time_start = m_current_window.time_start;
  ta.time_end = latest_tp_in_window.time_start + latest_tp_in_window.time_over_threshold;
  ta.time_peak = latest_tp_in_window.time_peak;
  ta.time_activity = latest_tp_in_window.time_peak;
  ta.channel_start = latest_tp_in_window.channel;
  ta.channel_end = latest_tp_in_window.channel;
  ta.channel_peak = latest_tp_in_window.channel;
  ta.adc_integral = m_current_window.get_total_adc();
  ta.adc_peak = latest_tp_in_window.adc_peak;
  ta.detid = latest_tp_in_window.detid;
  ta.type = TriggerActivity::Type::kTPC;
  ta.algorithm = TriggerActivity::Algorithm::kSupernova;
  ta.inputs = m_current_window.tp_list;
  return ta;
}

bool
TriggerActivityMakerSNCluster::adjacency_check(const Window& window) const
{
  if (window.tp_list.size() < m_n_adjacent_tps_min) return false;

  // Sort TPs by channel
  std::vector<TriggerPrimitive> sorted_tps = window.tp_list;
  std::sort(sorted_tps.begin(), sorted_tps.end(),
            [](const TriggerPrimitive& a, const TriggerPrimitive& b) {
              return a.channel < b.channel;
            });

  for (size_t i = 0; i < sorted_tps.size(); ++i) {
    std::vector<TriggerPrimitive> cluster;
    cluster.push_back(sorted_tps[i]);

    uint32_t gap_count = 0;
    uint32_t prev_channel = sorted_tps[i].channel;

    for (size_t j = i + 1; j < sorted_tps.size(); ++j) {
      uint32_t curr_channel = sorted_tps[j].channel;

      if (curr_channel > prev_channel + 1) {
        gap_count += (curr_channel - prev_channel - 1);
        if (gap_count > m_n_allowed_channel_gaps) break;
      }

      cluster.push_back(sorted_tps[j]);
      prev_channel = curr_channel;
    }

    if (cluster.size() >= m_n_adjacent_tps_min) {
      // Check span
      uint32_t span = cluster.back().channel - cluster.front().channel;
      if (span <= m_channel_span_max) {
        // Check ADC sum
        uint32_t adc_sum = 0;
        for (const auto& tp : cluster) {
          adc_sum += tp.adc_integral;
        }
        if (adc_sum >= m_adc_threshold) {
          return true;
        }
      }
    }

    // If the current cluster failed, try the next start index (i+1)
  }

  return false;
}

// Register algo in TA Factory
REGISTER_TRIGGER_ACTIVITY_MAKER(TRACE_NAME, TriggerActivityMakerSNCluster)
