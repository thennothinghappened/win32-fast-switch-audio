# win32-fast-switch-audio

As my first proper venture into both Windows programming, and C++, here's a little tool which allows you to easily
switch audio devices from the system tray.

Changing the default audio device is done via the undocumented [`IPolicyConfig`](./src/PolicyConfig.h) COM interface,
here copied from a project which apparently does the exact same thing as this, but better (I probably should've done
more research before starting this, but if I had, I wouldn't have the benefit of learning C++, so oh well.)
