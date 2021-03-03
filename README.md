Fan Control app for lenovo Ideapad Z580
---------------------------------------

This laptop has fan control feature using the [lenovo driver (Energy Management)](http://driverdl.lenovo.com.cn/lenovo/DriverFilesUploadFloder/36484/WIN8_EM.exe)

Hovewer SpeedFan failed to work correct with this laptop in windows 10, so here is a small utility build from reverse-engineering of the driver

Disclaimer
----------

This software is provided as is, use at you own risk, no warranty provided.

Usage (comman line)
-----

* **FanControl.exe fast** for fast Fan mode (in my case it consists of 12 high speed 10 seconds intervals with 2 seconds cooldown)
* **FanControl.exe normal** for normal operation mode
* **FanControl.exe read** returns driver value for CleanDustState
* **FanControl.exe keepfast <code, default = 3>** will resend request for the max Fan speed each time the return code differs from the specified
* **Execution without arguments, double click** will create Tray icon for the application and keep requesting max Fan speed. Right click icon to exit and return back to normal mode