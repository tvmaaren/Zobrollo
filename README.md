# Zobrollo
Zobrollo is a 2d minimalistic top-view racing game programmed in C using the Allegro library. At this point of development you can only play in time trial. It also keeps of track of records you have made.

# Dependencies
You first need to make sure you have allegro installed. In some cases you might have to compile this yourselve as well.

# Compilation
For GNU/Linux Go to the directory you extracted the source to and run.
```shell
sudo make install
```
After this you should be able to run the game by typing the command zobrollo.
For other platforms. Go to the directory you extracted the source to and run make. In the directory of the program an executable should appear. This you should run to play the game.

# Controls
`UP`		Accelerate
`DOWN`		Break
`LEFT`		Steer left
`RIGHT`		Steer right
`-`		Zoom out
`=`		Zoom in
`ESCAPE`	Go back to main menu
`F11`		Toggle full screen

# License
This program is licensed under GPLv3.

# To-do list
<ol>
	<li>Add more tracks.</li>
	<li>Be able to race against each other on a local network. I already have a working test for this I just haven't properly implemented it yet.</li>
	<li>Be able to race with anyone in the world. For this I will need to write server-side software.</li>
	<li>Be able to change controls in config file</li>
	<li>A settings menu.</li>
</ol>

# Screenshots

The main menu
![](screenshots/menu.png)
The example track
![](screenshots/example.png)
The Long Straight track
![](screenshots/Long%20Straight.png)
