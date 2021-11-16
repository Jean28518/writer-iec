# Writer IEC
Simple script for writing values to iec servers.

# Build Instructions on unix systems:
- Download lastest version of libiec61850.tar.gz and extract it.
- Download the required version of mbedtls (can be found in README.md of libiec archive): https://github.com/ARMmbed/mbedtls/releases
- Install required packages with e.g. `apt install build-essential gcc `
- Extract libiec.tar.gz and mbedtls.zip
- Rename the mbedtls folder to e.g. `mbedtls-2.16`
- Move the mbedtls folder to third_party/mbedtls/ in the libiec61850 folder.
- Run `make WITH_MBEDTLS=1` in the libiec61850 folder
- Move this repository to the root of the libiec61850 folder
- Run `make` in `libiec61850/writer-example` folder