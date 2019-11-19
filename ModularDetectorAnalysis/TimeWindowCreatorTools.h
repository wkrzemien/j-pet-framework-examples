/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file TimeWindowCreatorTools.h
 */

#ifndef TIMEWINDOWCREATORTOOLS_H
#define TIMEWINDOWCREATORTOOLS_H

#include "JPetStatistics/JPetStatistics.h"
#include "JPetParamBank/JPetParamBank.h"
#include "JPetChannel/JPetChannel.h"
#include "JPetSigCh/JPetSigCh.h"
#include "TDCChannel.h"
#include <vector>

/**
 * @brief Set of tools for Time Window Creator task
 *
 * Contains methods building Signals Channels from Unpacker eventsIII
 */
class TimeWindowCreatorTools {
public:
  static void sortByTime(std::vector<JPetSigCh> &input);

  static std::vector<JPetSigCh>
  buildSigChs(
    TDCChannel *tdcChannel, const JPetChannel &channel,
    double maxTime, double minTime, JPetStatistics &stats, bool saveHistos
  );
  static void flagSigChs(
    std::vector<JPetSigCh> &inputSigChs, JPetStatistics &stats, bool saveHistos
  );
  static JPetSigCh generateSigCh(
    double tdcChannelTime, const JPetChannel &channel, JPetSigCh::EdgeType edge
  );
};

#endif /* !TIMEWINDOWCREATORTOOLS_H */
