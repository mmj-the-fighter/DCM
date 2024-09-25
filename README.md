# DCM ( a.k.a Debug Connection Manager)
An example of how a Unity3D game can be programmed for accepting commands over a LAN and perform actions on those commands.  

# Example scenario    
>The game is on pre-release.   
>And the game is running fullscreen on a tablet.  
>The I.P. address of the tablet is known.  
>The tester wants a lot of things tested in short time.  
>The developer writes cheat codes and integrate them with DebugConnectionManager.  
>Now the testers can play the game in full screen and send cheat codes to the game from another computer over LAN.  

# Building
>Refer DebugConnectionManager.cs for API implementation details  
>Refer HealthController.cs and SpinControllers.cs for how to use API  
>Make a Unity3D project   
>Make a scene with any 3D model and Main Camera focused on it  
>Attach SpinController.cs and HealthController.cs to the model  
>Run the project  
>Click on start server  
>Build the client.c for windows, linux or mac machines  
>Run the client  
>Note down the IP address of the server machine  
>Use that IP and port number 64000 to connect to the server  
>On getting acknowledgement send commands  
>Only two clients are allowed to connect at the same time  
>There is an option to support more clients by changing the value of maxNumOfClientsSupported variable in DebugConnectionManager.cs  
>The result of the command is also send from server to client. It is either 0 (succeeded) or 1 (failed)  
>Note: You need to disable firewalls in server and client machines to make the project working fine  

# Important URLs
- Android Client APK built for Android 12
> https://www.dropbox.com/scl/fi/qqd0doh44abrnje1e1gk5/dcmc3.apk?rlkey=tahvcs9fnsihh9zyb577rt0xp&dl=0   

- Blog post 
> https://gamedev1001.blogspot.com/2017/07/game-testing-framework-idea.html 
- Vlog post 
[Demo on YouTube](https://www.youtube.com/watch?v=O3h13B2mhZw)

# Acknowlegements  
>Thanks to the developers of Unreal Tournament demo (1999) for making an in-game console for commands. That was my inspiration.   
>Thanks to the developers who posted articles and source code on C# .NET Sockets at www.codeproject.com  
>Thanks to the developers who posted articles and source code on delgates at https://msdn.microsoft.com and www.codeproject.com   
>Thanks to the operators and visitors of #gamedev ( irc.AfterNET.org ) for their support and encouragements for the development  

