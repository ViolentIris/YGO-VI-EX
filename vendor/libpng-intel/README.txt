libpng intel/ sources (SSE2 filter optimizations)
=================================================

Why this folder exists
----------------------
The irrlicht fork used by CI (mercury233/irrlicht, branch "test") ships a
trimmed libpng that is missing its intel/ directory, while this repository's
own build script (premake/irrlicht/premake5.lua) does compile:

    source/Irrlicht/libpng/intel/filter_sse2_intrinsics.c
    source/Irrlicht/libpng/intel/intel_init.c

so the CI build failed with:
    fatal error C1083: Cannot open source file: '..\irrlicht\source\Irrlicht\libpng\intel\...'

Keeping these two upstream libpng files here (instead of downloading them on
every CI run) makes the CI depend only on this repository.

Provenance
----------
Taken verbatim from libpng 1.6.55 (the version bundled in irrlicht here).
Upstream: https://github.com/pnggroup/libpng  (tag v1.6.55)
Mirror  : https://downloads.sourceforge.net/project/libpng/libpng16/1.6.55/libpng-1.6.55.tar.gz

License
-------
libpng License (permissive, zlib-like). See the libpng LICENSE file upstream.
These files are unmodified upstream sources.

Usage
-----
CI only: copied into the freshly cloned irrlicht tree, which is discarded
afterwards. Nothing in the local build references this folder.
