# Service
Launch application as a Windows service

Create a INI file called like the executable and located in the same folder with a structure like this:
```ini
[Service]<br/>Name=Test<br/>

[LineToLaunch]<br/>
Executable=c:\j2sdk1.8.0_45\jre\bin\Test.exe<br/>
Options= -classpath "c:\Test\jars\*" it.TestClass 
```
