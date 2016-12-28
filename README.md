A library to procedurally generate buildings. The current implementation will generate suburban homes, but future additions could extend it to office buildings and other rectilinear designs.

Building libpb
*   Install CMake (at least version 3.0)
*   Download the project
*   Create a directory to contain the build files
*   Run CMake in that directory, pointing to libpb. E.g. from command line, run cmake &lt;libpb directory&gt; -G"Visual Studio 14 2015" to generate build files for Visual Studio 2015.
*   Run the install command (in Visual Studio, the INSTALL target; for Make, sudo make install). You should
    now be able to use this from other projects using find_package(libpb), which will add the pb and pb_util
    targets to your project.

Building libpb with tests
*   Install the check libraries.
    *  This should Just Work for Mac and Linux by following the installation guide on Check's website: (http://check.sourceforge.net/web/install.html)
    *  On Windows, not so much. Clone my fork of check from here: https://github.com/Shane-S/check, then build from source (using the same CMake commands as for libpb) and run the INSTALL target.
    *  In the Visual Studio Project for libpb, highlight pb_internal_test, pb_public_test, and pb_util_test, right-click, and select Properties.
    *  Under the C/C++ tab, add (without quotes) "C:\Program Files (x86)\check\include" (or "C:\Program Files\check\include" if you built 64-bit libraries) to the "Additional Include Directories" field.
    *  Under the Linker->Input tab, add "C:\Program Files (x86)\check\lib\compat.lib" (without quotes) to the "Additional Dependencies" field.
    *  These addition of include directories and library inputs need to be done for every configuration (Release, Debug, etc.).