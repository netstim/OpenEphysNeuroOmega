/*
	------------------------------------------------------------------

	This file is part of the Open Ephys GUI
	Copyright (C) 2020 Open Ephys

	------------------------------------------------------------------

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DEVICETHREAD_H_2C4CBD67__
#define __DEVICETHREAD_H_2C4CBD67__

#include <DataThreadHeaders.h>

#include <stdio.h>
#include <string.h>
#include <array>
#include <atomic>

// AlphaOmega SDK
namespace AO
{
#include "AOTypes.h"
#include "AOSystemAPI.h"
#include "StreamFormat.h"
}

namespace AONode
{
	/**
		Communicates with a device running Alpha Omega's SDK

		@see DataThread, SourceNode
	*/
	class DeviceThread : public DataThread
	{

	public:
		/** Constructor */
		DeviceThread(SourceNode *sn);

		/** Destructor */
		~DeviceThread();

		/** Creates the UI for this plugin */
		std::unique_ptr<GenericEditor> createEditor(SourceNode *sn);

		/** Fills the DataBuffer with incoming sourceBufferData */
		bool updateBuffer() override;

		/** Initializes sourceBufferData transfer*/
		bool startAcquisition() override;

		/** Stops sourceBufferData transfer */
		bool stopAcquisition() override;

		/* Passes the processor's info objects to DataThread, to allow them to be configured */
		void updateSettings(OwnedArray<ContinuousChannel> *continuousChannels,
							OwnedArray<EventChannel> *eventChannels,
							OwnedArray<SpikeChannel> *spikeChannels,
							OwnedArray<DataStream> *sourceStreams,
							OwnedArray<DeviceInfo> *devices,
							OwnedArray<ConfigurationObject> *configurationObjects) override;

		/** Allow the thread to respond to messages sent by other plugins */
		void handleBroadcastMessage(String msg) override;

		/** Informs the DataThread about whether to expect saved settings to be loaded*/
		void initialize(bool signalChainIsLoading) override;

		// for communication with SourceNode processors:
		bool foundInputSource() override;

		static DataThread *createDataThread(SourceNode *sn);

		XmlElement *channelsXmlList = nullptr;
		XmlElement *streamsXmlList = nullptr;

	private:
		// Channels info
		AO::uint32 numberOfChannels;
		int numberOfStreams;

		// Neuro Omega Buffer
		AO::int16 *streamDataArray;
		int deviceDataArraySize;
		AO::ULONG deviceTimeStamp;
		int numberOfChannelsInStream;

		// Neuro Omega distance to target
		float dtt;
		float previous_dtt;

		// Source Buffer
		float *sourceBufferData;
		int numItems;
		int64 *sampleCount;
		double *timeStamps;
		uint64 *eventCodes;
		int chunkSize;
		Array<int64> sourceBuffersSampleCount;

		/** True if sourceBufferData is streaming*/
		bool isTransmitting;

		/** True if change in settings is needed during acquisition*/
		bool updateSettingsDuringAcquisition;

		/** Open the connection to the neuro omega*/
		void queryUserStartConnection();
		void waitForConnection();

		void setUpDefaultStream();

		int updateStreamDataArrayFromAOAndGetNumberOfSamples(int streamID);
		int updateStreamDataArrayFromTestDataAndGetNumberOfSamples(int streamID);
		DataStream::Settings getStreamSettingsFromID(int streamID);
		int *getChannelIDsArrayFromStreamID(int streamID);
		void updateSampleCountAndTimeStampsAndEventCodes(int streamID, int numberOfSamplesPerChannel);
		void resetStreamsTotalSamplesSinceStart();
		void clearSourceBuffers();
		void queryDistanceToTarget();

		/** Testing */
		bool testing = true;
		int sleepTimeMiliS;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceThread);
	};

}
#endif // __DEVICETHREAD_H_2C4CBD67__
