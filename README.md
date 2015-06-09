## vstemplate

This is a Visual Studio 2013 solution generator.  Its purpose is described here:
<https://github.com/Mischanix/nix>

### Usage

```
vstemplate generate <solution name> [opts ...]
```
opts:
- project_name: changes the name of the vcxproj (default: solution name)
- compiler: changes the compiler/toolset (default: v120)
- root: changes the root of the include path (default: D:\nix) (fixme: atm, you
  have to recompile to change the path templates are loaded from, which is based
  off of this root directory as well, so this is kind of pointless.)

example that uses the VS2015 compiler toolset:
```
vstemplate generate glorious_chicken -compiler=v140
```

```vstemplate add <project> <file name>```
adds a .h or .cpp file to the project.  if this is a .cpp file, it will be
configured to not be compiled, in accordance with the build.cpp system.  you
will have to add an `#include "file"` line to build.cpp yourself.
