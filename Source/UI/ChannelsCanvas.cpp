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

#include "ChannelsCanvas.h"
#include "XmlTable.h"

#include "../DeviceThread.h"

using namespace AONode;

/**********************************************/

ChannelsCanvas::ChannelsCanvas(DeviceThread *board_,
                               DeviceEditor *editor_) : board(board_),
                                                        editor(editor_)
{

    channelViewport = std::make_unique<Viewport>();
    channelTab = std::make_unique<TabbedComponent>(TabbedButtonBar::TabsAtRight);

    channelsTable = std::make_unique<XmlTableMainComponent>(board->getChannelsInformation());

    channelTab->addTab("Channels", Colours::grey, channelsTable.get(), 0, 0);
    channelTab->addTab("Streams", Colours::grey, channelsTable.get(), 0, 1);

    channelViewport->setViewedComponent(channelTab.get(), false);
    addAndMakeVisible(channelViewport.get());

    update();
    resized();
}

void ChannelsCanvas::paint(Graphics &g)
{
    g.fillAll(Colours::lightgrey);
}

void ChannelsCanvas::refresh()
{
    repaint();
}

void ChannelsCanvas::refreshState()
{
    resized();
}

void ChannelsCanvas::update()
{

    // channelList->update(); // TODO
}

void ChannelsCanvas::beginAnimation()
{
    // channelList->disableAll();
}

void ChannelsCanvas::endAnimation()
{
    // channelList->enableAll();
}

void ChannelsCanvas::resized()
{
    channelViewport->setBounds(0, 0, getWidth(), getHeight());
    channelTab->setBounds(0, 0, getWidth(), getHeight());
}
