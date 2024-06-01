
# memtracer

## C++ memory allocation trace and report library for windows program

### Feature
- Multi-threaded support.

### Step 1
- Trace memory leak
  - Report call stack dump for all memory allocations.
- Trace total memory allocation amount and count.
- Trace memory allocation **from a specific point in time**.

### Step 2
- Call stack dump report analyze tool
  - View node graph by specific functions. (See memory allocation from specific function)

### Dependency
- C++ 17
- Windows
- rapidjson
