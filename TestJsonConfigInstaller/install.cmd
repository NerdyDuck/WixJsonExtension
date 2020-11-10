msiexec /x {C9A3F6AF-7F4F-473F-BFFC-6143EB1AA0D4} /qn /lvx TestJsonConfigInstaller.msi.uninstall.log
msiexec /i TestJsonConfigInstaller.msi MY_VALUE="TestValue" /qn /lvx TestJsonConfigInstaller.msi.install.log
