/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file TaskC.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "TaskC.h"
#include <algorithm>

using namespace std;


TaskC::TaskC(const char * name, const char * description):JPetTask(name, description){}
TaskC::~TaskC(){}

void TaskC::init(const JPetTaskInterface::Options& opts){

  getStatistics().createHistogram(new TH1F("timeSepLarge",
					   "time differences between subsequent hits; #Delta t [ns]",
					   1000,
					   0.,
					   700000.
					   )
				  );
  
  getStatistics().createHistogram(new TH1F("timeSepSmall",
					   "time differences between subsequent hits; #Delta t [ns]",
					   1000,
					   0.,
					   200.
					   )
				  );
 
  
}

void TaskC::exec(){
	//getting the data from event in propriate format
	if(auto currSignal = dynamic_cast<const JPetRawSignal*const>(getEvent())){
		getStatistics().getCounter("No. initial signals")++;
		if (fSignals.empty()) {
			fSignals.push_back(*currSignal);
		} else {
			if (fSignals[0].getTimeWindowIndex() == currSignal->getTimeWindowIndex()) {
				fSignals.push_back(*currSignal);
			} else {

			  vector<JPetHit> hits = createHits(fSignals);
			  sortHits(hits);
			  studyTimeWindow(hits);
			  saveHits(hits);
			  
			  fSignals.clear();
			  fSignals.push_back(*currSignal);
			}
		}
	}
}
vector<JPetHit> TaskC::createHits(const vector<JPetRawSignal>&signals){
	vector<JPetHit> hits;
	for (auto i = signals.begin(); i != signals.end(); ++i) {
		for (auto j = i; ++j != signals.end();) {
			if (i->getPM().getScin() == j->getPM().getScin()) {
				// found 2 signals from the same scintillator
				// wrap the RawSignal objects into RecoSignal and PhysSignal
				// for now this is just wrapping opne object into another
				// in the future analyses it will involve more logic like
				// reconstructing the signal's shape, charge, amplitude etc.
				JPetRecoSignal recoSignalA;
				JPetRecoSignal recoSignalB;
				JPetPhysSignal physSignalA;
				JPetPhysSignal physSignalB;
				// assign sides A and B properly
				if( 
					(i->getPM().getSide() == JPetPM::SideA)
					&&(j->getPM().getSide() == JPetPM::SideB)
				){
					recoSignalA.setRawSignal(*i);
					recoSignalB.setRawSignal(*j);
				} else if(
					(j->getPM().getSide() == JPetPM::SideA)
					&&(i->getPM().getSide() == JPetPM::SideB)
				){
					recoSignalA.setRawSignal(*j);
					recoSignalB.setRawSignal(*i);
				} else {
					// if two hits on the same side, ignore
					WARNING("TWO hits on the same scintillator side we ignore it");         
					continue;
				}
				physSignalA.setRecoSignal(recoSignalA);
				physSignalB.setRecoSignal(recoSignalB);
				auto leading_points_a = physSignalA.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
				auto leading_points_b = physSignalB.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);

				//skip signals with no information on 1st threshold
				if(leading_points_a.count(1) == 0) continue;
				if(leading_points_b.count(1) == 0) continue;
				
				physSignalA.setTime(leading_points_a.at(1));
				physSignalB.setTime(leading_points_b.at(1));

				
				JPetHit hit;
				hit.setSignalA(physSignalA);
				hit.setSignalB(physSignalB);
				hit.setScintillator(i->getPM().getScin());
				hit.setBarrelSlot(i->getPM().getScin().getBarrelSlot());
				
				physSignalA.setTime(physSignalA.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading).at(1));
				physSignalB.setTime(physSignalB.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading).at(1));

				hit.setTime( 0.5 * ( hit.getSignalA().getTime() + hit.getSignalB().getTime()) );
				
				hits.push_back(hit);
				getStatistics().getCounter("No. found hits")++;
			}
		}
	}
	return hits;
}

void TaskC::terminate(){
  //	saveHits(createHits(fSignals)); //if there is something left
	INFO( Form("From %d initial signals %d hits were paired.", 
		   static_cast<int>(getStatistics().getCounter("No. initial signals")),
		   static_cast<int>(getStatistics().getCounter("No. found hits")) )
	);
}


void TaskC::sortHits(vector<JPetHit>&hits){

  sort( hits.begin(), hits.end(),
  	[](const JPetHit & A, const JPetHit & B){
  	  return (A.getTime() < B.getTime());
  	}
  	);

}

void TaskC::studyTimeWindow(const vector<JPetHit>&hits){

  for(int i=1; i<hits.size(); ++i){

    // make sure the hits are really sorted
    assert(hits.at(i-1).getTime() <= hits.at(i).getTime());

    double dt = hits.at(i).getTime() - hits.at(i-1).getTime();

    getStatistics().getHisto1D("timeSepSmall").Fill(dt / 1000.); // we fill the histo in [ns]
    getStatistics().getHisto1D("timeSepLarge").Fill(dt / 1000.); // we fill the histo in [ns]
  }
  

}

void TaskC::saveHits(const vector<JPetHit>&hits){
	assert(fWriter);
      	for (auto hit : hits){
		// here one can impose any conditions on hits that should be
		// saved or skipped
		// for now, all hits are written to the output file
		// without checking anything
		fWriter->write(hit);
	}
}
void TaskC::setWriter(JPetWriter* writer){fWriter =writer;}
