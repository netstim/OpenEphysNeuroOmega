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

#define TEST_MODE_ON false
#define TEST_SLEEP_TIME_MS 100

static const float DRIVE_ZERO_POSITION_MILIM = 25.0;

DataThread *DeviceThread::createDataThread(SourceNode *sn)
{
    return new DeviceThread(sn);
}

DeviceThread::DeviceThread(SourceNode *sn) : DataThread(sn),
                                             isTransmitting(false),
                                             updateSettingsDuringAcquisition(false)
{
    // start with 2 channels and automatically resize
    // removing this will make the gui crash
    sourceBuffers.add(new DataBuffer(2, SOURCE_BUFFER_SIZE));

    queryUserStartConnection();
    if (foundInputSource())
        updateChannelsFromAOInfo();
}

void DeviceThread::updateChannelsFromAOInfo()
{
    AO::uint32 AONumberOfChannels = 0;
    AO::GetChannelsCount(&AONumberOfChannels);
    AO::SInformation *pChannelsInfo;

    if (TEST_MODE_ON)
        pChannelsInfo = populateInfoWithTestData(&AONumberOfChannels);
    else
    {
        pChannelsInfo = new AO::SInformation[AONumberOfChannels];
        AO::GetAllChannels(pChannelsInfo, AONumberOfChannels);
    }

    LOGC("Found ", AONumberOfChannels, " AO channels:");
    for (int i = 0; i < AONumberOfChannels; i++)
        LOGC("ID: ", pChannelsInfo[i].channelID, " Name: ", pChannelsInfo[i].channelName);

    File configsDir;
    if (File::getSpecialLocation(File::currentApplicationFile).getFullPathName().contains("plugin-GUI" + File::getSeparatorString() + "Build"))
        configsDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("configs");
    else
        configsDir = File::getSpecialLocation(File::SpecialLocationType::commonApplicationDataDirectory).getChildFile("Open Ephys").getChildFile("configs-api8");

    FileOutputStream logChannels(configsDir.getChildFile("ChannelsAvailable.log"));
    logChannels.setPosition(0);
    logChannels.truncate();
    for (int i = 0; i < AONumberOfChannels; i++)
    {
        logChannels.writeText(String(pChannelsInfo[i].channelID) + ": " + pChannelsInfo[i].channelName + "\n", false, false, nullptr);
        logChannels.flush();
    }

    channelsXmlList = new XmlElement("CHANNELS");
    streamsXmlList = new XmlElement("STREAMS");

    XmlElement *channel, *stream, *defaultStream, *defaultChannel;
    String AOChannelName, channelName, streamName;
    int streamID = -1;
    numberOfChannels = AONumberOfChannels;
    XmlElement *defaultStreamsXmlList = parseDefaultFileByName("STREAMS");
    XmlElement* defaultChannelsXmlList = parseDefaultFileByName("CHANNELS");

    for (int ch = 0; ch < AONumberOfChannels; ch++)
    {
        AOChannelName = String(pChannelsInfo[ch].channelName);
        if (pChannelsInfo[ch].channelID > 11100)
        {
            numberOfChannels--;
            continue;
        }

        if (AOChannelName.endsWith("Central") || AOChannelName.endsWith("Anterior") || AOChannelName.endsWith("Medial") || AOChannelName.endsWith("Posterior") || AOChannelName.endsWith("Lateral"))
            AOChannelName = AOChannelName.replace(" / ", "-");

        if (AOChannelName.contains(" / "))
            streamName = AOChannelName.upToFirstOccurrenceOf(" / ", false, false);
        else
            streamName = AOChannelName.upToLastOccurrenceOf(" ", false, false);
        streamName = streamName.replace("- ", "");
        channelName = AOChannelName.fromLastOccurrenceOf(" ", false, false);

        if (streamID < 0 || (!streamName.equalsIgnoreCase(streamsXmlList->getChildElement(streamID)->getStringAttribute("Stream_Name"))))
        {
            defaultStream = getStreamMatchingName(defaultStreamsXmlList, &streamName);
            streamID++;
            stream = new XmlElement("STREAM");
            stream->setAttribute("ID", streamID);
            stream->setAttribute("Stream_Name", streamName);
            stream->setAttribute("Sampling_Rate", (defaultStream != nullptr) ? defaultStream->getDoubleAttribute("Sampling_Rate") : 1000);
            stream->setAttribute("Bit_Resolution", (defaultStream != nullptr) ? defaultStream->getDoubleAttribute("Bit_Resolution") : 1);
            stream->setAttribute("Gain", (defaultStream != nullptr) ? defaultStream->getDoubleAttribute("Gain") : 1);
            stream->setAttribute("Channel_IDs", "");
            stream->setAttribute("Number_Of_Channels", "");
            stream->setAttribute("Enabled", (defaultStream != nullptr) ? defaultStream->getBoolAttribute("Enabled") : false);
            streamsXmlList->addChildElement(stream);
        }

        stream->setAttribute("Channel_IDs", "");
        stream->setAttribute("Number_Of_Channels", "0");
        channel = new XmlElement("CHANNEL");
        channel->setAttribute("ID", pChannelsInfo[ch].channelID);
        channel->setAttribute("Stream_ID", streamID);
        channel->setAttribute("Stream_Name", streamName);
        channel->setAttribute("Channel_Name", channelName);
        //channel->setAttribute("Enabled", true);
        defaultChannel = getChannelMatchingName(defaultChannelsXmlList, &streamName, &channelName);
        channel->setAttribute("Enabled", (defaultChannel != nullptr)? defaultChannel->getBoolAttribute("Enabled") : false);
        channelsXmlList->addChildElement(channel);
    }

    numberOfStreams = streamsXmlList->getNumChildElements();
    updateChannelsStreamsEnabled();
    streamsXmlList->writeTo(configsDir.getChildFile("ChannelsFiltered.xml"));
}

AO::SInformation *DeviceThread::populateInfoWithTestData(AO::uint32 *AONumberOfChannels)
{
    *AONumberOfChannels = 10;
    AO::SInformation *pChannelsInfo = new AO::SInformation[*AONumberOfChannels];
    pChannelsInfo[0].channelID = 10000;
    pChannelsInfo[1].channelID = 10001;
    pChannelsInfo[2].channelID = 10002;
    pChannelsInfo[3].channelID = 10003;
    pChannelsInfo[4].channelID = 10004;
    pChannelsInfo[5].channelID = 10005;
    pChannelsInfo[6].channelID = 10006;
    pChannelsInfo[7].channelID = 10007;
    pChannelsInfo[8].channelID = 10008;
    pChannelsInfo[9].channelID = 10009;
    strcpy(pChannelsInfo[0].channelName, "LFP 01 / Central");
    strcpy(pChannelsInfo[1].channelName, "LFP 02 / Posteriolateral");
    strcpy(pChannelsInfo[2].channelName, "LFP 03");
    strcpy(pChannelsInfo[3].channelName, "LFP 04");
    strcpy(pChannelsInfo[4].channelName, "LFP 05");
    strcpy(pChannelsInfo[5].channelName, "Macro_LFP 01 / Central");
    strcpy(pChannelsInfo[6].channelName, "Macro_LFP 02 / Posteriolateral");
    strcpy(pChannelsInfo[7].channelName, "Macro_LFP 03");
    strcpy(pChannelsInfo[8].channelName, "Macro_LFP 04");
    strcpy(pChannelsInfo[9].channelName, "Macro_LFP 05");
    return pChannelsInfo;
}

void DeviceThread::updateChannelsFromDefaults()
{
    channelsXmlList = parseDefaultFileByName("CHANNELS");
    streamsXmlList = parseDefaultFileByName("STREAMS");

    if ((channelsXmlList == nullptr) || (streamsXmlList == nullptr))
        return;

    numberOfChannels = channelsXmlList->getNumChildElements();
    numberOfStreams = streamsXmlList->getNumChildElements();

    updateChannelsStreamsEnabled();
}

XmlElement *DeviceThread::parseDefaultFileByName(String name)
{
    File configsDir;
    if (File::getSpecialLocation(File::currentApplicationFile).getFullPathName().contains("plugin-GUI" + File::getSeparatorString() + "Build"))
        configsDir = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("configs");
    else
        configsDir = File::getSpecialLocation(File::SpecialLocationType::commonApplicationDataDirectory).getChildFile("Open Ephys").getChildFile("configs-api8");

    File defaultsFile(configsDir.getChildFile("AO" + name + ".xml"));
    if (!defaultsFile.existsAsFile())
    {
        LOGC("Config file not found: ", defaultsFile.getFullPathName());
        return nullptr;
    }
    std::unique_ptr<juce::XmlElement> fileData = XmlDocument::parse(defaultsFile);
    return new XmlElement(*fileData->getChildByName(name));
}

XmlElement *DeviceThread::getStreamMatchingName(XmlElement *list, String *name)
{
    for (auto *child : list->getChildIterator())
        if (name->startsWithIgnoreCase(child->getStringAttribute("Stream_Name")))
            return new XmlElement(*child);
    return nullptr;
}

XmlElement* DeviceThread::getChannelMatchingName(XmlElement *list, String *Stream_Name, String *Channel_Name)
{
    for (auto* child : list->getChildIterator())
        if (Stream_Name->equalsIgnoreCase(child->getStringAttribute("Stream_Name")) && Channel_Name->equalsIgnoreCase(child->getStringAttribute("Channel_Name")))
            return new XmlElement(*child);
    return nullptr;
}

void DeviceThread::updateChannelsStreamsEnabled()
{
    XmlElement *channel, *enabledChannel, *stream;
    StringArray channelsIDs;
    bool atLeastOneStreamEnabled = false;

    for (int st = 0; st < numberOfStreams; st++)
    {
        stream = streamsXmlList->getChildElement(st);
        stream->setAttribute("Channel_IDs", "");
        stream->setAttribute("Number_Of_Channels", "0");
    }

    for (int ch = 0; ch < numberOfChannels; ch++)
    {
        channel = channelsXmlList->getChildElement(ch);
        if (!channel->getBoolAttribute("Enabled"))
            continue;

        stream = streamsXmlList->getChildElement(channel->getIntAttribute("Stream_ID"));

        channelsIDs.addTokens(stream->getStringAttribute("Channel_IDs"), ",", "\"");
        channelsIDs.add(channel->getStringAttribute("ID"));

        stream->setAttribute("Channel_IDs", channelsIDs.joinIntoString(","));
        stream->setAttribute("Number_Of_Channels", channelsIDs.size());

        channelsIDs.clear();
        enabledChannel = channelsXmlList->getChildElement(ch);
    }

    for (int st = 0; st < numberOfStreams; st++)
    {
        stream = streamsXmlList->getChildElement(st);
        if (!stream->getBoolAttribute("Enabled"))
            continue;
        stream->setAttribute("Enabled", (stream->getIntAttribute("Number_Of_Channels") > 0) ? true : false);
        atLeastOneStreamEnabled = (atLeastOneStreamEnabled || stream->getBoolAttribute("Enabled"));
    }

    if (!atLeastOneStreamEnabled)
        streamsXmlList->getChildElement(enabledChannel->getIntAttribute("Stream_ID"))->setAttribute("Enabled", true);
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

    bool streamEnabled, channelEnabled;
    int streamID = -1, thisChannelStreamID = -1;
    DataStream *stream;

    for (int ch = 0; ch < numberOfChannels; ch++)
    {
        thisChannelStreamID = channelsXmlList->getChildElement(ch)->getIntAttribute("Stream_ID");
        streamEnabled = streamsXmlList->getChildElement(thisChannelStreamID)->getBoolAttribute("Enabled");
        channelEnabled = channelsXmlList->getChildElement(ch)->getBoolAttribute("Enabled");
        if (!(streamEnabled && channelEnabled))
            continue;
        if (streamID != thisChannelStreamID)
        {
            streamID = thisChannelStreamID;
            DataStream::Settings dataStreamSettings = getStreamSettingsFromID(streamID);
            stream = new DataStream(dataStreamSettings);
            sourceStreams->add(stream);
            sourceBuffers.add(new DataBuffer(streamsXmlList->getChildElement(streamID)->getIntAttribute("Number_Of_Channels"), SOURCE_BUFFER_SIZE));
            sourceBuffersSampleCount.add(0);
        }
        String channelName = channelsXmlList->getChildElement(ch)->getStringAttribute("Channel_Name");
        float bitVolts = streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Bit_Resolution");
        ContinuousChannel::Settings channelSettings{
            ContinuousChannel::ELECTRODE,
            channelName,
            "description",
            "neuro-omega-device.continuous.headstage",
            bitVolts,
            stream};
        continuousChannels->add(new ContinuousChannel(channelSettings));
        continuousChannels->getLast()->setUnits("uV");
    }

    // Add an event channel.
    // This is not used currently but RecordNode.cpp (line 447) calls eventChannels.getLast() and app crashes
    // TODO: Fix and send PR
    EventChannel::Settings settings{
        EventChannel::Type::TTL,
        "Neuro Omega TTL Input",
        "Events on digital input lines of a Neuro Omega device",
        "neuro-omega-device.events",
        stream,
        8};

    eventChannels->add(new EventChannel(settings));
}

DataStream::Settings DeviceThread::getStreamSettingsFromID(int streamID)
{
    DataStream::Settings dataStreamSettings{
        streamsXmlList->getChildElement(streamID)->getStringAttribute("Stream_Name"),
        "description",
        "neuro-omega-device.data",
        float(streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Sampling_Rate"))};
    return dataStreamSettings;
}

bool DeviceThread::foundInputSource()
{
    return ((AO::isConnected() == AO::eAO_CONNECTED) || TEST_MODE_ON);
}

bool DeviceThread::startAcquisition()
{
    // Neuro Omega Buffer
    deviceDataArraySize = 10000;
    streamDataArray = new AO::int16[deviceDataArraySize];

    for (int i = 0; i < numberOfChannels; i++)
    {
        if (channelsXmlList->getChildElement(i)->getBoolAttribute("Enabled"))
        {
            LOGC("AO::AddBufferChannel(", channelsXmlList->getChildElement(i)->getIntAttribute("ID"), ", ", AO_BUFFER_SIZE_MS, ")");
            AO::AddBufferChannel(channelsXmlList->getChildElement(i)->getIntAttribute("ID"), AO_BUFFER_SIZE_MS);
        }
    }
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
    if (TEST_MODE_ON)
    {
        deviceTimeStamp = std::time(0);
        Thread::sleep(TEST_SLEEP_TIME_MS);
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

        numberOfChannelsInStream = streamsXmlList->getChildElement(streamID)->getIntAttribute("Number_Of_Channels");
        bitVolts = streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Bit_Resolution");

        if (TEST_MODE_ON)
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
            timeStamps[samp] = float(deviceTimeStamp) + samp / streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Sampling_Rate");
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
    if (TEST_MODE_ON)
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
    int *arrChannel = new int[streamsXmlList->getChildElement(streamID)->getIntAttribute("Number_Of_Channels")];
    StringArray channelIDs;
    channelIDs.addTokens(streamsXmlList->getChildElement(streamID)->getStringAttribute("Channel_IDs"), ",", "\"");
    for (int ch = 0; ch < channelIDs.size(); ch++)
        arrChannel[ch] = channelIDs[ch].getIntValue();
    return arrChannel;
}

int DeviceThread::updateStreamDataArrayFromTestDataAndGetNumberOfSamples(int streamID)
{
    int numberOfSamplesPerChannel = TEST_SLEEP_TIME_MS / 1000.0 * streamsXmlList->getChildElement(streamID)->getDoubleAttribute("Sampling_Rate");
    int numberOfSamplesFromDevice = numberOfSamplesPerChannel * numberOfChannelsInStream;
    for (int samp = 0; samp < numberOfSamplesPerChannel; samp++)
    {
        for (int chan = 0; chan < numberOfChannelsInStream; chan++)
            streamDataArray[(chan * numberOfSamplesPerChannel) + samp] = pow(-1, streamID) * samp * (chan + 1);
    }
    return numberOfSamplesFromDevice;
}