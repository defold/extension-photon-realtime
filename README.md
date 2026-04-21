![](https://img.shields.io/badge/Realtime-5.0.14.3-green)

# Photon Realtime Extension for Defold

Defold [native extension](https://www.defold.com/manuals/extensions/) for the [Photon Realtime](https://www.photonengine.com/realtime) framework.

[Manual, API and setup instructions](https://www.defold.com/extension-photon-realtime/) is available on the official Defold site.


## Notes when updating the SDK

* Do not include `Common-cpp/inc/platform_defintions.h`. The platform define is set in `ext.manifest`.
* Open `Common-cpp/inc/defines.h` and change `include <Mmsystem.h>` to `include <mmsystem.h>`