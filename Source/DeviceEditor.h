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

#include <VisualizerEditorHeaders.h>

namespace AONode
{
	class DeviceThread;
	class ChannelsStreamsCanvas;

	class DeviceEditor : public VisualizerEditor,
						 public ActionListener

	{
	public:
		/** Constructor */
		DeviceEditor(GenericProcessor *parentNode, DeviceThread *thread);

		/** Destructor*/
		~DeviceEditor() {}

		/** Disable UI during acquisition*/
		void startAcquisition();

		/** Enable UI after acquisition is finished*/
		void stopAcquisition();

		/** Updates channel canvas*/
		void updateSettings();

		/** Creates an interface with additional channel settings*/
		Visualizer *createNewCanvas(void);

		/** Called when a new message is received. */
		void actionListenerCallback(const String &message);

	private:
		DeviceThread *board;
		ChannelsStreamsCanvas *canvas;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceEditor);
	};

}
