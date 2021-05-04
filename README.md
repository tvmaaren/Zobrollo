# Zobrollo

Zobrollo is a 2d minimalistic top-view racing game programmed in C using the <a href=https://liballeg.org/>Allegro</a> library for Windows 10 and GNU/Linux. At this point of development you can play in Time Trial and the development for multiplayer is just getting started. The game also keeps of track of records you have made and you are able to rewatch previous races and play against them.

# Dependencies
Dependencies are <a href=https://liballeg.org/>Allegro</a> and <a href=https://github.com/jmasterx/Agui>Agui</a>. Agui will probably need to be compiled from source.

# Compilation
For GNU/Linux Go to the directory you extracted the source to and run.
```shell
sudo make install
```
After this you should be able to run the game by typing the command zobrollo.

# Controls
`UP`		Accelerate
`DOWN`		Break
`LEFT`		Steer left
`RIGHT`		Steer right
`-`		Zoom out
`=`		Zoom in
`ESCAPE`	Go back
`F11`		Toggle full screen
# License
This program is licensed under GPLv3.

# To-do list
<ol>
	<li>Improve interface for multiplayer.</li>
	<li>Add more tracks.</li>
	<li>server-side-only software.</li>
	<li>Better error-handling for config-files.</li>
	<li>Be able to change controls in config file.</li>
</ol>

# Screenshots
The main menu
![](screenshots/menu.png)
The example track
![](screenshots/example.png)
The Long Straight track
![](screenshots/Long%20Straight.png)
