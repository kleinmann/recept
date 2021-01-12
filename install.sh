#!/bin/bash

echo "This script will install a library that will intercept pen events and smooth them."
echo
echo "If you haven't set up public key authentication on your remarkable yet, now would"
echo "be a good time to do so (otherwise you'll have to type your password multiple"
echo "times)."
echo
echo "Either way, make sure you have your remarkable password written down somewhere, you"
echo "might otherwise risk to lock yourself out if the GUI does not start up anymore."

echo
read -p "Enter the hostname or IP address of your remarkable device [remarkable]:" remarkable
remarkable=${remarkable:-remarkable}

echo
read -p "Install or uninstall library? (0=uninstall/1=install) [1]:" mode
mode=${mode:-1}

if [ "${mode}" -eq "0" ]
then \
  echo "Uninstalling ReCept..."
  ssh root@$remarkable "grep -qxF 'Environment=LD_PRELOAD=/usr/lib/librecept.so' /lib/systemd/system/xochitl.service && sed -i '/Environment=LD_PRELOAD=\/usr\/lib\/librecept.so/d' /lib/systemd/system/xochitl.service"
else \
  echo "Installing ReCept..."
  scp ./precompiled/librecept.so root@$remarkable:/usr/lib/librecept.so
  ssh root@$remarkable "grep -qxF 'Environment=LD_PRELOAD=/usr/lib/librecept.so' /lib/systemd/system/xochitl.service || sed -i 's#\[Service\]#[Service]\nEnvironment=LD_PRELOAD=/usr/lib/librecept.so#' /lib/systemd/system/xochitl.service"
fi

echo "...done."
echo "Restarting xochitl..."
ssh root@$remarkable "systemctl daemon-reload; systemctl restart xochitl"
echo "...done."
