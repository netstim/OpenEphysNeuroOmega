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

#include "ChannelsStreamsCanvas.h"
#include "XmlTable.h"

#include "../DeviceThread.h"

using namespace AONode;

/**********************************************/

ChannelsStreamsCanvas::ChannelsStreamsCanvas(DeviceThread *board_,
                                             DeviceEditor *editor_) : board(board_),
                                                                      editor(editor_)
{

    channelStreamViewport = std::make_unique<Viewport>();
    channelStreamTabs = std::make_unique<TabbedComponent>(TabbedButtonBar::TabsAtTop);

    channelsTable = std::make_unique<XmlTableMainComponent>(board->channelsXmlList);
    streamsTable = std::make_unique<XmlTableMainComponent>(board->streamsXmlList);

    channelStreamTabs->addTab("Channels", Colours::grey, channelsTable.get(), 0, 0);
    channelStreamTabs->addTab("Streams", Colours::grey, streamsTable.get(), 0, 1);

    channelStreamViewport->setViewedComponent(channelStreamTabs.get(), false);
    addAndMakeVisible(channelStreamViewport.get());

    update();
    resized();
}

void ChannelsStreamsCanvas::paint(Graphics &g)
{
    g.fillAll(Colours::lightgrey);
}

void ChannelsStreamsCanvas::refresh()
{
    repaint();
}

void ChannelsStreamsCanvas::refreshState()
{
    resized();
}

void ChannelsStreamsCanvas::update()
{
    // TODO
}

void ChannelsStreamsCanvas::beginAnimation()
{
    // TODO
}

void ChannelsStreamsCanvas::endAnimation()
{
    // TODO
}

void ChannelsStreamsCanvas::resized()
{
    channelStreamViewport->setBounds(0, 0, getWidth(), getHeight());
    channelStreamTabs->setBounds(0, 0, getWidth(), getHeight());
}
