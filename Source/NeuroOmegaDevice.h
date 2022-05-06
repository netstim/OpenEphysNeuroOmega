/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2020 Open Ephys

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

#ifndef __ACQUISITIONBOARD_H_2C4CBD67__
#define __ACQUISITIONBOARD_H_2C4CBD67__

#include "DeviceThread.h"

namespace AONode
{

    /**
        Communicates with the Neuro Omega Device

        https://www.alphaomega-eng.com/Nero-Omega

        @see DataThread, SourceNode
        */

    class NeuroOmegaDevice : public DeviceThread
    {
    public:
        /** Constructor */
        NeuroOmegaDevice(SourceNode *sn) : DeviceThread(sn) {}

        /** Destructor */
        ~NeuroOmegaDevice() {}
    };
}

#endif
