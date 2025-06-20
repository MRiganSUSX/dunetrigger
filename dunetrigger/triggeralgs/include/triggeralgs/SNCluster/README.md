# TriggerActivityMakerSNCluster

## Overview

`TriggerActivityMakerSNCluster` is a Supernova-specific trigger algorithm for use within the DUNE trigger system. It processes individual `TriggerPrimitive`s (TPs), groups them into sliding time windows, and generates `TriggerActivity` objects when a window meets certain conditions that suggest possible supernova activity.

## Purpose

This algorithm is designed to detect low-energy, long-duration activity indicative of supernova bursts by forming clusters of TPs over extended time windows and applying thresholds on quantity, signal strength (ADC), and spatial adjacency.

---

## Core Logic

1. **Input Handling**:
   - Accepts individual `TriggerPrimitive` objects as input.
   - Stores and evaluates them within a sliding `Window`.

2. **Time Windowing**:
   - A fixed-length time window accumulates TPs.
   - If a new TP would fall outside the current window, a series of checks are performed before shifting the window forward.

3. **Filtering Criteria**:
   - **ToT (Time over Threshold)**: Rejects TPs whose durations (in time) are too short or too long.
   - **Minimum TP Count**: Requires a minimum number of TPs in the window.
   - **Total ADC Threshold**: Sum of ADC integrals must exceed a configured threshold.
   - **Adjacency Check**:
     - TPs must be close in channel number.
     - Some channel gaps are tolerated, up to a configured limit.
     - A valid cluster must contain enough adjacent TPs and span a channel range no greater than the configured maximum.

4. **TriggerActivity Creation**:
   - If all criteria are met, a `TriggerActivity` is created from the current window.

---

## Configuration Parameters

These are configurable via JSON:

| Parameter                  | Description                                                      | Default       |
|---------------------------|------------------------------------------------------------------|---------------|
| `window_length`           | Size of the sliding time window (in clock ticks)                 | `100000`      |
| `n_of_TPs`                | Minimum number of TPs in the window                              | `10000`       |
| `adc_threshold`           | Minimum total ADC in the window                                  | `1200000`     |
| `tot_min` / `tot_max`     | ToT range for filtering out bad TPs (too short or too long in time) | `0` / `99999999` |
| `channel_span_max`        | Maximum allowed span of channels in a cluster                    | `20`          |
| `n_adjacent_tps_min`      | Minimum adjacent TPs required for a valid cluster                | `5`           |
| `n_allowed_channel_gaps`  | Allowed non-contiguous gaps in a cluster (channel-wise)          | `1`           |

---

## Output

When all trigger conditions are met, the algorithm emits a `TriggerActivity` object representing a potential supernova signal cluster. The TA includes time bounds, ADC totals, and all contributing TPs.

---

## Notes

- The `adjacency_check` function is the most computationally intensive step and is only performed when other conditions are satisfied.
- Debug output is printed to `stdout` with the `[TAM:SNCluster]` prefix for traceability.
- The algorithm is registered via the `REGISTER_TRIGGER_ACTIVITY_MAKER` macro for use in the DUNE trigger framework.

