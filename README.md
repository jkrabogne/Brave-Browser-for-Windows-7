# Brave-Browser-for-Windows-7
An independent research and development project investigating Brave browser compatibility with Windows 7 using VxKex API Extensions (https://github.com/i486/VxKex), Windows internals, reverse engineering, binary analysis, debugging, and PowerShell automation.


## Third-Party Dependencies and Acknowledgments

### VxKex — Windows 7 API Extensions

This project utilizes VxKex as a third-party application compatibility layer to investigate the feasibility of running modern versions of Brave on Windows 7.

**Original project:** [VxKex — Windows 7 API Extensions](https://github.com/i486/VxKex)

**Repository owner:** [i486](https://github.com/i486)

Full credit and acknowledgment are extended to the original developers and contributors of VxKex.

This repository focuses on additional browser-specific compatibility research, dependency analysis, troubleshooting, experimental modifications, debugging, and technical validation.

This project is independent and is not officially affiliated with or endorsed by the VxKex developers, Brave Software, Microsoft, Google, or the Chromium Project.

<img width="1637" height="916" alt="Screenshot 2026-11-25 190651" src="https://github.com/user-attachments/assets/836ae9da-a067-4033-bd24-b15bf69f5169" />


## Experimental Runtime Testing Instructions

### Prerequisites

- Windows 7 Service Pack 1 (64-bit)
- [VxKex — Windows 7 API Extensions](https://github.com/i486/VxKex)
- VxKex version 1.2.2.2316
- An isolated virtual machine with a recovery snapshot

### Testing Procedure

1. Obtain the following matching, authorized files from the experimental runtime testing package:
   - `KxBaseTimingTest.exe`
   - `KxBase.dll`
2. Create the following directory on Windows 7:

   `C:\BraveTest\RuntimeTest-x64`

3. Copy both files into the directory.
4. Verify the integrity of the downloaded files against the published SHA256 hashes.
5. Open Command Prompt.
6. Execute the following commands:

   ```cmd
   cd /d C:\BraveTest\RuntimeTest-x64
   KxBaseTimingTest.exe "C:\BraveTest\RuntimeTest-x64\KxBase.dll"
   echo Exit code: %ERRORLEVEL%
   ```

7. Review the runtime test output and record any errors.

### Important Notes

- Do not enable VxKex compatibility settings for `KxBaseTimingTest.exe`.
- Do not replace the original installed VxKex DLLs.
- Do not copy the testing files into the Brave installation directory.
- Do not modify Windows system files.

This runtime test validates an experimental implementation of the `QueryInterruptTimePrecise` API.

This is an isolated compatibility experiment, not a complete Brave Browser installation or compatibility patch.

The project remains under active research and development. Full browser compatibility has not yet been established.
