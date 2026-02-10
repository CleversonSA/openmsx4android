# OPENMSX4ANDROID
An unofficial port for Android of almost perfect MSX emulator available.

# DISCLAIMER
I'm not owner of OpenMSX project (https://openmsx.org/), I'm just a MSX enthusiast that loves this almost perfect MSX emulator to emulate Brazilian machines (MSX Expert, Sharp HotBit).

I just used the OpenMSX source files project as a giant library to enable it to run on most modern Android Platform, and made some patches that I'll give back to community to help creating an official Android version.

By the way, be warned:

```
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE
```

**Always make a security backup of your device, It´s a open source spare time project, all the issues and risk and damage is just at your RISK! :)**


# ABOUT PROJECT

This port project was started after I created an OpenMSX 21 image with some addons for Raspberry PI platform (checkout my repo: https://github.com/CleversonSA/uosxpi).

OpenMSX has uppon a time an official Android port, but it's outdated and doesn't work for modern Android devices, and the 21 version had optimizations that the old port was not prepared.

Android devices are more widely available than Raspberry PI's, and at Facebook's Brazilian MSX community (https://www.facebook.com/groups/msxbrasiloficial/?locale=pt_BR) the idea of a OpenMSX port was very accepted!

I own real MSX hardware devices, but I keep in mind to give who doesn't want to spend a bunch of money for a real and rare old hardware, in order to bring nostaugia, and for who that never used the MSX platform. Carrying an "MSX" inside the pocket, available anytime you want, just for games or BASIC programming, for me it's amazing!

I've spent about 2 months to the beta version, applying AI to helping me understanding the deep OpenMSX architeture (specially how to migrate OpenGL ES scripts to the version that Android understands), Android NDK compilation, Compiling extra needed libraries (tcl, SDL, ImGUI, etc) and so on.

This is not the final project, as I used the almost recent OpenMSX version from official repo (Version 21 from dec 2025). So I´ll keep updating it as possible as I can.

Enjoy **"fudebas"!** :)

# SUPPORTED ANDROID VERSIONS

**ATTENTION**: For the beta version (feb 2026), I had to use Android´s 14 NDK. So, older devices, older tablets, may not accept the APK FILE.

Check if your android device is Android 14 or newer. For development I used an cheap brand Android GO (14) tablet and a Redmi Note 14 device, both worked very well.

In future, as I got more experience in Android NDK development, I could try older NDK versions, to bring a new life for old tablet or smartphones.

# APK INSTALLING (NO ROOT NEEDED!)

### Beta version

**Attention**: This project is not available at Google Play Store yet, until finishing the beta version of this port. So you have to manually push the APK file to the device and trust install files from this source. No ROOT or special permissions needed, just a standard app.

- Download the latest "openmsx4android.apk" from "/dist" folder

- Push the file to the device by USB cable (no development mode needed). Save it to "Download" folder, i.e.

- At Android device, access Files App, go to "Download" folder and tap over the "openmsx4android" file

- Install the emulator, trusting the source of this project (com.openmsx4android).

- Wait a few seconds (+- 250MB needed just for APK)

- Go back to Home App and Tap over "openmsx4android" app. At this beta release (2026-02-09), It has the standard Android App icon. I´ll change to the default OpenMSX icon later.

- **IMPORTANT!!** -> Wait a few seconds for OpenMSX asset files decompress process finish (10-30sec). This process happens once, do not worry

- If all things going OK, you will see the OpenMSX 21 menu and C-BIOS screen.

# CMDLINE.TXT FILE (MY FEATURE)

As a Android APK, you don´t have a launcher script like ordinary Linux or Windows installation, so how can I put my settings options, machine choices, relate disk file appointments, etc?

For the port, before launch OpenMSX engine I check if the ```cmdline.txt``` file is present:

```
/storage/emulated/0/Android/media/com.openmsx.openmsx4android/openmsx/share/cmdline.txt
```

This file is not a batch file, but tokens to call openmsx passing some parameters, i.e.:

```
-machine "Gradiente_Expert_GPC-1"
-command "set fullscreen on"
-command "set scale_factor 2"
-command "set mute off"
-command "set blur 0"
-command "set auto_enable_reverse off"
-exta Musical_Memory_Mapper
-extb slotexpander
-ext DDX_3.0
-ext nextor
```

So I can start openmsx with my custom settings.

The ```cmdline.txt``` is not present by default installation, if you wanna custom settings, you have to create a new one at:

```
/storage/emulated/0/Android/media/com.openmsx.openmsx4android/openmsx/share
```

folder.


# INSTALLING SYSTEMROMS AND EXTENSIONS

**Attention**: The APK installation has NO hardware ROM files, or games, just C-BIOS, due copyright restrictions.

All the OpenMSX asset files (aka *share* folder), is installed under this folder:

```
Internal Storage/Android/media/com.openmsx.openmsx4android/openmsx/share
```

or more specific

```
/storage/emulated/0/Android/media/com.openmsx.openmsx4android/openmsx/share
```

So you can put your hardware ROMs files, extensions, etc in this folder, the same way you can do at a PC installation.

You don´t need to jailbreak/root/enable development to put files.


# WHAT´S SUPPORTED

* Virtual Android Native Keyboard
* Hardware keyboard (USB/BT)
* Disk manager
* Native OpenMSX 21 GUI Menus
* Sound

# WHAT´S NOT SUPPORTED (YET)

### 2026-02-09 release

* **No screen rotation** - Not about Android sensors, but how to patch the OpenMSX OSD engine to redraw screen with new screen aspects.
* **No user openMSX folders (like "persistent")** - Not hard to correct it, but got a effort to do it.
* **No overlay functions keys** - Not hard to implement, but takes time
* **No SD card support for share folder** - To enable user put ROMs files, I had to copy the assets for the user storage to the device. Otherwise, If I keep the share folder only on data folder, after Android 13 security features, the user had to access it using adb or rooting the device :(. So the share folder is almost hardcoded at this version.
* **Disk as folder not tested yet** - I don´t know if this feature works at Android, may be yes
* **Better overlay icons and GUI** - I put all the efforts to got emulator working almost 100% at android, Icons may be changed later.
* **No LaserDisc support**

# BUILDING SOURCES

This project was built using Windows 10 Pro (I had to use Linux, sorry) using:

* Android Studio 2025
* OpenJDK 8
* Android 14 and NDK 29

If you have Android Studio fully working, with Java paths and NDK working, Just clone the project and import the project files at IDE. 

**Attention**: As a BIG project, the first compilation may took 40min-1 hour (I use a i5 7gen 16GB RAM).

# CONTRIBUTING

It´s a just for fun project. I enjoy have at my Smartphone or tablet an MSX Gradiente Expert or Sharp Hotbit machine. I decided to keep open for contributing to improve this port and future help to OpenMSX community to build again an official Android Port of the emulator.

## 2026-02-09 release

This release uses the Nov/2025 latest closed release of OpenMSX 21. I tried to keep the project as submodules but It was a nightmare to push my PORT changes to my repo, before plan how to create a PR to the main OpenMSX Repo.

As I said, OpenMSX was used as a giant library, and some classes I changed with my stubs implementation (checkout ```src/main/cpp/openmsx_android``` and ```src/main/cpp/openmsx_generated``` folders)), and I modified my CMakeLists.txt to ignore the OpenMSX main class and use mine (checkout ```src/main.cpp```), where all things started.

Of course, to get possible run the emulator on Android Platform, using ANativeWindow, the port has a Java layer, created from SDL2 template (checkout ```src/main/java/org/libsdl/app/OpenMSX4AndroidSDLActivity```) as a "glue".



# KNOWN ISSUES

## 2026-02-09 release

* **Android´s virtual keyboard fails when SHIFT is pressed, sometimes works, sometimes not** - May be a problem at InputEventGenerator.cc logic to read SDL events and send them to the emulator engine. Needs improvment.

* **Sometimes some keys like backspace and enter is pressed forever at virtual keyboard** - I think I need double check or try a new logic at InputEventGenerator.cc. The workaround is press the same key again, until stops.

* **ImGUI menu is so small at my device** - I developed the port using a cheap low res 7 inch tablet. For hi hes DPI resolutions like modern smartphones, a new resize logic is needed, since the default font size is hardcoded at the OpenMSX core.

* **Sometimes I lost my BASIC files content after I exit and back the App** - This issue happened to me when I was typing a program and save the file using DISK BASIC. I got a 0 byte size file. May be the OpenMSX core is not ready for suspend state.

* **Hardware keyboard works well but If I type fast, some keys got stuck** - The hard and soft keyboard core at InputEventGenerator.cc needs some improvment.

* **When I installed the APK, my Photo Gallery has a lot of OpenMSX Assets!** - I got this problem also at my smatphone. If you *delete* the files, the OpenMSX hangs out. Idk how to correct this issue yet. Sorry.

* **I created the cmdline.txt file now OpenMSX hangs out** - Check settings and file/dir appointments inside the file, a incorrect parameter may hangs openmsx starting. If It happens for you, just delete cmdfile.txt and check again.

* **CAPS LOCK at Android´s virtual keyboard doesn´t work very well, I had to press so many times a key to got working** - Need to improve the InputEventGenerator.cc logic. 

* **I had an old tablet, and I couldn´t install OpenMSX, it hangs** - Only Android 14+ is supported.

* **Sometimes virtual keyboard and overlay buttons doesn´t respond** - It´s just overlay focus. Just tap into MSX screen and try again.


# CHANGELOG

## 2026-02-09 release

* Alpha version, it works, runs machines, extensions, sound, keys, etc, but got some issues.
* Just vertical screenm support
* Virtual Android Keyboard basic support
* OpenGL ES 3.0 support (tex and fragment files updated, also ImGui)
* User storage supported
* cmdline.txt to works like a batch file launcher.