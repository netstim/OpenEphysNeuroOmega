*This project is work in progress and does not have any regulatory approval.*

# Open Ephys Neuro Omega

This repository contains a plugin for the [Open Ephys GUI](https://github.com/open-ephys/plugin-GUI) to interface with AlphaOmega's [Neuro Omega device](https://www.alphaomega-eng.com/Nero-Omega). 

## Installation

The Neuro Omega (proprietary) SDK should already be installed in the computer and the `NeuroOmega_x64.dll` should be copied to the shared folder: `C:\ProgramData\Open Ephys\shared-api8`.

- Manual

The compiled dll for GUI v6 is available from the Releases page. It should be downloaded and placed under `C:\ProgramData\Open Ephys\plugins-api8`.

- Github CLI

Using Github CLI is easy to stay up to date with latest release using the following command:

```PowerShell
gh release download --clobber --dir "C:\ProgramData\Open Ephys\plugins-api8" --pattern *.dll --repo netstim/OpenEphysNeuroOmega
```

- From Source

Alternativly, one can also compile this plugin from source. See Open Ephys GUI Documentation for instructions.

- From the GUI

The plugin is currently not available from the GUI Plugin installer. Use one of the avobe methods.