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

// #include "rhythm-api/rhd2000evalboard.h"
// #include "rhythm-api/rhd2000registers.h"
// #include "rhythm-api/rhd2000datablock.h"
// #include "rhythm-api/okFrontPanelDLL.h"

#define CHIP_ID_RHD2132 1
#define CHIP_ID_RHD2216 2
#define CHIP_ID_RHD2164 4
#define CHIP_ID_RHD2164_B 1000
#define REGISTER_59_MISO_A 53
#define REGISTER_59_MISO_B 58
#define RHD2132_16CH_OFFSET 8

#define MAX_NUM_CHANNELS 64

#define MAX_NUM_HEADSTAGES 4

namespace AONode
{

	enum ChannelNamingScheme
	{
		GLOBAL_INDEX = 1,
		STREAM_INDEX = 2
	};

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

		/** Fills the DataBuffer with incoming data */
		bool updateBuffer() override;

		/** Initializes data transfer*/
		bool startAcquisition() override;

		/** Stops data transfer */
		bool stopAcquisition() override;

		/* Passes the processor's info objects to DataThread, to allow them to be configured */
		void updateSettings(OwnedArray<ContinuousChannel> *continuousChannels,
							OwnedArray<EventChannel> *eventChannels,
							OwnedArray<SpikeChannel> *spikeChannels,
							OwnedArray<DataStream> *sourceStreams,
							OwnedArray<DeviceInfo> *devices,
							OwnedArray<ConfigurationObject> *configurationObjects) override;

		/** Sets the method for determining channel names*/
		void setNamingScheme(ChannelNamingScheme scheme);

		/** Gets the method for determining channel names*/
		ChannelNamingScheme getNamingScheme();

		/** Allow the thread to respond to messages sent by other plugins */
		void handleBroadcastMessage(String msg) override;

		/** Informs the DataThread about whether to expect saved settings to be loaded*/
		void initialize(bool signalChainIsLoading) override;

		void setNumChannels(int hsNum, int nChannels);

		int getNumChannels();

		int getNumDataOutputs(ContinuousChannel::Type type);

		// for communication with SourceNode processors:
		bool foundInputSource() override;

		void scanPorts();

		String getChannelName(int ch) const;

		float getAdcBitVolts(int channelNum) const;

		void setSampleRate(int index, bool temporary = false);

		// double setUpperBandwidth(double upper); // set desired BW, returns actual BW
		// double setLowerBandwidth(double lower);

		int setNoiseSlicerLevel(int level);
		void setFastTTLSettle(bool state, int channel);
		void setTTLoutputMode(bool state);
		void setDAChpf(float cutoff, bool enabled);

		void enableAuxs(bool);
		void enableAdcs(bool);

		int TTL_OUTPUT_STATE[16];

		bool isAuxEnabled();
		bool isAcquisitionActive() const;

		Array<int> getDACchannels() const;

		void setDACchannel(int dacOutput, int channel);
		void setDACthreshold(int dacOutput, float threshold);

		int getHeadstageChannels(int hsNum) const;
		int getActiveChannelsInHeadstage(int hsNum) const;

		void runImpedanceTest();

		void enableBoardLeds(bool enable);

		int setClockDivider(int divide_ratio);

		void setAdcRange(int adcChannel, short rangeType);

		short getAdcRange(int adcChannel) const;

		static DataThread *createDataThread(SourceNode *sn);

		class DigitalOutputTimer : public Timer
		{
		public:
			/** Constructor */
			DigitalOutputTimer(DeviceThread *, int tllLine, int eventDurationMs);

			/** Destructor*/
			~DigitalOutputTimer() {}

			/** Sends signal to turn off event channel*/
			void timerCallback();

		private:
			DeviceThread *board;

			int tllOutputLine;
		};

		struct DigitalOutputCommand
		{
			int ttlLine;
			bool state;
		};

		void addDigitalOutputCommand(DigitalOutputTimer *timerToDelete,
									 int ttlLine,
									 bool state);

	private:
		std::queue<DigitalOutputCommand> digitalOutputCommands;

		OwnedArray<DigitalOutputTimer> digitalOutputTimers;

		void updateBoardStreams();
		void setCableLength(int hsNum, float length);

		/** Rhythm API classes*/
		// ScopedPointer<Rhd2000EvalBoard> evalBoard;
		// Rhd2000Registers chipRegisters;
		// ScopedPointer<Rhd2000DataBlock> dataBlock;
		// Array<Rhd2000EvalBoard::BoardDataSource> enabledStreams;

		/** Custom classes*/
		// OwnedArray<Headstage> headstages;
		// ScopedPointer<ImpedanceMeter> impedanceThread;

		/** True if device is available*/
		bool deviceFound;

		/** True if data is streaming*/
		bool isTransmitting;

		/** True if change in settings is needed during acquisition*/
		bool updateSettingsDuringAcquisition;

		/** Data buffers*/
		float thisSample[MAX_NUM_CHANNELS];

		float auxBuffer[MAX_NUM_CHANNELS]; // aux inputs are only sampled every 4th sample, so use this to buffer the
										   // samples so they can be handles just like the regular neural channels later

		float auxSamples[1][3];

		unsigned int blockSize;

		/** Cable length settings */
		// struct CableLength
		// {
		// 	float portA = 0.914f;
		// 	float portB = 0.914f;
		// 	float portC = 0.914f;
		// 	float portD = 0.914f;
		// 	float portE = 0.914f;
		// 	float portF = 0.914f;
		// 	float portG = 0.914f;
		// 	float portH = 0.914f;
		// };

		/** Dsp settings*/
		struct Dsp
		{
			bool enabled = true;
			double cutoffFreq = 0.5;
			double upperBandwidth = 7500.0f;
			double lowerBandwidth = 1.0f;
		};

		/** struct containing board settings*/
		struct Settings
		{
			bool acquireAux = false;
			bool acquireAdc = false;

			bool fastSettleEnabled = false;
			bool fastTTLSettleEnabled = false;
			int fastSettleTTLChannel = -1;
			bool ttlMode = false;

			Dsp dsp;

			int noiseSlicerLevel;

			bool desiredDAChpfState;
			double desiredDAChpf;
			float boardSampleRate = 30000.f;
			int savedSampleRateIndex = 16;

			// CableLength cableLength;

			int audioOutputL = -1;
			int audioOutputR = -1;
			bool ledsEnabled = true;
			bool newScan = true;
			int numberingScheme = 1;
			uint16 clockDivideFactor;

		} settings;

		/** Path to Opal Kelly library file*/
		// String libraryFilePath;

		/** Open the connection to the acquisition board*/
		bool openBoard(String pathToLibrary);

		/** Upload the bitfile*/
		bool uploadBitfile(String pathToBitfile);

		/** Initialize the board*/
		void initializeBoard();

		/** Update register settings*/
		void updateRegisters();

		int *dacChannels, *dacStream;
		float *dacThresholds;
		bool *dacChannelsToUpdate;
		Array<int> chipId;

		Array<int> numChannelsPerDataStream;

		ChannelNamingScheme channelNamingScheme;

		/** ADC info */
		std::array<std::atomic_short, 8> adcRangeSettings;
		Array<float> adcBitVolts;
		StringArray adcChannelNames;
		StringArray ttlLineNames;

		/** Impedance data*/
		// Impedances impedances;

		StringArray channelNames;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceThread);
	};

}
#endif // __DEVICETHREAD_H_2C4CBD67__
