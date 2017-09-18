# DRM+ decoding library and Qt-Frontend

## Known bugs:
- Demodulating QAM-16 is not implemented
- Decoding only AAC format (with SBR/PS)
- Command-line frontend supports only file (or stdin) input

Dependencies for libdrmplus:
- fftw3
- faad2 (drm enabled)

Dependencies for Qt-DRM+:
- Qt-5.9
- librtlsdr
- libsamplerate

## To compile library and command-line fromtend, do next steps:
```
./autogen.sh
./configure
make
sudo make install
```

## To compile graphical user-interface Qt-DRM+ frontend, do next:
```
cd qt-frontend
qt-creator qt-drmplus.pro
```

## Screensot of QT-DRM+ receiver application with additional graphs enabled:
[https://github.com/Opendigitalradio/qt-drmplus/blob/master/docs/qt-drmplus_screenshot.png|alt=QT-DRM+]
