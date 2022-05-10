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

// AlphaOmega SDK
namespace AO
{
#include "AOTypes.h"
#include "AOSystemAPI.h"
#include "StreamFormat.h"
}

using namespace AONode;

DataThread *DeviceThread::createDataThread(SourceNode *sn)
{
    return new DeviceThread(sn);
}

DeviceThread::DeviceThread(SourceNode *sn) : DataThread(sn),
                                             isTransmitting(false),
                                             updateSettingsDuringAcquisition(false)
{

    sourceBuffers.add(new DataBuffer(2, 10000)); // start with 2 channels and automatically resize
    queryUserStartConnection();

    channelNames.clear();

    AO::uint32 uChannelsCount = 0;
    AO::GetChannelsCount(&uChannelsCount);

    AO::SInformation *pChannelsInfo = new AO::SInformation[uChannelsCount];
    AO::GetAllChannels(pChannelsInfo, uChannelsCount);

    for (int i = 0; i < uChannelsCount; i++)
    {
        channelNames.add(pChannelsInfo[i].channelName);
    }

    numberOfChannels = channelNames.size();
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

    DataStream::Settings dataStreamSettings{
        "Device Data",
        "description",
        "identifier",
        static_cast<float>(44.000) // TODO: sampling rate
    };

    DataStream *stream = new DataStream(dataStreamSettings);

    sourceStreams->add(stream);

    for (int ch = 0; ch < numberOfChannels; ch++)
    {
        ContinuousChannel::Settings channelSettings{
            ContinuousChannel::ELECTRODE,
            getNthChannelName(ch),
            "description",
            "identifier",
            0.195,
            stream};

        continuousChannels->add(new ContinuousChannel(channelSettings));
        continuousChannels->getLast()->setUnits("uV");

        // if (impedances.valid)
        // {
        //     continuousChannels->getLast()->impedance.magnitude = headstage->getImpedanceMagnitude(ch);
        //     continuousChannels->getLast()->impedance.phase = headstage->getImpedancePhase(ch);
        // }
    }
}

bool DeviceThread::foundInputSource()
{
    return (AO::isConnected() == AO::eAO_CONNECTED);
}

bool DeviceThread::startAcquisition()
{
    pArray = new AO::int16[10000];
    ArraySize = 1000;
    actualData = 0;
    arrChannel[0] = 10000;
    arrChannel[1] = 10001;
    arrChannel[2] = 10002;

    if (foundInputSource())
    {
        AO::AddBufferChannel(10000, ArraySize);
        AO::AddBufferChannel(10001, ArraySize);
        AO::AddBufferChannel(10002, ArraySize);
        AO::ClearBuffers();
    }

    sourceBuffers[0]->resize(numberOfChannels, ArraySize);

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

    sourceBuffers[0]->clear();

    isTransmitting = false;

    return true;
}

bool DeviceThread::updateBuffer()
{
    float *data = new float[numberOfChannels];
    int numItems = 1;
    int64 *sampleNumbers = new int64[numItems];
    double *timestamps = new double[numItems];
    uint64 *eventCodes = new uint64[numItems];
    int chunkSize = 1;

    // Gather Data
    if (foundInputSource())
    {
        AO::GetAlignedData(pArray, ArraySize, &actualData, arrChannel, numberOfChannels, &TS_Begin);
    }
    else
    {
        int sleepTimeMiliS = 10;
        Thread::sleep(sleepTimeMiliS);
        actualData = sleepTimeMiliS / 1000.0 * 44000.0 * numberOfChannels;
        for (int i = 0; i < numberOfChannels; i++)
        {
            for (int j = 0; j < (actualData / numberOfChannels); j++)
            {
                pArray[(i * (actualData / numberOfChannels)) + j] = j;
            }
        }
    }

    // output
    timestamps[0] = 0;
    sampleNumbers[0] = 1;
    eventCodes[0] = 1;

    int nSamps = actualData / numberOfChannels;
    for (int samp = 0; samp < nSamps; samp++)
    {
        for (int chan = 0; chan < numberOfChannels; chan++)
        {
            data[chan] = pArray[(chan * nSamps) + samp];
        }
        timestamps[0] = float(std::time(0));

        sourceBuffers[0]->addToBuffer(data,
                                      sampleNumbers,
                                      timestamps,
                                      eventCodes,
                                      numItems,
                                      chunkSize);
    }

    return true;
}
