# MedicalLabAR
MedicalLabAR is a native augmented reality application for the HoloLens 2. It can name and display the individual anatomical structures of a corresponding skin block model. The applikation is developed without a game engine such Unity or Unreal by using OpenXR and the XrSceneLib_uwp library from Microsoft. The XrSceneLib_uwp library uses C++17, OpenXR and D3D11 and can be found in the [OpenXR-MixedReality](https://github.com/microsoft/OpenXR-MixedReality) repo.

## Setup, build an run the app
- To deploy the application to the HoloLens 2 or HoloLens 2 Emulator, you will need Visual Studio 2019 16.2 or later. For more details, see the [Installation 
Checklist](https://learn.microsoft.com/en-us/windows/mixed-reality/develop/install-the-tools#installation-checklist), which lists all the tools requiered to build the application.

- You can simply clone or download the repo.

- Open the MedicalLabAR.sln solution file in Visual Studio. Depending on the platform you want to run the application on, you will need to select an appropriate solution platform architecture. To run on a HoloLens 2 device, select the ARM64 platform. To run on a HoloLens 2 emulator on a Windows desktop PC, select the x64 or x86 platform. 
