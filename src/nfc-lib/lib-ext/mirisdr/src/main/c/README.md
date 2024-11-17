LibMiriSDR-4
============

This is (yet) another flavour of libmirisdr initiated with original libmirisdr-2 from Miroslav Slugen and additions of Leif Asbrink SM5BSZ in libmirisdr-3-bsz. Bear with me for the missing special characters on both authors names.

The initial release contains these improvements and bug fixes from the originals:

<h2>Improvements</h2>

  - Better support of SDRPlay through a "flavour" option in the open function. This indicator can be used throughout the code if necessary. At present it drives the frequency plan that drives the choice between the different receiving paths of the MSi001 depending on frequency.
  - Remove useless auto gain feature that is just fixed gain in fact. The setter/getter still exists for compatibility but effectively does nothing.
  - Set the RPATH on the executables so you don't have to set LD_LIBRARY_PATH with the binaries installed by cmake.
  - Use Unix framework when compiling under Windows witn MinGW. This may fix possible bugs.
  - Use more meaningful variable names for what is actually gain reductions and not gains.
  - Some comments in the code were translated from Czech to English (Google translated) to ease understanding by the masses.
  
<h2>Bug fixes</h2>

  - Stop using a deprecated version of libusb.h (1.0.13) and rely on the one installed in the system or specified in the cmake command line.
  - Restore gain settings after a frequency, bandwidth or IF change as this affects the gain settings.
  - Corrected baseband gain setting.
