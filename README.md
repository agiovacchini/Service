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
Executable=c:\Service\Service.exe
Options=-classpath "c:\Service\*" it.TestClass 
Path=c:\Service
```
### Service installation
Launch service.exe with this parameters:
```bat
Service.exe install execution_path ini_file_name
ex: Service.exe install c:\Service Service.ini
```
Ini file must be inside execution_path
