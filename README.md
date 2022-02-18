# cpp-screen-recorder

Project for the "Programmazione di sistema" course @ Politecnico di Torino a.y. 2020/21

- Alessio Corvaglia
- Davide Fersino


Multiplatform C++ screen recorder library based on FFmpeg libraries and WxWidgets-based application.

The library and the application are able to:
- Start, pause, resume and stop the recording
- Record with or without audio
- Select the path and the name of the .mp4 output file
- Let the user choose the input video and audio device
- Let the user choose the fps of the recorded video
- Capture only a portion of the screen using the crop function
- Works on Linux, Windows and MacOS (the application has not been tested on MacOS, while the library works also on MacOS)

![Main Windows] (/images/main.png)
![Settings] (/images/settings.png)

FFmpeg libraries and WxWidgets must be installed on the system.
