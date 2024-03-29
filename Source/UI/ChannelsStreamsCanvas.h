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

#ifndef __CHANNELCANVAS_H_2AD3C591__
#define __CHANNELCANVAS_H_2AD3C591__

#include "XmlTable.h"

#include <VisualizerEditorHeaders.h>

namespace AONode
{

	class DeviceThread;

	/**

	  Allows the user to edit channel metadata
	  and check electrode impedance values.

	  @see SourceNode

	  */

	class ChannelsStreamsCanvas : public Visualizer
	{
	public:
		/** Constructor */
		ChannelsStreamsCanvas(DeviceEditor *editor);

		/** Destructor */
		~ChannelsStreamsCanvas() {}

		/** Render the background */
		void paint(Graphics &g);

		/** Sets the layout of sub-components*/
		void resized();

		/** Called when the component's tab becomes visible again*/
		void refreshState();

		void setEnabled(bool shouldBeEnabled);

		/** Called when parameters of the underlying data processor are changed*/
		void update();

		void updateContent();

		/** Called instead of repaint to avoid redrawing underlying components*/
		void refresh();

		/** Called when data acquisition starts*/
		void beginAnimation();

		/** Called when data acquisition ends*/
		void endAnimation();

		/** Child components*/
		std::unique_ptr<Viewport> channelStreamViewport;
		std::unique_ptr<TabbedComponent> channelStreamTabs;
		std::unique_ptr<XmlTableMainComponent> channelsTable;
		std::unique_ptr<XmlTableMainComponent> streamsTable;

		/** Pointer to the editor object*/
		DeviceEditor *editor;
	};

}
#endif // __CHANNELCANVAS_H_2AD3C591__
