# pysub
A command-line interpreter for a basic Python-like syntax. Supports incremental parsing and script execution.

## Dependencies
The project uses Visual Studio with MSBuild. vcpkg will be needed to install necessary packages.

- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) with C++ development workload
- [git](https://git-scm.com/install/windows)

## Setup
Navigate to the directory where you want to set up the project. Clone the repository:
```
git clone https://github.com/kuonicklas/pysub.git
```
Navigate to the directory where you want to set up vcpkg. Clone the repository:
```
git clone https://github.com/microsoft/vcpkg.git
```
Enter the newly created directory and run the following to download the vcpkg executable:
```
cd vcpkg; .\bootstrap-vcpkg.bat
```
Integrate with Visual Studio MSBuild:
```
.\vcpkg.exe integrate install
```
Open the pysub solution and build. vcpkg will detect and install any packages.

For additonal help with setting up vcpkg, [see here](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-msbuild?pivots=shell-powershell).

## Unit Tests
The project contains two solutions: the pysub program and a unit test solution. Unit tests are written with the Microsoft Unit Testing Framework for C++.

To run the unit tests: Tests > Test Explorer > Run.
