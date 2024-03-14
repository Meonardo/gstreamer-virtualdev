## Bridge GStreamer source to virtual camera and virtual microphone.

### Build
1. clone this repo `git clone --recursive https://github.com/Meonardo/gstreamer-virtualdev`;
2. create a GUID for register the virtual camera;
3. generate Visual Studio project file(generate x86 and x64 project file)
   ```bash
   cmake -B .\build -DGSTREAMER_PKG_DIR="D:\gstreamer\1.0\msvc_x86_64\lib\pkgconfig" -DVIRTUALCAM_GUID="530C341D-AC56-4234-8003-2048B1C2E715" -A x64
   cmake -B .\build_x86 -DVIRTUALCAM_GUID="530C341D-AC56-4234-8003-2048B1C2E715" -A Win32
   ```  
### Status
- [x] camera;
- [ ] microphone;

### Credit
- virtual camera module from [obs-studio](https://github.com/obsproject/libdshowcapture) project;
