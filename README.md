# rtl8814au
Drivers for the rtl8814au chipset for wireless adapters (D-Link DWA-192 rev A1)

# build & install
```
git clone https://github.com/coolshou/rtl8814au.git
cd driver
make
sudo make install
```

# debian dkms package (require dpkg-dev, dkms, dh-modaliases)
```
sudo apt install  debhelper dpkg-dev dkms dh-modaliases
cd driver
dpkg-buildpackage -b --no-sign
cd ..
dpkg -i rtl8814au-dkms_4.3.21.1-24835.20190115_all.deb
```

# USB2.0/3.0 mode switch
<pre>
 initial it will use USB2.0 mode which will limite 5G 11ac throughput (USB2.0 bandwidth only 480Mbps => throughput around 240Mbps)
when modprobe add following options will let it switch to USB3.0 mode at initial driver
options 8814au rtw_switch_usb_mode=1
</<pre>


## TODO: run time change usb2.0/3.0 mode
### usb2.0 => usb3.0
```
sudo sh -c "echo '1' > /sys/module/8814au/parameters/rtw_switch_usb_mode"
```
### usb3.0 => usb2.0
```
sudo sh -c "echo '2' > /sys/module/8814au/parameters/rtw_switch_usb_mode"
```
