# LibKairos

LibKairos provides all computational and validation functionality required by Kairos, a program used to analyze and verify time-tracking data exported from ZeusX. Its purpose is to evaluate recorded working times and check their validity in accordance with the German Working Time Act (Arbeitszeitgesetz).

## Requirements

- Qt 6
- C++17 or later  
- CMake 3.16 or newer 

## Building with Qt Creator

Qt Creator can open and build this library directly:

1. Start Qt Creator.  
2. Select  
   **File → Open File or Project**.  
3. Choose the `CMakeLists.txt` file located in the root of the LibKairos repository.  
4. Qt Creator detects the CMake configuration.  
5. Select a Qt kit that provides the required Qt version.  
6. Press **Configure Project**.  
7. After configuration completes, press **Build** to compile the static library.  
8. If `LIBKAIROS_BUILD_TESTS` is enabled (default), Qt Creator also generates the test targets.  
9. Tests can be executed via the **Projects** or **Build** panel, or via the Test Explorer.

To build LibKairos as part of a Qt project:

1. Add the repository as a submodule.  
2. Open your main project in Qt Creator.  
3. Qt Creator detects the updated CMake files.  
4. Trigger a reconfigure and build.

## Building manually


```bash
mkdir build
cd build
cmake ..
cmake --build .
```

If you do not want to run tests, configure libkairos with:
```
cmake -DLIBKAIROS_BUILD_TESTS=OFF ..
```

### Using LibKairos in Qt project

Add the library as a submodule:
```
git submodule add <git-url> external/libkairos
git submodule update --init --recursive
```

Include it in CMake:
```
add_subdirectory(external/libkairos)
```

When defining an executable target, link against the library:
```
target_link_libraries(Kairos PRIVATE
    libkairos
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Network
)
```

Disable tests through your project in CMakeLists.txt if necessary:
```
set(LIBKAIROS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/libkairos)
```

Use features of the library like:
```
#include "libkairos/time.h"
```

## License

LibKairos is released under the GNU General Public License version 3.
See the LICENSE file for the full license text.