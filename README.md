Since I'm in read-only/plan mode, here's how to get UniPack onto your Ubuntu machine and test it:
Option A: Git (simplest, recommended)
On Windows:
cd /path/to/unipack
git init
git add .
git commit -m "Initial commit"
# Create a private repo on GitHub, then:
git remote add origin https://github.com/YOUR_USER/unipack.git
git push -u origin main
On Ubuntu:
sudo apt install git cmake g++ libcurl4-openssl-dev
git clone https://github.com/YOUR_USER/unipack.git
cd unipack

# Option 1: CMake (uses cpr - proper HTTP library)
cmake -B build
cmake --build build
./build/unipack --help

# Option 2: Makefile (uses curl CLI - lighter)
make
./unipack --help
Option B: SCP/SSH (if Ubuntu is on same network)
On Windows (PowerShell):
# If you have OpenSSH client enabled
Compress-Archive -Path C:\Users\Admin\Downloads\unipack\* -DestinationPath unipack.zip
scp unipack.zip user@ubuntu-ip:/home/user/
On Ubuntu:
sudo apt install unzip cmake g++ libcurl4-openssl-dev
unzip unipack.zip -d unipack && cd unipack
make && ./unipack --help
Option C: USB drive
Copy the unipack folder (minus .deps/ and any .o files) to a USB drive, then on Ubuntu:
sudo apt install cmake g++ libcurl4-openssl-dev make
cd /media/usb/unipack
make
./unipack --version
Once on Ubuntu — test the CLI
# Search (apt-cache + live API calls to Snapcraft/Flathub)
./unipack search vscode

# Install (will prompt for sudo)
sudo ./unipack install vscode

# Install with preferred format
sudo ./unipack install vscode --prefer apt

# Remove
sudo ./unipack remove vscode

# Update all
sudo ./unipack update
Prerequisites on Ubuntu: sudo apt install cmake g++ libcurl4-openssl-dev
Which approach works best for your setup? Do you have a GitHub account, or would you prefer SCP/USB?



ere are the exact steps once you've cloned on Ubuntu:
# 1. Install build dependencies
sudo apt install cmake g++ libcurl4-openssl-dev make

# 2. Enter the cloned directory
cd unipack

# 3. Build with Makefile (simpler, uses curl CLI for HTTP)
make

# 4. Test the binary
./unipack --version
./unipack --help

# 5. Try a search (this makes live API calls to Snapcraft/Flathub)
./unipack search vscode

# 6. Install (needs sudo because apt/snap require root)
sudo ./unipack install vscode

# 7. Install with a preferred format
sudo ./unipack install vscode --prefer apt

# 8. Check the state file that tracks what was installed
cat ~/.config/unipack/installed.json

# 9. Remove
sudo ./unipack remove vscode

# 10. Update tracked packages
sudo ./unipack update
Alternative — CMake build (more robust, uses the cpr HTTP library):
cmake -B build
cmake --build build
sudo cp build/unipack /usr/local/bin/   # install system-wide
Quick verification that apt/snap/flatpak are available on your system:
which apt-cache    # should exist on Ubuntu
which snap         # may need: sudo apt install snapd
which flatpak      # may need: sudo apt install flatpak
