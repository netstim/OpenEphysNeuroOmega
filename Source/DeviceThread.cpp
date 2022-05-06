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

using namespace AONode;

DataThread *DeviceThread::createDataThread(SourceNode *sn)
{
    return new DeviceThread(sn);
}

DeviceThread::DeviceThread(SourceNode *sn) : DataThread(sn),
                                             deviceFound(false),
                                             isTransmitting(false),
                                             channelNamingScheme(GLOBAL_INDEX),
                                             updateSettingsDuringAcquisition(false)
{

    // memset(auxBuffer, 0, sizeof(auxBuffer));
    // memset(auxSamples, 0, sizeof(auxSamples));

    for (int i = 0; i < 8; i++)
        adcRangeSettings[i] = 0;

    // sourceBuffers.add(new DataBuffer(2, 10000)); // start with 2 channels and automatically resize

    // Open Opal Kelly XEM6010 board.
    // Returns 1 if successful, -1 if FrontPanel cannot be loaded, and -2 if XEM6010 can't be found.

    // #if defined(__APPLE__)
    //     File appBundle = File::getSpecialLocation(File::currentApplicationFile);
    //     const String executableDirectory = appBundle.getChildFile("Contents/Resources").getFullPathName();
    // #else
    //     File executable = File::getSpecialLocation(File::currentExecutableFile);
    //     const String executableDirectory = executable.getParentDirectory().getFullPathName();
    // #endif

    //     String dirName = executableDirectory;
    //     libraryFilePath = dirName;
    //     libraryFilePath += File::getSeparatorString();
    //     libraryFilePath += okLIB_NAME;

    //     dacStream = new int[8];
    //     dacChannels = new int[8];
    //     dacThresholds = new float[8];
    //     dacChannelsToUpdate = new bool[8];

    // if (openBoard(libraryFilePath))
    // {
    //     // dataBlock = new Rhd2000DataBlock(1, evalBoard->isUSB3());

    //     // upload bitfile and restore default settings
    //     initializeBoard();

    //     // if (evalBoard->isUSB3())
    //     //     LOGD("USB3 board mode enabled");

    //     // automatically find connected headstages
    //     scanPorts(); // things would appear to run more smoothly if this were done after the editor has been created

    //     for (int k = 0; k < 8; k++)
    //     {
    //         dacChannelsToUpdate[k] = true;
    //         dacStream[k] = 0;
    //         setDACthreshold(k, 65534);
    //         dacChannels[k] = 0;
    //         dacThresholds[k] = 0;
    //     }
    // }
}

DeviceThread::~DeviceThread()
{
    // LOGD("RHD2000 interface destroyed.");

    // if (deviceFound && boardType == ACQUISITION_BOARD)
    // {
    //     int ledArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    //     // evalBoard->setLedDisplay(ledArray);
    // }

    // if (deviceFound)
    //     evalBoard->resetFpga();

    delete[] dacStream;
    delete[] dacChannels;
    delete[] dacThresholds;
    delete[] dacChannelsToUpdate;
}

void DeviceThread::initialize(bool signalChainIsLoading)
{
    if (signalChainIsLoading)
        return;

    // // Let's turn one LED on to indicate that the board is now connected
    // if (boardType == ACQUISITION_BOARD && deviceFound)
    // {
    //     int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    //     // evalBoard->setLedDisplay(ledArray);
    // }
}

std::unique_ptr<GenericEditor> DeviceThread::createEditor(SourceNode *sn)
{

    std::unique_ptr<DeviceEditor> editor = std::make_unique<DeviceEditor>(sn, this);

    return editor;
}

void DeviceThread::handleBroadcastMessage(String msg)
{
    // StringArray parts = StringArray::fromTokens(msg, " ", "");

    // // std::cout << "Received " << msg << std::endl;

    // if (parts[0].equalsIgnoreCase("ACQBOARD"))
    // {
    //     if (parts.size() > 1)
    //     {
    //         String command = parts[1];

    //         if (command.equalsIgnoreCase("TRIGGER"))
    //         {
    //             if (parts.size() == 4)
    //             {
    //                 int ttlLine = parts[2].getIntValue() - 1;

    //                 if (ttlLine < 0 || ttlLine > 7)
    //                     return;

    //                 int eventDurationMs = parts[3].getIntValue();

    //                 if (eventDurationMs < 10 || eventDurationMs > 5000)
    //                     return;

    //                 DigitalOutputCommand command;
    //                 command.ttlLine = ttlLine;
    //                 command.state = true;

    //                 digitalOutputCommands.push(command);

    //                 DigitalOutputTimer *timer = new DigitalOutputTimer(this, ttlLine, eventDurationMs);

    //                 digitalOutputTimers.add(timer);
    //             }
    //         }
    //     }
    // }
}

void DeviceThread::addDigitalOutputCommand(DigitalOutputTimer *timerToDelete, int ttlLine, bool state)
{
    // DigitalOutputCommand command;
    // command.ttlLine = ttlLine;
    // command.state = state;

    // digitalOutputCommands.push(command);

    // digitalOutputTimers.removeObject(timerToDelete);
}

DeviceThread::DigitalOutputTimer::DigitalOutputTimer(DeviceThread *board_, int ttlLine_, int eventDurationMs)
    : board(board_)
{

    tllOutputLine = ttlLine_;

    startTimer(eventDurationMs);
}

void DeviceThread::DigitalOutputTimer::timerCallback()
{
    stopTimer();

    board->addDigitalOutputCommand(this, tllOutputLine, false);
}

void DeviceThread::setDACthreshold(int dacOutput, float threshold)
{
    // dacThresholds[dacOutput] = threshold;
    // dacChannelsToUpdate[dacOutput] = true;
    // updateSettingsDuringAcquisition = true;

    // evalBoard->setDacThresholdVoltage(dacOutput,threshold);
}

void DeviceThread::setDACchannel(int dacOutput, int channel)
{
    // if (channel < getNumDataOutputs(ContinuousChannel::ELECTRODE))
    // {
    //     int channelCount = 0;
    //     for (int i = 0; i < enabledStreams.size(); i++)
    //     {
    //         if (channel < channelCount + numChannelsPerDataStream[i])
    //         {
    //             dacChannels[dacOutput] = channel - channelCount;
    //             dacStream[dacOutput] = i;
    //             break;
    //         }
    //         else
    //         {
    //             channelCount += numChannelsPerDataStream[i];
    //         }
    //     }
    //     dacChannelsToUpdate[dacOutput] = true;
    //     updateSettingsDuringAcquisition = true;
    // }
}

Array<int> DeviceThread::getDACchannels() const
{
    Array<int> dacChannelsArray;

    for (int k = 0; k < 8; ++k)
    {
        dacChannelsArray.add(dacChannels[k]);
    }

    return dacChannelsArray;
}

bool DeviceThread::openBoard(String pathToLibrary)
{
    // int return_code = evalBoard->open(pathToLibrary.getCharPointer());

    // if (return_code == 1)
    // {
    //     deviceFound = true;
    // }
    // else if (return_code == -1) // dynamic library not found
    // {
    //     bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
    //                                                  "Opal Kelly library not found.",
    //                                                  "The Opal Kelly library file was not found in the directory of the executable. "
    //                                                  "Would you like to browse for it?",
    //                                                  "Yes", "No", 0, 0);
    //     if (response)
    //     {
    //         // browse for file
    //         FileChooser fc("Select the library file...",
    //                        File::getCurrentWorkingDirectory(),
    //                        okLIB_EXTENSION,
    //                        true);

    //         if (fc.browseForFileToOpen())
    //         {
    //             File currentFile = fc.getResult();
    //             libraryFilePath = currentFile.getFullPathName();
    //             openBoard(libraryFilePath); // call recursively
    //         }
    //         else
    //         {
    //             // sendActionMessage("No configuration selected.");
    //             deviceFound = false;
    //         }
    //     }
    //     else
    //     {
    //         deviceFound = false;
    //     }
    // }
    // else if (return_code == -2) // board could not be opened
    // {
    //     bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
    //                                                  "Acquisition board not found.",
    //                                                  "An acquisition board could not be found. Please connect one now.",
    //                                                  "OK", "Cancel", 0, 0);

    //     if (response)
    //     {
    //         openBoard(libraryFilePath.getCharPointer()); // call recursively
    //     }
    //     else
    //     {
    //         deviceFound = false;
    //     }
    // }

    return true;
}

bool DeviceThread::uploadBitfile(String bitfilename)
{

    deviceFound = true;

    // if (!evalBoard->uploadFpgaBitfile(bitfilename.toStdString()))
    // {
    //     LOGD("Couldn't upload bitfile from ", bitfilename);

    //     bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
    //                                                  "FPGA bitfile not found.",
    //                                                  (evalBoard->isUSB3() ? "The rhd2000_usb3.bit file was not found in the directory of the executable. Would you like to browse for it?" : "The rhd2000.bit file was not found in the directory of the executable. Would you like to browse for it?"),
    //                                                  "Yes", "No", 0, 0);
    //     if (response)
    //     {
    //         // browse for file
    //         FileChooser fc("Select the FPGA bitfile...",
    //                        File::getCurrentWorkingDirectory(),
    //                        "*.bit",
    //                        true);

    //         if (fc.browseForFileToOpen())
    //         {
    //             File currentFile = fc.getResult();

    //             uploadBitfile(currentFile.getFullPathName()); // call recursively
    //         }
    //         else
    //         {
    //             deviceFound = false;
    //         }
    //     }
    //     else
    //     {
    //         deviceFound = false;
    //     }
    // }

    return deviceFound;
}

void DeviceThread::initializeBoard()
{
    //     String bitfilename;

    // #if defined(__APPLE__)
    //     File appBundle = File::getSpecialLocation(File::currentApplicationFile);
    //     const String executableDirectory = appBundle.getChildFile("Contents/Resources").getFullPathName();
    // #else
    //     File executable = File::getSpecialLocation(File::currentExecutableFile);
    //     const String executableDirectory = executable.getParentDirectory().getFullPathName();
    // #endif

    //     bitfilename = executableDirectory;
    //     bitfilename += File::getSeparatorString();
    //     bitfilename += "shared";
    //     bitfilename += File::getSeparatorString();

    //     if (boardType == ACQUISITION_BOARD)
    //         bitfilename += evalBoard->isUSB3() ? "rhd2000_usb3.bit" : "rhd2000.bit";
    //     else if (boardType == INTAN_RHD_USB)
    //         bitfilename += "intan_rhd_usb.bit";
    //     else if (boardType == RHD_RECORDING_CONTROLLER)
    //         bitfilename += "intan_rec_controller.bit";

    //     if (!uploadBitfile(bitfilename))
    //     {
    //         return;
    //     }

    // Initialize the board
    // LOGD("Initializing RHD2000 board.");
    // evalBoard->initialize();
    // This applies the following settings:
    //  - sample rate to 30 kHz
    //  - aux command banks to zero
    //  - aux command lengths to zero
    //  - continuous run mode to 'true'
    //  - maxTimeStep to 2^32 - 1
    //  - all cable lengths to 3 feet
    //  - dspSettle to 'false'
    //  - data source mapping as 0->PortA1, 1->PortB1, 2->PortC1, 3->PortD1, etc.
    //  - enables all data streams
    //  - clears the ttlOut
    //  - disables all DACs and sets gain to 0

    // setSampleRate(Rhd2000EvalBoard::SampleRate30000Hz);

    // evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortA, settings.cableLength.portA);
    // evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortB, settings.cableLength.portB);
    // evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortC, settings.cableLength.portC);
    // evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortD, settings.cableLength.portD);

    // if (boardType == RHD_RECORDING_CONTROLLER)
    // {
    //     evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortE, settings.cableLength.portE);
    //     evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortF, settings.cableLength.portF);
    //     evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortG, settings.cableLength.portG);
    //     evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortH, settings.cableLength.portH);
    // }

    // // Select RAM Bank 0 for AuxCmd3 initially, so the ADC is calibrated.
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3, 0);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd3, 0);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd3, 0);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd3, 0);

    // if (boardType == RHD_RECORDING_CONTROLLER)
    // {
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortE, Rhd2000EvalBoard::AuxCmd3, 0);
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortF, Rhd2000EvalBoard::AuxCmd3, 0);
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortG, Rhd2000EvalBoard::AuxCmd3, 0);
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortH, Rhd2000EvalBoard::AuxCmd3, 0);
    // }

    // // Since our longest command sequence is 60 commands, run the SPI interface for
    // // 60 samples (64 for usb3 power-of two needs)
    // evalBoard->setMaxTimeStep(INIT_STEP);
    // evalBoard->setContinuousRunMode(false);

    // // Start SPI interface
    // evalBoard->run();

    // // Wait for the 60-sample run to complete
    // while (evalBoard->isRunning())
    // {
    //     ;
    // }

    // // Read the resulting single data block from the USB interface. We don't
    // // need to do anything with this, since it was only used for ADC calibration
    // ScopedPointer<Rhd2000DataBlock> dataBlock = new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams(), evalBoard->isUSB3());

    // evalBoard->readDataBlock(dataBlock, INIT_STEP);
    // // Now that ADC calibration has been performed, we switch to the command sequence
    // // that does not execute ADC calibration.
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3,
    //                                 settings.fastSettleEnabled ? 2 : 1);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd3,
    //                                 settings.fastSettleEnabled ? 2 : 1);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd3,
    //                                 settings.fastSettleEnabled ? 2 : 1);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd3,
    //                                 settings.fastSettleEnabled ? 2 : 1);

    // if (boardType == RHD_RECORDING_CONTROLLER)
    // {
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortE, Rhd2000EvalBoard::AuxCmd3,
    //                                     settings.fastSettleEnabled ? 2 : 1);
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortF, Rhd2000EvalBoard::AuxCmd3,
    //                                     settings.fastSettleEnabled ? 2 : 1);
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortG, Rhd2000EvalBoard::AuxCmd3,
    //                                     settings.fastSettleEnabled ? 2 : 1);
    //     evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortH, Rhd2000EvalBoard::AuxCmd3,
    //                                     settings.fastSettleEnabled ? 2 : 1);
    // }

    // adcChannelNames.clear();
    // ttlLineNames.clear();

    // for (int i = 0; i < 8; i++)
    // {
    //     adcChannelNames.add("ADC" + String(i + 1));
    //     ttlLineNames.add("TTL" + String(i + 1));
    // }
}

void DeviceThread::updateSettings(OwnedArray<ContinuousChannel> *continuousChannels,
                                  OwnedArray<EventChannel> *eventChannels,
                                  OwnedArray<SpikeChannel> *spikeChannels,
                                  OwnedArray<DataStream> *sourceStreams,
                                  OwnedArray<DeviceInfo> *devices,
                                  OwnedArray<ConfigurationObject> *configurationObjects)
{

    // if (!deviceFound)
    //     return;

    // continuousChannels->clear();
    // eventChannels->clear();
    // spikeChannels->clear();
    // sourceStreams->clear();
    // devices->clear();
    // configurationObjects->clear();

    // channelNames.clear();

    // // create device
    // // CODE GOES HERE

    // DataStream::Settings dataStreamSettings{
    //     "Device Data",
    //     "description",
    //     "identifier",

    //     static_cast<float>(evalBoard->getSampleRate())

    // };

    // DataStream *stream = new DataStream(dataStreamSettings);

    // sourceStreams->add(stream);

    // int hsIndex = -1;

    // for (auto headstage : headstages)
    // {
    //     hsIndex++;

    //     if (headstage->isConnected())
    //     {
    //         for (int ch = 0; ch < headstage->getNumChannels(); ch++)
    //         {

    //             ContinuousChannel::Settings channelSettings{
    //                 ContinuousChannel::ELECTRODE,
    //                 headstage->getChannelName(ch),
    //                 "description",
    //                 "identifier",

    //                 0.195,

    //                 stream};

    //             continuousChannels->add(new ContinuousChannel(channelSettings));
    //             continuousChannels->getLast()->setUnits("uV");

    //             if (impedances.valid)
    //             {
    //                 continuousChannels->getLast()->impedance.magnitude = headstage->getImpedanceMagnitude(ch);
    //                 continuousChannels->getLast()->impedance.phase = headstage->getImpedancePhase(ch);
    //             }
    //         }

    //         if (settings.acquireAux)
    //         {
    //             for (int ch = 0; ch < 3; ch++)
    //             {

    //                 ContinuousChannel::Settings channelSettings{
    //                     ContinuousChannel::AUX,
    //                     headstage->getStreamPrefix() + "_AUX" + String(ch + 1),
    //                     "description",
    //                     "identifier",

    //                     0.0000374,

    //                     stream};

    //                 continuousChannels->add(new ContinuousChannel(channelSettings));
    //                 continuousChannels->getLast()->setUnits("mV");
    //             }
    //         }
    //     }
    // }

    // if (settings.acquireAdc)
    // {
    //     for (int ch = 0; ch < 8; ch++)
    //     {

    //         String name = "ADC" + String(ch + 1);

    //         ContinuousChannel::Settings channelSettings{
    //             ContinuousChannel::ADC,
    //             name,
    //             "description",
    //             "identifier",

    //             getAdcBitVolts(ch),

    //             stream};

    //         continuousChannels->add(new ContinuousChannel(channelSettings));
    //         continuousChannels->getLast()->setUnits("V");
    //     }
    // }

    // EventChannel::Settings settings{
    //     EventChannel::Type::TTL,
    //     "name",
    //     "description",
    //     "identifier",
    //     stream,
    //     8};

    // eventChannels->add(new EventChannel(settings));
}

String DeviceThread::getChannelName(int i) const
{
    return channelNames[i];
}

bool DeviceThread::isAcquisitionActive() const
{
    return isTransmitting;
}

void DeviceThread::setNamingScheme(ChannelNamingScheme scheme)
{

    // channelNamingScheme = scheme;

    // for (auto hs : headstages)
    // {
    //     hs->setNamingScheme(scheme);
    // }
}

ChannelNamingScheme DeviceThread::getNamingScheme()
{
    return channelNamingScheme;
}

void DeviceThread::setNumChannels(int hsNum, int numChannels)
{
    // if (headstages[hsNum]->getNumChannels() == 32)
    // {
    //     if (numChannels < headstages[hsNum]->getNumChannels())
    //         headstages[hsNum]->setHalfChannels(true);
    //     else
    //         headstages[hsNum]->setHalfChannels(false);

    //     numChannelsPerDataStream.set(headstages[hsNum]->getStreamIndex(0), numChannels);
    // }

    int channelIndex = 0;

    // for (auto hs : headstages)
    // {
    //     if (hs->isConnected())
    //     {
    //         hs->setFirstChannel(channelIndex);

    //         channelIndex += hs->getNumActiveChannels();
    //     }
    // }
}

int DeviceThread::getHeadstageChannels(int hsNum) const
{
    return 0; // headstages[hsNum]->getNumChannels();
}

int DeviceThread::getNumChannels()
{
    // int totalChannels = getNumDataOutputs(ContinuousChannel::ELECTRODE) + getNumDataOutputs(ContinuousChannel::AUX) + getNumDataOutputs(ContinuousChannel::ADC);

    return 0; // totalChannels;
}

int DeviceThread::getNumDataOutputs(ContinuousChannel::Type type)
{

    // if (type == ContinuousChannel::ELECTRODE)
    // {
    //     int totalChannels = 0;

    //     for (auto headstage : headstages)
    //     {
    //         if (headstage->isConnected())
    //         {
    //             totalChannels += headstage->getNumActiveChannels();
    //         }
    //     }

    //     return totalChannels;
    // }
    // if (type == ContinuousChannel::AUX)
    // {
    //     if (settings.acquireAux)
    //     {
    //         int numAuxOutputs = 0;

    //         for (auto headstage : headstages)
    //         {
    //             if (headstage->isConnected())
    //             {
    //                 numAuxOutputs += 3;
    //             }
    //         }
    //         return numAuxOutputs;
    //     }
    //     else
    //     {
    //         return 0;
    //     }
    // }
    // if (type == ContinuousChannel::ADC)
    // {
    //     if (settings.acquireAdc)
    //     {
    //         return 8;
    //     }
    //     else
    //     {
    //         return 0;
    //     }
    // }

    return 0;
}

float DeviceThread::getAdcBitVolts(int chan) const
{
    // if (chan < adcBitVolts.size())
    // {
    //     return adcBitVolts[chan];
    // }
    // else
    // {
    //     if (boardType == ACQUISITION_BOARD)
    //     {
    //         return 0.00015258789; // +/-5V / pow(2,16)
    //     }
    //     else if (boardType == INTAN_RHD_USB)
    //     {
    return 0.0000503540039; // 3.3V / pow(2,16)
    //     }
    // }
}

// double DeviceThread::setUpperBandwidth(double upper)
// {
//     // impedanceThread->stopThreadSafely();

//     settings.dsp.upperBandwidth = upper;

//     updateRegisters();

//     return settings.dsp.upperBandwidth;
// }

// double DeviceThread::setLowerBandwidth(double lower)
// {
//     // impedanceThread->stopThreadSafely();

//     // settings.dsp.lowerBandwidth = lower;

//     updateRegisters();

//     return settings.dsp.lowerBandwidth;
// }

// double DeviceThread::

void DeviceThread::setTTLoutputMode(bool state)
{
    settings.ttlMode = state;

    updateSettingsDuringAcquisition = true;
}

void DeviceThread::setDAChpf(float cutoff, bool enabled)
{
    settings.desiredDAChpf = cutoff;

    settings.desiredDAChpfState = enabled;

    updateSettingsDuringAcquisition = true;
}

void DeviceThread::setFastTTLSettle(bool state, int channel)
{
    settings.fastTTLSettleEnabled = state;

    settings.fastSettleTTLChannel = channel;

    updateSettingsDuringAcquisition = true;
}

int DeviceThread::setNoiseSlicerLevel(int level)
{
    settings.noiseSlicerLevel = level;

    // if (deviceFound)
    //     evalBoard->setAudioNoiseSuppress(settings.noiseSlicerLevel);

    // Level has been checked once before this and then is checked again in setAudioNoiseSuppress.
    // This may be overkill - maybe API should change so that the final function returns the value?

    return settings.noiseSlicerLevel;
}

bool DeviceThread::foundInputSource()
{
    return deviceFound;
}

bool DeviceThread::startAcquisition()
{
    // if (!deviceFound || (getNumChannels() == 0))
    //     return false;

    // impedanceThread->waitSafely();
    // dataBlock = new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams(), evalBoard->isUSB3());

    // LOGD("Expecting ", getNumChannels(), " channels.");

    // if (boardType == ACQUISITION_BOARD)
    // {
    //     int ledArray[8] = {1, 1, 0, 0, 0, 0, 0, 0};
    //     evalBoard->setLedDisplay(ledArray);
    // }

    // // reset TTL output state
    // for (int k = 0; k < 16; k++)
    // {
    //     TTL_OUTPUT_STATE[k] = 0;
    // }

    // // LOGD( "Number of 16-bit words in FIFO: ", evalBoard->numWordsInFifo());
    // // LOGD("Is eval board running: ", evalBoard->isRunning());

    // // LOGD("RHD2000 data thread starting acquisition.");

    // if (1)
    // {
    //     // LOGD("Flushing FIFO.");
    //     evalBoard->flush();
    //     evalBoard->setContinuousRunMode(true);
    //     evalBoard->run();
    // }

    // blockSize = dataBlock->calculateDataBlockSizeInWords(evalBoard->getNumEnabledDataStreams(), evalBoard->isUSB3());
    // // LOGD("Expecting blocksize of ", blockSize, " for ", evalBoard->getNumEnabledDataStreams(), " streams");

    startThread();

    isTransmitting = true;

    return true;
}

bool DeviceThread::stopAcquisition()
{

    // LOGD("RHD2000 data thread stopping acquisition.");

    // if (isThreadRunning())
    // {
    //     signalThreadShouldExit();
    // }

    // if (waitForThreadToExit(500))
    // {
    //     // LOGD("RHD2000 data thread exited.");
    // }
    // else
    // {
    //     // LOGD("RHD2000 data thread failed to exit, continuing anyway...");
    // }

    // if (deviceFound)
    // {
    //     evalBoard->setContinuousRunMode(false);
    //     evalBoard->setMaxTimeStep(0);
    //     LOGD("Flushing FIFO.");
    //     evalBoard->flush();
    // }

    // sourceBuffers[0]->clear();

    // if (deviceFound && boardType == ACQUISITION_BOARD)
    // {
    //     LOGD("Number of 16-bit words in FIFO: ", evalBoard->numWordsInFifo());

    //     int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    //     evalBoard->setLedDisplay(ledArray);
    // }

    // isTransmitting = false;
    // updateSettingsDuringAcquisition = false;

    // // remove timers
    // digitalOutputTimers.clear();

    // // remove commands
    // while (!digitalOutputCommands.empty())
    //     digitalOutputCommands.pop();

    return true;
}

bool DeviceThread::updateBuffer()
{
    // // int chOffset;
    // unsigned char *bufferPtr;
    // double ts;
    // // cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;
    // // cout << "Block size: " << blockSize << endl;

    // // LOGD( "Current number of words: " <<  evalBoard->numWordsInFifo() << " for " << blockSize );
    // if (evalBoard->isUSB3() || evalBoard->numWordsInFifo() >= blockSize)
    // {
    //     bool return_code;

    //     return_code = evalBoard->readRawDataBlock(&bufferPtr);
    //     // see Rhd2000DataBlock::fillFromUsbBuffer() for an idea of data order in bufferPtr

    //     int index = 0;
    //     int auxIndex, chanIndex;
    //     int numStreams = enabledStreams.size();
    //     int nSamps = Rhd2000DataBlock::getSamplesPerDataBlock(evalBoard->isUSB3());

    //     // evalBoard->printFIFOmetrics();
    //     for (int samp = 0; samp < nSamps; samp++)
    //     {
    //         int channel = -1;

    //         if (!Rhd2000DataBlock::checkUsbHeader(bufferPtr, index))
    //         {
    //             LOGE("Error in Rhd2000EvalBoard::readDataBlock: Incorrect header.");
    //             break;
    //         }

    //         index += 8; // magic number header width (bytes)
    //         int64 timestamp = Rhd2000DataBlock::convertUsbTimeStamp(bufferPtr, index);
    //         index += 4;       // timestamp width
    //         auxIndex = index; // aux chans start at this offset
    //         // skip aux channels for now
    //         index += 6 * numStreams; // width of the 3 aux chans
    //         // copy 64 neural data channels
    //         for (int dataStream = 0; dataStream < numStreams; dataStream++)
    //         {
    //             int nChans = numChannelsPerDataStream[dataStream];
    //             chanIndex = index + 2 * dataStream;
    //             if ((chipId[dataStream] == CHIP_ID_RHD2132) && (nChans == 16)) // RHD2132 16ch. headstage
    //             {
    //                 chanIndex += 2 * RHD2132_16CH_OFFSET * numStreams;
    //             }
    //             for (int chan = 0; chan < nChans; chan++)
    //             {
    //                 channel++;
    //                 thisSample[channel] = float(*(uint16 *)(bufferPtr + chanIndex) - 32768) * 0.195f;
    //                 chanIndex += 2 * numStreams; // single chan width (2 bytes)
    //             }
    //         }
    //         index += 64 * numStreams;   // neural data width
    //         auxIndex += 2 * numStreams; // skip AuxCmd1 slots (see updateRegisters())
    //         // copy the 3 aux channels
    //         if (settings.acquireAux)
    //         {
    //             for (int dataStream = 0; dataStream < numStreams; dataStream++)
    //             {
    //                 if (chipId[dataStream] != CHIP_ID_RHD2164_B)
    //                 {
    //                     int auxNum = (samp + 3) % 4;
    //                     if (auxNum < 3)
    //                     {
    //                         auxSamples[dataStream][auxNum] = float(*(uint16 *)(bufferPtr + auxIndex) - 32768) * 0.0000374;
    //                     }
    //                     for (int chan = 0; chan < 3; chan++)
    //                     {
    //                         channel++;
    //                         if (auxNum == 3)
    //                         {
    //                             auxBuffer[channel] = auxSamples[dataStream][chan];
    //                         }
    //                         thisSample[channel] = auxBuffer[channel];
    //                     }
    //                 }
    //                 auxIndex += 2; // single chan width (2 bytes)
    //             }
    //         }
    //         index += 2 * numStreams; // skip over filler word at the end of each data stream
    //         // copy the 8 ADC channels
    //         if (settings.acquireAdc)
    //         {
    //             for (int adcChan = 0; adcChan < 8; ++adcChan)
    //             {

    //                 channel++;
    //                 // ADC waveform units = volts

    //                 if (boardType == ACQUISITION_BOARD)
    //                 {
    //                     thisSample[channel] = adcRangeSettings[adcChan] == 0 ? 0.00015258789 * float(*(uint16 *)(bufferPtr + index)) - 5 - 0.4096 : // account for +/-5V input range and DC offset
    //                                               0.00030517578 * float(*(uint16 *)(bufferPtr + index));                                            // shouldn't this be half the value, not 2x?
    //                 }
    //                 else if (boardType == INTAN_RHD_USB)
    //                 {
    //                     thisSample[channel] = 0.000050354 * float(dataBlock->boardAdcData[adcChan][samp]);
    //                 }
    //                 index += 2; // single chan width (2 bytes)
    //             }
    //         }
    //         else
    //         {
    //             index += 16; // skip ADC chans (8 * 2 bytes)
    //         }

    //         uint64 ttlEventWord = *(uint64 *)(bufferPtr + index) & 65535;

    //         index += 4;

    //         sourceBuffers[0]->addToBuffer(thisSample,
    //                                       &timestamp,
    //                                       &ts,
    //                                       &ttlEventWord,
    //                                       1);
    //     }
    // }

    // if (updateSettingsDuringAcquisition)
    // {
    //     LOGD("DAC");
    //     for (int k = 0; k < 8; k++)
    //     {
    //         if (dacChannelsToUpdate[k])
    //         {
    //             dacChannelsToUpdate[k] = false;
    //             if (dacChannels[k] >= 0)
    //             {
    //                 evalBoard->enableDac(k, true);
    //                 evalBoard->selectDacDataStream(k, dacStream[k]);
    //                 evalBoard->selectDacDataChannel(k, dacChannels[k]);
    //                 evalBoard->setDacThreshold(k, (int)abs((dacThresholds[k] / 0.195) + 32768), dacThresholds[k] >= 0);
    //                 // evalBoard->setDacThresholdVoltage(k, (int) dacThresholds[k]);
    //             }
    //             else
    //             {
    //                 evalBoard->enableDac(k, false);
    //             }
    //         }
    //     }

    //     evalBoard->setTtlMode(settings.ttlMode ? 1 : 0);
    //     evalBoard->enableExternalFastSettle(settings.fastTTLSettleEnabled);
    //     evalBoard->setExternalFastSettleChannel(settings.fastSettleTTLChannel);
    //     evalBoard->setDacHighpassFilter(settings.desiredDAChpf);
    //     evalBoard->enableDacHighpassFilter(settings.desiredDAChpfState);
    //     evalBoard->enableBoardLeds(settings.ledsEnabled);
    //     evalBoard->setClockDivider(settings.clockDivideFactor);

    //     updateSettingsDuringAcquisition = false;
    // }

    // if (!digitalOutputCommands.empty())
    // {

    //     while (!digitalOutputCommands.empty())
    //     {
    //         DigitalOutputCommand command = digitalOutputCommands.front();
    //         TTL_OUTPUT_STATE[command.ttlLine] = command.state;
    //         digitalOutputCommands.pop();
    //     }

    //     evalBoard->setTtlOut(TTL_OUTPUT_STATE);

    //     LOGB("TTL OUTPUT STATE: ",
    //          TTL_OUTPUT_STATE[0],
    //          TTL_OUTPUT_STATE[1],
    //          TTL_OUTPUT_STATE[2],
    //          TTL_OUTPUT_STATE[3],
    //          TTL_OUTPUT_STATE[4],
    //          TTL_OUTPUT_STATE[5],
    //          TTL_OUTPUT_STATE[6],
    //          TTL_OUTPUT_STATE[7]);
    // }

    return true;
}

void DeviceThread::setAdcRange(int channel, short range)
{
    adcRangeSettings[channel] = range;
}

short DeviceThread::getAdcRange(int channel) const
{
    return adcRangeSettings[channel];
}
