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

DataThread *DeviceThread::createDataThread(SourceNode *sn)
{
    return new DeviceThread(sn);
}

DeviceThread::DeviceThread(SourceNode *sn) : DataThread(sn),
                                             isTransmitting(false),
                                             updateSettingsDuringAcquisition(false)
{

    queryUserStartConnection();

    AO::uint32 uChannelsCount = 0;
    AO::GetChannelsCount(&uChannelsCount);
    if (testing)
    {
        uChannelsCount = 4;
    }

    AO::SInformation *pChannelsInfo = new AO::SInformation[uChannelsCount];
    AO::GetAllChannels(pChannelsInfo, uChannelsCount);
    std::vector<std::string> testChannelNames;
    if (testing)
    {
        pChannelsInfo[0].channelID = 0;
        pChannelsInfo[1].channelID = 1;
        pChannelsInfo[2].channelID = 2;
        pChannelsInfo[3].channelID = 3;
        testChannelNames.push_back("RAW Name1");
        testChannelNames.push_back("RAW Name1");
        testChannelNames.push_back("SPK Name1");
        testChannelNames.push_back("SPK Name2");
    }

    channelsInformation = new XmlElement("CHANNELS");
    streamsInformation = new XmlElement("STREAMS");

    XmlElement *channel, *stream;
    std::string channelName, streamName;
    int streamID = -1;

    for (int ch = 0; ch < uChannelsCount; ch++)
    {
        if (testing)
            channelName = testChannelNames[ch];
        else
            channelName = pChannelsInfo[ch].channelName;
        streamName = channelName.substr(0, channelName.find(" "));
        if (streamID < 0 || (streamName.compare(streamsInformation->getChildElement(streamID)->getStringAttribute("Name").toStdString()) != 0))
        {
            streamID++;
            stream = new XmlElement("STREAM");
            stream->setAttribute("ID", streamID);
            stream->setAttribute("Name", streamName);
            stream->setAttribute("Sampling Rate", 44000);
            stream->setAttribute("Bit Resolution", 38.147);
            stream->setAttribute("Gain", 20);
            streamsInformation->addChildElement(stream);
        }
        channel = new XmlElement("CHANNEL");
        channel->setAttribute("ID", pChannelsInfo[ch].channelID);
        channel->setAttribute("Name", channelName);
        channel->setAttribute("Stream ID", streamID);
        channelsInformation->addChildElement(channel);
    }

    numberOfChannels = channelsInformation->getNumChildElements();
    numberOfStreams = streamsInformation->getNumChildElements();

    std::string channelIDs;
    int numberOfChannelsInStream;

    for (int i = 0; i < numberOfStreams; i++)
    {
        streamsInformation->getChildElement(i)->setAttribute("Channel IDs", "");
        numberOfChannelsInStream = 0;
        streamsInformation->getChildElement(i)->setAttribute("Number Of Channels", 0);
        for (int ch = 0; ch < numberOfChannels; ch++)
        {
            if (channelsInformation->getChildElement(ch)->getIntAttribute("Stream ID") == i)
            {
                channelIDs = streamsInformation->getChildElement(i)->getStringAttribute("Channel IDs").toStdString();
                if (channelIDs.compare("") != 0)
                    channelIDs.append(",");
                channelIDs.append(channelsInformation->getChildElement(ch)->getStringAttribute("ID").toStdString());
                streamsInformation->getChildElement(i)->setAttribute("Channel IDs", channelIDs);
                numberOfChannelsInStream++;
            }
        }
        streamsInformation->getChildElement(i)->setAttribute("Number Of Channels", numberOfChannelsInStream);
        sourceBuffers.add(new DataBuffer(numberOfChannelsInStream, SOURCE_BUFFER_SIZE));
    }
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
    auto *aw = new AlertWindow(TRANS("Neuro Omega: start connection"),
                               TRANS("Enter the system MAC adress"),
                               AlertWindow::NoIcon, nullptr);

    aw->addTextEditor("System MAC", String("AA:BB:CC:DD:EE:FF"), String(), false);
    aw->addButton(TRANS("Connect"), 1, KeyPress(KeyPress::returnKey));
    aw->addButton(TRANS("Cancel"), 0, KeyPress(KeyPress::escapeKey));

    if (aw->runModalLoop())
    {
        AO::MAC_ADDR sysMAC = {0};
        sscanf(aw->getTextEditorContents("System MAC").toStdString().c_str(), "%x:%x:%x:%x:%x:%x",
               &sysMAC.addr[0], &sysMAC.addr[1], &sysMAC.addr[2], &sysMAC.addr[3], &sysMAC.addr[4], &sysMAC.addr[5]);
        AO::DefaultStartConnection(&sysMAC, 0);
        for (int i = 0; i < 10; i++)
        {
            Thread::sleep(1000);
            if (foundInputSource())
                break;
        }
    }

    aw->setVisible(false);
    AlertWindow::showMessageBox(AlertWindow::NoIcon, "Neuro Omega",
                                ((foundInputSource()) ? "Connected!" : "Unable to connect"),
                                "OK", nullptr);
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

    int streamID = -1;
    DataStream *stream;

    for (int ch = 0; ch < numberOfChannels; ch++)
    {
        if (streamID != channelsInformation->getChildElement(ch)->getIntAttribute("Stream ID"))
        {
            streamID = channelsInformation->getChildElement(ch)->getIntAttribute("Stream ID");
            DataStream::Settings dataStreamSettings{
                streamsInformation->getChildElement(streamID)->getStringAttribute("Name"),
                "description",
                "identifier",
                streamsInformation->getChildElement(streamID)->getDoubleAttribute("Sampling Rate")};
            stream = new DataStream(dataStreamSettings);
            sourceStreams->add(stream);
        }
        ContinuousChannel::Settings channelSettings{
            ContinuousChannel::ELECTRODE,
            channelsInformation->getChildElement(ch)->getStringAttribute("Name"),
            "description",
            "identifier",
            streamsInformation->getChildElement(streamID)->getDoubleAttribute("Bit Resolution") / streamsInformation->getChildElement(streamID)->getDoubleAttribute("Gain"),
            stream};
        continuousChannels->add(new ContinuousChannel(channelSettings));
        continuousChannels->getLast()->setUnits("uV");
    }
}

bool DeviceThread::foundInputSource()
{
    return ((AO::isConnected() == AO::eAO_CONNECTED) || testing);
}

bool DeviceThread::startAcquisition()
{
    // Neuro Omega Buffer
    deviceDataArraySize = 10000;
    deviceDataArray = new AO::int16[deviceDataArraySize];

    if (foundInputSource())
    {
        for (int i = 0; i < numberOfChannels; i++)
        {
            AO::AddBufferChannel(channelsInformation->getChildElement(i)->getIntAttribute("ID"), AO_BUFFER_SIZE_MS);
        }
        AO::ClearBuffers();
    }

    for (int i = 0; i < numberOfStreams; i++)
    {
        streamsInformation->getChildElement(i)->setAttribute("totalSamplesSinceStart", 0);
    }

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

    for (int i = 0; i < numberOfStreams; i++)
    {
        sourceBuffers[i]->clear();
        streamsInformation->getChildElement(i)->setAttribute("totalSamplesSinceStart", 0);
    }

    isTransmitting = false;

    return true;
}

bool DeviceThread::updateBuffer()
{
    int numberOfSamplesPerChannel;

    // Gather Data
    // if (testing)
    // {
    //     int sleepTimeMiliS = 100;
    //     Thread::sleep(sleepTimeMiliS);
    //     numberOfSamplesPerChannel = sleepTimeMiliS / 1000.0 * testingSamplingRate;
    //     numberOfSamplesFromDevice = numberOfSamplesPerChannel * numberOfChannels;
    //     for (int samp = 0; samp < numberOfSamplesPerChannel; samp++)
    //     {
    //         for (int chan = 0; chan < numberOfChannels; chan++)
    //         {
    //             deviceDataArray[(chan * numberOfSamplesPerChannel) + samp] = pow(-1, chan) * samp;
    //         }
    //     }
    //     timeStamps[0] = float(std::time(0));
    //     eventCodes[0] = 1;
    // }
    // else if (foundInputSource() && !testing)
    // {
    //     int *arrChannel = new int[numberOfChannels];
    //     for (int i = 0; i < numberOfChannels; i++)
    //         arrChannel[i] = channelsInformation->getChildElement(i)->getIntAttribute("ID");
    //     int status = AO::eAO_MEM_EMPTY;
    //     while (status == AO::eAO_MEM_EMPTY || numberOfSamplesFromDevice == 0)
    //         status = AO::GetAlignedData(deviceDataArray, deviceDataArraySize, &numberOfSamplesFromDevice, arrChannel, numberOfChannels, &deviceTimeStamp);
    //     numberOfSamplesPerChannel = numberOfSamplesFromDevice / numberOfChannels;
    //     timeStamps[0] = float(deviceTimeStamp);
    //     eventCodes[0] = 1;
    // }
    // else
    // {
    //     return false;
    // }

    for (int i = 0; i < numberOfStreams; i++)
    {

        int numberOfChannelsInStream = streamsInformation->getChildElement(i)->getIntAttribute("Number Of Channels");
        int *arrChannel = new int[numberOfChannelsInStream];
        StringArray channelIDs;
        channelIDs.addTokens(streamsInformation->getChildElement(i)->getStringAttribute("Channel IDs"), ",", "\"");
        for (int ch = 0; ch < channelIDs.size(); ch++)
        {
            arrChannel[ch] = std::stoi(channelIDs[ch].toStdString());
        }

        int status = AO::eAO_MEM_EMPTY;
        while (status == AO::eAO_MEM_EMPTY || numberOfSamplesFromDevice == 0)
            status = AO::GetAlignedData(deviceDataArray, deviceDataArraySize, &numberOfSamplesFromDevice, arrChannel, numberOfChannelsInStream, &deviceTimeStamp);
        numberOfSamplesPerChannel = numberOfSamplesFromDevice / numberOfChannelsInStream;

        // add to source buffer
        totalSamplesSinceStart = new int64[numberOfSamplesPerChannel];
        timeStamps = new double[numberOfSamplesPerChannel];
        eventCodes = new uint64[numberOfSamplesPerChannel];

        for (int s = 0; s < numberOfSamplesPerChannel; s++)
        {
            totalSamplesSinceStart[s] = streamsInformation->getChildElement(i)->getIntAttribute("totalSamplesSinceStart") + s;
            timeStamps[s] = float(deviceTimeStamp) + s / streamsInformation->getChildElement(i)->getDoubleAttribute("Sampling Rate");
            eventCodes[s] = 0;
        }

        streamsInformation->getChildElement(i)->setAttribute("totalSamplesSinceStart", std::to_string(totalSamplesSinceStart[numberOfSamplesPerChannel - 1] + 1));

        sourceBufferData = new float[numberOfSamplesFromDevice];
        for (int samp = 0; samp < numberOfSamplesFromDevice; samp++)
        {
            sourceBufferData[samp] = deviceDataArray[samp] * streamsInformation->getChildElement(i)->getDoubleAttribute("Bit Resolution") / streamsInformation->getChildElement(i)->getDoubleAttribute("Gain");
        }

        sourceBuffers[i]->addToBuffer(sourceBufferData,
                                      totalSamplesSinceStart,
                                      timeStamps,
                                      eventCodes,
                                      numberOfSamplesPerChannel,
                                      numberOfSamplesPerChannel);
    }
    return true;
}
