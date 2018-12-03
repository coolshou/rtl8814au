# rtl8814au
Drivers for the rtl8814au chipset for wireless adapters (D-Link DWA-192 rev A1) 

# build & install
```
git clone https://github.com/coolshou/rtl8814au.git
cd driver
make
sudo make install
```

# debian dkms package (require dpkg-dev, dkms)
```
cd driver
dpkg-buildpackage -b

```
