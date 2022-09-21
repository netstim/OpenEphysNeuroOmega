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

    // TODO: add UI
}

void DeviceEditor::updateSettings()
{
    if (canvas != nullptr)
    {
        canvas->update();
    }
}

void DeviceEditor::startAcquisition()
{
    if (canvas != nullptr)
        canvas->streamsTable->setEnabled(false);
}

void DeviceEditor::stopAcquisition()
{
    if (canvas != nullptr)
        canvas->streamsTable->setEnabled(true);
}

Visualizer *DeviceEditor::createNewCanvas()
{
    GenericProcessor *processor = (GenericProcessor *)getProcessor();

    if (processor->getTotalContinuousChannels() > 0)
    {
        canvas = new ChannelsStreamsCanvas(board, this);
        canvas->streamsTable->addXmlModifiedListener(this);
    }
    return canvas;
}

void DeviceEditor::actionListenerCallback(const String &message)
{
    CoreServices::updateSignalChain(this);
}
