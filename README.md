## Bridge GStreamer source to virtual camera and virtual microphone.

### Build
1. clone this repo `git clone --recursive https://github.com/Meonardo/gstreamer-virtualdev`;
2. create a GUID for register the virtual camera;
3. run & generate Visual Studio project file
   ```bash
   cmake -B .\build -DGSTREAMER_PKG_DIR="D:\gstreamer\1.0\msvc_x86_64\lib\pkgconfig" -DVIRTUALCAM_GUID="B2B95002-B4F6-4F0C-81C9-1E0CF8D384A2"
   ```  
### Status
- [x] camera;
- [ ] microphone;

### Credit
- virtual camera module from [obs-studio](https://github.com/obsproject/libdshowcapture) project;
