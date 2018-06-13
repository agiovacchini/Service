# Service
Launch application as a Windows service

## Building
Building requires [Visual Studio Express](https://www.visualstudio.com/vs/express/) and [Boost 1.60](https://www.boost.org/users/history/version_1_60_0.html)

## Using

### Configuration
Create a INI file called like the executable and located in the same folder with a structure like this:
```ini
[Service]
Name=Test

[LineToLaunch]
Executable=c:\j2sdk1.8.0_45\jre\bin\Test.exe
Options=-classpath "c:\Test\jars\*" it.TestClass 
```
### Service installation
Launch service.exe with this parameters:
```bat
Service.exe install execution_path ini_file_name
ex: Service.exe install C:\ws-visualstudio\Service\x64\Debug Test.ini
```
Ini file must be inside execution_path
