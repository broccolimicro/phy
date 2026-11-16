# phy

`phy` is a C++20 library for integrated circuit cell layout. It represents cell geometry and fabrication process rules so that layout tools can create shapes, trace electrical nets, and calculate spacing between cells.

## Features

- **Cell geometry.** Store rectangles, polygons, and labels by layer. A layout also stores its boundary, nets, and references to other cells. Layout coordinates use integer database units.
- **Process definitions.** Define GDS layer mappings, transistor models, diffusion and well layers, routing layers, vias, and dielectrics. The process definition also stores width, spacing, and enclosure rules.
- **Geometry operations.** Merge shapes, test for overlap, and derive layers with Boolean and interaction operations. Adding geometry by process level can also add mask and well geometry using enclosure rules.
- **Connectivity tracing.** Use `Layout::trace()` to identify electrical nets from connected geometry across layers.
- **Cell spacing.** Use `minOffset()` to calculate the minimum offset between two layouts along a selected axis from the applicable spacing rules.

## Build

Run these commands from the `phy` library directory in the CAD source tree. The Makefile expects `common` and `gdstk` in sibling directories.

```sh
make
```

This builds the static library `libphy.a`. The Makefile uses `python3-config` to locate Python headers for the technology script loader. Set `PYTHON` to select a different Python executable.

To build and run the tests, install or build GoogleTest at the location expected by the Makefile, or set `GTEST` to its root directory.

```sh
make tests
./test
```

## Use the library

1. Create a `phy::Tech` object. Call `phy::loadTech(&tech, path, args)` to load a Python technology script. If `path` is a directory, the loader looks for `tech.py` in that directory.
2. In the script, use the embedded `bok` module to define layers, devices, and rules. For example, `paint()` defines a GDS layer mapping, `route()` defines a routing level, and `spacing()` adds a spacing rule.
3. Create a `phy::Layout` with the loaded technology. Keep the `Tech` object alive while the layout uses it. Add geometry by layer ID or process level, then call `trace()` if you need to identify nets from the geometry.

## Design-rule support

The library stores width, spacing, and enclosure rules. It uses spacing rules to calculate cell offsets and enclosure rules to add geometry around supported process levels. The layer evaluator handles Boolean and interaction operations. Full design-rule checking is outside the current implementation.

## License

Licensed by Broccoli, LLC under GNU GPL v3.

Written by Ned Bingham.
Copyright © 2024 Broccoli, LLC.

Bok is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Bok is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License may be found in COPYRIGHT.
Otherwise, see <https://www.gnu.org/licenses/>.

