**Disclaimer** : This software is meant for educational purposes only. I'm not responsible for any malicious use of the app.

zzrat
=====
Remote desktop, file transfer, remote shell ...


Currently supported
-------------------
- Remote desktop view
- File transfer
- Remote shell
- Memory load
- And much more

System requirements
-------------------
- Windows 10 or higher (x86_64 CPU)

Build the project (Windows)
-------------------
- Instructions on how to build *zzrat*
```bash
git clone --recursive https://github.com/987699873/zzrat.git
cd zzrat && mkdir build && cd build
cmake ..
```

Screenshots
-------------------
![ScreenShot](/images/zzrat.png)

Videos
-------------------
[https://www.bilibili.com/video/BV1eZpnePECJ](https://www.bilibili.com/video/BV1eZpnePECJ/?share_source=copy_web&vd_source=00d9c5239881898af722fa8476a14a92)

Dependencies
-------------------
- Thanks a lot to dchapyshev for [aspia](https://github.com/dchapyshev/aspia) whose elegant architecture has greatly inspired and improved the code in this repository
- Thanks to ocornut for their amazing [Dear ImGui](https://github.com/ocornut/imgui) which is used for building the entire interface
  - Thanks to aiekick for [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) used for displaying the file selection dialog
  - Thanks to leiradel for [ImGuiAl](https://github.com/leiradel/ImGuiAl) used for the logger
- Thanks to nothings for [stb](https://github.com/nothings/stb) used for loading/decoding images from file/memory
- Thanks to qicosmos for [rest_rpc](https://github.com/qicosmos/rest_rpc) inspired by its RPC design
- Thanks to nlohmann for their [json](https://github.com/nlohmann/json) library used for serialization and deserialization
- Thanks to strivexjun for their [MemoryModulePP](https://github.com/strivexjun/MemoryModulePP) library used to load a DLL from memory

Contribute
-------------------
- Quality contributions are welcome!

Contacts
--------
- QQ: 3875657991
- QQ Group: 956074112