libkullo
========

A platform-independent C++ client library for the Kullo protocol.

Requirements
------------

* OpenGL runtime and headers (Ubuntu: `libgl1-mesa-dev`)
* google-breakpad, checked out to and built in the same workspace as this project
* boost and smartsqlite, checked out in the same workspace

Configuring Qt Creator for development
--------------------------------------

Open `libkullo/CMakeLists.txt` in Qt Creator. Next, you should configure both a debug and a release build.

### Debug build

* Build configuration name: `Debug`
* Build path: `build-libkullo-debug`
* Make: Select target `install`, additional args should be `-j4` (or whatever you prefer)
* When asked to run CMake, run without args

### Release build

* Build configuration name: `Release`
* Build path: `build-libkullo`
* Make: same as for the debug build
* When asked to run CMake, run with arg `-DCMAKE_BUILD_TYPE=Release`
    * CMake stores this setting in its cache file which is located in the build dir. Thus, setting the build type is only necessary when the build dir doesn't exist yet or is empty.

Testing
-------

* From Qt Creator: Run the `libkullo_tests` target
* From command line: cd to build dir, run `make test` or `ctest`

Measuring test coverage
-----------------------

* Ubuntu/Fedora: install `lcov`
* Copy `coverage_settings.sample` to `coverage_settings` and edit it
* Run the `coverage.sh` script

Generating docs
---------------

* Install `doxygen` and `graphviz`
* Run `doxygen` from the main directory (containing `Doxyfile`)

Tips and Tricks
---------------

### Speed up Make
* Add make command line option `-j N` to use N parallel build processes

### Speed up g++ through ccache
* Install `ccache`
* Linux: Add `"QMAKE_CXX=ccache g++"` to the qmake arguments in the project build settings

### Checking -mmacosx-min-version/MACOSX\_DEPLOYMENT\_TARGET
`otool -l example.dylib | grep -A3 LC_VERSION_MIN_MACOSX`

Changelog
---------

### v38 (2015-11-30)

 * New Api: Let models update their data in synchronous calls (Messages.setRed/setDone, Messages.remove, Conversations.remove)
 * New Api: Replace interface Senders.getLatest with Messages.latestForSender
 * New Api: Add interface Messages.conversation

### v37 (2015-11-26)

 * New Api: Fix Conversations.remove
 * New Api: Fix error return value of Conversations.get (must be -1)
 * New Api: Fix Messages.remove
 * New Api: Rename event MessageDeleted -> MessageRemoved
 * New Api: Remove Messages.isDeleted

### v36 (2015-11-25)

 * Update SmartSqlite v3
 * Remove type KulloVersion

### v35 (2015-11-24)

 * Split libkullo version from Kullo for Desktop version
 * Add missing events when message was added, removed or modified
 * Fix implementation of Messages.setRead/setDone
 * Handle filesystem error in Attachments.saveToAsync
 * Let Senders.getLatest return -1 if not found
 * Create indices to improve database speed
 * Update Botan 1.11.24
