/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "DeviceEditor.h"
#include "DeviceThread.h"

#include "UI/ChannelsStreamsCanvas.h"

using namespace AONode;

#ifdef WIN32
#endif

DeviceEditor::DeviceEditor(GenericProcessor *parentNode,
                           DeviceThread *board_)
    : VisualizerEditor(parentNode, "tabText", 340), board(board_)
{
    desiredWidth = 150;
    canvas = nullptr;
    tabText = "Neuro Omega";

    updateChannelsFromLabel = new Label("UpdateChannels", "Update Channels From:");
    updateChannelsFromLabel->setBounds(10, 25, 130, 20);
    addAndMakeVisible(updateChannelsFromLabel);

    updateChannelsFromSelector = new ComboBox("");
    updateChannelsFromSelector->setBounds(15, 50, 120, 20);
    updateChannelsFromSelector->setVisible(true);
    updateChannelsFromSelector->addItem(String("AO Info"), 1);
    updateChannelsFromSelector->addItem(String("Defaults"), 2);
    updateChannelsFromSelector->setSelectedItemIndex(0, dontSendNotification);
    updateChannelsFromSelector->setEnabled(board->foundInputSource());
    updateChannelsFromSelector->onChange = [this]
    { updateChannelsFromChanged(); };
    addChildComponent(updateChannelsFromSelector);
}

void DeviceEditor::updateChannelsFromChanged()
{
    switch (updateChannelsFromSelector->getSelectedId())
    {
    case 1:
        board->updateChannelsFromAOInfo();
        break;
    case 2:
        board->updateChannelsFromDefaults();
        break;
    }
    setUpCanvas();
    CoreServices::updateSignalChain(this);
}

void DeviceEditor::updateSettings()
{
    if (canvas != nullptr)
        canvas->update();
}

void DeviceEditor::startAcquisition()
{
    if (updateChannelsFromSelector != nullptr)
        updateChannelsFromSelector->setEnabled(false);
    if (canvas != nullptr)
        canvas->setEnabled(false);
}

void DeviceEditor::stopAcquisition()
{
    if (updateChannelsFromSelector != nullptr)
        updateChannelsFromSelector->setEnabled(true);
    if (canvas != nullptr)
        canvas->setEnabled(true);
}

Visualizer *DeviceEditor::createNewCanvas()
{
    GenericProcessor *processor = (GenericProcessor *)getProcessor();

    if (processor->getTotalContinuousChannels() > 0)
    {
        canvas = new ChannelsStreamsCanvas(this);
        setUpCanvas();
    }
    return canvas;
}

void DeviceEditor::setUpCanvas()
{
    if (canvas != nullptr)
    {
        canvas->channelsTable->initFromXml(board->channelsXmlList);
        canvas->channelsTable->addXmlModifiedListener(this);
        canvas->streamsTable->initFromXml(board->streamsXmlList);
        canvas->streamsTable->addXmlModifiedListener(this);
        canvas->channelStreamTabs->setCurrentTabIndex(1);
        canvas->channelStreamTabs->setCurrentTabIndex(0);
        canvas->updateContent();
        canvas->resized();
    }
}

void DeviceEditor::actionListenerCallback(const String &message)
{
    board->updateChannelsStreamsEnabled();
    if (canvas != nullptr)
        canvas->updateContent();
    CoreServices::updateSignalChain(this);
}
