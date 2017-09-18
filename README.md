# DRM+ decoding library and Qt-Frontend

# Known bugs:
- Demodulating QAM-16 is not implemented
- Decoding only AAC format (witn SBR and PS).

Dependencies:
- fftw3
- faad2 (drm enabled)

# To compile library do:
```
./autogen.sh
./configure
make
sudo make install
```

# To compile Qt-DRM+ frontend:
```
cd qt-frontend
qt-creator qt-drmplus.pro
```
