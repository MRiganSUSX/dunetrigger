/**
 * @file TriggerActivityMakerSNCluster.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TRIGGERALGS_SNCLUSTER_TRIGGERACTIVITYMAKERSNCLUSTER_HPP_
#define TRIGGERALGS_SNCLUSTER_TRIGGERACTIVITYMAKERSNCLUSTER_HPP_

#include "dunetrigger/triggeralgs/include/triggeralgs/TriggerActivityFactory.hpp"
#include "dunetrigger/triggeralgs/include/triggeralgs/Types.hpp"

#include <vector>

namespace triggeralgs {
class TriggerActivityMakerSNCluster : public TriggerActivityMaker
{

public:
  void operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_ta);
  
  void configure(const nlohmann::json &config);

private:  
  class Window {
    public:
      bool is_empty() const{
        return tp_list.empty();
      };
      void add(TriggerPrimitive const &input_tp){
        tp_list.emplace_back(input_tp);
      };
      void clear(){
        tp_list.clear();
      };
      void move(TriggerPrimitive const &input_tp, timestamp_t const &window_length){
        // Find all of the TPs in the window that need to be removed
        // if the input_tp is to be added and the size of the window
        // is to be conserved.
        uint32_t n_tps_to_erase = 0;
        for(auto tp : tp_list){
          if(!(input_tp.time_start-tp.time_start < window_length)){
            n_tps_to_erase++;
          }
          else break;
        }
        // Erase the TPs from the window.
        if (n_tps_to_erase > tp_list.size()) {
          n_tps_to_erase = tp_list.size();
        }
	tp_list.erase(tp_list.begin(), tp_list.begin()+n_tps_to_erase);
        // Make the window start time the start time of what is now the
        // first TP.
        if(tp_list.size()!=0){
          time_start = tp_list.front().time_start;
          add(input_tp);
        }
        else reset(input_tp);
      };
      void reset(TriggerPrimitive const &input_tp){
        // Empty the TP list.
        tp_list.clear();
        // Set the start time of the window to be the start time of the 
        // input_tp.
        time_start = input_tp.time_start;
        // Add the input TP to the TP list.
        tp_list.emplace_back(input_tp);
      };
      friend std::ostream& operator<<(std::ostream& os, const Window& window){
        if(window.is_empty()) os << "Window is empty!\n";
        else{
          os << "Window start: " << window.time_start << ", end: " << window.tp_list.back().time_start;
          os << ". Total of: " << window.tp_list.size() << " TPs.\n"; 
        }
        return os;
      };

      // New SN functions
      int get_n_members() const{
        return tp_list.size();
      };

      uint64_t get_total_adc() const{
        uint64_t total_adc = 0;
        for(auto tp : tp_list){
          total_adc += tp.adc_integral;
        }
        return total_adc;
      };

      timestamp_t time_start;
      std::vector<TriggerPrimitive> tp_list;
  };

  TriggerActivity construct_ta() const;
  bool adjacency_check(const Window& window) const;

  Window m_current_window;
  uint64_t m_primitive_count = 0;

  // Configurable parameters.
  timestamp_t m_window_length = 100000;
  uint32_t m_n_TPs_threshold = 10000;
  uint32_t m_adc_threshold = 1200000;
  uint32_t m_tot_min = 0; 
  uint32_t m_tot_max = 99999999;
  uint32_t m_channel_span_max = 20;           // Max allowed span in channel IDs
  uint32_t m_n_adjacent_tps_min = 5;          // Min adjacent TPs to count as a cluster
  uint32_t m_n_allowed_channel_gaps = 1;      // Max allowed gaps between adjacent TPs
};
} // namespace triggeralgs

#endif // TRIGGERALGS_SNCLUSTER_TRIGGERACTIVITYMAKERSNCLUSTER_HPP_
