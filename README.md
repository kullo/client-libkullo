This code is part of the Kullo client. An overview of all related
repositories can be found here:
[https://www.kullo.net/open/](https://www.kullo.net/open/)
***********

libkullo
========

A platform-independent C++ client library for the Kullo protocol.

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

### v67 (2018-05-09)
 
 * Build paths adjustments of dependencies

### v66 (2017-09-25)

 * Use non-nullable types where possible
 * Add Argon2 password hashing library
 * Add support for encrypted MasterKey storage
 * Improve URI detection/highlighting
 * Make buildable with Google's Android toolchain
 * Update JsonCpp to 1.8.2

### v65 (2017-06-09)

 * Sort search results by date

### v64 (2017-06-01)

 * Implement full text search
 * Gracefully handle failures when sending messages

### v63 (2017-05-08)

 * Allow upper case when highlighting Kullo addresses
 * Let setRead/setDone return true iff changed
 * Rename and fix event ConversationLatestMessageTimestampChanged
 * Add event UserSettingsChanged

### v62 (2017-04-14)

 * Fix crash bug in messageModified
 * Set a HTTP timeout of 10 seconds for non-attachment requests
 * Add single attachment download progress to sync progress

### v61 (2017-02-28)

 * Link highlighting: improve detection for links in brackets
 * Link highlighting: allow some special characters in URL path
 * Improve implementation of concurrent database access

### v60 (2017-01-30)

 * Process plan and storage information from Kullo server
 * User agent: add Android/iOS version; improve OSX version detection
 * Work around MSVC std::regex implementation
 * Reduce noisy SQLite autoindex logging
 * Make it possible to cancel long running sync operations
 * Fix size estimations for message upload/download progress
 * Improve link highlighting speed
 * Support Kullo address highlighting

### v59 (2016-12-19)

 * Add terms of service handling in registration
 * Add OSX version to user agent
 * Use Botan's HKDF expand operation (requires Botan 1.11.34)
 * Bugfix: Draft without text was not sent in some situations

### v58 (2016-11-26)

 * Improve syncer signals
 * Bugfix: send missing progressed signal when adding attachment

### v57 (2016-11-24)

 * Signal database migrations through the API
 * Check size of attachments when adding them
 * Check size of message body and meta before sending
 * Track sync progress for incoming attachments and outgoing messages
 * Improve link detection in messages text
 * Fix implementation detection for AES
 * Speed-up adding attachments
 * Signal progress when adding attachments

### v56 (2016-10-01)

 * Add interfaces: Conversations.incomingMessages/outgoingMessages
 * Initial support for encrypted databases
 * Update JsonCpp to 1.7.6

### v55 (2016-09-12)

 * Implement recursive removing of conversations with messages and draft
 * Allow resetting a StlTaskRunner

### v54 (2016-08-08)

 * Fix HTTP 400 issue when sending message to multiple recipients caused by large keySafe
 * Reduce gzip compression level to improve performance

### v53 (2016-07-13)

 * Remove 60 seconds network timeout, which was hit when sending large files

### v52 (2016-07-05)

 * Fix crash that occured when an unknown NetworkError was written to log
 * Use zlib directly
 * Move Botan to separate repository

### v51 (2016-06-20)

 * Update Botan to 1.11.30, fixing a compatibility issue with GCM

### v50 (2016-06-17)

 * Improve sync speed by reusing a single HTTP connection for multiple requests

### v49 (2016-06-04)

 * Windows: Fix bug in handling filenames with special chars when adding files to drafts
 * Update SmartSqlite to v10

### v48 (2016-05-27)

 * Sync user settings
 * Update SmartSqlite to v10
 * Update Boost to 1.61.0

### v47 (2016-04-14)

 * Deduplicate avatars to speed up creating a session
 * Update SmartSqlite to v9

### v46 (2016-04-06)

 * Fix some signals
 * Minor changes for network timeout
 * Add MSVC 2015 compatibility

### v45 (2016-03-31)

 * Increase attachment size limit for sending to 100 MB
 * Calculate Conversations.lastMessageTime from receiving time on the server
 * Implement http mime multipart for sending messages with attachments
 * Update SmartSqlite to v8
 * Update Botan to 1.11.29

### v44 (2016-03-03)

 * Make downloading attachments more memory-efficient through streaming
 * Increase attachment size limit for receiving to 100 MB
 * Update SmartSqlite to v6 (version list shows v5 because of missing version bump)

### v43 (2016-02-12)

 * Store time of last sync
 * Differentiate push notifications on Android and iOS
 * Parallelize RSA operations on iOS
 * Improve HTTP status handling

### v42 (2016-02-02)

 * Retry push token registration on server or network error
 * Update to Botan to 1.11.28

### v41 (2016-01-26)

 * Implement push notifications
 * Update Djinni
 * Update Botan to 1.11.26

### v40 (2015-12-22)

 * Bugfix: Continue sync when handling a message with sender in recipients list
 * New Api: Use nowide for streams to ensure filenames work on Windows
 * New Api: Disallow creating a conversation with current user in recipients
 * New Api: Add NetworkError.Unknown as fallback for errors in syncer

### v39 (2015-12-11)

 * New Api: Make sync progress available
 * New Api: Move sync logic from caller to libkullo
 * Update Botan to 1.11.25

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

 * Update SmartSqlite to v3
 * Remove type KulloVersion

### v35 (2015-11-24)

 * Split libkullo version from Kullo for Desktop version
 * Add missing events when message was added, removed or modified
 * Fix implementation of Messages.setRead/setDone
 * Handle filesystem error in Attachments.saveToAsync
 * Let Senders.getLatest return -1 if not found
 * Create indices to improve database speed
 * Update Botan to 1.11.24
