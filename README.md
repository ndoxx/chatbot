                                            _
      /\  /\__ _ _ __ _ __ ___   ___  _ __ (_)_______ _ __
     / /_/ / _` | '__| '_ ` _ \ / _ \| '_ \| |_  / _ \ '__|
    / __  / (_| | |  | | | | | | (_) | | | | |/ /  __/ |
    \/ /_/ \__,_|_|  |_| |_| |_|\___/|_| |_|_/___\___|_|

# Harmonizer
A brain-dead implementation of a guitar tab generator, following the wild specifications of my dear friend Albert Tomas Onster.

# Prerequisites
* No external dependency.
* CMake, version 3.1.0 or higher

# Cloning and building
>> git clone https://github.com/ndoxx/chatbot.git

>> mkdir build; cd build

>> cmake ..

>> make chatbot

I lazily hard-wired CmakeLists.txt to use the Clang7 C++ compiler. Just comment/replace lines 3-21 if needed. Your compiler must be able to handle C++17 code.

# Usage
"Wizard" mode:
>> ./chatbot

Generate 100 random tabs (all different):
>> ./chatbot --random -n 100

# Licence
This project is licensed under the Do What the Fuck You Want to Public License - see the LICENSE.md file for details
