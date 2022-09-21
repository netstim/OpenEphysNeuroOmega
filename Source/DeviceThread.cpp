/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifdef _WIN32
#define NOMINMAX
#endif

#include "DeviceThread.h"
#include "DeviceEditor.h"

#include <ctime>
#include <math.h>
#include <stdlib.h>

// AlphaOmega SDK
namespace AO
{
#include "AOTypes.h"
#include "AOSystemAPI.h"
#include "StreamFormat.h"
}

using namespace AONode;

#define AO_BUFFER_SIZE_MS 5000
#define SOURCE_BUFFER_SIZE 10000

static const float DRIVE_ZERO_POSITION_MILIM = 25.0;

DataThread *DeviceThread::createDataThread(SourceNode *sn)
{
    return new DeviceThread(sn);
}

DeviceThread::DeviceThread(SourceNode *sn) : DataThread(sn),
                                             isTransmitting(false),
                                             updateSettingsDuringAcquisition(false)
{

    queryUserStartConnection();

    // start with 2 channels and automatically resize
    // removing this will make the gui crash
    sourceBuffers.add(new DataBuffer(2, SOURCE_BUFFER_SIZE));

    AO::uint32 AONumberOfChannels = 0;
    AO::GetChannelsCount(&AONumberOfChannels);
    if (testing)
        AONumberOfChannels = 5;

    AO::SInformation *pChannelsInfo = new AO::SInformation[AONumberOfChannels];
    AO::GetAllChannels(pChannelsInfo, AONumberOfChannels);
    if (testing)
    {
        pChannelsInfo[0].channelID = 0;
        pChannelsInfo[1].channelID = 1;
        pChannelsInfo[2].channelID = 2;
        pChannelsInfo[3].channelID = 3;
        pChannelsInfo[4].channelID = 4;
        strcpy(pChannelsInfo[0].channelName, "RAW / Name1");
        strcpy(pChannelsInfo[1].channelName, "RAW / Name2");
        strcpy(pChannelsInfo[2].channelName, "SPK / Name1");
        strcpy(pChannelsInfo[3].channelName, "SPK / Name2");
        strcpy(pChannelsInfo[4].channelName, "SPK Name3");
    }

    channelsXmlList = new XmlElement("CHANNELS");
    streamsXmlList = new XmlElement("STREAMS");

    XmlElement *channel, *stream;
    String streamName;
    StringArray channelsIDs;
    int streamID = -1;
    numberOfChannels = AONumberOfChannels;

    for (int ch = 0; ch < AONumberOfChannels; ch++)
    {
        if (!String(pChannelsInfo[ch].channelName).containsChar('/'))
        {
            numberOfChannels--;
            continue;
        }
        streamName = String(pChannelsInfo[ch].channelName).upToFirstOccurrenceOf(" ", false, false);
        if (streamID < 0 || (!streamName.equalsIgnoreCase(streamsXmlList->getChildElement(streamID)->getStringAttribute("Name"))))
        {
            channelsIDs.clear();
            streamID++;
            stream = new XmlElement("STREAM");
            stream->setAttribute("ID", streamID);
            stream->setAttribute("Name", streamName);
            stream->setAttribute("Sampling Rate", 1000);
            stream->setAttribute("Bit Resolution", 1);
            stream->setAttribute("Gain", 1);
            stream->setAttribute("Channel IDs", "");
            stream->setAttribute("Number Of Channels", "");
            stream->setAttribute("Enabled", false);
            streamsXmlList->addChildElement(stream);
        }
        channelsIDs.add(String(pChannelsInfo[ch].channelID));
        stream->setAttribute("Channel IDs", channelsIDs.joinIntoString(","));
        stream->setAttribute("Number Of Channels", channelsIDs.size());
        channel = new XmlElement("CHANNEL");
        channel->setAttribute("ID", pChannelsInfo[ch].channelID);
        channel->setAttribute("Name", pChannelsInfo[ch].channelName);
        channel->setAttribute("Stream ID", streamID);
        channelsXmlList->addChildElement(channel);
    }

    numberOfStreams = streamsXmlList->getNumChildElements();
    setUpDefaultStream();
}

void DeviceThread::setUpDefaultStream()
{
    for (int streamID = 0; streamID < numberOfStreams; streamID++)
    {
        if (streamsXmlList->getChildElement(streamID)->getStringAttribute("Name").equalsIgnoreCase("RAW"))
        {
            streamsXmlList->getChildElement(streamID)->setAttribute("Sampling Rate", 44000);
            streamsXmlList->getChildElement(streamID)->setAttribute("Bit Resolution", 38.147);
            streamsXmlList->getChildElement(streamID)->setAttribute("Gain", 20);
            streamsXmlList->getChildElement(streamID)->setAttribute("Enabled", true);
            return;
        }
    }
    if (numberOfStreams > 0)
        streamsXmlList->getChildElement(0)->setAttribute("Enabled", true);
}

DeviceThread::~DeviceThread()
{
    if (foundInputSource())
    {
        AO::CloseConnection();
    }
}

void DeviceThread::initialize(bool signalChainIsLoading)
{
    // TODO
}

std::unique_ptr<GenericEditor> DeviceThread::createEditor(SourceNode *sn)
{
    std::unique_ptr<DeviceEditor> editor = std::make_unique<DeviceEditor>(sn, this);
    return editor;
}

void DeviceThread::handleBroadcastMessage(String msg)
{
    // TODO
}

void DeviceThread::queryUserStartConnection()
{
    auto *connectAW = new AlertWindow(TRANS("Neuro Omega: start connection"),
                                      TRANS("Enter the system MAC adress"),
                                      AlertWindow::NoIcon, nullptr);

    connectAW->addTextEditor("System MAC", String("AA:BB:CC:DD:EE:FF"), String(), false);
    connectAW->addButton(TRANS("Connect"), 1, KeyPress(KeyPress::returnKey));
    connectAW->addButton(TRANS("Cancel"), 0, KeyPress(KeyPress::escapeKey));

    if (connectAW->runModalLoop())
    {
        MouseCursor::showWaitCursor();
        AO::MAC_ADDR sysMAC = {0};
        sscanf(connectAW->getTextEditorContents("System MAC").toStdString().c_str(), "%x:%x:%x:%x:%x:%x",
               &sysMAC.addr[0], &sysMAC.addr[1], &sysMAC.addr[2], &sysMAC.addr[3], &sysMAC.addr[4], &sysMAC.addr[5]);
        AO::DefaultStartConnection(&sysMAC, 0);
        waitForConnection();
        MouseCursor::hideWaitCursor();
    }
    connectAW->setVisible(false);

    auto *retryAW = new AlertWindow(TRANS("Neuro Omega"),
                                    TRANS((foundInputSource()) ? "Connected!" : ("Unable to connect\n" + getLastAOSDKError())),
                                    AlertWindow::NoIcon, nullptr);

    if (!foundInputSource())
        retryAW->addButton(TRANS("Retry"), 1, KeyPress(KeyPress::returnKey));
    retryAW->addButton(TRANS("OK"), 0, KeyPress(KeyPress::escapeKey));

    bool retry = retryAW->runModalLoop();
    retryAW->setVisible(false);
    if (retry)
        queryUserStartConnection();
}

String DeviceThread::getLastAOSDKError()
{
    char sError[1000] = {0};
    int nErrorCount = 0;
    AO::ErrorHandlingfunc(&nErrorCount, sError, 1000);
    return String(sError);
}

void DeviceThread::waitForConnection()
{
    for (int i = 0; i < 10; i++)
    {
        Thread::sleep(1000);
        if (foundInputSource())
            return;
    }
}

void DeviceThread::updateSettings(OwnedArray<ContinuousChannel> *continuousChannels,
                                  OwnedArray<EventChannel> *eventChannels,
                                  OwnedArray<SpikeChannel> *spikeChannels,
                                  OwnedArray<DataStream> *sourceStreams,
                                  OwnedArray<DeviceInfo> *devices,
                                  OwnedArray<ConfigurationObject> *configurationObjects)
{
    if (!foundInputSource())
        return;

    continuousChannels->clear();
    eventChannels->clear();
    spikeChannels->clear();
    sourceStreams->clear();
    devices->clear();
    configurationObjects->clear();
    sourceBuffers.clear();
    sourceBuffersSampleCount.clear();

    int streamID, thisChannelStreamID = -1;
    DataStream *stream;

    for (int ch = 0; ch < numberOfChannels; ch++)
    {
        thisChannelStreamID = channelsXmlList->getChildElement(ch)->getIntAttribute("Stream ID");
        if (!streamsXmlList->getChildElement(thisChannelStreamID)->getBoolAttribute("Enabled"))
            continue;
        if (streamID != thisChannelStreamID)
        {
            streamID = thisChannelStreamID;
            DataStream::Settings dataStreamSettings = getStreamSettingsFromID(streamID);
            stream = new DataStream(dataStreamSettings);
            sourceStreams->add(stream);
            sourceBuffers.add(new DataBuffer(streamsXmlList->getChildElement(streamID)->getIntAttribute("Number Of Channels"), SOURCE_BUFFER_SIZE));
            sourceBuffersSampleCount.add(0);
        }
        String channelName = channelsXmlList->getChildElement(ch)->getStringAttribute("Name");
        float bitVolts = streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Bit Resolution") / streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Gain");
        std::cout << "Adding channel: " << channelName << std::endl; // not sure why removing this makes the gui crash
        ContinuousChannel::Settings channelSettings{
            ContinuousChannel::ELECTRODE,
            channelName,
            "description",
            "identifier",
            bitVolts,
            stream};
        continuousChannels->add(new ContinuousChannel(channelSettings));
        continuousChannels->getLast()->setUnits("uV");
    }
}

DataStream::Settings DeviceThread::getStreamSettingsFromID(int streamID)
{
    DataStream::Settings dataStreamSettings{
        streamsXmlList->getChildElement(streamID)->getStringAttribute("Name"),
        "description",
        "identifier",
        streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Sampling Rate")};
    return dataStreamSettings;
}

bool DeviceThread::foundInputSource()
{
    return ((AO::isConnected() == AO::eAO_CONNECTED) || testing);
}

bool DeviceThread::startAcquisition()
{
    // Neuro Omega Buffer
    deviceDataArraySize = 10000;
    streamDataArray = new AO::int16[deviceDataArraySize];

    for (int i = 0; i < numberOfChannels; i++)
        AO::AddBufferChannel(channelsXmlList->getChildElement(i)->getIntAttribute("ID"), AO_BUFFER_SIZE_MS);
    AO::ClearBuffers();

    startThread();

    isTransmitting = true;

    return true;
}

bool DeviceThread::stopAcquisition()
{
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }

    clearSourceBuffers();

    isTransmitting = false;

    return true;
}

void DeviceThread::clearSourceBuffers()
{
    int sourceBufferIdx = 0;
    for (int i = 0; i < numberOfStreams; i++)
    {
        if (streamsXmlList->getChildElement(i)->getBoolAttribute("Enabled"))
        {
            sourceBuffersSampleCount.set(sourceBufferIdx, 0);
            sourceBuffers[sourceBufferIdx]->clear();
            sourceBufferIdx++;
        }
    }
}

bool DeviceThread::updateBuffer()
{
    sleepTimeMiliS = 100;
    if (testing)
    {
        deviceTimeStamp = std::time(0);
        Thread::sleep(sleepTimeMiliS);
    }

    int numberOfSamplesPerChannel;
    int numberOfSamplesFromDevice;
    int sourceBufferIdx = 0;
    int sourceBufferDataIdx;
    float bitVolts;

    for (int streamID = 0; streamID < numberOfStreams; streamID++)
    {
        if (!streamsXmlList->getChildElement(streamID)->getBoolAttribute("Enabled"))
            continue;

        numberOfChannelsInStream = streamsXmlList->getChildElement(streamID)->getIntAttribute("Number Of Channels");
        bitVolts = streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Bit Resolution") / streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Gain");

        if (testing)
            numberOfSamplesFromDevice = updateStreamDataArrayFromTestDataAndGetNumberOfSamples(streamID);
        else
            numberOfSamplesFromDevice = updateStreamDataArrayFromAOAndGetNumberOfSamples(streamID);

        numberOfSamplesPerChannel = numberOfSamplesFromDevice / numberOfChannelsInStream;

        sourceBufferData = new float[numberOfSamplesFromDevice];
        sampleCount = new int64[numberOfSamplesPerChannel];
        timeStamps = new double[numberOfSamplesPerChannel];
        eventCodes = new uint64[numberOfSamplesPerChannel];

        sourceBufferDataIdx = 0;
        for (int samp = 0; samp < numberOfSamplesPerChannel; samp++)
        {
            sampleCount[samp] = sourceBuffersSampleCount[sourceBufferIdx] + samp;
            timeStamps[samp] = float(deviceTimeStamp) + samp / streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Sampling Rate");
            eventCodes[samp] = 1;
            for (int chan = 0; chan < numberOfChannelsInStream; chan++)
            {
                sourceBufferData[sourceBufferDataIdx++] = streamDataArray[(chan * numberOfSamplesPerChannel) + samp] * bitVolts;
            }
        }

        sourceBuffersSampleCount.set(sourceBufferIdx, sampleCount[numberOfSamplesPerChannel - 1] + 1);
        sourceBuffers[sourceBufferIdx]->addToBuffer(sourceBufferData,
                                                    sampleCount,
                                                    timeStamps,
                                                    eventCodes,
                                                    numberOfSamplesPerChannel,
                                                    1);
        sourceBufferIdx++;
    }

    queryDistanceToTarget();

    return true;
}

void DeviceThread::queryDistanceToTarget()
{
    bool broadcast;
    if (testing)
    {
        dtt = 100 * ((float)rand()) / (float)RAND_MAX;
        broadcast = (dtt < 0.1);
    }
    else
    {
        AO::int32 nDepthUm = 0;
        AO::EAOResult eAORes = (AO::EAOResult)AO::GetDriveDepth(&nDepthUm);
        if (eAORes == AO::eAO_OK)
            dtt = DRIVE_ZERO_POSITION_MILIM - nDepthUm / 1000.0;
        broadcast = (eAORes == AO::eAO_OK) && (dtt != previous_dtt);
    }

    if (broadcast)
        broadcastMessage("MicroDrive:DistanceToTarget:" + std::to_string(dtt));

    previous_dtt = dtt;
}

int DeviceThread::updateStreamDataArrayFromAOAndGetNumberOfSamples(int streamID)
{
    int *arrChannel = getChannelIDsArrayFromStreamID(streamID);
    int status = AO::eAO_MEM_EMPTY;
    int numberOfSamplesFromDevice = 0;
    while (status == AO::eAO_MEM_EMPTY || numberOfSamplesFromDevice == 0)
        status = AO::GetAlignedData(streamDataArray, deviceDataArraySize, &numberOfSamplesFromDevice, arrChannel, numberOfChannelsInStream, &deviceTimeStamp);
    return numberOfSamplesFromDevice;
}

int *DeviceThread::getChannelIDsArrayFromStreamID(int streamID)
{
    int *arrChannel = new int[streamsXmlList->getChildElement(streamID)->getIntAttribute("Number Of Channels")];
    StringArray channelIDs;
    channelIDs.addTokens(streamsXmlList->getChildElement(streamID)->getStringAttribute("Channel IDs"), ",", "\"");
    for (int ch = 0; ch < channelIDs.size(); ch++)
        arrChannel[ch] = channelIDs[ch].getIntValue();
    return arrChannel;
}

int DeviceThread::updateStreamDataArrayFromTestDataAndGetNumberOfSamples(int streamID)
{
    int numberOfSamplesPerChannel = sleepTimeMiliS / 1000.0 * streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Sampling Rate");
    int numberOfSamplesFromDevice = numberOfSamplesPerChannel * numberOfChannelsInStream;
    for (int samp = 0; samp < numberOfSamplesPerChannel; samp++)
    {
        for (int chan = 0; chan < numberOfChannelsInStream; chan++)
            streamDataArray[(chan * numberOfSamplesPerChannel) + samp] = pow(-1, streamID) * samp * (chan + 1);
    }
    return numberOfSamplesFromDevice;
}